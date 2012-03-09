/* 
 * gstool.c
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



#include <stdlib.h>
#include <glib/gi18n-lib.h>
#include <glib/gprintf.h>
#include "json.h"
#include "gui-window.h"
#include "gstool.h"
#include "platform.h"
#include "gui-server-list.h"
#include "gui-console.h"
#include "gui-buddy-list.h"
#include "gui-preferences.h"
#include "gui-notification.h"


enum {
	PROP_0,
	PROP_UPDATE_RATE,
	PROP_PAUSE
};


GsApplication *app = NULL;


static GObject* gs_application_constructor (GType type, guint n_construct_params,
		GObjectConstructParam *construct_params);
static void gs_application_set_property (GObject *object, guint prop_id,
		const GValue *value, GParamSpec *pspec);
static void gs_application_get_property (GObject *object, guint prop_id,
		GValue *value, GParamSpec *pspec);
static void gs_application_finalize (GObject *object);
static void gs_application_activate (GApplication *app);
static gboolean gs_application_local_command_line (GApplication *app,
		gchar ***argumnets, int *exit_status);
static int gs_application_command_line (GApplication *app,
		GApplicationCommandLine *cmdline);
static void gs_application_startup (GApplication *app);

static void load_server_list (GsApplication *app);
static void load_buddy_list (GsApplication *app);
static void load_preferences (GsApplication *app);
static void add_servers (GsApplication *app, gchar **servers);
static void remove_server (GsApplication *app, GsClient *client);
static void free_buddy (GsBuddy *buddy);


G_DEFINE_TYPE (GsApplication, gs_application, GTK_TYPE_APPLICATION);


static void
gs_application_class_init (GsApplicationClass *class)
{
	GObjectClass *object_class = G_OBJECT_CLASS (class);
	object_class->constructor = gs_application_constructor;
	object_class->set_property = gs_application_set_property;
	object_class->get_property = gs_application_get_property;
	object_class->finalize = gs_application_finalize;
	
	GApplicationClass *app_class = G_APPLICATION_CLASS (object_class);
	app_class->startup = gs_application_startup;
	app_class->activate = gs_application_activate;
	app_class->local_command_line = gs_application_local_command_line;
	app_class->command_line = gs_application_command_line;
	
	g_object_class_install_property (object_class, PROP_UPDATE_RATE,
			g_param_spec_double ("update-rate", "Update rate", "Update rate",
			0.5, 30.0, 2.5, G_PARAM_READABLE | G_PARAM_WRITABLE | G_PARAM_CONSTRUCT));
	
	g_object_class_install_property (object_class, PROP_PAUSE,
			g_param_spec_boolean ("pause", "Pause", "Update pause",
			FALSE, G_PARAM_READABLE | G_PARAM_WRITABLE | G_PARAM_CONSTRUCT));
}


static void
gs_application_init (GsApplication *app)
{
	app->buddy_list = g_hash_table_new_full (g_str_hash, g_str_equal, NULL,
			(GDestroyNotify) free_buddy);
}


static GObject*
gs_application_constructor (GType type, guint n_construct_params,
		GObjectConstructParam *construct_params)
{
	static GObject *singleton = NULL;
	
	if (singleton)
		return singleton;
	
	singleton = G_OBJECT_CLASS (gs_application_parent_class)->constructor (
			type, n_construct_params, construct_params);
	return singleton;
}


static void
gs_application_finalize (GObject *object)
{
	GsApplication *app = GS_APPLICATION (object);
	g_hash_table_destroy (app->buddy_list);
	
	g_free (app->buddy_list_path);
	g_free (app->server_list_path);
	g_free (app->preferences_path);
	g_free (app->icon_dir);
	g_free (app->pixmap_dir);
	g_free (app->sound_dir);
#ifdef ENABLE_NLS
	g_free (app->locale_dir);
#endif
	
	if (app->specific_servers)
		g_strfreev (app->specific_servers);
	
	G_OBJECT_CLASS (gs_application_parent_class)->finalize (object);
}


static void
gs_application_set_property (GObject *object, guint prop_id, const GValue *value,
		GParamSpec *pspec)
{
	GsApplication *app = GS_APPLICATION (object);
	
	switch (prop_id) {
	case PROP_UPDATE_RATE:
		app->update_rate = g_value_get_double (value);
		break;
	case PROP_PAUSE:
		app->pause = g_value_get_boolean (value);
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
		break;
	}
}

static void
gs_application_get_property (GObject *object, guint prop_id, GValue *value,
		GParamSpec *pspec)
{
	GsApplication *app = GS_APPLICATION (object);
	
	switch (prop_id) {
	case PROP_UPDATE_RATE:
		g_value_set_double (value, app->update_rate);
		break;
	case PROP_PAUSE:
		g_value_set_boolean (value, app->pause);
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
		break;
    }
}


GsApplication *
gs_application_new ()
{
	return g_object_new (GS_TYPE_APPLICATION, "application-id", "org.gstool",
			"flags", G_APPLICATION_HANDLES_COMMAND_LINE, NULL);
}


static void
gs_application_activate (GApplication *app)
{
	if (G_APPLICATION_CLASS (gs_application_parent_class)->activate)
		G_APPLICATION_CLASS (gs_application_parent_class)->activate (app);
}


static void
initialize_paths (GsApplication *app, const gchar *exe)
{
	GFile *fexe = g_file_new_for_commandline_arg (exe);
	gchar *exepath = g_file_get_path (fexe);
	gchar *bin_dir = g_path_get_dirname (exepath);
	gchar *root_dir = g_path_get_dirname (bin_dir);
	g_free (bin_dir);
	g_free (exepath);
	g_object_unref (fexe);
	
	gchar *config_dir = g_build_filename (g_get_user_config_dir (), "gstool", NULL);
	g_mkdir_with_parents (config_dir, 0750);
	
	app->buddy_list_path = g_build_filename (config_dir, "buddylist.json", NULL);
	app->server_list_path = g_build_filename (config_dir, "serverlist.json", NULL);
	app->preferences_path = g_build_filename (config_dir, "preferences.json", NULL);
	
	g_free (config_dir);
	
#ifdef GS_DATADIR
	gchar *data_dir = GS_DATADIR;
#else
	gchar *data_dir = g_build_filename (root_dir, "share", "gstool", NULL);
#endif
	
	app->icon_dir = g_build_filename (data_dir, "icons", NULL);
	app->sound_dir = g_build_filename (data_dir, "sounds", NULL);
	app->pixmap_dir = g_build_filename (root_dir, "share", "pixmaps", NULL);
	
#ifdef ENABLE_NLS
#ifdef GS_LOCALEDIR
	app->locale_dir = GS_LOCALEDIR;
#else
	app->locale_dir = g_build_filename (root_dir, "share", "locale", NULL);
#endif
#endif
	
#ifndef GS_DATADIR
	g_free (data_dir);
#endif
	g_free (root_dir);
}


static gboolean
gs_application_local_command_line (GApplication *app, gchar ***argumnets,
		int *exit_status)
{
	initialize_paths (GS_APPLICATION (app), **argumnets);
	return G_APPLICATION_CLASS (gs_application_parent_class)->
			local_command_line (app, argumnets, exit_status);
}

static int
gs_application_command_line (GApplication *app, GApplicationCommandLine *cmdline)
{
	gboolean version = FALSE;
	gboolean minimized = FALSE;
	gboolean debug = FALSE;
	gboolean notifier = FALSE;
	
	gchar **argv;
	gint argc;
	GError *error = NULL;
	GOptionContext *context;
	GOptionEntry options[] = {
		{"version", 'v', 0, G_OPTION_ARG_NONE, &version,
				N_("Show version number and exit"), NULL},
		{"minimized", 'm', 0, G_OPTION_ARG_NONE, &minimized,
				N_("Start minimized in system tray"), NULL},
		{"debug", 'd', 0, G_OPTION_ARG_NONE, &debug,
				N_("Output all packets into stdout"), NULL},
		{"server", 's', 0, G_OPTION_ARG_STRING_ARRAY,
				&GS_APPLICATION (app)->specific_servers,
				N_("Watch only this server"), NULL},
#ifndef G_OS_WIN32
		{"notifier", 'n', 0, G_OPTION_ARG_NONE, &notifier,
				N_("Use internal notifier"), NULL},
#endif
		{NULL}
	};
	
	argv = g_application_command_line_get_arguments (cmdline, &argc);
	
	context = g_option_context_new (NULL);
	/* g_option_context_set_help_enabled (context, FALSE); */
	g_option_context_add_group (context, gtk_get_option_group (TRUE));
	g_option_context_add_main_entries (context, options, NULL);
	
	if (!g_option_context_parse (context, &argc, &argv, &error)) {
		g_application_command_line_printerr (cmdline, "%s\n", error->message);
		g_error_free (error);
		g_application_command_line_set_exit_status (cmdline, EXIT_FAILURE);
		goto finish;
	}
	
	if (version) {
		g_application_command_line_print (cmdline, "Game Server Tool v" GS_VERSION "\n");
		g_application_command_line_set_exit_status (cmdline, EXIT_SUCCESS);
		goto finish;
	}
	
	gs_notification_init (notifier);
	gsq_querier_set_debug_mode (debug);
	
	if (!minimized)
		gui_window_show ();
	
