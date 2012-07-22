/* 
 * gui-info.c
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



#include <GeoIP.h>
#include <glib/gprintf.h>
#include "gui-window.h"
#include "gui-server-list.h"
#include "gui-player-list.h"
#include "gui-info.h"



static GtkWidget *grid, *ctl_address, *ctl_name, *ctl_game, *ctl_map,
		*ctl_players, *ctl_password, *ctl_version, *ctl_location, *ctl_message;
static GtkWidget *toolbar, *ctl_remove, *ctl_favorite, *ctl_connect;


static void favorite_toggled (GtkToggleButton *button, gpointer udata);



void
gui_info_setup (GsClient *cl)
{
	if (cl) {
		/* enable and set up the toolbar */
		gtk_widget_set_sensitive (toolbar, TRUE);
		
		g_signal_handlers_block_by_func (ctl_favorite, favorite_toggled, NULL);
		gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (ctl_favorite),
				gs_client_get_favorite (cl));
		g_signal_handlers_unblock_by_func (ctl_favorite, favorite_toggled, NULL);
		
		/* enable and set up the information */
		gtk_widget_set_sensitive (grid, TRUE);
		
		gtk_label_set_text (GTK_LABEL (ctl_address), gsq_querier_get_address (cl->querier));
		gtk_label_set_text (GTK_LABEL (ctl_location), cl->country);
		gui_info_update (cl);
		
		gtk_widget_set_visible (ctl_message, !gsq_watcher_is_online (GSQ_WATCHER (cl->querier)));
	} else {
		/* none is selected. disable the toolbar and clear information grid */
		gtk_widget_set_sensitive (toolbar, FALSE);
		gtk_widget_set_sensitive (grid, FALSE);
		gtk_widget_set_visible (ctl_message, FALSE);
		
		gtk_label_set_text (GTK_LABEL (ctl_address), _("N/A"));
		gtk_label_set_text (GTK_LABEL (ctl_name), _("N/A"));
		gtk_label_set_text (GTK_LABEL (ctl_game), _("N/A"));
		gtk_label_set_text (GTK_LABEL (ctl_map), _("N/A"));
		gtk_label_set_text (GTK_LABEL (ctl_players), _("N/A"));
		gtk_label_set_text (GTK_LABEL (ctl_password), _("N/A"));
		gtk_label_set_text (GTK_LABEL (ctl_version), _("N/A"));
		gtk_label_set_text (GTK_LABEL (ctl_location), _("N/A"));
	}
}



void
gui_info_update (GsClient *client)
{
	gchar *players = g_strdup_printf ("%d / %d",
			client->querier->numplayers,
			client->querier->maxplayers);
	
	gchar *gamename = gs_client_get_game_name (client, TRUE);
	
	const gchar *mapname = client->querier->map->str;
	
	gtk_widget_set_visible (ctl_message, FALSE);
	gtk_label_set_text (GTK_LABEL (ctl_name), client->querier->name->str);
	gtk_label_set_text (GTK_LABEL (ctl_game), *gamename ? gamename : _("N/A"));
	gtk_label_set_text (GTK_LABEL (ctl_map), *mapname ? mapname : _("N/A"));
	gtk_label_set_text (GTK_LABEL (ctl_players), players);
	gtk_label_set_text (GTK_LABEL (ctl_password), client->password ? _("Yes") : _("No"));
	gtk_label_set_text (GTK_LABEL (ctl_version), client->version);
	
	g_free (players);
	g_free (gamename);
}



