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


static guint timer = 0;
static glong update_rate = 2500;
static gboolean update_pause = FALSE;
static gboolean read_only_server_list = FALSE;
static GList *servers = NULL;
static GList *iserver = NULL;
static GHashTable *buddies = NULL;



/* buddy list */

static void
gs_save_buddy_list_func (gchar *name, GsBuddy *buddy, GJsonNode *root)
{
	GJsonNode *node = g_json_object_new ();
	g_json_object_set_integer (node, "lastseen",
			g_date_time_to_unix (buddy->lastseen));
	g_json_object_set_string (node, "lastaddr", buddy->lastaddr);
	g_json_object_set_boolean (node, "notify", buddy->notify);
	g_json_object_set (root, buddy->name, node);
}

static void
gs_save_buddy_list ()
{
	GJsonNode *root = g_json_object_new ();
	g_hash_table_foreach (buddies, (GHFunc) gs_save_buddy_list_func, root);
	
	GError *error = NULL;
	if (!g_json_write_to_file (root, gs_get_buddy_list_path (), &error)) {
		g_warning (_("Couldn't save buddy list: %s"), error->message);
		g_error_free (error);
	}
	g_json_node_free (root);
}


GsBuddy *
gs_find_buddy (const gchar *name)
{
	return g_hash_table_lookup (buddies, name);
}


static void
update_buddy_server_list (GsBuddy *buddy)
{
	g_list_free (buddy->servers);
	buddy->servers = NULL;
	
	GList *isrv = servers;
	while (isrv) {
		GsClient *client = isrv->data;
		if (gsq_querier_find_player (client->querier, buddy->name))
			buddy->servers = g_list_append (buddy->servers, client);
		isrv = isrv->next;
	}
}


static void
gs_free_buddy (GsBuddy *buddy)
{
	g_free (buddy->name);
	g_date_time_unref (buddy->lastseen);
	if (buddy->lastaddr)
		g_free (buddy->lastaddr);
	g_list_free (buddy->servers);
	g_slice_free (GsBuddy, buddy);
}


void
gs_remove_buddy (const gchar *name)
{
	g_hash_table_remove (buddies, name);
	gs_save_buddy_list ();
}


void
gs_change_buddy (const gchar *name, gboolean notify)
{
	GsBuddy *buddy = gs_find_buddy (name);
	buddy->notify = notify;
	gs_save_buddy_list ();
}


static GsBuddy *
gs_add_buddy_real (const gchar *name, gint64 lastseen, const gchar *lastaddr,
		gboolean notify)
{
	GsBuddy *buddy = g_slice_new0 (GsBuddy);
	buddy->name = g_strdup (name);
	buddy->lastseen = g_date_time_new_from_unix_local (lastseen);
	buddy->lastaddr = lastaddr ? g_strdup (lastaddr) : NULL;
	buddy->notify = notify;
	update_buddy_server_list (buddy);
	g_hash_table_insert (buddies, buddy->name, buddy);
	return buddy;
}


GsBuddy *
gs_add_buddy (const gchar *name, gint64 lastseen, const gchar *lastaddr,
		gboolean notify)
{
	g_return_val_if_fail (name && *name, NULL);
	GsBuddy *buddy = gs_add_buddy_real (name, lastseen, lastaddr, notify);
	gs_save_buddy_list ();
	return buddy;
}


void
gs_load_buddy_list ()
{
	GJsonNode *root, *node;
	GError *error = NULL;
	if (!(root = g_json_read_from_file (gs_get_buddy_list_path (), &error))) {
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
		GsBuddy *buddy = gs_add_buddy_real (name, lastseen, lastaddr, notify);
		gui_blist_add (buddy);
	}
	g_json_node_free (root);
}



/* server list */

static void
gs_save_server_list_func (GsClient *client, GJsonNode *root)
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
gs_save_server_list ()
{
	if (read_only_server_list)
		return;
	
	GJsonNode *root = g_json_array_new ();
	g_list_foreach (servers, (GFunc) gs_save_server_list_func, root);
	
	GError *error = NULL;
	if (!g_json_write_to_file (root, gs_get_server_list_path (), &error)) {
		g_warning (_("Couldn't save server list: %s"), error->message);
		g_error_free (error);
	}
	g_json_node_free (root);
}


static gboolean
updater_func (gpointer data)
{
	if (update_pause || !servers)
		return TRUE;
	
	if (iserver == NULL)
		iserver = servers;
	
	gsq_querier_update (((GsClient *) iserver->data)->querier);
	iserver = g_list_next (iserver);
	
	return TRUE;
}

static void
update_timer ()
{
	if (timer) {
		g_source_remove (timer);
		timer = 0;
	}
	
	guint count = g_list_length (servers);
	if (count > 0)
		timer = g_timeout_add (update_rate / count, updater_func, NULL);
}


void
gs_set_update_rate (gdouble rate)
{
	rate = CLAMP (rate, 0.5, 10);
	update_rate = (glong) (rate * 1000);
	update_timer ();
}

gdouble
gs_get_update_rate ()
{
	return (gdouble) update_rate / 1000;
}