finish:
	g_strfreev (argv);
	g_option_context_free (context);
	
	return 0;
}

static void
gs_application_startup (GApplication *app)
{
	GsApplication *gsapp = GS_APPLICATION (app);
	if (G_APPLICATION_CLASS (gs_application_parent_class)->startup)
		G_APPLICATION_CLASS (gs_application_parent_class)->startup (app);
	
#ifdef ENABLE_NLS
	bindtextdomain (GETTEXT_PACKAGE, gsapp->locale_dir);
	bind_textdomain_codeset (GETTEXT_PACKAGE, "UTF-8");
	textdomain (GETTEXT_PACKAGE);
#endif
	
	g_set_application_name ("Game Server Tool");
	gtk_icon_theme_append_search_path (gtk_icon_theme_get_default (),
			gsapp->pixmap_dir);
	
	GtkWidget *window = gui_window_create ();
	gtk_application_add_window (GTK_APPLICATION (app), GTK_WINDOW (window));
	
	load_preferences (gsapp);
	load_buddy_list (gsapp);
	
	if (gsapp->specific_servers)
		add_servers (GS_APPLICATION (app), gsapp->specific_servers);
	else
		load_server_list (gsapp);
}


static gboolean
gs_application_shutdown_shot (GsApplication *app)
{
	while (app->server_list)
		remove_server (app, app->server_list->data);
	if (app->timer)
		g_source_remove (app->timer);
	gui_window_destroy ();
	
	return FALSE;
}

