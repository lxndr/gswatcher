/* 
 * gui-console.c
 * 
 * Copyright (C) 2011-2012 GSWatcher Developer(s)
 * 
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public License as
 * published by the Free Software Foundation; either version 3 of the
 * License, or (at your option) any later version.
 * 
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 * 
 * You should have received a copy of the GNU Library General Public
 * License along with the Gnome Library; see the file COPYING.LIB.  If not,
 * write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 * 
 */



#include <stdlib.h>
#include "platform.h"
#include "gui-window.h"
#include "gswatcher.h"
#include "gui-server-list.h"
#include "gui-console.h"
#include "gtkpopupbutton.h"


typedef struct _GuiConsoleContext {
	GtkTextBuffer *text_buffer;
} GuiConsoleContext;


static GtkWidget *page, *logview, *entry;
static GHashTable *history;
static GtkEntryCompletion *completion;
static GtkTreeIter history_iter;
static gboolean history_end;
static GtkWidget *toolbar, *port_label, *port, *password_label, *password;

static GtkTextTagTable *tag_table;
static GtkTextTag *tags[4] = {NULL};

static gboolean sysfont = TRUE;
static gchar *fontname = NULL;



static gboolean
gui_console_set_font_real (const gchar *name)
{
	g_return_val_if_fail (name != NULL, FALSE);
	
	PangoFontDescription *fontdesc = pango_font_description_from_string (name);
	if (!fontdesc)
		return FALSE;
	gtk_widget_override_font (logview, fontdesc);
	pango_font_description_free (fontdesc);
	return TRUE;
}


void
gui_console_set_use_system_font (gboolean use)
{
	if (use) {
		gchar *name = gs_get_system_font ();
		gui_console_set_font_real (name);
		g_free (name);
	} else {
		gui_console_set_font_real (fontname);
	}
	
	sysfont = use;
}

gboolean
gui_console_get_use_system_font ()
{
	return sysfont;
}


void
gui_console_set_font (const gchar *name)
{
	if (name == NULL)
		name = gs_get_system_font ();
	
	if (sysfont) {
		if (fontname)
			g_free (fontname);
		fontname = g_strdup (name);
	} else {
		if (gui_console_set_font_real (name)) {
			if (fontname)
				g_free (fontname);
			fontname = g_strdup (name);
		}
	}
}

const gchar *
gui_console_get_font ()
{
	return fontname;
}


void
gui_console_log (GsClient *client, GuiConsoleType type, const gchar *msg)
{
	GuiConsoleContext *context = client->console_gui;
	GtkTextIter iter;
	GtkTextMark *mark;
	
	gtk_text_buffer_get_end_iter (context->text_buffer, &iter);
	if (gui_slist_get_selected () == client)
		mark = gtk_text_buffer_create_mark (context->text_buffer,
				NULL, &iter, TRUE);
	
	gtk_text_buffer_insert_with_tags (context->text_buffer, &iter, msg, -1, tags[type], NULL);
	gtk_text_buffer_insert (context->text_buffer, &iter, "\n", -1);
	
	if (gui_slist_get_selected () == client) {
		gtk_text_view_scroll_to_mark (GTK_TEXT_VIEW (logview), mark, 0.0, TRUE, 0.0, 0.0);
		gtk_text_buffer_delete_mark (context->text_buffer, mark);
	}
}


static void
add_to_history (GsClient *client, const gchar *cmd)
{
	GtkTreeModel *model;
	GtkTreeIter iter;
	gchar *value;
	
	model = g_hash_table_lookup (history, client->querier->gameid->str);
	if (model == NULL) {
		model = GTK_TREE_MODEL (gtk_list_store_new (1, G_TYPE_STRING));
		g_hash_table_insert (history, client->querier->gameid->str, model);
	}
	
	if (gtk_tree_model_get_iter_first (model, &iter)) {
		do {
			gtk_tree_model_get (model, &iter, 0, &value, -1);
			if (strcmp (cmd, value) == 0) {
				gtk_list_store_remove (GTK_LIST_STORE (model), &iter);
				g_free (value);
				break;
			}
			g_free (value);
		} while (gtk_tree_model_iter_next (model, &iter));
	}
	
	gtk_list_store_append (GTK_LIST_STORE (model), &iter);
	gtk_list_store_set (GTK_LIST_STORE (model), &iter, 0, cmd, -1);
	history_end = TRUE;
}


