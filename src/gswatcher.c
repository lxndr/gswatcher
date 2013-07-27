/* 
 * gswatcher.c
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



#include <stdlib.h>
#include <glib/gi18n-lib.h>
#include <glib/gprintf.h>
#include <json-glib/json-glib.h>
#include "query/utils.h"
#include "gui-window.h"
#include "gswatcher.h"
#include "platform.h"
#include "gui-server-list.h"
#include "gui-console.h"
#include "gui-buddy-list.h"
#include "gui-preferences.h"
#include "gui-notification.h"


enum {
	PROP_0,
	PROP_INTERVAL,
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
	
	g_object_class_install_property (object_class, PROP_INTERVAL,
			g_param_spec_double ("interval", "Interval", "Update interval",
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
	app->default_port = 27500;
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
	case PROP_INTERVAL:
		app->interval = g_value_get_double (value);
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
	case PROP_INTERVAL:
		g_value_set_double (value, app->interval);
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
	return g_object_new (GS_TYPE_APPLICATION, "application-id", "org.gswatcher",
			"flags", G_APPLICATION_FLAGS_NONE, NULL);
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
	
	gchar *config_dir = g_build_filename (g_get_user_config_dir (), "gswatcher", NULL);
	g_mkdir_with_parents (config_dir, 0750);
	
	app->buddy_list_path = g_build_filename (config_dir, "buddylist.json", NULL);
	app->server_list_path = g_build_filename (config_dir, "serverlist.json", NULL);
	app->preferences_path = g_build_filename (config_dir, "preferences.json", NULL);
	
	g_free (config_dir);
	
#ifdef GS_DATADIR
	gchar *data_dir = g_build_filename (GS_DATADIR, "gswatcher", NULL);
#else
	gchar *data_dir = g_build_filename (root_dir, "share", "gswatcher", NULL);
#endif
	
	app->icon_dir = g_build_filename (data_dir, "icons", NULL);
	app->sound_dir = g_build_filename (data_dir, "sounds", NULL);
	app->pixmap_dir = g_build_filename (root_dir, "share", "pixmaps", NULL);
	
#ifdef ENABLE_NLS
#ifdef GS_LOCALEDIR
	app->locale_dir = g_strdup (GS_LOCALEDIR);
#else
	app->locale_dir = g_build_filename (root_dir, "share", "locale", NULL);
#endif
#endif
	
	g_free (data_dir);
	g_free (root_dir);
}


static gboolean
gs_application_local_command_line (GApplication *app, gchar ***argumnets,
		int *exit_status)
{
	/* count number of arguments */
	gchar **argv = *argumnets;
	gint argc = 0;
	
	while (argv[argc])
		argc++;
	
	/*  */
	GsApplication *gsapp = GS_APPLICATION (app);
	initialize_paths (gsapp, *argv);
	
	gboolean version = FALSE;
	gboolean debug = FALSE;
	gboolean notifier = FALSE;
	gchar *playbin = NULL;
	
	GError *error = NULL;
	GOptionContext *context;
	GOptionEntry options[] = {
		{"version", 'v', 0, G_OPTION_ARG_NONE, &version,
				N_("Show version number and exit"), NULL},
		{"minimized", 'm', 0, G_OPTION_ARG_NONE, &gsapp->minimized,
				N_("Start minimized in system tray"), NULL},
		{"debug", 'd', 0, G_OPTION_ARG_NONE, &debug,
				N_("Output all packets into stdout"), NULL},
		{"server", 's', 0, G_OPTION_ARG_STRING_ARRAY, &gsapp->specific_servers,
				N_("Watch only this server"), NULL},
#ifndef G_OS_WIN32
		{"notifier", 'n', 0, G_OPTION_ARG_NONE, &notifier,
				N_("Use internal notifier"), NULL},
#endif
		{"playbin", 'p', 0, G_OPTION_ARG_STRING, &playbin,
				N_("Command to play sounds"), NULL},
		{NULL}
	};
	
	context = g_option_context_new (NULL);
	g_option_context_add_group (context, gtk_get_option_group (TRUE));
	g_option_context_add_main_entries (context, options, NULL);
	
	if (!g_option_context_parse (context, &argc, &argv, &error)) {
		g_printerr ("%s\n", error->message);
		exit (EXIT_FAILURE);
	}
	
	if (version) {
		g_print ("Game Server Watcher v" GS_VERSION "\n");
		exit (EXIT_SUCCESS);
	}
	
	gs_notification_init (notifier);
	gsq_set_debug_flags (debug ?
			GSQ_DEBUG_INCOMING_DATA | GSQ_DEBUG_OUTGOING_DATA | GSQ_DEBUG_EVENT :
			GSQ_DEBUG_NONE);
	if (playbin)
		gs_notification_set_playbin (playbin);
	
	g_option_context_free (context);
	return G_APPLICATION_CLASS (gs_application_parent_class)->
			local_command_line (app, argumnets, exit_status);
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
	
	g_set_application_name ("Game Server Watcher");
	gtk_icon_theme_append_search_path (gtk_icon_theme_get_default (),
			gsapp->pixmap_dir);
	
	GtkWidget *window = gui_window_create ();
	gtk_application_add_window (GTK_APPLICATION (app), GTK_WINDOW (window));
	
	load_preferences (gsapp);
	load_buddy_list (gsapp);
	gsq_set_default_local_port (gsapp->default_port);
	
	if (gsapp->specific_servers)
		add_servers (gsapp, gsapp->specific_servers);
	else
		load_server_list (gsapp);
	
	if (!gsapp->minimized)
		gui_window_show ();
}


