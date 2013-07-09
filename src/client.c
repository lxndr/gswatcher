/* 
 * client.c
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



#include <glib/gi18n-lib.h>
#include <GeoIP.h>
#include "query/utils.h"
#include "gui-console.h"
#include "gui-log.h"
#include "gui-chat.h"
#include "client.h"


enum {
	PROP_0,
	PROP_FAVORITE,
	PROP_CONSOLE_PASSWORD
};


static GeoIP *geoip;
static gchar *logaddress = NULL;
static GRegex *re_say, *re_player;
static gchar *connect_command = NULL;


static void gs_client_set_property (GObject *object, guint prop_id,
		const GValue *value, GParamSpec *pspec);
static void gs_client_get_property (GObject *object, guint prop_id,
		GValue *value, GParamSpec *pspec);
static void gs_client_finalize (GObject *object);
static void gs_client_querier_resolved (GsqQuerier *querier, GsClient *client);
static void gs_client_querier_info_updated (GsqQuerier *querier, GsClient *client);
static void gs_client_querier_map_changed (GsqQuerier *querier, GsClient *client);
static void gs_client_querier_gameid_changed (GsqQuerier *querier, GsClient *client);
static void gs_client_querier_log (GsqQuerier *querier, const gchar *msg, GsClient *client);
static void gs_client_querier_error (GsqQuerier *querier, const GError *error, GsClient *client);
static void gs_client_console_connected (GsqConsole *console, GsClient *client);
static void gs_client_console_authenticated (GsqConsole *console, GsClient* client);
static void gs_client_console_disconnected (GsqConsole *console, GsClient *client);


G_DEFINE_TYPE (GsClient, gs_client, G_TYPE_OBJECT);


static void
gs_client_class_init (GsClientClass *class)
{
	GObjectClass *object_class = G_OBJECT_CLASS (class);
	object_class->set_property = gs_client_set_property;
	object_class->get_property = gs_client_get_property;
	object_class->finalize = gs_client_finalize;
	
	geoip = GeoIP_new (GEOIP_STANDARD);
	if (!geoip)
		exit (0);
	
	re_say = g_regex_new ("^L (\\d{2}/\\d{2}/\\d{4}) - (\\d{2}:\\d{2}:\\d{2}): \"(.+)\" (say|say_team) \"(.+)\"$",
			G_REGEX_OPTIMIZE | G_REGEX_UNGREEDY, 0, NULL);
	re_player = g_regex_new ("^(.+)<\\d+><STEAM_\\d:\\d:\\d+><(.+)><.*><.+><.+><.+><.+>$",
			G_REGEX_OPTIMIZE | G_REGEX_UNGREEDY, 0, NULL);
	
	g_object_class_install_property (object_class, PROP_FAVORITE,
			g_param_spec_boolean ("favorite", "Favorite", "Favorite server",
			FALSE, G_PARAM_READABLE | G_PARAM_WRITABLE));
	
	g_object_class_install_property (object_class, PROP_CONSOLE_PASSWORD,
			g_param_spec_string ("console-password", "Console password", "Console password",
			"", G_PARAM_READABLE | G_PARAM_WRITABLE));
}


static void
gs_client_init (GsClient *client)
{
}


static void
gs_client_finalize (GObject *object)
{
	GsClient *client = GS_CLIENT (object);
	
	g_free (client->address);
	if (client->version)
		g_free (client->version);
	g_object_unref (client->querier);
	
	/* console */
	if (client->console)
		g_object_unref (client->console);
	gui_console_free_context (client);
	if (client->console_password)
		g_free (client->console_password);
	
	/* log */
	if (client->log_address)
		g_free (client->log_address);
	g_object_unref (client->log_buffer);
	
	/* chat */
	g_object_unref (client->chat_buffer);
	
	G_OBJECT_CLASS (gs_client_parent_class)->finalize (object);
}


static void
gs_client_set_property (GObject *object, guint prop_id, const GValue *value,
		GParamSpec *pspec)
{
	GsClient *client = GS_CLIENT (object);
	
	switch (prop_id) {
	case PROP_FAVORITE:
		client->favorite = g_value_get_boolean (value);
		break;
	case PROP_CONSOLE_PASSWORD:
		gs_client_set_console_password (client, g_value_get_string (value));
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
		break;
	}
}

static void
gs_client_get_property (GObject *object, guint prop_id, GValue *value,
		GParamSpec *pspec)
{
	GsClient *client = GS_CLIENT (object);
	
	switch (prop_id) {
	case PROP_FAVORITE:
		g_value_set_boolean (value, client->favorite);
		break;
	case PROP_CONSOLE_PASSWORD:
		g_value_set_string (value, gs_client_get_console_password (client));
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
		break;
    }
}


