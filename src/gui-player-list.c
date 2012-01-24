/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*- */
/* 
 * gui-player-list.c
 * Copyright (C) lxndr 2011 <lxndr87@users.sourceforge.net>
 * 
 * gswatcher is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * GSWatcher is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License along
 * with this program.  If not, see <http://www.gnu.org/licenses/>.
 */


#include "gui-window.h"
#include "gui-buddy-list.h"
#include "gui-player-list.h"


static GtkWidget *listview, *add, *add_image;


void
gui_plist_update (GsClient *client)
{
	GtkTreeSelection *selection;
	GtkTreeModel *model;
	GtkTreeIter iter;
	gchar *selected = NULL;
	int n = 0; // player
	GList *iplayer;
	
	selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (listview));
	if (gtk_tree_selection_get_selected (selection, &model, &iter))
		gtk_tree_model_get (model, &iter, 1, &selected, -1);
	if (!model) return;
	gtk_list_store_clear (GTK_LIST_STORE (model));
	
	if (!client) return;
	
	iplayer = gsq_querier_get_players (client->querier);
	while (iplayer) {
		GsqPlayer *player = iplayer->data;
		n++;
		
		gtk_list_store_append (GTK_LIST_STORE (model), &iter);
		gtk_list_store_set (GTK_LIST_STORE (model), &iter,
				0, n,
				1, player->name,
				2, player->kills,
				3, player->time,
				-1);
		
		if (selected && strcmp (selected, player->name) == 0)
			gtk_tree_selection_select_iter (selection, &iter);
		iplayer = iplayer->next;
	}
	
	if (selected)
		g_free (selected);
}


static void
gui_plist_selection_changed (GtkTreeSelection *selection, gpointer udata)
{
	GtkTreeModel *model;
	GtkTreeIter iter;
	gchar *name;
	
	if (!gtk_tree_selection_get_selected (selection, &model, &iter)) {
		gtk_widget_set_sensitive (add, FALSE);
		return;
	}
	
	gtk_tree_model_get (model, &iter, 1, &name, -1);
	if (!*name) {
		gtk_widget_set_sensitive (add, FALSE);
		g_free (name);
		return;
	}
	
	if (gs_find_buddy (name)) {
		gtk_widget_set_sensitive (add, FALSE);
		return;
	}
	
	gtk_widget_set_sensitive (add, TRUE);
	g_free (name);
}


GtkWidget *
gui_plist_create ()
{
	GtkTreeViewColumn *column;
	GtkCellRenderer *cell;
	
	GtkListStore *model = gtk_list_store_new (4, G_TYPE_INT, G_TYPE_STRING,
			G_TYPE_INT, G_TYPE_STRING);
	listview = gtk_tree_view_new_with_model (GTK_TREE_MODEL (model));
	g_object_unref (model);
	
	GtkTreeSelection *selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (listview));
	g_signal_connect (selection, "changed", G_CALLBACK (gui_plist_selection_changed), NULL);
	
	
/*
	column = gtk_tree_view_column_new ();
	g_object_set (G_OBJECT (column),
			"title", _("#"),
			"clickable", TRUE,
			"sort-column-id", 0,
			NULL);
	cell = gtk_cell_renderer_text_new ();
	gtk_cell_layout_pack_start (GTK_CELL_LAYOUT (column), cell, TRUE);
	gtk_cell_layout_set_attributes (GTK_CELL_LAYOUT (column), cell,
			"text", 0,
			NULL);
	gtk_tree_view_insert_column (GTK_TREE_VIEW (listview), column, -1);
*/
	
	column = gtk_tree_view_column_new ();
	g_object_set (G_OBJECT (column),
			"title", _("Name"),
			"clickable", TRUE,
			"expand", TRUE,
			"sizing", GTK_TREE_VIEW_COLUMN_FIXED,
			"sort-column-id", 1,
			"min-width", 80,
			NULL);
	cell = gtk_cell_renderer_text_new ();
	g_object_set (G_OBJECT (cell),
			"ellipsize", PANGO_ELLIPSIZE_END,
			NULL);
	gtk_cell_layout_pack_start (GTK_CELL_LAYOUT (column), cell, TRUE);
	gtk_cell_layout_set_attributes (GTK_CELL_LAYOUT (column), cell,
			"text", 1,
			NULL);
	gtk_tree_view_insert_column (GTK_TREE_VIEW (listview), column, -1);
	
	column = gtk_tree_view_column_new ();
	g_object_set (G_OBJECT (column),
			"title", _("Kills"),
			"clickable", TRUE,
			"sort-column-id", 2,
			"sort-indicator", FALSE,
			NULL);
	cell = gtk_cell_renderer_text_new ();
	gtk_cell_layout_pack_start (GTK_CELL_LAYOUT (column), cell, TRUE);
	gtk_cell_layout_set_attributes (GTK_CELL_LAYOUT (column), cell,
			"text", 2,
			NULL);
	gtk_tree_view_insert_column (GTK_TREE_VIEW (listview), column, -1);
	
	column = gtk_tree_view_column_new ();
	g_object_set (G_OBJECT (column),
			"title", _("Time"),
			"clickable", TRUE,
			"sort-column-id", 3,
			"sort-indicator", FALSE,
			NULL);
	cell = gtk_cell_renderer_text_new ();
	gtk_cell_layout_pack_start (GTK_CELL_LAYOUT (column), cell, TRUE);
	gtk_cell_layout_set_attributes (GTK_CELL_LAYOUT (column), cell,
			"text", 3,
			NULL);
	gtk_tree_view_insert_column (GTK_TREE_VIEW (listview), column, -1);
	
	GtkWidget *scrolled = gtk_scrolled_window_new (NULL, NULL);
	g_object_set (G_OBJECT (scrolled),
			"visible", TRUE,
			"shadow-type", GTK_SHADOW_IN,
			"hscrollbar-policy", GTK_POLICY_NEVER,
			NULL);
	gtk_container_add (GTK_CONTAINER (scrolled), listview);
	gtk_widget_show_all (scrolled);
	return scrolled;
}


static void
gui_plist_add_clicked (GtkButton *button, gpointer udata)
{
	GtkTreeModel *model;
	GtkTreeIter iter;
	gchar *name;
	
	GtkTreeSelection * selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (listview));
	if (!gtk_tree_selection_get_selected (selection, &model, &iter)) {
		gtk_widget_set_sensitive (add, FALSE);
		return;
	}
	
	gtk_tree_model_get (model, &iter, 1, &name, -1);
	if (!*name) {
		gtk_widget_set_sensitive (add, FALSE);
		g_free (name);
		return;
	}
	
	if (gs_find_buddy (name)) {
		gtk_widget_set_sensitive (add, FALSE);
		g_free (name);
		return;
	}
	
	GsBuddy *buddy = gs_add_buddy (name, 0, NULL, TRUE);
	gui_blist_add (buddy);
	g_free (name);
	gtk_widget_set_sensitive (add, FALSE);
}

GtkWidget *
gui_plist_create_bar ()
{
	add_image = gtk_image_new_from_stock (GTK_STOCK_ADD, GTK_ICON_SIZE_MENU);
	
	add = gtk_button_new ();
	g_object_set (G_OBJECT (add),
			"image", add_image,
			"tooltip-text", _("Add player to your buddy list"),
			"sensitive", FALSE,
			NULL);
	g_signal_connect (add, "clicked", G_CALLBACK (gui_plist_add_clicked), NULL);
	
	GtkWidget *bar = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 4);
	gtk_box_pack_start (GTK_BOX (bar), add, FALSE, TRUE, 0);
	gtk_widget_show_all (bar);
	return bar;
}