static void
send_command (GsClient *client, const gchar *cmd)
{
	if (strcmp (cmd, "exit") == 0 || strcmp (cmd, "quit") == 0) {
		GtkWidget *dlg = gtk_message_dialog_new (GTK_WINDOW (window),
				GTK_DIALOG_MODAL, GTK_MESSAGE_WARNING, GTK_BUTTONS_YES_NO,
				_("This command will shut down the server. Are you sure you want to continue?"));
		if (gtk_dialog_run (GTK_DIALOG (dlg)) == GTK_RESPONSE_NO) {
			gtk_widget_destroy (dlg);
			return;
		}
		gtk_widget_destroy (dlg);
	}
	
	add_to_history (client, cmd);
	gui_console_log (client, GUI_CONSOLE_COMMAND, cmd);
	gs_client_send_command (client, cmd);
}


static void
gs_console_entry_activated (GtkEntry *entry, gpointer udata)
{
	send_command (gui_slist_get_selected (), gtk_entry_get_text (entry));
	gtk_entry_set_text (entry, "");
}


static void
gs_console_entry_icon_clicked (GtkEntry *entry, GtkEntryIconPosition icon,
		GdkEvent *event, gpointer udata)
{
	if (icon == GTK_ENTRY_ICON_SECONDARY) {
		send_command (gui_slist_get_selected (), gtk_entry_get_text (entry));
		gtk_entry_set_text (entry, "");
	}
}


static gboolean
gui_console_key_pressed (GtkWidget *widget, GdkEventKey *event, gpointer udata)
{
	GsClient *client = gui_slist_get_selected ();
	if (!client)
		return FALSE;
	
	GtkTreeModel *model = g_hash_table_lookup (history, client->querier->gameid->str);
	if (!model) {
		if (event->keyval == GDK_KEY_Up || event->keyval == GDK_KEY_Down)
			gtk_widget_error_bell (widget);
		return FALSE;
	}
	
	if (event->keyval == GDK_KEY_Up) {
		if (history_end) {
			gint n = gtk_tree_model_iter_n_children (model, NULL);
			if (n == 0) {
				gtk_widget_error_bell (widget);
				return TRUE;	// there is no items
			} else {
				gtk_tree_model_iter_nth_child (model, &history_iter, NULL, n - 1);
				history_end = FALSE;
			}
		} else {
			GtkTreeIter tmp = history_iter;
			if (!gtk_tree_model_iter_previous (model, &history_iter)) {
				history_iter = tmp;
				gtk_widget_error_bell (widget);
				return TRUE;
			}
		}
	} else if (event->keyval == GDK_KEY_Down) {
		if (history_end) {
			gtk_widget_error_bell (widget);
			return TRUE;
		}
		
		if (!gtk_tree_model_iter_next (model, &history_iter)) {
			gtk_entry_set_text (GTK_ENTRY (entry), "");
			history_end = TRUE;
			return TRUE;
		}
	} else {
		return FALSE; 			// if any other key, do default action
	}
	
	gchar *cmd;
	gtk_tree_model_get (model, &history_iter, 0, &cmd, -1);
	gtk_entry_set_text (GTK_ENTRY (entry), cmd);
	gtk_editable_set_position (GTK_EDITABLE (entry),
			gtk_entry_get_text_length (GTK_ENTRY (entry)));
	g_free (cmd);
	
	return TRUE;
}


static void
gui_console_settings_popup (GtkPopupButton *popup_button, gpointer udata)
{
	GsClient *client = gui_slist_get_selected ();
	
	gchar tmp[8];
	g_snprintf (tmp, 8, "%" G_GUINT16_FORMAT, gs_client_get_console_port (client));
	gtk_entry_set_text (GTK_ENTRY (port), tmp);
	
	const gchar *pass = gs_client_get_console_password (client);
	gtk_entry_set_text (GTK_ENTRY (password), pass ? pass : "");
	
	gtk_widget_set_sensitive (password, client->console_settings & GUI_CONSOLE_PASS);
	gtk_widget_set_sensitive (port, client->console_settings & GUI_CONSOLE_PORT);
}