GsClient*
gs_client_new (const gchar *address)
{
	g_return_val_if_fail (address != NULL, NULL);
	GsClient *client = g_object_new (GS_TYPE_CLIENT, NULL);
	
	client->address = g_strdup (address);
	
	client->querier = GSQ_QUERIER (gsq_watcher_new (address));
	g_string_safe_assign (client->querier->name, address);
	g_signal_connect (client->querier, "resolve",
			G_CALLBACK (gs_client_querier_resolved), client);
	g_signal_connect (client->querier, "info-update",
			G_CALLBACK (gs_client_querier_info_updated), client);
	g_signal_connect (client->querier, "map-changed",
			G_CALLBACK (gs_client_querier_map_changed), client);
	g_signal_connect (client->querier, "gameid-changed",
			G_CALLBACK (gs_client_querier_gameid_changed), client);
	g_signal_connect (client->querier, "log",
			G_CALLBACK (gs_client_querier_log), client);
	g_signal_connect (client->querier, "error",
			G_CALLBACK (gs_client_querier_error), client);
	
	gui_console_init_context (client);
	gui_log_init (client);
	gui_chat_init (client);
	
	return client;
}


const gchar *
gs_client_get_address (GsClient *client)
{
	return client->address;
}


void
gs_client_set_favorite (GsClient *client, gboolean favorite)
{
	g_return_if_fail (GS_IS_CLIENT (client));
	client->favorite = favorite;
	g_object_notify (G_OBJECT (client), "favorite");
}

gboolean
gs_client_get_favorite (GsClient *client)
{
	g_return_val_if_fail (GS_IS_CLIENT (client), FALSE);
	return client->favorite;
}


void
gs_client_set_console_port (GsClient *client, guint16 port)
{
	g_return_if_fail (GS_IS_CLIENT (client));
	client->console_port = port;
	if (client->console)
		gsq_console_set_port (client->console, port);
}

guint16
gs_client_get_console_port (GsClient *client)
{
	g_return_val_if_fail (GS_IS_CLIENT (client), FALSE);
	return client->console_port;
}


void
gs_client_set_console_password (GsClient *client, const gchar *password)
{
	g_return_if_fail (GS_IS_CLIENT (client));
	if (client->console_password)
		g_free (client->console_password);
	client->console_password = g_strdup (password);
	if (client->console)
		gsq_console_set_password (client->console, password);
}

const gchar *
gs_client_get_console_password (GsClient *client)
{
	g_return_val_if_fail (GS_IS_CLIENT (client), FALSE);
	return client->console_password;
}


gchar *
gs_client_get_game_name (GsClient* client, gboolean extra)
{
	GsqQuerier *querier = client->querier;
	const gchar *game = querier->gamename->str;
	const gchar *mode = querier->gamemode->str;
	
	if (*mode && extra)
		return g_strdup_printf ("%s (%s)", game, mode);
	else
		return g_strdup (game);
}


static void
gs_client_querier_resolved (GsqQuerier *querier, GsClient *client)
{
	GInetAddress *iaddr = gsq_querier_get_iaddr (querier);
	gchar *ip = g_inet_address_to_string (iaddr);
	
	int country_id;
	if (g_inet_address_get_family (iaddr) == G_SOCKET_FAMILY_IPV4)
		country_id = GeoIP_id_by_addr (geoip, ip);
	else if (g_inet_address_get_family (iaddr) == G_SOCKET_FAMILY_IPV6 &&
			GeoIP_database_edition (geoip) == GEOIP_COUNTRY_EDITION_V6)
		country_id = GeoIP_id_by_addr_v6 (geoip, ip);
	else
		country_id = 0;
	
	client->country_code = GeoIP_code_by_id (country_id);
	client->country = GeoIP_name_by_id (country_id);
	
	g_free (ip);
}


static void
gs_client_querier_info_updated (GsqQuerier *querier, GsClient *client)
{
	/* password field */
	gchar *password = gsq_querier_get_extra (querier, "password");
	client->password = password ? strcmp (password, "true") == 0 : FALSE;
	
	/* version field */
	if (client->version)
		g_free (client->version);
	
	const gchar *version = querier->version->str;
	
	if (strcmp (gsq_querier_get_protocol (querier), "source") == 0) {
		gchar *type = gsq_querier_get_extra (querier, "type");
		client->version = g_strdup_printf (*type ? "%s (%s, %s, %s)" : "%s (%s, %s)",
				version, gsq_querier_get_extra (querier, "protocol-version"),
				gsq_querier_get_extra (querier, "os"), type);
	} else {
		client->version = g_strdup (version);
	}
}


