/* 
 * gui-buddy-list.c
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



#include <string.h>
#include "gui-buddy-list.h"
#include "gui-window.h"
#include "gswatcher.h"


enum {
	COLUMN_NAME,
	COLUMN_LAST_SEEN,
	COLUMN_PLACE,
	COLUMN_NOTIFY,
	COLUMN_NUMBER
};


static GObject *ctl_nickname, *ctl_listview;
static GtkListStore *liststore;


static gboolean
gui_blist_find_iter (const gchar *name, GtkTreeIter *iter)
{
	GtkTreeModel *model = GTK_TREE_MODEL (liststore);
	gchar *tmpname;
	
	if (gtk_tree_model_get_iter_first (model, iter)) {
		do {
			gtk_tree_model_get (model, iter, COLUMN_NAME, &tmpname, -1);
			if (strcmp (name, tmpname) == 0) {
				g_free (tmpname);
				return TRUE;
			}
			g_free (tmpname);
		} while (gtk_tree_model_iter_next (model, iter));
	}
	
	return FALSE;
}


static void
gui_blist_update_real (GsBuddy *buddy, GtkTreeIter *iter)
{
	gchar *lastseen = NULL, *place = NULL;
	
	GtkTreeSelection *sel = gtk_tree_view_get_selection (GTK_TREE_VIEW (ctl_listview));
	if (gtk_tree_selection_iter_is_selected (sel, iter)) {
		if (buddy->lastseen && buddy->lastaddr) {
			gchar *time = g_date_time_format (buddy->lastseen, "%x %X");
			lastseen = g_strdup_printf ("%s\n\t<small>at %s</small>",
					time, buddy->lastaddr);
			g_free (time);
		}
		
		GString *tmp = g_string_sized_new (256);
		GList *isrv = buddy->servers;
		while (isrv) {
			GsClient *client = isrv->data;
			g_string_append_printf (tmp, "%s\n<small>\t%s</small>",
					client->querier->name->str,
					gs_client_get_address (client));
			if (isrv->next)
				g_string_append_c (tmp, '\n');
			isrv = isrv->next;
		}
		place = tmp->str;
		g_string_free (tmp, FALSE);
	} else {
		if (buddy->lastseen && buddy->lastaddr)
			lastseen = g_date_time_format (buddy->lastseen, "%x %X");
		
		guint count = g_list_length (buddy->servers);
		if (count > 0) {
			const gchar *srvname = ((GsClient *) buddy->servers->data)->querier->name->str;
			if (count == 1) {
				place = g_strdup (srvname);
			} else if (count > 1) {
				place = g_strdup_printf ("%s <small>(+%d)</small>",
						srvname, count - 1);
			}
		}
	}
	
	gtk_list_store_set (liststore, iter,
			COLUMN_LAST_SEEN, lastseen ? lastseen : "",
			COLUMN_PLACE, place ? place : "",
			-1);
	
	if (lastseen)
		g_free (lastseen);
	if (place)
		g_free (place);
}


void
gui_blist_update (GsBuddy *buddy)
{
	GtkTreeIter iter;
	gui_blist_find_iter (buddy->name, &iter);
	gui_blist_update_real (buddy, &iter);
}


void
gui_blist_add (GsBuddy *buddy)
{
	GtkTreeIter iter;
	gtk_list_store_append (liststore, &iter);
	gtk_list_store_set (liststore, &iter,
			COLUMN_NAME, buddy->name,
			COLUMN_NOTIFY, buddy->notify,
			-1);
	
	GtkTreeSelection *sel = gtk_tree_view_get_selection (GTK_TREE_VIEW (ctl_listview));
	if (gtk_tree_selection_count_selected_rows (sel) == 0)
		gtk_tree_selection_select_iter (sel, &iter);
	
	gui_blist_update_real (buddy, &iter);
}


void G_MODULE_EXPORT
gui_blist_selection_changed (GtkTreeSelection *selection, gpointer udata)
{
	GtkTreeIter iter;
	GsBuddy *buddy;
	gchar *name;
	
	if (gtk_tree_model_get_iter_first (GTK_TREE_MODEL (liststore), &iter)) {
		do {
			gtk_tree_model_get (GTK_TREE_MODEL (liststore), &iter, 0, &name, -1);
			if (gtk_tree_selection_iter_is_selected (selection, &iter))
				gtk_entry_set_text (GTK_ENTRY (ctl_nickname), name);
			buddy = gs_application_find_buddy (name);
			g_free (name);
			gui_blist_update_real (buddy, &iter);
		} while (gtk_tree_model_iter_next (GTK_TREE_MODEL (liststore), &iter));
	} else {
		gtk_entry_set_text (GTK_ENTRY (ctl_nickname), "");
	}
}


void G_MODULE_EXPORT
gui_blist_name_activated (GtkEntry *entry, gpointer udata)
{
	const gchar *name = gtk_entry_get_text (entry);
	GsBuddy *buddy = gs_application_add_buddy (name, 0, NULL, TRUE);
	if (buddy) {
		gui_blist_add (buddy);
		gtk_entry_set_icon_sensitive (entry, GTK_ENTRY_ICON_SECONDARY, FALSE);
	}
}


void G_MODULE_EXPORT
gui_blist_name_icon_clicked (GtkEntry *entry, GtkEntryIconPosition icon_pos,
		GdkEvent *event, gpointer udata)
{
	if (icon_pos == GTK_ENTRY_ICON_SECONDARY) {
		const gchar *name = gtk_entry_get_text (GTK_ENTRY (entry));
		GsBuddy *buddy = gs_application_add_buddy (name, 0, NULL, TRUE);
		if (buddy) {
			gui_blist_add (buddy);
			gtk_entry_set_icon_sensitive (entry, GTK_ENTRY_ICON_SECONDARY, FALSE);
		}
	}
}


void G_MODULE_EXPORT
gui_blist_name_changed (GtkEntry *entry, gpointer udata)
{
	const gchar *name = gtk_entry_get_text (entry);
	gboolean active = *name && !gs_application_find_buddy (name);
	gtk_entry_set_icon_sensitive (entry, GTK_ENTRY_ICON_SECONDARY, active);
	gtk_entry_set_icon_activatable (entry, GTK_ENTRY_ICON_SECONDARY, active);
}


gboolean G_MODULE_EXPORT
gui_blist_clicked (GtkTreeView *treeview, GdkEventButton *event, gpointer udata)
{
	if (!(event->type == GDK_BUTTON_RELEASE && event->button == 1))
		return FALSE;
	
	GtkTreePath *path;
	GtkTreeViewColumn *column;
	if (!gtk_tree_view_get_path_at_pos (treeview, event->x, event->y, &path,
			&column, NULL, NULL))
		return FALSE;
	if (strcmp (gtk_tree_view_column_get_title (column), "") != 0) {
		gtk_tree_path_free (path);
		return FALSE;
	}
	
	gchar *name;
	GtkTreeIter iter;
	gtk_tree_model_get_iter (GTK_TREE_MODEL (liststore), &iter, path);
	gtk_tree_model_get (GTK_TREE_MODEL (liststore), &iter,
			COLUMN_NAME, &name,
			-1);
	
	GtkWidget *dialog = gtk_message_dialog_new (GTK_WINDOW (window),
			GTK_DIALOG_DESTROY_WITH_PARENT, GTK_MESSAGE_QUESTION,
			GTK_BUTTONS_YES_NO, "Are you sure you want to remove \"%s\"?", name);
	if (gtk_dialog_run (GTK_DIALOG (dialog)) == GTK_RESPONSE_YES) {
		//gs_buddylist_select_another (iter);
		gtk_list_store_remove (liststore, &iter);
		gs_application_remove_buddy (name);
	}
	
	gtk_widget_destroy (dialog);
	gtk_tree_path_free (path);
	g_free (name);
	
	return FALSE;
}


void G_MODULE_EXPORT
gui_blist_notify_toggled (GtkCellRendererToggle *cell, gchar *p, gpointer udata)
{
	GtkTreeIter iter;
	GtkTreePath *path;
	gchar *name;
	gboolean notify;
	
	path = gtk_tree_path_new_from_string (p);
	gtk_tree_model_get_iter (GTK_TREE_MODEL (liststore), &iter, path);
	gtk_tree_path_free (path);
	gtk_tree_model_get (GTK_TREE_MODEL (liststore), &iter,
			COLUMN_NAME, &name,
			COLUMN_NOTIFY, &notify,
			-1);
	notify = !notify;
	gtk_list_store_set (liststore, &iter, COLUMN_NOTIFY, notify, -1);
	gs_application_change_buddy (name, notify);
	g_free (name);
}


GtkWidget *
gui_blist_create ()
{
	GError *error = NULL;
	GtkBuilder *builder = gtk_builder_new ();
	if (gtk_builder_add_from_resource (builder, "/org/gswatcher/ui/buddy-list.ui", &error) == 0) {
		g_warning (error->message);
		g_object_unref (builder);
		g_error_free (error);
		return NULL;
	}
	
	GObject *root = gtk_builder_get_object (builder, "root");
	liststore = GTK_LIST_STORE (gtk_builder_get_object (builder, "liststore"));
	
	ctl_nickname = gtk_builder_get_object (builder, "nickname");
	ctl_listview = gtk_builder_get_object (builder, "listview");
	
	gtk_builder_connect_signals (builder, NULL);
	
	g_object_ref (root);
	g_object_unref (builder);
	g_object_force_floating (root);
	return GTK_WIDGET (root);
}