void
gs_application_shutdown (GsApplication *app)
{
	g_idle_add ((GSourceFunc) gs_application_shutdown_shot, app);
}



/* buddy list */

static void
save_buddy_list_func (gchar *name, GsBuddy *buddy, GJsonNode *root)
{
	GJsonNode *node = g_json_object_new ();
	g_json_object_set_integer (node, "lastseen",
			g_date_time_to_unix (buddy->lastseen));
	g_json_object_set_string (node, "lastaddr", buddy->lastaddr);
	g_json_object_set_boolean (node, "notify", buddy->notify);
	g_json_object_set (root, buddy->name, node);
}

static void
save_buddy_list (GsApplication *app)
{
	GJsonNode *root = g_json_object_new ();
	g_hash_table_foreach (app->buddy_list, (GHFunc) save_buddy_list_func, root);
	
	GError *error = NULL;
	if (!g_json_write_to_file (root, app->buddy_list_path, &error)) {
		g_warning (_("Couldn't save buddy list: %s"), error->message);
		g_error_free (error);
	}
	g_json_node_free (root);
}


GsBuddy *
gs_application_find_buddy (GsApplication *app, const gchar *name)
{
	g_return_val_if_fail (GS_IS_APPLICATION (app), NULL);
	g_return_val_if_fail (name != NULL, NULL);
	return g_hash_table_lookup (app->buddy_list, name);
}


