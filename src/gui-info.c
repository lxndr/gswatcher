/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*- */
/* 
 * gui-info.c
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


#include <GeoIP.h>
#include <glib/gprintf.h>
#include "gui-window.h"


static GtkWidget *address, *name, *game, *map, *numplayers, *password,
		*version, *location;


void
gui_info_update (GsClient *client)
{
	gchar tmp[16];
	
	if (client)
		g_snprintf (tmp, 16, "%d / %d",
				client->querier->numplayers, client->querier->maxplayers);
	else
		g_strlcpy (tmp, _("N/A"), 16);
	
	gtk_label_set_text (GTK_LABEL (address),
			client ? gsq_querier_get_address (client->querier) : _("N/A"));
	gtk_label_set_text (GTK_LABEL (name),
			client ? client->querier->name : _("N/A"));
	gtk_label_set_text (GTK_LABEL (game),
			client ? client->game : _("N/A"));
	gtk_label_set_text (GTK_LABEL (map),
			client ? client->querier->map : _("N/A"));
	gtk_label_set_text (GTK_LABEL (numplayers),
			tmp);
	gtk_label_set_text (GTK_LABEL (password),
			client ? (client->password ? _("Yes") : _("No")) : _("N/A"));
	gtk_label_set_text (GTK_LABEL (version),
			client ? client->version : _("N/A"));
	gtk_label_set_text (GTK_LABEL (location),
			client ? client->country : _("N/A"));
}


GtkWidget *
gui_info_create ()
{
	GtkWidget *label;
	
	GtkWidget *grid = gtk_grid_new ();
	g_object_set (G_OBJECT (grid),
			"column-spacing", 8,
			"row-spacing", 4,
			"border-width", 4,
			NULL);
	
	label = gtk_label_new (_("Name:"));
	g_object_set (G_OBJECT (label),
			"xalign", 0.0f,
			NULL);
	gtk_grid_attach (GTK_GRID (grid), label, 0, 0, 1, 1);
	
	name = gtk_label_new (NULL);
	g_object_set (G_OBJECT (name),
			"xalign", 0.0f,
			"hexpand", TRUE,
			NULL);
	gtk_grid_attach (GTK_GRID (grid), name, 1, 0, 1, 1);
	
	label = gtk_label_new (_("Address:"));
	g_object_set (G_OBJECT (label),
			"xalign", 0.0f,
			NULL);
	gtk_grid_attach (GTK_GRID (grid), label, 0, 1, 1, 1);
	
	address = gtk_label_new (NULL);
	g_object_set (G_OBJECT (address),
			"xalign", 0.0f,
			"hexpand", TRUE,
			NULL);
	gtk_grid_attach (GTK_GRID (grid), address, 1, 1, 1, 1);
	
	label = gtk_label_new (_("Game:"));
	g_object_set (G_OBJECT (label),
			"xalign", 0.0f,
			NULL);
	gtk_grid_attach (GTK_GRID (grid), label, 0, 2, 1, 1);
	
	game = gtk_label_new (NULL);
	g_object_set (G_OBJECT (game),
			"xalign", 0.0f,
			"hexpand", TRUE,
			NULL);
	gtk_grid_attach (GTK_GRID (grid), game, 1, 2, 1, 1);
	
	label = gtk_label_new (_("Map:"));
	g_object_set (G_OBJECT (label),
			"xalign", 0.0f,
			NULL);
	gtk_grid_attach (GTK_GRID (grid), label, 0, 3, 1, 1);
	
	map = gtk_label_new (NULL);
	g_object_set (G_OBJECT (map),
			"xalign", 0.0f,
			"hexpand", TRUE,
			NULL);
	gtk_grid_attach (GTK_GRID (grid), map, 1, 3, 1, 1);
	
	label = gtk_label_new (_("Players:"));
	g_object_set (G_OBJECT (label),
			"xalign", 0.0f,
			NULL);
	gtk_grid_attach (GTK_GRID (grid), label, 0, 4, 1, 1);
	
	numplayers = gtk_label_new (NULL);
	g_object_set (G_OBJECT (numplayers),
			"xalign", 0.0f,
			"hexpand", TRUE,
			NULL);
	gtk_grid_attach (GTK_GRID (grid), numplayers, 1, 4, 1, 1);
	
	label = gtk_label_new (_("Password:"));
	g_object_set (G_OBJECT (label),
			"xalign", 0.0f,
			NULL);
	gtk_grid_attach (GTK_GRID (grid), label, 0, 5, 1, 1);
	
	password = gtk_label_new (NULL);
	g_object_set (G_OBJECT (password),
			"xalign", 0.0f,
			"hexpand", TRUE,
			NULL);
	gtk_grid_attach (GTK_GRID (grid), password, 1, 5, 1, 1);
	
	label = gtk_label_new (_("Version:"));
	g_object_set (G_OBJECT (label),
			"xalign", 0.0f,
			NULL);
	gtk_grid_attach (GTK_GRID (grid), label, 0, 6, 1, 1);
	
	version = gtk_label_new (NULL);
	g_object_set (G_OBJECT (version),
			"xalign", 0.0f,
			"hexpand", TRUE,
			NULL);
	gtk_grid_attach (GTK_GRID (grid), version, 1, 6, 1, 1);
	
	label = gtk_label_new (_("Location:"));
	g_object_set (G_OBJECT (label),
			"xalign", 0.0f,
			NULL);
	gtk_grid_attach (GTK_GRID (grid), label, 0, 7, 1, 1);
	
	location = gtk_label_new (NULL);
	g_object_set (G_OBJECT (location),
			"xalign", 0.0f,
			"hexpand", TRUE,
			NULL);
	gtk_grid_attach (GTK_GRID (grid), location, 1, 7, 1, 1);
	
	gtk_widget_show_all (grid);
	return grid;
}