static void
gui_console_settings_popdown (GtkPopupButton *popup_button, gpointer udata)
{
	GsClient *client = gui_slist_get_selected ();
	gs_client_set_console_password (client, gtk_entry_get_text (GTK_ENTRY (password)));
	gs_client_set_console_port (client, atoi (gtk_entry_get_text (GTK_ENTRY (port))));
	gs_application_save_server_list ();
}


void
gui_console_setup (GsClient *client)
{
	if (client) {
		GuiConsoleContext *context = client->console_gui;
		
		gtk_widget_set_sensitive (page, TRUE);
		gtk_widget_set_sensitive (toolbar, TRUE);
		
		GtkTextIter iter;
		gtk_text_view_set_buffer (GTK_TEXT_VIEW (logview), context->text_buffer);
		gtk_text_buffer_get_end_iter (context->text_buffer, &iter);
		
		GtkTextMark *mark = gtk_text_buffer_create_mark (context->text_buffer, NULL, &iter, TRUE);
		gtk_text_view_scroll_mark_onscreen (GTK_TEXT_VIEW (logview), mark);
		gtk_text_buffer_delete_mark (context->text_buffer, mark);
		
		history_end = TRUE;
		
		gtk_entry_completion_set_model (completion,
				g_hash_table_lookup (history, client->querier->gameid->str));
	} else {
		gtk_widget_set_sensitive (page, FALSE);
		gtk_widget_set_sensitive (toolbar, FALSE);
		
		gtk_text_view_set_buffer (GTK_TEXT_VIEW (logview), NULL);
		gtk_entry_completion_set_model (completion, NULL);
	}
}


void
gui_console_init_context (GsClient *client)
{
	g_return_if_fail (GS_IS_CLIENT (client));
	g_return_if_fail (tag_table != NULL);
	
	GuiConsoleContext *context = g_slice_new (GuiConsoleContext);
	context->text_buffer = gtk_text_buffer_new (tag_table);
	client->console_gui = context;
}


void
gui_console_free_context (GsClient *client)
{
	g_return_if_fail (GS_IS_CLIENT (client));
	
	GuiConsoleContext *context = client->console_gui;
	g_object_unref (context->text_buffer);
	g_slice_free (GuiConsoleContext, context);
	
	client->console_gui = NULL;
}


GtkWidget *
gui_console_create_bar ()
{
	/* settings window */
	port = gtk_entry_new ();
	
	port_label = gtk_label_new (_("P_ort:"));
	g_object_set (G_OBJECT (port_label),
			"xalign", 0.0f,
			"use-underline", TRUE,
			"mnemonic-widget", port,
			NULL);
	
	password = gtk_entry_new ();
	g_object_set (G_OBJECT (password),
			"visibility", FALSE,
			NULL);
	
	password_label = gtk_label_new (_("_Password:"));
	g_object_set (G_OBJECT (password_label),
			"xalign", 0.0f,
			"use-underline", TRUE,
			"mnemonic-widget", password,
			NULL);
	
	GtkWidget *grid = gtk_grid_new ();
	gtk_grid_set_row_spacing (GTK_GRID (grid), 2);
	gtk_grid_set_column_spacing (GTK_GRID (grid), 2);
	gtk_grid_attach (GTK_GRID (grid), port_label, 0, 0, 1, 1);
	gtk_grid_attach (GTK_GRID (grid), port, 1, 0, 1, 1);
	gtk_grid_attach (GTK_GRID (grid), password_label, 0, 1, 1, 1);
	gtk_grid_attach (GTK_GRID (grid), password, 1, 1, 1, 1);
	gtk_widget_show_all (grid);
	
	/* settings button */
	GtkWidget *label = gtk_label_new (_("Settings"));
	
	GtkWidget *separator = gtk_separator_new (GTK_ORIENTATION_VERTICAL);
	
	GtkWidget *arrow = gtk_arrow_new (GTK_ARROW_UP, GTK_SHADOW_NONE);
	
	GtkWidget *box = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 4);
	gtk_box_pack_start (GTK_BOX (box), label, TRUE, TRUE, 0);
	gtk_box_pack_start (GTK_BOX (box), separator, FALSE, TRUE, 0);
	gtk_box_pack_start (GTK_BOX (box), arrow, FALSE, TRUE, 0);
	
	GtkWidget *settings = gtk_popup_button_new ();
	gtk_container_add (GTK_CONTAINER (settings), box);
	GtkWidget *popup_window = gtk_popup_button_get_popup (GTK_POPUP_BUTTON (settings));
	gtk_container_add (GTK_CONTAINER (popup_window), grid);
	g_signal_connect (settings, "popup",
			G_CALLBACK (gui_console_settings_popup), NULL);
	g_signal_connect (settings, "popdown",
			G_CALLBACK (gui_console_settings_popdown), NULL);
	
	toolbar = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 4);
	g_object_set (G_OBJECT (toolbar),
			"sensitive", FALSE,
			NULL);
	gtk_box_pack_start (GTK_BOX (toolbar), settings, FALSE, TRUE, 0);
	gtk_widget_show_all (toolbar);
	return toolbar;
}


