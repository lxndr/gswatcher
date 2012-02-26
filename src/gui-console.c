/* 
 * gui-console.c
 * 
 * Copyright (C) 2011-2012 GSTool Developer(s)
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



#include "platform.h"
#include "gui-window.h"
#include "gstool.h"
#include "gui-server-list.h"
#include "gui-console.h"


static GtkWidget *logview;
static GtkWidget *entry;
static GtkEntryCompletion *history;
static GtkTreeIter history_iter;
static gboolean history_end;
static GtkWidget *password;

static GtkTextTagTable *tag_table;
static GtkTextTag *tags[4] = {NULL};

static gboolean sysfont = TRUE;
static gchar *fontname = NULL;



static gboolean
gui_console_set_font_real (const gchar *name)
{
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
	if (!name)
		name = GS_DEFAULT_MONOSPACE_FONT;
	
	if (!sysfont && gui_console_set_font_real (name)) {
		if (fontname)
			g_free (fontname);
		fontname = g_strdup (name);
	}
}

const gchar *
gui_console_get_font ()
{
	return fontname;
}


void
gs_console_log (GsClient *client, GsLogType type, const gchar *msg)
{
	GtkTextIter iter;
	gtk_text_buffer_get_end_iter (client->console_buffer, &iter);
	gtk_text_buffer_insert_with_tags (client->console_buffer, &iter, msg, -1, tags[type], NULL);
	gtk_text_buffer_insert (client->console_buffer, &iter, "\n", -1);
	
	if (gui_slist_get_selected () == client) {
		gint pos, end;
		g_object_get (client->console_buffer, "cursor-position", &pos, NULL);
		end = gtk_text_buffer_get_char_count (client->console_buffer);
		
		/* if cursor put at the end */
		if (pos == end) {
			GtkTextMark *mark = gtk_text_buffer_create_mark (client->console_buffer,
					NULL, &iter, TRUE);
			gtk_text_view_scroll_mark_onscreen (GTK_TEXT_VIEW (logview), mark);
			gtk_text_buffer_delete_mark (client->console_buffer, mark);
		}
	}
}


static void
gs_console_add_history (GsClient *client, const gchar *cmd)
{
	GtkTreeModel *model = GTK_TREE_MODEL (client->console_history);
	GtkTreeIter iter;
	gchar *value;
	
	if (gtk_tree_model_get_iter_first (model, &iter)) {
		do {
			gtk_tree_model_get (model, &iter, 0, &value, -1);
			if (strcmp (cmd, value) == 0) {
				gtk_list_store_remove (client->console_history, &iter);
				g_free (value);
				break;
			}
			g_free (value);
		} while (gtk_tree_model_iter_next (model, &iter));
	}
	
	gtk_list_store_append (client->console_history, &iter);
	gtk_list_store_set (client->console_history, &iter, 0, cmd, -1);
	history_end = TRUE;
}


void
gs_console_send (GsClient *cl, const gchar *cmd)
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
	
	gs_console_add_history (cl, cmd);
	gs_console_log (cl, GS_CONSOLE_COMMAND, cmd);
	gs_client_send_command (cl, cmd);
}


static void
gs_console_entry_activated (GtkEntry *entry, gpointer udata)
{
	gs_console_send (gui_slist_get_selected (), gtk_entry_get_text (entry));
	gtk_entry_set_text (entry, "");
}


static void
gs_console_entry_icon_clicked (GtkEntry *entry, GtkEntryIconPosition icon,
		GdkEvent *event, gpointer udata)
{
	if (icon == GTK_ENTRY_ICON_SECONDARY) {
		gs_console_send (gui_slist_get_selected (), gtk_entry_get_text (entry));
		gtk_entry_set_text (entry, "");
	}
}


static gboolean
gui_console_key_pressed (GtkWidget *widget, GdkEventKey *event, gpointer udata)
{
	GsClient *client = gui_slist_get_selected ();
	GtkTreeModel *model = GTK_TREE_MODEL (client->console_history);
	gchar *cmd;
	
	if (!client)
		return FALSE;
	
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
	
	gtk_tree_model_get (model, &history_iter, 0, &cmd, -1);
	gtk_entry_set_text (GTK_ENTRY (entry), cmd);
	gtk_editable_set_position (GTK_EDITABLE (entry),
			gtk_entry_get_text_length (GTK_ENTRY (entry)));
	g_free (cmd);
	
	return TRUE;
}
														

static void
gs_console_password_changed (GtkEntry *entry, gpointer udata)
{
	GsClient *client = gui_slist_get_selected ();
	gsq_console_set_password (client->console, gtk_entry_get_text (entry));
	gs_save_server_list ();
}