static gboolean
gs_application_shutdown_shot (GsApplication *app)
{
	gs_application_save_preferences (app);
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
save_buddy_list_func (gchar *name, GsBuddy *buddy, JsonObject* list)
{
	JsonObject* obj = json_object_new ();
	json_object_set_int_member (obj, "lastseen",
			g_date_time_to_unix (buddy->lastseen));
	json_object_set_string_member (obj, "lastaddr", buddy->lastaddr);
	json_object_set_boolean_member (obj, "notify", buddy->notify);
	json_object_set_object_member (list, buddy->name, obj);
}

static void
save_buddy_list (GsApplication *app)
{
	JsonObject* list = json_object_new ();
	g_hash_table_foreach (app->buddy_list, (GHFunc) save_buddy_list_func, list);
	
	JsonNode *root = json_node_alloc ();
	json_node_init_object (root, list);
	
	GError *error = NULL;
	JsonGenerator* gen = json_generator_new ();
	json_generator_set_pretty (gen, TRUE);
	json_generator_set_root (gen, root);
	
	if (!json_generator_to_file (gen, app->buddy_list_path, &error)) {
		g_warning (_("Couldn't save buddy list: %s"), error->message);
		g_error_free (error);
	}
	
	g_object_unref (gen);
	json_node_free (root);
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
load_buddy_list_func (JsonObject *list, const gchar *name, JsonNode *node,
		GsApplication* app)
{
	if (!JSON_NODE_HOLDS_OBJECT (node))
		return;
	
	JsonObject* obj = json_node_get_object (node);
	GsBuddy *buddy = add_buddy_real (app, name,
			json_object_get_int_member (obj, "lastseen"),
			json_object_get_string_member (obj, "lastaddr"),
			json_object_get_boolean_member (obj, "notify"));
	gui_blist_add (buddy);
}


static void
load_buddy_list (GsApplication *app)
{
	GError *error = NULL;
	JsonParser* parser = json_parser_new ();
	if (!(json_parser_load_from_file (parser, app->buddy_list_path, &error))) {
		g_warning (_("Couldn't load buddy list: %s"), error->message);
		g_error_free (error);
		return;
	}
	
	JsonNode *root = json_parser_get_root (parser);
	if (JSON_NODE_HOLDS_OBJECT (root)) {
		JsonObject* list = json_node_get_object (root);
		json_object_foreach_member (list, (JsonObjectForeach) load_buddy_list_func, app);
	}
	
	g_object_unref (parser);
}



/* server list */

static void
save_server_list_func (GsClient *client, JsonArray *list)
{
	JsonObject* obj = json_object_new ();
	json_object_set_string_member (obj, "name", client->querier->name->str);
	json_object_set_string_member (obj, "address", gs_client_get_address (client));
	json_object_set_boolean_member (obj, "favorite", client->favorite);
	if (client->console_settings & GUI_CONSOLE_PASS)
		json_object_set_string_member (obj, "console-password", client->console_password);
	if (client->console_settings & GUI_CONSOLE_PORT)
		json_object_set_int_member (obj, "console-port", client->console_port);
	json_array_add_object_element (list, obj);
}

void
gs_application_save_server_list (GsApplication *app)
{
	/* do not save server list if the user has specified servers */
	if (app->specific_servers)
		return;
	
	JsonArray* list = json_array_new ();
	g_list_foreach (app->server_list, (GFunc) save_server_list_func, list);
	
	JsonNode *root = json_node_alloc ();
	json_node_init_array (root, list);
	
	GError *error = NULL;
	JsonGenerator* gen = json_generator_new ();
	json_generator_set_pretty (gen, TRUE);
	json_generator_set_root (gen, root);
	
	if (!json_generator_to_file (gen, app->server_list_path, &error)) {
		g_warning (_("Couldn't save server list: %s"), error->message);
		g_error_free (error);
	}
	
	g_object_unref (gen);
	json_node_free (root);
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
		glong interval = (glong) (app->interval * 1000);
		app->timer = g_timeout_add (interval / count,
				(GSourceFunc) updater_func, app);
	}
}


void
gs_application_set_interval (GsApplication *app, gdouble interval)
{
	g_return_if_fail (GS_IS_APPLICATION (app));
	
	interval = CLAMP (interval, 0.5, 30.0);
	app->interval = interval;
	update_timer (app);
}

gdouble
gs_application_get_interval (GsApplication *app)
{
	g_return_val_if_fail (GS_IS_APPLICATION (app), 0);
	return app->interval;
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
player_online (GsqQuerier *querier, GsqPlayer *player, GsClient *client)
{
	GsBuddy *buddy = gs_application_find_buddy (app, player->name);
	
	if (buddy) {
		update_buddy (app, buddy);
		g_date_time_unref (buddy->lastseen);
		buddy->lastseen = g_date_time_new_now_local ();
		if (buddy->lastaddr)
			g_free (buddy->lastaddr);
		buddy->lastaddr = g_strdup (gs_client_get_address (client));
		gui_blist_update (buddy);
		save_buddy_list (app);
		
		if (buddy->notify) {
			gchar *title = g_strdup_printf (_("Player %s has connected"), buddy->name);
			gchar *text = g_strdup_printf (_("Address: %s\nServer: %s\nPlayers: %d / %d"),
					gs_client_get_address (client),
					querier->name->str,
					querier->numplayers,
					querier->maxplayers);
			gs_notification_message (title, text);
			g_free (title);
			g_free (text);
		}
	}
}

static void
player_offline (GsqQuerier *querier, GsqPlayer *player, GsClient *client)
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
					gs_client_get_address (client),
					querier->name->str,
					querier->numplayers,
					querier->maxplayers);
			gs_notification_message (title, text);
			g_free (title);
			g_free (text);
		}
		l = l->next;
	}
}


