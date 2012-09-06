/* 
 * gui-log.c
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



#include "gui-window.h"
#include "gui-server-list.h"
#include "gui-log.h"
#include "gtkpopupbutton.h"


static GtkWidget *scrolled, *logview;
static GtkWidget *toolbar, *enable_log, *disable_log, *auto_button,
		*show_output, *text_wrapping;


void
gui_log_init (GsClient *client)
{
	client->log_buffer = gtk_text_buffer_new (NULL);
}


void
gui_log_setup (GsClient *client)
{
	if (client) {
		gtk_text_view_set_wrap_mode (GTK_TEXT_VIEW (logview),
				client->log_wrapping ? GTK_WRAP_WORD_CHAR : GTK_WRAP_NONE);
		gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolled),
				client->log_wrapping ? GTK_POLICY_NEVER : GTK_POLICY_AUTOMATIC,
				GTK_POLICY_AUTOMATIC);
		
		gtk_widget_set_sensitive (scrolled, TRUE);
		gtk_widget_set_sensitive (toolbar, TRUE);
		
		GtkTextIter iter;
		gtk_text_view_set_buffer (GTK_TEXT_VIEW (logview), client->log_buffer);
		gtk_text_buffer_get_end_iter (client->log_buffer, &iter);
		
		GtkTextMark *mark = gtk_text_buffer_create_mark (client->log_buffer,
				NULL, &iter, TRUE);
		gtk_text_view_scroll_mark_onscreen (GTK_TEXT_VIEW (logview), mark);
		gtk_text_buffer_delete_mark (client->log_buffer, mark);
	} else {
		gtk_widget_set_sensitive (scrolled, FALSE);
		gtk_widget_set_sensitive (toolbar, FALSE);
		
		gtk_text_view_set_buffer (GTK_TEXT_VIEW (logview), NULL);
	}
}


void
gui_log_print (GsClient *client, const gchar *msg)
{
	if (!client->log_output)
		return;
	
	if (gtk_text_buffer_get_line_count (client->log_buffer) > 500) {
		GtkTextIter start, end;
		gtk_text_buffer_get_start_iter (client->log_buffer, &start);
		gtk_text_buffer_get_iter_at_line (client->log_buffer, &end, 250);
		gtk_text_buffer_delete (client->log_buffer, &start, &end);
	}
	
	GtkTextIter iter;
	gtk_text_buffer_get_end_iter (client->log_buffer, &iter);
	gtk_text_buffer_insert (client->log_buffer, &iter, msg, -1);
	
	if (gui_slist_get_selected () == client) {
		GtkAdjustment *adj = gtk_scrollable_get_vadjustment (GTK_SCROLLABLE (logview));
		gdouble upper, value, page_size;
		g_object_get (adj, "value", &value, "upper", &upper, "page-size", &page_size, NULL);
		if (value + page_size >= upper) {
			GtkTextMark *mark = gtk_text_buffer_create_mark (
					client->log_buffer, NULL, &iter, TRUE);
			gtk_text_view_scroll_mark_onscreen (GTK_TEXT_VIEW (logview), mark);
			gtk_text_buffer_delete_mark (client->log_buffer, mark);
		}
	}
}


static void
gui_log_enable_clicked (GtkButton *button, gpointer udata)
{
	GsClient *client = gui_slist_get_selected ();
	if (!client)
		return;
	gs_client_enable_log (client, TRUE);
}

static void
gui_log_disable_clicked (GtkButton *button, gpointer udata)
{
	GsClient *client = gui_slist_get_selected ();
	if (!client)
		return;
	gs_client_enable_log (client, FALSE);
}

static void
gui_log_auto_toggled (GtkButton *button, gpointer udata)
{
	GsClient *client = gui_slist_get_selected ();
	if (!client)
		return;
	client->log_auto = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (auto_button));
}


static void
gui_log_view_popup (GtkPopupButton *popup_button, gpointer udata)
{
	GsClient *client = gui_slist_get_selected ();
	
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (show_output), client->log_output);
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (text_wrapping), client->log_wrapping);
}


static void
gui_log_view_popdown (GtkPopupButton *popup_button, gpointer udata)
{
	GsClient *client = gui_slist_get_selected ();
	
	client->log_output = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (show_output));
	client->log_wrapping = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (text_wrapping));
	
	gtk_text_view_set_wrap_mode (GTK_TEXT_VIEW (logview),
			client->log_wrapping ? GTK_WRAP_WORD_CHAR : GTK_WRAP_NONE);
	gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolled),
			client->log_wrapping ? GTK_POLICY_NEVER : GTK_POLICY_AUTOMATIC,
			GTK_POLICY_AUTOMATIC);
}


static void
gui_log_clear_clicked (GtkButton *button, gpointer udata)
{
	GsClient *client = gui_slist_get_selected ();
	gtk_text_buffer_set_text (client->log_buffer, "", -1);
}


GtkWidget *
gui_log_create_bar ()
{
	/* view panel */
	show_output = gtk_check_button_new_with_label (_("Show output"));
	text_wrapping = gtk_check_button_new_with_label (_("Text wrapping"));
	
	GtkWidget *clear_log = gtk_button_new_with_label (_("Clear log"));
	g_signal_connect (clear_log, "clicked", G_CALLBACK (gui_log_clear_clicked), NULL);
	
	GtkWidget *grid = gtk_grid_new ();
	gtk_grid_set_row_spacing (GTK_GRID (grid), 2);
	gtk_grid_set_column_spacing (GTK_GRID (grid), 2);
	gtk_grid_attach (GTK_GRID (grid), show_output, 0, 0, 1, 1);
	gtk_grid_attach (GTK_GRID (grid), text_wrapping, 0, 2, 1, 1);
	gtk_grid_attach (GTK_GRID (grid), clear_log, 0, 3, 1, 1);
	gtk_widget_show_all (grid);
	
	/* view button */
	GtkWidget *label = gtk_label_new (_("View"));
	GtkWidget *separator = gtk_separator_new (GTK_ORIENTATION_VERTICAL);
	GtkWidget *arrow = gtk_arrow_new (GTK_ARROW_UP, GTK_SHADOW_NONE);
	
	GtkWidget *box = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 4);
	gtk_box_pack_start (GTK_BOX (box), label, TRUE, TRUE, 0);
	gtk_box_pack_start (GTK_BOX (box), separator, FALSE, TRUE, 0);
	gtk_box_pack_start (GTK_BOX (box), arrow, FALSE, TRUE, 0);
	
	GtkWidget *view = gtk_popup_button_new ();
	gtk_container_add (GTK_CONTAINER (view), box);
	GtkWidget *popup_window = gtk_popup_button_get_popup (GTK_POPUP_BUTTON (view));
	gtk_container_add (GTK_CONTAINER (popup_window), grid);
	g_signal_connect (view, "popup", G_CALLBACK (gui_log_view_popup), NULL);
	g_signal_connect (view, "popdown", G_CALLBACK (gui_log_view_popdown), NULL);
	
	/* settings */
	enable_log = gtk_button_new_with_label (_("Enable logging"));
	g_signal_connect (enable_log, "clicked", G_CALLBACK (gui_log_enable_clicked), NULL);
	
	disable_log = gtk_button_new_with_label (_("Disable logging"));
	g_signal_connect (disable_log, "clicked", G_CALLBACK (gui_log_disable_clicked), NULL);
	
	auto_button = gtk_check_button_new_with_label (_("Keep on"));
	g_signal_connect (auto_button, "clicked", G_CALLBACK (gui_log_auto_toggled), NULL);
	
	grid = gtk_grid_new ();
	gtk_grid_set_row_spacing (GTK_GRID (grid), 2);
	gtk_grid_set_column_spacing (GTK_GRID (grid), 2);
	gtk_grid_attach (GTK_GRID (grid), enable_log, 0, 0, 1, 1);
	gtk_grid_attach (GTK_GRID (grid), disable_log, 0, 1, 1, 1);
	gtk_grid_attach (GTK_GRID (grid), auto_button, 0, 2, 1, 1);
	gtk_widget_show_all (grid);
	
	/* settings button */
	label = gtk_label_new (_("Settings"));
	separator = gtk_separator_new (GTK_ORIENTATION_VERTICAL);
	arrow = gtk_arrow_new (GTK_ARROW_UP, GTK_SHADOW_NONE);
	
	box = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 4);
	gtk_box_pack_start (GTK_BOX (box), label, TRUE, TRUE, 0);
	gtk_box_pack_start (GTK_BOX (box), separator, FALSE, TRUE, 0);
	gtk_box_pack_start (GTK_BOX (box), arrow, FALSE, TRUE, 0);
	
	GtkWidget *settings = gtk_popup_button_new ();
	gtk_container_add (GTK_CONTAINER (settings), box);
	popup_window = gtk_popup_button_get_popup (GTK_POPUP_BUTTON (settings));
	gtk_container_add (GTK_CONTAINER (popup_window), grid);
	
	
	toolbar = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 4);
	g_object_set (G_OBJECT (toolbar),
			"sensitive", FALSE,
			NULL);
	gtk_box_pack_start (GTK_BOX (toolbar), settings, FALSE, TRUE, 0);
	gtk_box_pack_start (GTK_BOX (toolbar), view, FALSE, TRUE, 0);
	gtk_widget_show_all (toolbar);
	
	return toolbar;
}


GtkWidget *
gui_log_create ()
{
	logview = gtk_text_view_new ();
	g_object_set (G_OBJECT (logview),
			"editable", FALSE,
			"wrap-mode", GTK_WRAP_WORD_CHAR,
			NULL);
	
	scrolled = gtk_scrolled_window_new (NULL, NULL);
	g_object_set (G_OBJECT (scrolled),
			"border-width", 2,
			"shadow-type", GTK_SHADOW_IN,
			"hscrollbar-policy", GTK_POLICY_NEVER,
			"vscrollbar-policy", GTK_POLICY_ALWAYS,
			"sensitive", FALSE,
			NULL);
	gtk_container_add (GTK_CONTAINER (scrolled), logview);
	gtk_widget_show_all (scrolled);
	
	return scrolled;
}
