/* 
 * gui-chat.c
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



#include "platform.h"
#include "gui-window.h"
#include "gswatcher.h"
#include "gui-server-list.h"
#include "gui-chat.h"


static GtkWidget *page, *logview, *entry;

static GtkTextTagTable *tag_table;
static GtkTextTag *tags[4] = {NULL};


void
gui_chat_log (GsClient *client, const gchar *name, gint team, const gchar *msg)
{
	GtkTextIter iter;
	gtk_text_buffer_get_end_iter (client->chat_buffer, &iter);
	gtk_text_buffer_insert_with_tags (client->chat_buffer, &iter,
			name, -1, tags[team], NULL);
	gtk_text_buffer_insert (client->chat_buffer, &iter, ": ", -1);
	gtk_text_buffer_insert (client->chat_buffer, &iter, msg, -1);
	gtk_text_buffer_insert (client->chat_buffer, &iter, "\n", -1);
	
	if (gui_slist_get_selected () == client) {
		gint pos, end;
		g_object_get (client->chat_buffer, "cursor-position", &pos, NULL);
		end = gtk_text_buffer_get_char_count (client->chat_buffer);
		
		/* if cursor put at the end */
		if (pos == end) {
			GtkTextMark *mark = gtk_text_buffer_create_mark (client->chat_buffer,
					NULL, &iter, TRUE);
			gtk_text_view_scroll_mark_onscreen (GTK_TEXT_VIEW (logview), mark);
			gtk_text_buffer_delete_mark (client->chat_buffer, mark);
		}
	}
}


void
gui_chat_send (GsClient *client, const gchar *msg)
{
	gs_client_send_message (client, msg);
}


static void
gui_chat_entry_activated (GtkEntry *entry, gpointer udata)
{
	gui_chat_send (gui_slist_get_selected (), gtk_entry_get_text (entry));
	gtk_entry_set_text (entry, "");
}


static void
gui_chat_entry_icon_clicked (GtkEntry *entry, GtkEntryIconPosition icon,
		GdkEvent *event, gpointer udata)
{
	if (icon == GTK_ENTRY_ICON_SECONDARY) {
		gui_chat_send (gui_slist_get_selected (), gtk_entry_get_text (entry));
		gtk_entry_set_text (entry, "");
	}
}


void
gui_chat_setup (GsClient *client)
{
	if (client) {
		gtk_widget_set_sensitive (page, TRUE);
		
		GtkTextIter iter;
		gtk_text_view_set_buffer (GTK_TEXT_VIEW (logview), client->chat_buffer);
		gtk_text_buffer_get_end_iter (client->chat_buffer, &iter);
		
		GtkTextMark *mark = gtk_text_buffer_create_mark (client->chat_buffer, NULL, &iter, TRUE);
		gtk_text_view_scroll_mark_onscreen (GTK_TEXT_VIEW (logview), mark);
		gtk_text_buffer_delete_mark (client->chat_buffer, mark);
	} else {
		gtk_widget_set_sensitive (page, FALSE);
		
		gtk_text_view_set_buffer (GTK_TEXT_VIEW (logview), NULL);
	}
}


void
gui_chat_init (GsClient *client)
{
	client->chat_buffer = gtk_text_buffer_new (tag_table);
}


GtkWidget *
gui_chat_create ()
{
	/* tag table */
	tag_table = gtk_text_tag_table_new ();
	
	tags[0] = gtk_text_tag_new ("team0");
	g_object_set (tags[0], "foreground", "orange", NULL);
	gtk_text_tag_table_add (tag_table, tags[0]);
	
	tags[1] = gtk_text_tag_new ("team1");
	g_object_set (tags[1], "foreground", "dark red", NULL);
	gtk_text_tag_table_add (tag_table, tags[1]);
	
	tags[2] = gtk_text_tag_new ("team2");
	g_object_set (tags[2], "foreground", "dark blue", NULL);
	gtk_text_tag_table_add (tag_table, tags[2]);
	
	tags[3] = gtk_text_tag_new ("team3");
	g_object_set (tags[3], "foreground", "dark gray", NULL);
	gtk_text_tag_table_add (tag_table, tags[3]);
	
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
	
	entry = gtk_entry_new ();
	g_object_set (G_OBJECT (entry),
			"secondary-icon-stock", GTK_STOCK_MEDIA_PLAY,
			NULL);
	g_signal_connect (entry, "activate", G_CALLBACK (gui_chat_entry_activated), NULL);
	g_signal_connect (entry, "icon-release", G_CALLBACK (gui_chat_entry_icon_clicked), NULL);
	
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