static void
gs_console_set_password (const gchar *pass)
{
	g_signal_handlers_block_by_func (password, gs_console_password_changed, NULL);
	gtk_entry_set_text (GTK_ENTRY (password), pass ? pass : "");
	g_signal_handlers_unblock_by_func (password, gs_console_password_changed, NULL);
}


void
gs_console_set (GsClient *client)
{
	if (client) {
		GtkTextIter iter;
		gtk_text_view_set_buffer (GTK_TEXT_VIEW (logview), client->console_buffer);
		gtk_text_buffer_get_end_iter (client->console_buffer, &iter);
		
		GtkTextMark *mark = gtk_text_buffer_create_mark (client->console_buffer, NULL, &iter, TRUE);
		gtk_text_view_scroll_mark_onscreen (GTK_TEXT_VIEW (logview), mark);
		gtk_text_buffer_delete_mark (client->console_buffer, mark);
		
		history_end = TRUE;
		
		gtk_entry_completion_set_model (history, GTK_TREE_MODEL (client->console_history));
//		gint n = gtk_tree_model_iter_n_children (GTK_TREE_MODEL (client->console_history), NULL);
//		if (n != 0)
//			gtk_tree_model_iter_nth_child (GTK_TREE_MODEL (client->console_history),
//					&history_iter, NULL, n - 1);
		
		gs_console_set_password (gsq_console_get_password (client->console));
	} else {
		gtk_text_view_set_buffer (GTK_TEXT_VIEW (logview), NULL);
		gtk_entry_completion_set_model (history, NULL);
		gs_console_set_password (NULL);
	}
}


void
gs_console_init (GsClient *client)
{
	client->console_buffer = gtk_text_buffer_new (tag_table);
	client->console_history = gtk_list_store_new (1, G_TYPE_STRING);
}


GtkWidget *
gs_console_create_bar ()
{
	GtkWidget *password_label = gtk_label_new (_("Password:"));
	g_object_set (G_OBJECT (password_label),
			"use-underline", TRUE,
			"mnemonic-widget", password,
			NULL);
	
	password = gtk_entry_new ();
	g_object_set (G_OBJECT (password),
			"visibility", FALSE,
			NULL);
	g_signal_connect (password, "changed", G_CALLBACK (gs_console_password_changed),
			NULL);
	
	GtkWidget *bar = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 4);
	gtk_box_pack_start (GTK_BOX (bar), password_label, FALSE, TRUE, 0);
	gtk_box_pack_start (GTK_BOX (bar), password, TRUE, TRUE, 0);
	gtk_widget_show_all (bar);
	return bar;
}


GtkWidget *
gs_console_create ()
{
	tag_table = gtk_text_tag_table_new ();
	
	tags[GS_CONSOLE_INFO] = gtk_text_tag_new ("inf");
	g_object_set (tags[GS_CONSOLE_INFO], "foreground", "dark gray", NULL);
	gtk_text_tag_table_add (tag_table, tags[GS_CONSOLE_INFO]);
	
	tags[GS_CONSOLE_COMMAND] = gtk_text_tag_new ("cmd");
	g_object_set (tags[GS_CONSOLE_COMMAND], "foreground", "dark green", NULL);
	gtk_text_tag_table_add (tag_table, tags[GS_CONSOLE_COMMAND]);
	
	tags[GS_CONSOLE_ERROR] = gtk_text_tag_new ("err");
	g_object_set (tags[GS_CONSOLE_ERROR], "foreground", "dark red", NULL);
	gtk_text_tag_table_add (tag_table, tags[GS_CONSOLE_ERROR]);
	
// widgets
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
	
	history = gtk_entry_completion_new ();
	gtk_entry_completion_set_text_column (history, 0);
	
	entry = gtk_entry_new ();
	g_object_set (G_OBJECT (entry),
			"secondary-icon-stock", GTK_STOCK_MEDIA_PLAY,
			NULL);
	gtk_entry_set_completion (GTK_ENTRY (entry), history);
	g_object_unref (history);
	g_signal_connect (entry, "activate", G_CALLBACK (gs_console_entry_activated), NULL);
	g_signal_connect (entry, "icon-release", G_CALLBACK (gs_console_entry_icon_clicked), NULL);
	g_signal_connect (entry, "key-press-event", G_CALLBACK (gui_console_key_pressed), NULL);
	
	GtkWidget *console = gtk_box_new (GTK_ORIENTATION_VERTICAL, 4);
	gtk_box_pack_start (GTK_BOX (console), scrolled, TRUE, TRUE, 0);
	gtk_box_pack_start (GTK_BOX (console), entry, FALSE, TRUE, 0);
	gtk_widget_show_all (console);
	
	return console;
}