static void
update_buddy (GsApplication *app, GsBuddy *buddy)
{
	g_list_free (buddy->servers);
	buddy->servers = NULL;
	
	GList *isrv = app->server_list;
	while (isrv) {
		GsClient *client = isrv->data;
		if (gsq_querier_find_player (client->querier, buddy->name))
			buddy->servers = g_list_append (buddy->servers, client);
		isrv = isrv->next;
	}
}


static void
free_buddy (GsBuddy *buddy)
{
	g_free (buddy->name);
	g_date_time_unref (buddy->lastseen);
	if (buddy->lastaddr)
		g_free (buddy->lastaddr);
	g_list_free (buddy->servers);
	g_slice_free (GsBuddy, buddy);
}


void
gs_application_remove_buddy (GsApplication *app, const gchar *name)
{
	g_return_if_fail (GS_IS_APPLICATION (app));
	g_return_if_fail (name && *name);
	
	g_hash_table_remove (app->buddy_list, name);
	save_buddy_list (app);
}


void
gs_application_change_buddy (GsApplication *app, const gchar *name,
		gboolean notify)
{
	g_return_if_fail (GS_IS_APPLICATION (app));
	g_return_if_fail (name && *name);
	
	GsBuddy *buddy = gs_application_find_buddy (app, name);
	buddy->notify = notify;
	save_buddy_list (app);
}


static GsBuddy *
add_buddy_real (GsApplication *app, const gchar *name, gint64 lastseen,
		const gchar *lastaddr, gboolean notify)
{
	GsBuddy *buddy = g_slice_new0 (GsBuddy);
	buddy->name = g_strdup (name);
	buddy->lastseen = g_date_time_new_from_unix_local (lastseen);
	buddy->lastaddr = lastaddr ? g_strdup (lastaddr) : NULL;
	buddy->notify = notify;
	update_buddy (app, buddy);
	g_hash_table_insert (app->buddy_list, buddy->name, buddy);
	return buddy;
}


GsBuddy *
gs_application_add_buddy (GsApplication *app, const gchar *name, gint64 lastseen,
		const gchar *lastaddr, gboolean notify)
{
	g_return_val_if_fail (GS_IS_APPLICATION (app), NULL);
	g_return_val_if_fail (name && *name, NULL);
	
	GsBuddy *buddy = add_buddy_real (app, name, lastseen, lastaddr, notify);
	save_buddy_list (app);
	return buddy;
}


static void
load_buddy_list (GsApplication *app)
{
	GJsonNode *root, *node;
	GError *error = NULL;
	if (!(root = g_json_read_from_file (app->buddy_list_path, &error))) {
		g_warning (_("Couldn't load buddy list: %s"), error->message);
		g_error_free (error);
		return;
	}
	
	GJsonIter iter;
	gchar *name;
	g_json_iter_init (&iter, root);
	while (g_json_iter_next_object (&iter, &name, &node)) {
		gint64 lastseen = g_json_object_get_integer (node, "lastseen");
		const gchar *lastaddr = g_json_object_get_string (node, "lastaddr");
		gboolean notify = g_json_object_get_boolean (node, "notify");
		GsBuddy *buddy = add_buddy_real (app, name, lastseen, lastaddr, notify);
		gui_blist_add (buddy);
	}
	g_json_node_free (root);
}