static void
gs_client_querier_map_changed (GsqQuerier *querier, GsClient *client)
{
	if (client->log_auto)
		gs_client_enable_log (client, TRUE);
}


static void
gs_client_querier_gameid_changed (GsqQuerier *querier, GsClient *client)
{
	if (client->console) {
		g_object_unref (client->console);
		client->console = NULL;
	}
	
	const gchar *gameid = querier->gameid->str;
	
	GType type;
	if (strcmp (gameid, "ss3") == 0)
		type = GSQ_TYPE_CONSOLE_TELNET;
	else
		type = GSQ_TYPE_CONSOLE_SOURCE;
	
	const gchar *host = gsq_querier_get_address (client->querier);
	guint16 gport = gsq_querier_get_gport (client->querier);
	
	if (strcmp (gameid, "ss3") == 0) {		/* Serious Sam 3: BFE */
		client->console_settings = GUI_CONSOLE_PORT | GUI_CONSOLE_PASS;
		if (client->console_port == 0)
			client->console_port = 27015;
	} else if (strcmp (gameid, "mc") == 0) {
		client->console_settings = GUI_CONSOLE_PORT | GUI_CONSOLE_PASS;
		if (client->console_port == 0)
			client->console_port = 25575;
	} else {								/*  */
		client->console_settings = GUI_CONSOLE_PASS;
		client->console_port = gport > 0 ? gport : 27015;
	}
	
	client->console = g_object_new (type,
			"host", host,
			"port", client->console_port,
			"password", client->console_password,
			NULL);
	
	g_signal_connect (client->console, "connected",
			G_CALLBACK (gs_client_console_connected), client);
	g_signal_connect (client->console, "authenticated",
			G_CALLBACK (gs_client_console_authenticated), client);
	g_signal_connect (client->console, "disconnected",
			G_CALLBACK (gs_client_console_disconnected), client);
}


static void
gs_client_querier_log (GsqQuerier *querier, const gchar *msg, GsClient *client)
{
	gui_log_print (client, msg + 2);
	
	gchar **say_parts = g_regex_split (re_say, msg, 0);
	if (!*say_parts[0]) {
		if (strcmp (say_parts[3], "Console<0>") == 0) {
			gui_chat_log (client, "Console", 0, say_parts[5]);
		} else {
			gchar *player = gsq_utf8_repair (say_parts[3]);
			gchar **player_parts = g_regex_split (re_player, player, 0);
			if (!*player_parts[0]) {
				gint team;
				if (strcmp (player_parts[2], "Survivor") == 0)
					team = 2;
				else if (strcmp (player_parts[2], "Infected") == 0)
					team = 1;
				else
					team = 3;
				
				gui_chat_log (client, player_parts[1], team, say_parts[5]);
			}
			g_strfreev (player_parts);
			g_free (player);
		}
	}
	g_strfreev (say_parts);
}


static void
gs_client_querier_error (GsqQuerier *querier, const GError *error, GsClient *client)
{
	if (client->error)
		g_error_free (client->error);
	client->error = g_error_copy (error);
}


static void
gs_client_console_connected (GsqConsole *console, GsClient* client)
{
	gui_console_log (client, GUI_CONSOLE_INFO, _("Connected"));
}

static void
gs_client_console_authenticated (GsqConsole *console, GsClient* client)
{
	gui_console_log (client, GUI_CONSOLE_INFO, _("Authenticated"));
}

static void
gs_client_console_disconnected (GsqConsole *console, GsClient* client)
{
	gui_console_log (client, GUI_CONSOLE_INFO, _("Disconnected"));
}


static void
command_callback (GsqConsole *console, GAsyncResult *result, GsClient *client)
{
	GError *error = NULL;
	gchar *output = gsq_console_send_finish (console, result, &error);
	if (output)
		gui_console_log (client, GUI_CONSOLE_RESPOND, output);
	else
		gui_console_log (client, GUI_CONSOLE_ERROR, error->message);
}


void
gs_client_send_command (GsClient* client, const gchar *cmd)
{
	if (!client->console) {
		gui_console_log (client, GUI_CONSOLE_ERROR, _("Server's protocol is not determined"));
		return;
	}
	
	gsq_console_send (client->console, cmd,
			(GAsyncReadyCallback) command_callback, client);
}



