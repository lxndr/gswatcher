/* 
 * gui-player-list.c
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
#include "gui-buddy-list.h"
#include "gui-player-list.h"
#include "gui-server-list.h"


static GtkWidget *listview, *add, *add_image;


void
gui_plist_update (GsClient *client)
{
	GtkTreeSelection *selection;
	GtkTreeModel *model;
	GtkListStore *liststore;
	GtkTreeIter iter;
	gchar *selected = NULL;
	GList *iplayer;
	
	// store selected player
	selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (listview));
	if (gtk_tree_selection_get_selected (selection, &model, &iter))
		gtk_tree_model_get (model, &iter, 0, &selected, -1);
	liststore = GTK_LIST_STORE (model);
	gtk_list_store_clear (liststore);
	
	if (!client)
		goto finish;
	
	iplayer = gsq_querier_get_players (client->querier);
	while (iplayer) {
		GsqPlayer *player = iplayer->data;
		
		gtk_list_store_append (liststore, &iter);
		gtk_list_store_set (liststore, &iter,
				0, player->name,
				1, player->kills,
				2, player->time,
				-1);
		
		// restore selected player
		if (selected && strcmp (selected, player->name) == 0)
			gtk_tree_selection_select_iter (selection, &iter);
		iplayer = iplayer->next;
	}
	
finish:
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
	
	gtk_tree_model_get (model, &iter, 0, &name, -1);
	if (!*name) {
		gtk_widget_set_sensitive (add, FALSE);
		goto finish;
	}
	
	if (gs_find_buddy (name)) {
		gtk_widget_set_sensitive (add, FALSE);
		goto finish;
	}
	
	gtk_widget_set_sensitive (add, TRUE);

finish:
	g_free (name);
}


GtkWidget *
gui_plist_create ()
{
	GtkTreeViewColumn *column;
	GtkCellRenderer *cell;
	
	GtkListStore *model = gtk_list_store_new (3, G_TYPE_STRING, G_TYPE_INT,
			G_TYPE_STRING);
	listview = gtk_tree_view_new_with_model (GTK_TREE_MODEL (model));
	g_object_unref (model);
	
	GtkTreeSelection *sel = gtk_tree_view_get_selection (GTK_TREE_VIEW (listview));
	g_signal_connect (sel,
			"changed", G_CALLBACK (gui_plist_selection_changed), NULL);
	
	column = gtk_tree_view_column_new ();
	g_object_set (G_OBJECT (column),
			"title", _("Name"),
			"clickable", TRUE,
			"expand", TRUE,
			"sizing", GTK_TREE_VIEW_COLUMN_FIXED,
			"sort-column-id", 0,
			"min-width", 80,
			NULL);
	cell = gtk_cell_renderer_text_new ();
	g_object_set (G_OBJECT (cell),
			"ellipsize", PANGO_ELLIPSIZE_END,
			NULL);
	gtk_cell_layout_pack_start (GTK_CELL_LAYOUT (column), cell, TRUE);
	gtk_cell_layout_set_attributes (GTK_CELL_LAYOUT (column), cell,
			"text", 0,
			NULL);
	gtk_tree_view_append_column (GTK_TREE_VIEW (listview), column);
	
	column = gtk_tree_view_column_new ();
	g_object_set (G_OBJECT (column),
			"title", _("Kills"),
			"clickable", TRUE,
			"sort-column-id", 1,
			"sort-indicator", FALSE,
			NULL);
	cell = gtk_cell_renderer_text_new ();
	gtk_cell_layout_pack_start (GTK_CELL_LAYOUT (column), cell, TRUE);
	gtk_cell_layout_set_attributes (GTK_CELL_LAYOUT (column), cell,
			"text", 1,
			NULL);
	gtk_tree_view_append_column (GTK_TREE_VIEW (listview), column);
	
	column = gtk_tree_view_column_new ();
	g_object_set (G_OBJECT (column),
			"title", _("Time"),
			"clickable", TRUE,
			"sort-column-id", 2,
			"sort-indicator", FALSE,
			NULL);
	cell = gtk_cell_renderer_text_new ();
	gtk_cell_layout_pack_start (GTK_CELL_LAYOUT (column), cell, TRUE);
	gtk_cell_layout_set_attributes (GTK_CELL_LAYOUT (column), cell,
			"text", 2,
			NULL);
	gtk_tree_view_append_column (GTK_TREE_VIEW (listview), column);
	
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
	
	gtk_tree_model_get (model, &iter, 0, &name, -1);
	if (name && !*name) {
		gtk_widget_set_sensitive (add, FALSE);
		g_free (name);
		return;
	}
	
	if (gs_find_buddy (name)) {
		gtk_widget_set_sensitive (add, FALSE);
		g_free (name);
		return;
	}
	
	GsClient *srv = gui_slist_get_selected ();
	
	GsBuddy *buddy = gs_add_buddy (name, time (NULL),
			gsq_querier_get_address (srv->querier), TRUE);
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