/* server list */

static void
save_server_list_func (GsClient *client, GJsonNode *root)
{
	GJsonNode *node = g_json_object_new ();
	g_json_object_set_string (node, "name", client->querier->name);
	g_json_object_set_string (node, "address",
			gsq_querier_get_address (client->querier));
	g_json_object_set_boolean (node, "favorite", client->favorite);
	g_json_object_set_string (node, "rcon-password",
			gsq_console_get_password (client->console));
	g_json_array_add (root, node);
}

void
gs_application_save_server_list (GsApplication *app)
{
	/* do not save server list if the user specified servers */
	if (app->specific_servers)
		return;
	
	GJsonNode *root = g_json_array_new ();
	g_list_foreach (app->server_list, (GFunc) save_server_list_func, root);
	
	GError *error = NULL;
	if (!g_json_write_to_file (root, app->server_list_path, &error)) {
		g_warning (_("Couldn't save server list: %s"), error->message);
		g_error_free (error);
	}
	g_json_node_free (root);
}


static gboolean
updater_func (GsApplication *app)
{
	if (app->pause || !app->server_list)
		return TRUE;
	
	if (app->server_iter == NULL)
		app->server_iter = app->server_list;
	
	gsq_querier_update (((GsClient *) app->server_iter->data)->querier);
	app->server_iter = g_list_next (app->server_iter);
	
	return TRUE;
}

static void
update_timer (GsApplication *app)
{
	if (app->timer) {
		g_source_remove (app->timer);
		app->timer = 0;
	}
	
	guint count = g_list_length (app->server_list);
	if (count > 0) {
		glong update_rate = (glong) (app->update_rate * 1000);
		app->timer = g_timeout_add (update_rate / count,
				(GSourceFunc) updater_func, app);
	}
}


void
gs_application_set_update_rate (GsApplication *app, gdouble rate)
{
	g_return_if_fail (GS_IS_APPLICATION (app));
	
	rate = CLAMP (rate, 0.5, 30.0);
	app->update_rate = rate;
	update_timer (app);
}

gdouble
gs_application_get_update_rate (GsApplication *app)
{
	g_return_val_if_fail (GS_IS_APPLICATION (app), 0.0);
	return app->update_rate;
}


void
gs_application_set_pause (GsApplication *app, gboolean pause)
{
	g_return_if_fail (GS_IS_APPLICATION (app));
	app->pause = pause;
}

gboolean
gs_application_get_pause (GsApplication *app)
{
	g_return_val_if_fail (GS_IS_APPLICATION (app), FALSE);
	return app->pause;
}


static void
player_online (GsqQuerier *querier, GsqPlayer *player, GsApplication *app)
{
	GsBuddy *buddy = gs_application_find_buddy (app, player->name);
	
	if (buddy) {
		update_buddy (app, buddy);
		g_date_time_unref (buddy->lastseen);
		buddy->lastseen = g_date_time_new_now_local ();
		if (buddy->lastaddr)
			g_free (buddy->lastaddr);
		buddy->lastaddr = g_strdup (gsq_querier_get_address (querier));
		gui_blist_update (buddy);
		save_buddy_list (app);
		
		if (buddy->notify) {
			gchar *title = g_strdup_printf (_("Player %s has connected"), buddy->name);
			gchar *text = g_strdup_printf (_("Address: %s\nServer: %s\nPlayers: %d / %d"),
					gsq_querier_get_address (querier), querier->name,
					querier->numplayers, querier->maxplayers);
			gs_notification_message (title, text);
			g_free (title);
			g_free (text);
		}
		
	}
}

