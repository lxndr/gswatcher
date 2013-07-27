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



static GObject *grid, *ctl_address, *ctl_name, *ctl_game, *ctl_map,
		*ctl_players, *ctl_private, *ctl_version, *ctl_location, *ctl_message;
static GObject *toolbar, *ctl_remove, *ctl_favorite, *ctl_connect;


void gui_info_favorite_toggled (GtkToggleButton *button, gpointer udata);



void
gui_info_setup (GsClient *cl)
{
	if (cl) {
		/* enable and set up the toolbar */
		gtk_widget_set_sensitive (GTK_WIDGET (toolbar), TRUE);
		
		g_signal_handlers_block_by_func (ctl_favorite, gui_info_favorite_toggled, NULL);
		gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (ctl_favorite),
				gs_client_get_favorite (cl));
		g_signal_handlers_unblock_by_func (ctl_favorite, gui_info_favorite_toggled, NULL);
		
		/* enable and set up the information */
		gtk_widget_set_sensitive (GTK_WIDGET (grid), TRUE);
		
		gtk_label_set_text (GTK_LABEL (ctl_address), gs_client_get_address (cl));
		gtk_label_set_text (GTK_LABEL (ctl_location), cl->country);
		gui_info_update (cl);
		
		if (cl->error) {
			if (g_error_matches (cl->error, GSQ_QUERIER_ERROR, GSQ_QUERIER_ERROR_TIMEOUT))
				gtk_label_set_text (GTK_LABEL (ctl_message),
						_("This server is not responding and most likely offline."));
			else
				gtk_label_set_text (GTK_LABEL (ctl_message), cl->error->message);
			gtk_widget_set_visible (GTK_WIDGET (ctl_message), TRUE);
		} else {
			gtk_widget_set_visible (GTK_WIDGET (ctl_message), FALSE);
			gtk_label_set_text (GTK_LABEL (ctl_message), NULL);
		}
	} else {
		/* none is selected. disable the toolbar and clear information grid */
		gtk_widget_set_sensitive (GTK_WIDGET (toolbar), FALSE);
		gtk_widget_set_sensitive (GTK_WIDGET (grid), FALSE);
		gtk_widget_set_visible (GTK_WIDGET (ctl_message), FALSE);
		gtk_label_set_text (GTK_LABEL (ctl_message), NULL);
		
		gtk_label_set_text (GTK_LABEL (ctl_address), _("N/A"));
		gtk_label_set_text (GTK_LABEL (ctl_name), _("N/A"));
		gtk_label_set_text (GTK_LABEL (ctl_game), _("N/A"));
		gtk_label_set_text (GTK_LABEL (ctl_map), _("N/A"));
		gtk_label_set_text (GTK_LABEL (ctl_players), _("N/A"));
		gtk_label_set_text (GTK_LABEL (ctl_private), _("N/A"));
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
	
	gtk_widget_set_visible (GTK_WIDGET (ctl_message), FALSE);
	gtk_label_set_text (GTK_LABEL (ctl_name), client->querier->name->str);
	gtk_label_set_text (GTK_LABEL (ctl_game), *gamename ? gamename : _("N/A"));
	gtk_label_set_text (GTK_LABEL (ctl_map), *mapname ? mapname : _("N/A"));
	gtk_label_set_text (GTK_LABEL (ctl_players), players);
	gtk_label_set_text (GTK_LABEL (ctl_private), client->password ? _("Yes") : _("No"));
	gtk_label_set_text (GTK_LABEL (ctl_version), client->version);
	
	g_free (players);
	g_free (gamename);
}



GtkWidget *
gui_info_create ()
{
	GError *error = NULL;
	GtkBuilder *builder = gtk_builder_new ();
	if (gtk_builder_add_from_resource (builder, "/org/gswatcher/ui/info.ui", &error) == 0) {
		g_warning (error->message);
		g_object_unref (builder);
		g_error_free (error);
		return NULL;
	}
	
	grid         = gtk_builder_get_object (builder, "panel");
	ctl_name     = gtk_builder_get_object (builder, "name");
	ctl_address  = gtk_builder_get_object (builder, "address");
	ctl_game     = gtk_builder_get_object (builder, "game");
	ctl_map      = gtk_builder_get_object (builder, "map");
	ctl_players  = gtk_builder_get_object (builder, "players");
	ctl_private  = gtk_builder_get_object (builder, "private");
	ctl_version  = gtk_builder_get_object (builder, "version");
	ctl_location = gtk_builder_get_object (builder, "location");
	ctl_message  = gtk_builder_get_object (builder, "message");
	
	grid = g_object_ref (grid);
	g_object_unref (builder);
	return GTK_WIDGET (grid);
}


void G_MODULE_EXPORT
gui_info_remove_clicked (GtkButton *button, gpointer udata)
{
	GsClient *client = gui_slist_get_selected ();
	gs_application_remove_server_ask (app, client);
}

void G_MODULE_EXPORT
gui_info_favorite_toggled (GtkToggleButton *button, gpointer udata)
{
	GsClient *client = gui_slist_get_selected ();
	gboolean favorite = gtk_toggle_button_get_active (button);
	gs_client_set_favorite (client, favorite);
	gui_slist_update (client);
	gs_application_save_server_list (app);
}

void G_MODULE_EXPORT
gui_info_connect_clicked (GtkButton *button, gpointer udata)
{
	GsClient *client = gui_slist_get_selected ();
	gs_client_connect_to_game (client);
}


GtkWidget *
gui_info_create_bar ()
{
	GError *error = NULL;
	GtkBuilder *builder = gtk_builder_new ();
	if (gtk_builder_add_from_resource (builder, "/org/gswatcher/ui/info.ui", &error) == 0) {
		g_warning (error->message);
		g_object_unref (builder);
		g_error_free (error);
		return NULL;
	}
	
	toolbar      = gtk_builder_get_object (builder, "toolbar");
	ctl_remove   = gtk_builder_get_object (builder, "remove");
	ctl_favorite = gtk_builder_get_object (builder, "favorite");
	ctl_connect  = gtk_builder_get_object (builder, "connect");
	
	gtk_builder_connect_signals (builder, NULL);
	toolbar = g_object_ref (toolbar);
	
	GtkWidget *sep = gtk_separator_new (GTK_ORIENTATION_VERTICAL);
	GtkWidget *add = gui_plist_create_bar ();
	gtk_box_pack_start (GTK_BOX (toolbar), sep, FALSE, TRUE, 0);
	gtk_widget_show (sep);
	gtk_box_pack_start (GTK_BOX (toolbar), add, FALSE, TRUE, 0);
	gtk_widget_show (add);
	
	gui_info_setup (NULL);
	
	return GTK_WIDGET (toolbar);
}
