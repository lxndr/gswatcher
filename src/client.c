/* 
 * client.c
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



#include <GeoIP.h>
#include "query/utils.h"
#include "gui-console.h"
#include "gui-log.h"
#include "gui-chat.h"
#include "client.h"


static GeoIP *geoip;
static GHashTable *gamelist;
static gchar *logaddress = NULL;
static GRegex *re_say, *re_player;
static gchar *connect_command = NULL;


static void gs_client_finalize (GObject *object);
static void gs_client_querier_resolved (GsqQuerier *querier, GsClient *client);
static void gs_client_querier_info_updated (GsqQuerier *querier, GsClient *client);
static void gs_client_watcher_map_changed (GsqQuerier *querier, GsClient *client);
static void gs_client_querier_log (GsqQuerier *querier, const gchar *msg, GsClient *client);
static void gs_client_console_connect (GsqConsole *console, GsClient *client);
static void gs_client_console_disconnect (GsqConsole *console, GsClient *client);


G_DEFINE_TYPE (GsClient, gs_client, G_TYPE_OBJECT);


static void
gs_client_class_init (GsClientClass *class)
{
	GObjectClass *gobject_class = G_OBJECT_CLASS (class);
	gobject_class->finalize = gs_client_finalize;
	
	geoip = GeoIP_new (GEOIP_STANDARD);
	if (!geoip)
		exit (0);
	
	gamelist = g_hash_table_new (g_str_hash,  g_str_equal);
	g_hash_table_insert (gamelist, "hl", "Half-Life: Deathmatch");
	g_hash_table_insert (gamelist, "cs", "Counter-Strike");
	g_hash_table_insert (gamelist, "tf", "Team Fortress Classic");
	g_hash_table_insert (gamelist, "dod", "Day od Defeat");
	g_hash_table_insert (gamelist, "czero", "Counter-Strike: Condition Zero");
	g_hash_table_insert (gamelist, "css", "Counter-Strike: Source");
	g_hash_table_insert (gamelist, "dods", "Day of Defeat: Source");
	g_hash_table_insert (gamelist, "hl2dm", "Half-Life 2: Deathmatch");
	g_hash_table_insert (gamelist, "tf2", "Team Fortress 2");
	g_hash_table_insert (gamelist, "l4d", "Left 4 Dead");
	g_hash_table_insert (gamelist, "l4d2", "Left 4 Dead 2");
	g_hash_table_insert (gamelist, "as", "Alien Swarm");
	g_hash_table_insert (gamelist, "zp", "Zombie Panic!");
	g_hash_table_insert (gamelist, "syn", "Synergy");
	g_hash_table_insert (gamelist, "ins", "Insurgency");
	g_hash_table_insert (gamelist, "nd", "Nuclear dawn");
	g_hash_table_insert (gamelist, "", "Source-based");
	
	re_say = g_regex_new ("^L (\\d{2}/\\d{2}/\\d{4}) - (\\d{2}:\\d{2}:\\d{2}): \"(.+)\" (say|say_team) \"(.+)\"$",
			G_REGEX_OPTIMIZE | G_REGEX_UNGREEDY, 0, NULL);
	re_player = g_regex_new ("^(.+)<\\d+><STEAM_\\d:\\d:\\d+><(.+)><.*><.+><.+><.+><.+>$",
			G_REGEX_OPTIMIZE | G_REGEX_UNGREEDY, 0, NULL);
}


static void
gs_client_init (GsClient *client)
{
}


static void
gs_client_finalize (GObject *object)
{
	GsClient *client = GS_CLIENT (object);
	
	if (client->game)
		g_free (client->game);
	if (client->version)
		g_free (client->version);
	g_object_unref (client->querier);
	
	gs_client_enable_log (client, FALSE);
	g_object_unref (client->console_buffer);
	g_object_unref (client->console_history);
	g_object_unref (client->console);
	
	g_object_unref (client->log_buffer);
	
	g_object_unref (client->chat_buffer);
	
	G_OBJECT_CLASS (gs_client_parent_class)->finalize (object);
}


GsClient*
gs_client_new (const gchar *address)
{
	g_return_val_if_fail (address != NULL, NULL);
	GsClient *client = g_object_new (GS_TYPE_CLIENT, NULL);
	
	client->querier = gsq_watcher_new (address);
	g_signal_connect (client->querier, "resolve",
			G_CALLBACK (gs_client_querier_resolved), client);
	g_signal_connect (client->querier, "info-update",
			G_CALLBACK (gs_client_querier_info_updated), client);
	g_signal_connect (client->querier, "map-change",
			G_CALLBACK (gs_client_watcher_map_changed), client);
	g_signal_connect (client->querier, "log",
			G_CALLBACK (gs_client_querier_log), client);
	gui_log_init (client);
	
	client->console = gsq_console_new (address);
	g_signal_connect (client->console, "connect",
			G_CALLBACK (gs_client_console_connect), client);
	g_signal_connect (client->console, "disconnect",
			G_CALLBACK (gs_client_console_disconnect), client);
	gs_console_init (client);
	
	gui_chat_init (client);
	
	return client;
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
	if (client->game)
		g_free (client->game);
	gchar *gamename = g_hash_table_lookup (gamelist, querier->game);
	gchar *mode = gsq_querier_get_extra (querier, "mode");
	if (G_UNLIKELY (strcmp (querier->game, "") == 0))
			client->game = g_strdup_printf ("%s (%s)", gamename,
					(gchar *) gsq_querier_get_extra (querier, "appid"));
	else
		client->game = g_strdup_printf ((mode && *mode) ?
				"%s (%s)" : "%s", gamename, mode);
	
	gchar *password = gsq_querier_get_extra (querier, "password");
	client->password = password ? strcmp (password, "true") == 0 : FALSE;
	
	if (client->version)
		g_free (client->version);
	gchar *type = gsq_querier_get_extra (querier, "type");
	client->version = g_strdup_printf (*type ? "%s (%s, %s, %s)" : "%s (%s, %s)",
			querier->version, gsq_querier_get_extra (querier, "protocol-version"),
			gsq_querier_get_extra (querier, "os"), type);
}


static void
gs_client_watcher_map_changed (GsqQuerier *querier, GsClient *client)
{
	if (client->log_auto)
		gs_client_enable_log (client, TRUE);
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
gs_client_console_connect (GsqConsole *console, GsClient* client)
{
	gs_console_log (client, GS_CONSOLE_INFO, "Connected");
}

static void
gs_client_console_disconnect (GsqConsole *console, GsClient* client)
{
	gs_console_log (client, GS_CONSOLE_INFO, "Disconnected");
}


static void
command_callback (GsqConsole *console, GAsyncResult *result, GsClient *client)
{
	GError *error = NULL;
	gchar *output = gsq_console_send_finish (console, result, &error);
	if (output)
		gs_console_log (client, GS_CONSOLE_RESPOND, output);
	else
		gs_console_log (client, GS_CONSOLE_ERROR, error->message);
}


void
gs_client_send_command (GsClient* client, const gchar *cmd)
{
	gsq_console_send (client->console, cmd,
			(GAsyncReadyCallback) command_callback, client);
}



void
gs_client_set_logaddress (const gchar *address)
{
	if (logaddress)
		g_free (logaddress);
	logaddress = g_strdup (address);
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
		gs_console_log (client, GS_CONSOLE_RESPOND, output);
	else
		gs_console_log (client, GS_CONSOLE_ERROR, error->message);
}

void
gs_client_enable_log (GsClient *client, gboolean enable)
{
	if (enable) {
		guint16 userport;
		gchar *userhost = gsq_parse_address (logaddress, &userport, NULL);
		if (!userhost) {
			gs_console_log (client, GS_CONSOLE_ERROR,
					"You have to provide a log address");
			return;
		}
		
		if (client->logaddress)
			g_free (client->logaddress);
		guint16 port = gsq_querier_get_ipv4_local_port (client->querier);
		client->logaddress = g_strdup_printf ("%s:%d", userhost, port);
		g_free (userhost);
		
		gchar *cmd = g_strdup_printf ("logaddress_add %s;log on", client->logaddress);
		gsq_console_send (client->console, cmd,
				(GAsyncReadyCallback) log_command_callback, client);
		g_free (cmd);
	} else {
		if (client->logaddress) {
			gchar *cmd = g_strdup_printf ("logaddress_del %s;log off", client->logaddress);
			gsq_console_send (client->console, cmd,
					(GAsyncReadyCallback) log_command_callback, client);
			g_free (cmd);
			
			g_free (client->logaddress);
			client->logaddress = NULL;
		}
	}
}


static void
chat_message_callback (GsqConsole *console, GAsyncResult *result, GsClient *client)
{
	GError *error = NULL;
	gchar *output = gsq_console_send_finish (console, result, &error);
	if (output)
		gs_console_log (client, GS_CONSOLE_RESPOND, output);
	else
		gs_console_log (client, GS_CONSOLE_ERROR, error->message);
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
	return connect_command;
}

static gboolean
command_eval_callback (const GMatchInfo *minfo, GString *result, GsClient *client)
{
	gchar *match = g_match_info_fetch (minfo, 0);
	if (strcmp (match, "$(address)") == 0)
		g_string_append (result, gsq_querier_get_address (client->querier));
	else
		g_string_append_printf (result, "$(%s)", match);
	g_free (match);
	return FALSE;
}

void
gs_client_connect_to_game (GsClient *client)
{
#ifdef G_OS_WIN32
	GRegex *reg = g_regex_new ("\\$\\(.+\\)", 0, 0, NULL);
	gchar *command = g_regex_replace_eval (reg, connect_command, -1, 0, 0,
			(GRegexEvalCallback) command_eval_callback, client, NULL);
	g_regex_unref (reg);
	
	int ret = (int) ShellExecute (NULL, "open", command, NULL, NULL, SW_SHOWNORMAL);
	if (ret <= 32)
		g_warning ("ShellExecute error: code %d", (int) ret);
	
	g_free (command);
#endif
}