static void
player_offline (GsqQuerier *querier, GsqPlayer *player, GsApplication *app)
{
	GsBuddy *buddy = gs_application_find_buddy (app, player->name);
	if (buddy) {
		update_buddy (app, buddy);
		gui_blist_update (buddy);
		return;
	}
	
	GList *l = gsq_querier_get_players (querier);
	while (l) {
		GsqPlayer *pl = l->data;
		buddy = gs_application_find_buddy (app, pl->name);
		
		if (buddy && buddy->notify && querier->maxplayers - querier->numplayers == 1) {
			gchar *title = g_strdup_printf (_("Server with %s has free slot"), buddy->name);
			gchar *text = g_strdup_printf (_("Address: %s\nServer: %s\nPlayers: %d / %d"),
					gsq_querier_get_address (querier), querier->name,
					querier->numplayers, querier->maxplayers);
			gs_notification_message (title, text);
			g_free (title);
			g_free (text);
		}
		l = l->next;
	}
}


static GsClient *
add_server (GsApplication *app, const gchar *address, const gchar *name,
		gboolean favorite, const gchar *rcon_password)
{
	if (gs_application_find_server (app, address))
		return NULL;
	
	GsClient *client = gs_client_new (address);
	client->querier->name = g_strdup (name);
	client->favorite = favorite;
	gsq_console_set_password (client->console, rcon_password);
	g_signal_connect (client->querier, "player-online",
			G_CALLBACK (player_online), app);
	g_signal_connect (client->querier, "player-offline",
			G_CALLBACK (player_offline), app);
	app->server_list = g_list_append (app->server_list, client);
	gui_slist_add (client);
	update_timer (app);
	gsq_querier_update (client->querier);
	return client;
}


GsClient *
gs_application_add_server (GsApplication *app, const gchar *address)
{
	g_return_val_if_fail (GS_IS_APPLICATION (app), NULL);
	g_return_val_if_fail (address != NULL, NULL);
	
	GsClient *client = add_server (app, address, NULL, FALSE, NULL);
	gs_application_save_server_list (app);
	return client;
}


static void
add_servers (GsApplication *app, gchar **servers)
{
	while (*servers) {
		add_server (app, *servers, NULL, TRUE, NULL);
		servers++;
	}
}


static void
remove_server (GsApplication *app, GsClient *client)
{
	if (app->server_iter && client == app->server_iter->data)
		app->server_iter = g_list_next (app->server_iter);
	app->server_list = g_list_remove (app->server_list, client);
	g_object_unref (client);
	update_timer (app);
}


void
gs_application_remove_server (GsApplication *app, GsClient *client)
{
	g_return_if_fail (GS_IS_APPLICATION (app));
	g_return_if_fail (GS_IS_CLIENT (app));
	
	remove_server (app, client);
	gs_application_save_server_list (app);
}


GsClient *
gs_application_find_server (GsApplication *app, const gchar *address)
{
	GList *iter = app->server_list;
	GsClient *client;
	
	while (iter) {
		client = iter->data;
		if (strcmp (gsq_querier_get_address (client->querier), address) == 0)
			return client;
		iter = iter->next;
	}
	
	return NULL;
}


GList *
gs_application_server_list (GsApplication *app)
{
	return app->server_list;
}


static void
load_server_list (GsApplication *app)
{
	GJsonNode *root, *node;
	GError *error = NULL;
	if (!(root = g_json_read_from_file (app->server_list_path, &error))) {
		g_warning ("Couldn't load server list: %s", error->message);
		g_error_free (error);
		return;
	}
	
	GJsonIter iter;
	g_json_iter_init (&iter, root);
	while (g_json_iter_next_array (&iter, &node)) {
		if (g_json_object_has (node, "address")) {
			add_server (app,
				g_json_object_get_string (node, "address"),
				g_json_object_get_string (node, "name"),
				g_json_object_get_boolean (node, "favorite"),
				g_json_object_get_string (node, "rcon-password")
			);
		}
	}
	g_json_node_free (root);
}