GtkWidget *
gui_info_create ()
{
	GtkWidget *label;
	
	grid = gtk_grid_new ();
	g_object_set (G_OBJECT (grid),
			"column-spacing", 8,
			"row-spacing", 4,
			"border-width", 4,
			"sensitive", FALSE,
			NULL);
	
	label = gtk_label_new (_("Name:"));
	g_object_set (G_OBJECT (label),
			"xalign", 0.0f,
			"yalign", 0.0f,
			NULL);
	gtk_grid_attach (GTK_GRID (grid), label, 0, 0, 1, 1);
	
	ctl_name = gtk_label_new (NULL);
	g_object_set (G_OBJECT (ctl_name),
			"xalign", 0.0f,
			"yalign", 0.0f,
			"hexpand", TRUE,
			"wrap", TRUE,
			NULL);
	gtk_grid_attach (GTK_GRID (grid), ctl_name, 1, 0, 1, 1);
	
	label = gtk_label_new (_("Address:"));
	g_object_set (G_OBJECT (label),
			"xalign", 0.0f,
			NULL);
	gtk_grid_attach (GTK_GRID (grid), label, 0, 1, 1, 1);
	
	ctl_address = gtk_label_new (NULL);
	g_object_set (G_OBJECT (ctl_address),
			"xalign", 0.0f,
			"hexpand", TRUE,
			NULL);
	gtk_grid_attach (GTK_GRID (grid), ctl_address, 1, 1, 1, 1);
	
	label = gtk_label_new (_("Game:"));
	g_object_set (G_OBJECT (label),
			"xalign", 0.0f,
			NULL);
	gtk_grid_attach (GTK_GRID (grid), label, 0, 2, 1, 1);
	
	ctl_game = gtk_label_new (NULL);
	g_object_set (G_OBJECT (ctl_game),
			"xalign", 0.0f,
			"hexpand", TRUE,
			NULL);
	gtk_grid_attach (GTK_GRID (grid), ctl_game, 1, 2, 1, 1);
	
	label = gtk_label_new (_("Map:"));
	g_object_set (G_OBJECT (label),
			"xalign", 0.0f,
			NULL);
	gtk_grid_attach (GTK_GRID (grid), label, 0, 3, 1, 1);
	
	ctl_map = gtk_label_new (NULL);
	g_object_set (G_OBJECT (ctl_map),
			"xalign", 0.0f,
			"hexpand", TRUE,
			NULL);
	gtk_grid_attach (GTK_GRID (grid), ctl_map, 1, 3, 1, 1);
	
	label = gtk_label_new (_("Players:"));
	g_object_set (G_OBJECT (label),
			"xalign", 0.0f,
			NULL);
	gtk_grid_attach (GTK_GRID (grid), label, 0, 4, 1, 1);
	
	ctl_players = gtk_label_new (NULL);
	g_object_set (G_OBJECT (ctl_players),
			"xalign", 0.0f,
			"hexpand", TRUE,
			NULL);
	gtk_grid_attach (GTK_GRID (grid), ctl_players, 1, 4, 1, 1);
	
	label = gtk_label_new (_("Password:"));
	g_object_set (G_OBJECT (label),
			"xalign", 0.0f,
			NULL);
	gtk_grid_attach (GTK_GRID (grid), label, 0, 5, 1, 1);
	
	ctl_password = gtk_label_new (NULL);
	g_object_set (G_OBJECT (ctl_password),
			"xalign", 0.0f,
			"hexpand", TRUE,
			NULL);
	gtk_grid_attach (GTK_GRID (grid), ctl_password, 1, 5, 1, 1);
	
	label = gtk_label_new (_("Version:"));
	g_object_set (G_OBJECT (label),
			"xalign", 0.0f,
			NULL);
	gtk_grid_attach (GTK_GRID (grid), label, 0, 6, 1, 1);
	
	ctl_version = gtk_label_new (NULL);
	g_object_set (G_OBJECT (ctl_version),
			"xalign", 0.0f,
			"hexpand", TRUE,
			NULL);
	gtk_grid_attach (GTK_GRID (grid), ctl_version, 1, 6, 1, 1);
	
	label = gtk_label_new (_("Location:"));
	g_object_set (G_OBJECT (label),
			"xalign", 0.0f,
			NULL);
	gtk_grid_attach (GTK_GRID (grid), label, 0, 7, 1, 1);
	
	ctl_location = gtk_label_new (NULL);
	g_object_set (G_OBJECT (ctl_location),
			"xalign", 0.0f,
			"hexpand", TRUE,
			NULL);
	gtk_grid_attach (GTK_GRID (grid), ctl_location, 1, 7, 1, 1);
	
	gtk_widget_show_all (grid);
	
	ctl_message = gtk_label_new (_("<span color=\"red\">This server is not responding and most likely offline.</span>"));
	g_object_set (G_OBJECT (ctl_message),
			"visible", FALSE,
			"use-markup", TRUE,
			"hexpand", TRUE,
			NULL);
	gtk_grid_attach (GTK_GRID (grid), ctl_message, 1, 8, 2, 1);
	
	return grid;
}


static void
remove_clicked (GtkButton *button, gpointer udata)
{
	GsClient *client = gui_slist_get_selected ();
	gs_application_remove_server_ask (app, client);
}

static void
favorite_toggled (GtkToggleButton *button, gpointer udata)
{
	GsClient *client = gui_slist_get_selected ();
	gboolean favorite = gtk_toggle_button_get_active (button);
	gs_client_set_favorite (client, favorite);
	gui_slist_update (client);
	gs_application_save_server_list (app);
}

static void
connect_clicked (GtkButton *button, gpointer udata)
{
	GsClient *client = gui_slist_get_selected ();
	gs_client_connect_to_game (client);
}


GtkWidget *
gui_info_create_bar ()
{
	GtkWidget *image;
	
	image = gtk_image_new_from_stock (GTK_STOCK_REMOVE, GTK_ICON_SIZE_MENU);
	ctl_remove = gtk_button_new ();
	g_object_set (G_OBJECT (ctl_remove),
			"tooltip-text", _("Remove selected server"),
			"image", image,
			NULL);
	g_signal_connect (ctl_remove, "clicked", G_CALLBACK (remove_clicked), NULL);
	
	image = gtk_image_new_from_icon_name ("emblem-favorite", GTK_ICON_SIZE_MENU);
	ctl_favorite = gtk_toggle_button_new ();
	g_object_set (G_OBJECT (ctl_favorite),
			"tooltip-text", _("Favor selected server"),
			"image", image,
			NULL);
	g_signal_connect (ctl_favorite, "toggled", G_CALLBACK (favorite_toggled), NULL);
	
	image = gtk_image_new_from_stock (GTK_STOCK_MEDIA_PLAY, GTK_ICON_SIZE_MENU);
	ctl_connect = gtk_button_new_with_label (_("Connect"));
	g_object_set (G_OBJECT (ctl_connect),
			"tooltip-text", _("Connect to selected server"),
			"image", image,
			NULL);
	g_signal_connect (ctl_connect, "clicked", G_CALLBACK (connect_clicked), NULL);
	
	GtkWidget *sep = gtk_separator_new (GTK_ORIENTATION_VERTICAL);
	
	GtkWidget *add = gui_plist_create_bar ();
	
	toolbar = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 4);
	g_object_set (G_OBJECT (toolbar),
			"sensitive", FALSE,
			NULL);
	gtk_box_pack_start (GTK_BOX (toolbar), ctl_remove, FALSE, TRUE, 0);
	gtk_box_pack_start (GTK_BOX (toolbar), ctl_favorite, FALSE, TRUE, 0);
	gtk_box_pack_start (GTK_BOX (toolbar), ctl_connect, FALSE, TRUE, 0);
	gtk_box_pack_start (GTK_BOX (toolbar), sep, FALSE, TRUE, 0);
	gtk_box_pack_start (GTK_BOX (toolbar), add, FALSE, TRUE, 0);
	gtk_widget_show_all (toolbar);
	return toolbar;
}