void
gui_console_destroy_bar ()
{
	g_object_unref (toolbar);
}


GtkWidget *
gui_console_create ()
{
	/* log tag table */
	tag_table = gtk_text_tag_table_new ();
	
	tags[GUI_CONSOLE_INFO] = gtk_text_tag_new ("inf");
	g_object_set (tags[GUI_CONSOLE_INFO], "foreground", "dark gray", NULL);
	gtk_text_tag_table_add (tag_table, tags[GUI_CONSOLE_INFO]);
	
	tags[GUI_CONSOLE_COMMAND] = gtk_text_tag_new ("cmd");
	g_object_set (tags[GUI_CONSOLE_COMMAND], "foreground", "dark green", NULL);
	gtk_text_tag_table_add (tag_table, tags[GUI_CONSOLE_COMMAND]);
	
	tags[GUI_CONSOLE_ERROR] = gtk_text_tag_new ("err");
	g_object_set (tags[GUI_CONSOLE_ERROR], "foreground", "dark red", NULL);
	gtk_text_tag_table_add (tag_table, tags[GUI_CONSOLE_ERROR]);
	
	/* history */
	history = g_hash_table_new_full (g_str_hash, g_str_equal, NULL, g_object_unref);
	
	/* widgets */
	logview = gtk_text_view_new ();
	g_object_set (G_OBJECT (logview),
			"editable", FALSE,
			"wrap-mode", GTK_WRAP_WORD,
			NULL);
	
	GtkWidget *scrolled = gtk_scrolled_window_new (NULL, NULL);
	g_object_set (G_OBJECT (scrolled),
			"shadow-type", GTK_SHADOW_IN,
			"hscrollbar-policy", GTK_POLICY_NEVER,
			"vscrollbar-policy", GTK_POLICY_ALWAYS,
			NULL);
	gtk_container_add (GTK_CONTAINER (scrolled), logview);
	
	completion = gtk_entry_completion_new ();
	gtk_entry_completion_set_text_column (completion, 0);
	
	entry = gtk_entry_new ();
	g_object_set (G_OBJECT (entry),
			"secondary-icon-stock", GTK_STOCK_MEDIA_PLAY,
			NULL);
	gtk_entry_set_completion (GTK_ENTRY (entry), completion);
	g_object_unref (completion); /* ? */
	g_signal_connect (entry, "activate", G_CALLBACK (gs_console_entry_activated), NULL);
	g_signal_connect (entry, "icon-release", G_CALLBACK (gs_console_entry_icon_clicked), NULL);
	g_signal_connect (entry, "key-press-event", G_CALLBACK (gui_console_key_pressed), NULL);
	
	page = gtk_box_new (GTK_ORIENTATION_VERTICAL, 4);
	g_object_set (G_OBJECT (page),
			"border-width", 2,
			"sensitive", FALSE,
			NULL);
	gtk_box_pack_start (GTK_BOX (page), scrolled, TRUE, TRUE, 0);
	gtk_box_pack_start (GTK_BOX (page), entry, FALSE, TRUE, 0);
	gtk_widget_show_all (page);
	
	return page;
}


void
gui_console_destroy ()
{
	if (fontname)
		g_free (fontname);
	
	g_hash_table_destroy (history);
	g_object_unref (tag_table);
}