static void
load_preferences (GsApplication *app)
{
	GJsonNode *node;
	GError *error = NULL;
	
	if ((node = g_json_read_from_file (app->preferences_path, &error))) {
		if (g_json_object_has (node, "update-rate"))
			gs_application_set_update_rate (app,
					g_json_object_get_float (node, "update-rate"));
		
		if (g_json_object_has (node, "game-column"))
			gui_slist_set_game_column_mode (
					g_json_object_get_float (node, "game-column"));
		
		if (g_json_object_has (node, "port"))
			gsq_querier_set_default_port (
					g_json_object_get_integer (node, "port"));
		
		if (g_json_object_has (node, "connect-command"))
			gs_client_set_connect_command (g_json_object_get_string (node, "connect-command"));
		
		if (g_json_object_has (node, "notification-enable"))
			gs_notification_set_enable (
					g_json_object_get_boolean (node, "notification-enable"));
		
		const gchar *sound = NULL;
		if (g_json_object_has (node, "notification-sound"))
			sound = g_json_object_get_string (node, "notification-sound");
		gs_notification_set_sound (sound);
		
		if (g_json_object_has (node, "font"))
			gui_console_set_font (g_json_object_get_string (node, "font"));
		
		gboolean use_system_font = TRUE;
		if (g_json_object_has (node, "system-font"))
			use_system_font = g_json_object_get_boolean (node, "system-font");
		gui_console_set_use_system_font (use_system_font);
		
		if (g_json_object_has (node, "log-address"))
			gs_client_set_logaddress (g_json_object_get_string (node, "log-address"));
		
		GJsonNode *geometry = g_json_object_get (node, "geometry");
		if (geometry)
			gui_window_load_geometry (geometry);
		
		g_json_node_free (node);
	} else {
		g_warning ("Couldn't load preferences from %s: %s",
				app->preferences_path, error->message);
		g_error_free (error);
	}
	
	gui_prefs_set_update_rate (gs_application_get_update_rate (app));
	gui_prefs_set_game_column_mode (gui_slist_get_game_column_mode ());
	gui_prefs_set_port (gsq_querier_get_default_port ());
	gui_prefs_set_connect_command (gs_client_get_connect_command ());
	gui_prefs_set_enable_notifications (gs_notification_get_enable ());
	gui_prefs_set_notification_sound (gs_notification_get_sound ());
	gui_prefs_set_use_system_font (gui_console_get_use_system_font ());
	gui_prefs_set_font (gui_console_get_font ());
	gui_prefs_set_logaddress (gs_client_get_logaddress ());
}


void
gs_application_save_preferences (GsApplication *app)
{
	GJsonNode *node = g_json_object_new ();
	
	g_json_object_set_float (node, "update-rate",
			gs_application_get_update_rate (app));
	g_json_object_set_integer (node, "game-column",
			gui_slist_get_game_column_mode ());
	g_json_object_set_integer (node, "port",
			gsq_querier_get_default_port ());
	g_json_object_set_string (node, "connect-command",
			gs_client_get_connect_command ());
	
	g_json_object_set_boolean (node, "notification-enable",
			gs_notification_get_enable ());
	g_json_object_set_string (node, "notification-sound",
			gs_notification_get_sound ());
	
	g_json_object_set_boolean (node, "system-font",
			gui_console_get_use_system_font ());
	g_json_object_set_string (node, "font",
			gui_console_get_font ());
	
	g_json_object_set_string (node, "log-address",
			gs_client_get_logaddress ());
	
	GJsonNode *geometry = gui_window_save_geometry ();
	g_json_object_set (node, "geometry", geometry);
	
	GError *error = NULL;
	if (!g_json_write_to_file (node, app->preferences_path, &error)) {
		g_warning (_("Couldn't save preferences to %s: %s"),
				app->preferences_path, error->message);
		g_error_free (error);
	}
	g_json_node_free (node);
}



int
main (int argc, char **argv)
{
	g_type_init ();
	app = gs_application_new ();
	int ret = g_application_run (G_APPLICATION (app), argc, argv);
	g_object_unref (app);
	return ret;
}
