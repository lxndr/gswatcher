/* 
 * gui-player-list.c
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
#include "gui-buddy-list.h"
#include "gui-player-list.h"
#include "gui-server-list.h"


static GObject *ctl_listview;
static GtkWidget *ctl_add;


void
gui_plist_setup (GsClient *client)
{
	GtkTreeViewColumn *column;
	
	/* remove columns */
	GList *columns = gtk_tree_view_get_columns (GTK_TREE_VIEW (ctl_listview));
	GList *icolumn = g_list_next (columns); /* skip name column */
	while (icolumn) {
		gtk_tree_view_remove_column (GTK_TREE_VIEW (ctl_listview), icolumn->data);
		icolumn = g_list_next (icolumn);
	}
	g_list_free (columns);
	
	/* remove players */
	gtk_tree_view_set_model (GTK_TREE_VIEW (ctl_listview), NULL);
	
	if (!client)
		return;
	
	gint i;
	GtkCellRenderer *cell;
	GsqField *field;
	GArray *fields = gsq_querier_get_fields (client->querier);
	
	GType *types = g_new (GType, fields->len + 1);
	types[0] = G_TYPE_STRING;
	
	for (i = 0; i < fields->len; i++) {
		field = &g_array_index (fields, GsqField, i);
		cell = gtk_cell_renderer_text_new ();
		column = gtk_tree_view_column_new_with_attributes (
				gettext (field->name), cell, "text", i + 1, NULL);
		gtk_tree_view_column_set_sort_column_id (column, i + 1);
		gtk_tree_view_append_column (GTK_TREE_VIEW (ctl_listview), column);
		types[i + 1] = field->type;
	}
	
	GtkListStore *store = gtk_list_store_newv (fields->len + 1, types);
	gtk_tree_view_set_model (GTK_TREE_VIEW (ctl_listview), GTK_TREE_MODEL (store));
	g_object_unref (store);
	g_free (types);
}


void
gui_plist_update (GsClient *client)
{
	GtkTreeSelection *selection;
	GtkTreeModel *model;
	GtkListStore *liststore;
	GtkTreeIter iter;
	gchar *selected = NULL;
	GList *iplayer;
	GArray *fields;
	guint i;
	
	if (!client)
		return;
	
	/* store selected player */
	selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (ctl_listview));
	if (gtk_tree_selection_get_selected (selection, &model, &iter))
		gtk_tree_model_get (model, &iter, 0, &selected, -1);
	liststore = GTK_LIST_STORE (model);
	gtk_list_store_clear (liststore);
	
	fields = gsq_querier_get_fields (client->querier);
	iplayer = gsq_querier_get_players (client->querier);
	while (iplayer) {
		GsqPlayer *player = iplayer->data;
		
		gtk_list_store_append (liststore, &iter);
		gtk_list_store_set (liststore, &iter, 0, player->name, -1);
		
		for (i = 0; i < fields->len; i++)
			gtk_list_store_set_value (liststore, &iter, i + 1, &player->values[i]);
		
		/* restore selected player */
		if (selected && strcmp (selected, player->name) == 0)
			gtk_tree_selection_select_iter (selection, &iter);
		iplayer = iplayer->next;
	}
	
	if (selected)
		g_free (selected);
}


void G_MODULE_EXPORT
gui_plist_selection_changed (GtkTreeSelection *selection, gpointer udata)
{
	GtkTreeModel *model;
	GtkTreeIter iter;
	gchar *name;
	
	if (!gtk_tree_selection_get_selected (selection, &model, &iter)) {
		gtk_widget_set_sensitive (GTK_WIDGET (ctl_add), FALSE);
		return;
	}
	
	gtk_tree_model_get (model, &iter, 0, &name, -1);
	if (!*name) {
		gtk_widget_set_sensitive (GTK_WIDGET (ctl_add), FALSE);
		goto finish;
	}
	
	if (gs_application_find_buddy (name)) {
		gtk_widget_set_sensitive (GTK_WIDGET (ctl_add), FALSE);
		goto finish;
	}
	
	gtk_widget_set_sensitive (GTK_WIDGET (ctl_add), TRUE);

finish:
	g_free (name);
}


GtkWidget *
gui_plist_create ()
{
	GError *error = NULL;
	GtkBuilder *builder = gtk_builder_new ();
	if (gtk_builder_add_from_resource (builder, "/org/gswatcher/ui/player-list.ui", &error) == 0)
		g_error (error->message);
	
	GObject *root = gtk_builder_get_object (builder, "root");
	ctl_listview  = gtk_builder_get_object (builder, "listview");
	
	gtk_builder_connect_signals (builder, NULL);
	root = g_object_ref (root);
	g_object_unref (builder);
	return GTK_WIDGET (root);
}


void G_MODULE_EXPORT
gui_plist_add_clicked (GtkButton *button, gpointer udata)
{
	GtkTreeModel *model;
	GtkTreeIter iter;
	gchar *name;
	
	GtkTreeSelection * selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (ctl_listview));
	if (!gtk_tree_selection_get_selected (selection, &model, &iter)) {
		gtk_widget_set_sensitive (GTK_WIDGET (ctl_add), FALSE);
		return;
	}
	
	gtk_tree_model_get (model, &iter, 0, &name, -1);
	if (name && !*name) {
		gtk_widget_set_sensitive (GTK_WIDGET (ctl_add), FALSE);
		g_free (name);
		return;
	}
	
	if (gs_application_find_buddy (name)) {
		gtk_widget_set_sensitive (GTK_WIDGET (ctl_add), FALSE);
		g_free (name);
		return;
	}
	
	GsClient *srv = gui_slist_get_selected ();
	
	GsBuddy *buddy = gs_application_add_buddy (name, time (NULL),
			gsq_querier_get_address (srv->querier), TRUE);
	gui_blist_add (buddy);
	g_free (name);
	gtk_widget_set_sensitive (GTK_WIDGET (ctl_add), FALSE);
}


GtkWidget *
gui_plist_create_bar ()
{
	GtkWidget *add_image = gtk_image_new_from_icon_name ("list-add", GTK_ICON_SIZE_MENU);
	
	ctl_add = gtk_button_new ();
	g_object_set (G_OBJECT (ctl_add),
			"image", add_image,
			"tooltip-text", _("Add player to your buddy list"),
			"sensitive", FALSE,
			NULL);
	g_signal_connect (ctl_add, "clicked", G_CALLBACK (gui_plist_add_clicked), NULL);
	
	return ctl_add;
}