static GsClient *
add_server (GsApplication *app, const gchar *address)
{
	if (!(address && *address))
		return NULL;
	
	if (gs_application_find_server (app, address))
		return NULL;
	
	GsClient *client = gs_client_new (address);
	g_signal_connect (client->querier, "player-online",
			G_CALLBACK (player_online), client);
	g_signal_connect (client->querier, "player-offline",
			G_CALLBACK (player_offline), client);
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
	
	GsClient *client = add_server (app, address);
	gs_application_save_server_list (app);
	return client;
}


static void
add_servers (GsApplication *app, gchar **servers)
{
	while (*servers) {
		add_server (app, *servers);
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
	g_return_if_fail (GS_IS_CLIENT (client));
	
	remove_server (app, client);
	gs_application_save_server_list (app);
}


void
gs_application_remove_server_ask (GsApplication *app, GsClient *client)
{
	g_return_if_fail (GS_IS_APPLICATION (app));
	g_return_if_fail (GS_IS_CLIENT (client));
	
	GtkWidget *dialog = gtk_message_dialog_new (GTK_WINDOW (window),
			GTK_DIALOG_DESTROY_WITH_PARENT, GTK_MESSAGE_QUESTION, GTK_BUTTONS_YES_NO,
			_("Are you sure you want to remove \"%s\" ( %s) from the list?"),
			client->querier->name->str, gs_client_get_address (client));
	if (gtk_dialog_run (GTK_DIALOG (dialog)) == GTK_RESPONSE_YES) {
		gui_slist_remove (client);
		gs_application_remove_server (app, client);
	}
	
	gtk_widget_destroy (dialog);
}


GsClient *
gs_application_find_server (GsApplication *app, const gchar *address)
{
	g_return_val_if_fail (GS_IS_APPLICATION (app), NULL);
	g_return_val_if_fail (address != NULL, NULL);
	
	GsClient *found = NULL;
	guint16 gport, qport;
	gchar *host = gsq_parse_address (address, &gport, &qport);
	if (!host)
		return NULL;
	
	GList *iter;
	for (iter = app->server_list; iter; iter = iter->next) {
		GsClient *client = iter->data;
		
		/* compare host names */
		if (strcmp (host, gsq_querier_get_address (client->querier)) != 0) {
			GInetAddress *iaddr = gsq_querier_get_iaddr (client->querier);
			if (!iaddr)
				continue;
			
			gchar *ip = g_inet_address_to_string (iaddr);
			/* compare ip addresses */
			if (strcmp (host, ip) != 0) {
				g_free (ip);
				continue;
			}
			g_free (ip);
		}
		
		/* compare game ports */
		if (gport > 0) {
			if (gsq_querier_get_gport (client->querier) == gport) {
				found = client;
				break;
			}
		} else {
			if (gsq_querier_get_gport_auto (client->querier)) {
				found = client;
				break;
			}
		}
		
		/* TODO: we probably need to compare query ports too */
	}
	
	g_free (host);
	return found;
}


GList *
gs_application_server_list (GsApplication *app)
{
	return app->server_list;
}


static void
load_server_list_func (JsonArray *list, guint index, JsonNode *node, GsApplication* app)
{
	if (!JSON_NODE_HOLDS_OBJECT (node))
		return;
	
	JsonObject* obj = json_node_get_object (node);
	
	const gchar *address = json_object_get_string_member (obj, "address");
	GsClient *client = add_server (app, address);
	if (client) {
		g_string_assign (client->querier->name, json_object_get_string_member (obj, "name"));
		client->favorite = json_object_get_boolean_member (obj, "favorite");
		if ((node = json_object_get_member (obj, "console-password")))
			gs_client_set_console_password (client, json_node_get_string (node));
		if ((node = json_object_get_member (obj, "console-port")))
			gs_client_set_console_port (client, json_node_get_int (node));
		gui_slist_update (client);
	}
}


static void
load_server_list (GsApplication *app)
{
	GError *error = NULL;
	JsonParser* parser = json_parser_new ();
	if (!(json_parser_load_from_file (parser, app->server_list_path, &error))) {
		g_warning (_("Couldn't load server list: %s"), error->message);
		g_error_free (error);
		return;
	}
	
	JsonNode *root = json_parser_get_root (parser);
	if (JSON_NODE_HOLDS_ARRAY (root)) {
		JsonArray* list = json_node_get_array(root);
		json_array_foreach_element (list, (JsonArrayForeach) load_server_list_func, app);
	}
	
	g_object_unref (parser);
}



static void
load_preferences (GsApplication *app)
{
	GError *error = NULL;
	JsonParser* parser = json_parser_new ();
	if (!(json_parser_load_from_file (parser, app->preferences_path, &error))) {
		g_warning (_("Couldn't load user preferences: %s"), error->message);
		g_error_free (error);
		return;
	}
	
	JsonNode *root = json_parser_get_root (parser);
	if (!JSON_NODE_HOLDS_OBJECT (root)) {
		g_object_unref (parser);
		return;
	}
	
	JsonNode* node;
	JsonObject* obj = json_node_get_object (root);
	/* update interval */
	if ((node = json_object_get_member (obj, "interval")))
		gs_application_set_interval (app, json_node_get_double (node));
	/* game column mode */
	if ((node = json_object_get_member (obj, "game-column")))
		gui_slist_set_game_column_mode (json_node_get_int (node));
	/* outgoing port */
	if ((node = json_object_get_member (obj, "port")))
		app->default_port = json_node_get_int (node);
	/* connect command */
	if ((node = json_object_get_member (obj, "connect-command")))
		gs_client_set_connect_command (json_node_get_string (node));
	/* enable notifications */
	if ((node = json_object_get_member (obj, "notification-enable")))
		gs_notification_set_enable (json_node_get_boolean (node));
	/* enable sound notifications */
	if ((node = json_object_get_member (obj, "notification-sound")))
		gs_notification_set_sound (json_node_get_string (node));
	/* font */
	if ((node = json_object_get_member (obj, "font")))
		gui_console_set_font (json_node_get_string (node));
	/* use system font */
	if ((node = json_object_get_member (obj, "system-font")))
		gui_console_set_use_system_font (json_node_get_boolean (node));
	/* log incoming address */
	if ((node = json_object_get_member (obj, "log-address")))
		gs_client_set_logaddress (json_node_get_string (node));
	
	/* main window geometry */
	JsonObject* geom = json_object_get_object_member (obj, "geometry");
	if (geom)
		gui_window_load_geometry (geom);
	
	g_object_unref (parser);
	
	/* initialize preference widgets */
	gui_prefs_set_interval (gs_application_get_interval (app));
	gui_prefs_set_game_column_mode (gui_slist_get_game_column_mode ());
	gui_prefs_set_port (app->default_port);
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
	JsonObject* obj = json_object_new ();
	
	/* update interval */
	json_object_set_double_member (obj, "interval",
			gs_application_get_interval (app));
	/* game column mode */
	json_object_set_int_member (obj, "game-column",
			gui_slist_get_game_column_mode ());
	/* outgoing port */
	json_object_set_int_member (obj, "port",
			app->default_port);
	/* connect command */
	json_object_set_string_member (obj, "connect-command",
			gs_client_get_connect_command ());
	/* enable notifications */
	json_object_set_boolean_member (obj, "notification-enable",
			gs_notification_get_enable ());
	/* notification sound */
	json_object_set_string_member (obj, "notification-sound",
			gs_notification_get_sound ());
	/* use system monospace font */
	json_object_set_boolean_member (obj, "system-font",
			gui_console_get_use_system_font ());
	/* monospace font */
	json_object_set_string_member (obj, "font",
			gui_console_get_font ());
	/* log incoming address */
	json_object_set_string_member (obj, "log-address",
			gs_client_get_logaddress ());
	/* main window geometry */
	JsonObject *geom = gui_window_save_geometry ();
	json_object_set_object_member (obj, "geometry", geom);
	
	JsonNode *root = json_node_alloc ();
	json_node_init_object (root, obj);
	
	GError *error = NULL;
	JsonGenerator* gen = json_generator_new ();
	json_generator_set_pretty (gen, TRUE);
	json_generator_set_root (gen, root);
	
	if (!json_generator_to_file (gen, app->preferences_path, &error)) {
		g_warning (_("Couldn't save user preferences: %s"), error->message);
		g_error_free (error);
	}
	
	g_object_unref (gen);
	json_node_free (root);
}



int
main (int argc, char **argv)
{
#if GLIB_MINOR_VERSION < 36
	g_type_init ();
#endif
	app = gs_application_new ();
	int ret = g_application_run (G_APPLICATION (app), argc, argv);
	g_object_unref (app);
	return ret;
}