void
gs_client_set_logaddress (const gchar *address)
{
	if (logaddress)
		g_free (logaddress);
	logaddress = g_strdup (address ? address : "");
}

gchar *
gs_client_get_logaddress ()
{
	return logaddress;
}


static void
log_command_callback (GsqConsole *console, GAsyncResult *result, GsClient *client)
{
	GError *error = NULL;
	gchar *output = gsq_console_send_finish (console, result, &error);
	if (output)
		gui_console_log (client, GUI_CONSOLE_RESPOND, output);
	else
		gui_console_log (client, GUI_CONSOLE_ERROR, error->message);
}

void
gs_client_enable_log (GsClient *client, gboolean enable)
{
	if (enable) {
		guint16 userport;
		gchar *userhost = gsq_parse_address (logaddress, &userport, NULL);
		if (!userhost) {
			gui_console_log (client, GUI_CONSOLE_ERROR,
					_("You have to provide a log address"));
			return;
		}
		
		if (client->log_address) {
			g_free (client->log_address);
			client->log_address = NULL;
		}
		
		GError *error = NULL;
		GResolver *resolver = g_resolver_get_default ();
		GList *addresses = g_resolver_lookup_by_name (resolver, userhost,
				NULL, &error);
		if (!addresses) {
			g_free (userhost);
			g_object_unref (resolver);
			return;
		}
		g_object_unref (resolver);
		g_free (userhost);
		
		userhost = g_inet_address_to_string (addresses->data);
		if (userport == 0)
			userport = gsq_get_local_ipv4_port (client->querier);
		
		client->log_address = g_strdup_printf ("%s:%d", userhost, userport);
		g_free (userhost);
		
		gchar *cmd = g_strdup_printf ("logaddress_add %s;log on", client->log_address);
		gsq_console_send_full (client->console, cmd, 3,
				(GAsyncReadyCallback) log_command_callback, client);
		g_free (cmd);
	} else {
		if (client->log_address) {
			gchar *cmd = g_strdup_printf ("logaddress_del %s;log off", client->log_address);
			gsq_console_send (client->console, cmd,
					(GAsyncReadyCallback) log_command_callback, client);
			g_free (cmd);
			
			g_free (client->log_address);
			client->log_address = NULL;
		}
	}
}


static void
chat_message_callback (GsqConsole *console, GAsyncResult *result, GsClient *client)
{
	GError *error = NULL;
	gchar *output = gsq_console_send_finish (console, result, &error);
	if (output)
		gui_console_log (client, GUI_CONSOLE_RESPOND, output);
	else
		gui_console_log (client, GUI_CONSOLE_ERROR, error->message);
}

void
gs_client_send_message (GsClient *client, const gchar *msg)
{
	gchar *cmd = g_strdup_printf ("say %s", msg);
	gsq_console_send (client->console, cmd,
			(GAsyncReadyCallback) chat_message_callback, client);
	g_free (cmd);
}


void
gs_client_set_connect_command (const gchar *command)
{
	if (connect_command)
		g_free (connect_command);
	connect_command = g_strdup (command ? command :
			"steam://connect/$(address)");
}

gchar *
gs_client_get_connect_command ()
{
	if (!connect_command)
		connect_command = g_strdup ("steam://connect/$(address)");
	return connect_command;
}


static gboolean
command_eval_callback (const GMatchInfo *minfo, GString *result, GsClient *client)
{
	gchar *match = g_match_info_fetch (minfo, 0);
	if (strcmp (match, "$(address)") == 0)
		g_string_append (result, client->address);
	else
		g_string_append_printf (result, "$(%s)", match);
	g_free (match);
	return FALSE;
}


void
gs_client_connect_to_game (GsClient *client)
{
	GRegex *reg = g_regex_new ("\\$\\(.+\\)", 0, 0, NULL);
	gchar *command = g_regex_replace_eval (reg, connect_command, -1, 0, 0,
			(GRegexEvalCallback) command_eval_callback, client, NULL);
	g_regex_unref (reg);
	
#if defined (G_OS_WIN32)
	int ret = (int) ShellExecute (NULL, "open", command, NULL, NULL, SW_SHOWNORMAL);
	if (ret <= 32)
		g_warning ("ShellExecute error: code %d", (int) ret);
#elif defined (G_OS_UNIX)
	GError *error = NULL;
	if (!g_app_info_launch_default_for_uri (command, NULL, &error)) {
		g_warning ("Could not run commact command: %s", error->message);
		g_error_free (error);
	}
#endif
	
	g_free (command);
}