void
gs_pause (gboolean pause)
{
	update_pause = pause;
}

gboolean
gs_is_pause ()
{
	return update_pause;
}


static void
player_online (GsqQuerier *querier, GsqPlayer *player, GsClient *client)
{
	GsBuddy *buddy = gs_find_buddy (player->name);
	
	if (buddy) {
		update_buddy_server_list (buddy);
		g_date_time_unref (buddy->lastseen);
		buddy->lastseen = g_date_time_new_now_local ();
		if (buddy->lastaddr)
			g_free (buddy->lastaddr);
		buddy->lastaddr = g_strdup (gsq_querier_get_address (client->querier));
		gui_blist_update (buddy);
		gs_save_buddy_list ();
		
		if (buddy->notify) {
			gchar *title = g_strdup_printf (_("Player %s has connected"), buddy->name);
			gchar *text = g_strdup_printf (_("Address: %s\nServer: %s\nPlayers: %d / %d"),
					gsq_querier_get_address (client->querier), client->querier->name,
					client->querier->numplayers, client->querier->maxplayers);
			gs_notification_message (title, text);
			g_free (title);
			g_free (text);
		}
		
	}
}

static void
player_offline (GsqQuerier *querier, GsqPlayer *player, GsClient *client)
{
	GsBuddy *buddy = gs_find_buddy (player->name);
	if (buddy) {
		update_buddy_server_list (buddy);
		gui_blist_update (buddy);
		return;
	}
	
	GList *l = gsq_querier_get_players (client->querier);
	while (l) {
		GsqPlayer *pl = l->data;
		buddy = gs_find_buddy (pl->name);
		
		if (buddy && buddy->notify && client->querier->maxplayers - client->querier->numplayers == 1) {
			gchar *title = g_strdup_printf (_("Server with %s has free slot"), buddy->name);
			gchar *text = g_strdup_printf (_("Address: %s\nServer: %s\nPlayers: %d / %d"),
					gsq_querier_get_address (client->querier), client->querier->name,
					client->querier->numplayers, client->querier->maxplayers);
			gs_notification_message (title, text);
			g_free (title);
			g_free (text);
		}
		l = l->next;
	}
}


static GsClient *
add_server (const gchar *address, const gchar *name, gboolean favorite,
		const gchar *rcon_password)
{
	if (gs_find_server (address))
		return NULL;
	
	GsClient *client = gs_client_new (address);
	client->querier->name = g_strdup (name);
	client->favorite = favorite;
	gsq_console_set_password (client->console, rcon_password);
	g_signal_connect (client->querier, "player-online", G_CALLBACK (player_online), client);
	g_signal_connect (client->querier, "player-offline", G_CALLBACK (player_offline), client);
	servers = g_list_append (servers, client);
	gui_slist_add (client);
	update_timer ();
	gsq_querier_update (client->querier);
	return client;
}


GsClient *
gs_add_server (const gchar *address)
{
	GsClient *client = add_server (address, NULL, FALSE, NULL);
	gs_save_server_list ();
	return client;
}


static void
add_servers (gchar **servers)
{
	while (*servers) {
		add_server (*servers, NULL, TRUE, NULL);
		servers++;
	}
}


static void
remove_server (GsClient *client)
{
	if (iserver && client == iserver->data)
		iserver = g_list_next (iserver);
	servers = g_list_remove (servers, client);
	g_object_unref (client);
	update_timer ();
}

static void
free_server_list ()
{
	while (servers)
		remove_server (servers->data);
}

void
gs_remove_server (GsClient *client)
{
	remove_server (client);
	gs_save_server_list ();
}


GsClient *
gs_find_server (const gchar *address)
{
	GList *isrv = servers;
	GsClient *client;
	
	while (isrv) {
		client = isrv->data;
		if (strcmp (gsq_querier_get_address (client->querier), address) == 0)
			return client;
		isrv = isrv->next;
	}
	
	return NULL;
}


GList *
gs_get_server_list ()
{
	return servers;
}


void
gs_load_server_list ()
{
	GJsonNode *root, *node;
	GError *error = NULL;
	if (!(root = g_json_read_from_file (gs_get_server_list_path (), &error))) {
		g_warning ("Couldn't load server list: %s", error->message);
		g_error_free (error);
		return;
	}
	
	GJsonIter iter;
	g_json_iter_init (&iter, root);
	while (g_json_iter_next_array (&iter, &node)) {
		if (g_json_object_has (node, "address")) {
			add_server (
				g_json_object_get_string (node, "address"),
				g_json_object_get_string (node, "name"),
				g_json_object_get_boolean (node, "favorite"),
				g_json_object_get_string (node, "rcon-password")
			);
		}
	}
	g_json_node_free (root);
}



