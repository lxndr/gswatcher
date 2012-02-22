#include <GeoIP.h>
#include "query/utils.h"
#include "gui-console.h"
#include "gui-log.h"
#include "client.h"


static GeoIP *geoip;
static GHashTable *gamelist;
static gchar *logaddress;


static void gs_client_finalize (GObject *object);
static void gs_client_querier_resolved (GsqQuerier *querier, GsClient *client);
static void gs_client_querier_info_updated (GsqQuerier *querier, GsClient *client);
static void gs_client_querier_log (GsqQuerier *querier, const gchar *msg, GsClient *client);
static void gs_client_console_connect (GsqConsole *console, GsClient *client);
static void gs_client_console_disconnect (GsqConsole *console, GsClient *client);
static void gs_client_console_respond (GsqConsole *console, const gchar *msg, GsClient *client);
static void gs_client_console_error (GsqConsole *console, const gchar *msg, GsClient *client);


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
	
	// gs_client_enable_log (client, FALSE);
	g_object_unref (client->console_buffer);
	g_object_unref (client->console_history);
	g_object_unref (client->console);
	
	g_object_unref (client->log_buffer);
	
	G_OBJECT_CLASS (gs_client_parent_class)->finalize (object);
}


GsClient*
gs_client_new (const gchar *address)
{
	g_return_val_if_fail (address != NULL, NULL);
	GsClient *client = g_object_new (GS_TYPE_CLIENT, NULL);
	
	client->querier = gsq_querier_new (address);
	g_signal_connect (client->querier, "resolve",
			G_CALLBACK (gs_client_querier_resolved), client);
	g_signal_connect (client->querier, "info-update",
			G_CALLBACK (gs_client_querier_info_updated), client);
	g_signal_connect (client->querier, "log",
			G_CALLBACK (gs_client_querier_log), client);
	gui_log_init (client);
	
	client->console = gsq_console_new (address);
	g_signal_connect (client->console, "connect",
			G_CALLBACK (gs_client_console_connect), client);
	g_signal_connect (client->console, "disconnect",
			G_CALLBACK (gs_client_console_disconnect), client);
	g_signal_connect (client->console, "respond",
			G_CALLBACK (gs_client_console_respond), client);
	g_signal_connect (client->console, "error",
			G_CALLBACK (gs_client_console_error), client);
	gs_console_init (client);
	
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
gs_client_querier_log (GsqQuerier *querier, const gchar *msg, GsClient *client)
{
	gui_log_print (client, msg);
}


static void
gs_client_console_connect (GsqConsole *console, GsClient* client)
{
	gs_console_log (client, GS_CONSOLE_INFO, "Connected");
	while (client->commands) {
		gchar *command = client->commands->data;
		gsq_console_send (client->console, command);
		g_free (command);
		client->commands = g_list_delete_link (client->commands, client->commands);
	}
}

static void
gs_client_console_disconnect (GsqConsole *console, GsClient* client)
{
	while (client->commands) {
		gchar *command = client->commands->data;
		g_free (command);
		client->commands = g_list_delete_link (client->commands, client->commands);
	}
	gs_console_log (client, GS_CONSOLE_INFO, "Disconnected");
}

static void
gs_client_console_respond (GsqConsole *console, const gchar *msg, GsClient* client)
{
	gs_console_log (client, GS_CONSOLE_RESPOND, msg);
}

static void
gs_client_console_error (GsqConsole *console, const gchar *msg, GsClient* client)
{
	gs_console_log (client, GS_CONSOLE_ERROR, msg);
}

void
gs_client_send_command (GsClient* client, const gchar *cmd)
{
	if (gsq_console_is_connected (client->console)) {
		gsq_console_send (client->console, cmd);
	} else {
		if (gsq_console_get_password (client->console)) {
			gs_console_log (client, GS_CONSOLE_INFO, "Connecting...");
			client->commands = g_list_append (client->commands, g_strdup (cmd));
			gsq_console_connect (client->console);
		} else {
			gs_console_log (client, GS_CONSOLE_ERROR, "Please set a password");
		}
	}
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

void
gs_client_enable_log (GsClient *client, gboolean enable)
{
	guint16 port;
	gchar *host = gsq_parse_address (logaddress, &port, NULL);
	
	if (enable) {
		gchar *cmd = g_strdup_printf ("logaddress_add %s:%d\n", host,
				gsq_querier_get_ipv4_socket_port (client->querier));
		gsq_console_send (client->console, cmd);
		g_free (cmd);
	} else {
		gchar *cmd = g_strdup_printf ("logaddress_del %s:%d\n", host,
				gsq_querier_get_ipv4_socket_port (client->querier));
		gsq_console_send (client->console, cmd);
		g_free (cmd);
	}
	
	g_free (host);
}