/* 
 * gui-log.c
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



#include "gui-window.h"
#include "gui-server-list.h"
#include "gui-log.h"


static GtkWidget *logview;
static GtkWidget *enable_log, *disable_log, *auto_button;


void
gui_log_init (GsClient *client)
{
	client->log_buffer = gtk_text_buffer_new (NULL);
}


void
gui_log_set (GsClient *client)
{
	if (client) {
		GtkTextIter iter;
		gtk_text_view_set_buffer (GTK_TEXT_VIEW (logview), client->log_buffer);
		gtk_text_buffer_get_end_iter (client->log_buffer, &iter);
		
		GtkTextMark *mark = gtk_text_buffer_create_mark (client->log_buffer, NULL, &iter, TRUE);
		gtk_text_view_scroll_mark_onscreen (GTK_TEXT_VIEW (logview), mark);
		gtk_text_buffer_delete_mark (client->log_buffer, mark);
		
		gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (auto_button), client->log_auto);
	} else {
		gtk_text_view_set_buffer (GTK_TEXT_VIEW (logview), NULL);
		gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (auto_button), FALSE);
	}
}


void
gui_log_print (GsClient *client, const gchar *msg)
{
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
		GtkTextMark *mark = gtk_text_buffer_create_mark (client->log_buffer, NULL, &iter, TRUE);
		gtk_text_view_scroll_mark_onscreen (GTK_TEXT_VIEW (logview), mark);
		gtk_text_buffer_delete_mark (client->log_buffer, mark);
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


GtkWidget *
gui_log_create_bar ()
{
	enable_log = gtk_button_new_with_label (_("Enable log"));
	g_signal_connect (enable_log, "clicked", G_CALLBACK (gui_log_enable_clicked), NULL);
	
	disable_log = gtk_button_new_with_label (_("Disable log"));
	g_signal_connect (disable_log, "clicked", G_CALLBACK (gui_log_disable_clicked), NULL);
	
	auto_button = gtk_check_button_new_with_label (_("Keep on"));
	g_signal_connect (auto_button, "clicked", G_CALLBACK (gui_log_auto_toggled), NULL);
	
	GtkWidget *bar = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 4);
	gtk_box_pack_start (GTK_BOX (bar), enable_log, FALSE, TRUE, 0);
	gtk_box_pack_start (GTK_BOX (bar), disable_log, FALSE, TRUE, 0);
	gtk_box_pack_start (GTK_BOX (bar), auto_button, FALSE, TRUE, 0);
	gtk_widget_show_all (bar);
	
	return bar;
}


GtkWidget *
gui_log_create ()
{
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
	gtk_widget_show_all (scrolled);
	
	return scrolled;
}