void
gs_load_preferences ()
{
	GJsonNode *node;
	GError *error = NULL;
	
	if ((node = g_json_read_from_file (gs_get_preferences_path (), &error))) {
		if (g_json_object_has (node, "update-rate"))
			gs_set_update_rate (
					g_json_object_get_float (node, "update-rate"));
		
		if (g_json_object_has (node, "game-column"))
			gui_slist_set_game_column_mode (
					g_json_object_get_float (node, "game-column"));
		
		if (g_json_object_has (node, "notification-enable"))
			gs_notification_set_enable (
					g_json_object_get_boolean (node, "notification-enable"));
		
		const gchar *sound = NULL;
		if (g_json_object_has (node, "notification-sound"))
			sound = g_json_object_get_string (node, "notification-sound");
		gs_notification_set_sound (sound);
		
		gboolean use_system_font = TRUE;
		if (g_json_object_has (node, "system-font"))
			use_system_font = g_json_object_get_boolean (node, "system-font");
		gui_console_set_use_system_font (use_system_font);
		
		if (g_json_object_has (node, "font"))
			gui_console_set_font (g_json_object_get_string (node, "font"));
		
		GJsonNode *geometry = g_json_object_get (node, "geometry");
		if (geometry)
			gui_window_load_geometry (geometry);
		
		g_json_node_free (node);
	} else {
		g_warning ("Couldn't load preferences from %s: %s",
				gs_get_preferences_path (), error->message);
		g_error_free (error);
	}
	
	gui_prefs_set_update_rate (gs_get_update_rate ());
	gui_prefs_set_game_column_mode (gui_slist_get_game_column_mode ());
	gui_prefs_set_enable_notifications (gs_notification_get_enable ());
	gui_prefs_set_notification_sound (gs_notification_get_sound ());
	gui_prefs_set_use_system_font (gui_console_get_use_system_font ());
	gui_prefs_set_font (gui_console_get_font ());
}

void
gs_save_preferences ()
{
	GJsonNode *node = g_json_object_new ();
	
	g_json_object_set_float (node, "update-rate",
			gs_get_update_rate ());
	g_json_object_set_integer (node, "game-column",
			gui_slist_get_game_column_mode ());
	
	g_json_object_set_boolean (node, "notification-enable",
			gs_notification_get_enable ());
	g_json_object_set_string (node, "notification-sound",
			gs_notification_get_sound ());
	
	g_json_object_set_boolean (node, "system-font",
			gui_console_get_use_system_font ());
	g_json_object_set_string (node, "font",
			gui_console_get_font ());
	
	GJsonNode *geometry = gui_window_save_geometry ();
	g_json_object_set (node, "geometry", geometry);
	
	GError *error = NULL;
	if (!g_json_write_to_file (node, gs_get_preferences_path (), &error)) {
		g_warning (_("Couldn't save preferences to %s: %s"),
				gs_get_preferences_path (), error->message);
		g_error_free (error);
	}
	g_json_node_free (node);
}


int
main (int argc, char **argv)
{
	g_type_init ();
	gs_initialize_directories (argv[0]);
	
#ifdef ENABLE_NLS
	bindtextdomain (GETTEXT_PACKAGE, gs_get_locale_dir ());
	bind_textdomain_codeset (GETTEXT_PACKAGE, "UTF-8");
	textdomain (GETTEXT_PACKAGE);
#endif
	
	gboolean showversion = FALSE;
	gboolean startminimized = FALSE;
	gboolean debug = FALSE;
	gboolean notifier = FALSE;
	gchar **servers = NULL;
	GError *error = NULL;
	
	GOptionEntry options[] = {
		{"version", 'v', 0, G_OPTION_ARG_NONE, &showversion,
				N_("Show version number and exit"), NULL},
		{"minimized", 'm', 0, G_OPTION_ARG_NONE, &startminimized,
				N_("Start minimized in system tray"), NULL},
		{"debug", 'd', 0, G_OPTION_ARG_NONE, &debug,
				N_("Output all packets into stdout"), NULL},
		{"server", 's', 0, G_OPTION_ARG_STRING_ARRAY, &servers,
				N_("Watch only this server"), NULL},
		{"notifier", 'n', 0, G_OPTION_ARG_NONE, &notifier,
				N_("Use internal notifier"), NULL},
		{NULL}
	};
	
	
	g_set_application_name ("Game Server Tool");
	if (!gtk_init_with_args (&argc, &argv, "", options, GETTEXT_PACKAGE, &error))
		g_error ("%s", error->message);
	
	if (showversion) {
		g_print ("Game Server Tool v" GS_VERSION "\n");
		return 0;
	}
	
	gui_window_create ();
	gs_notification_init (notifier);
	
	buddies = g_hash_table_new_full (g_str_hash, g_str_equal, NULL,
			(GDestroyNotify) gs_free_buddy);
	gs_load_preferences ();
	gs_load_buddy_list ();
	
	gsq_querier_set_debug_mode (debug);
	
	if (servers) {
		add_servers (servers);
		read_only_server_list = TRUE;
	} else {
		gs_load_server_list ();
	}
	
	if (startminimized)
		gui_window_hide ();
	else
		gui_window_show ();
	
	gtk_main ();
	
	gs_save_preferences ();
	
	/* let's do it beautifully */
	free_server_list ();
	if (timer)
		g_source_remove (timer);
	g_hash_table_destroy (buddies);
	gui_window_destroy ();
	
	return 0;
}
