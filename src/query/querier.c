/* 
 * querier.c: querier implementation
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



#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <glib/gprintf.h>
#include "proto-source.h"
#include "querier.h"
#include "querier-private.h"
#include "utils.h"


enum {
	PROP_0,
	PROP_ADDRESS,
};

enum {
	SIGNAL_RESOLVE,
	SIGNAL_INFO_UPDATE,
	SIGNAL_PLAYERS_UPDATE,
	SIGNAL_LOG,
	SIGNAL_TIMEOUT,
	SIGNAL_ERROR,
	SIGNAL_PLAYER_ONLINE,
	SIGNAL_PLAYER_OFFLINE,
	LAST_SIGNAL
};

struct _GsqQuerierPrivate {
	gchar *address;
	glong ping;
	GHashTable *extra;
	
	gboolean working;
	GTimer *timer;
	GCancellable *cancellable;
	GInetAddress *iaddr;
	guint16 port;
	gpointer pdata;
	GArray *fields;
	GList *players;
	GList *newplayers;
};


static guint16 default_port = 27500;
static GSocket *ip4sock = NULL, *ip6sock = NULL;
static GList *servers = NULL;
static gboolean debug_mode = FALSE;
static guint signals[LAST_SIGNAL] = { 0 };

static void gsq_querier_resolve (GsqQuerier *querier);
static void gsq_querier_query (GsqQuerier *querier);
static void gsq_querier_players_updated (GsqQuerier *querier);
static void gsq_querier_info_updated (GsqQuerier *querier);
static gboolean gsq_socket_recveived (GSocket *socket, GIOCondition condition,
		gpointer udata);


#define gsq_safefree(ptr) if (ptr) {g_free (ptr); (ptr) = NULL;}


G_DEFINE_TYPE (GsqQuerier, gsq_querier, G_TYPE_OBJECT);


static void
gsq_querier_free_player (GsqPlayer *player)
{
	g_free (player->name);
	g_free (player->time);
	g_slice_free (GsqPlayer, player);
}


static void
gsq_querier_free_players (GList *players)
{
	while (players) {
		gsq_querier_free_player (players->data);
		players = g_list_delete_link (players, players);
	}
}

static void
gsq_querier_free_fields (GArray *fields)
{
	guint i;
	for (i = 0; i < fields->len; i++)
		g_free (g_array_index (fields, GsqField, i).name);
	g_array_set_size (fields, 0);
}


static void
gsq_querier_clear (GsqQuerier *querier)
{
	GsqQuerierPrivate *priv = querier->priv;
	
	gsq_safefree (querier->name);
	gsq_safefree (querier->game);
	gsq_safefree (querier->map);
	querier->numplayers = 0;
	querier->maxplayers = 0;
	gsq_safefree (querier->version);
	g_hash_table_remove_all (priv->extra);
	gsq_querier_free_fields (priv->fields);
	
	if (priv->iaddr) {
		g_object_unref (priv->iaddr);
		priv->iaddr = NULL;
	}
	priv->port = 0;
	
	if (priv->pdata) {
		gsq_source_free (querier);
		priv->pdata = NULL;
	}
	priv->working = FALSE;
	priv->ping = 0;
	
	priv->newplayers = NULL;
	gsq_querier_players_updated (querier);
}


static gboolean
gsq_querier_check_timeout (gpointer udata)
{
	GList *iter = servers;
	while (iter) {
		GsqQuerier *querier = iter->data;
		if (querier->priv->iaddr && querier->priv->working) {
			gdouble time = g_timer_elapsed (querier->priv->timer, NULL);
			if (time >= 10) {
				g_signal_emit (querier, signals[SIGNAL_TIMEOUT], 0);
				gsq_querier_clear (querier);
			} else if (time >= 3) {
				gsq_querier_query (querier);
			}
		}
		iter = iter->next;
	}
	return TRUE;
}


static void
gsq_querier_finalize (GObject *object)
{
	GsqQuerier *querier = GSQ_QUERIER (object);
	servers = g_list_remove (servers, querier);
	
	g_cancellable_cancel (querier->priv->cancellable);
	g_object_unref (querier->priv->cancellable);
	g_timer_destroy (querier->priv->timer);
	gsq_querier_clear (querier);
	gsq_safefree (querier->priv->address);
	g_hash_table_destroy (querier->priv->extra);
	g_array_free (querier->priv->fields, TRUE);
	
	if (G_OBJECT_CLASS (gsq_querier_parent_class)->finalize)
		G_OBJECT_CLASS (gsq_querier_parent_class)->finalize (object);
}


static void
gsq_querier_set_property (GObject *object, guint prop_id, const GValue *value,
		GParamSpec *pspec)
{
	GsqQuerier *querier = GSQ_QUERIER (object);
	
	switch (prop_id) {
	case PROP_ADDRESS:
		querier->priv->address = g_strdup (g_value_get_string (value));
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
		break;
	}
}

static void
gsq_querier_get_property (GObject *object, guint prop_id, GValue *value,
		GParamSpec *pspec)
{
	GsqQuerier *querier = GSQ_QUERIER (object);
	
	switch (prop_id) {
	case PROP_ADDRESS:
		g_value_set_string (value, querier->priv->address);
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
		break;
    }
}


static void
gsq_querier_class_init (GsqQuerierClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);
	object_class->set_property = gsq_querier_set_property;
	object_class->get_property = gsq_querier_get_property;
	object_class->finalize = gsq_querier_finalize;
	
	g_type_class_add_private (object_class, sizeof (GsqQuerierPrivate));
	
	g_object_class_install_property (object_class, PROP_ADDRESS,
			g_param_spec_string ("address", "Address", "Host and port of the server",
			NULL, G_PARAM_WRITABLE | G_PARAM_CONSTRUCT_ONLY));
	
	signals[SIGNAL_RESOLVE] = g_signal_new ("resolve",
			G_OBJECT_CLASS_TYPE (klass), G_SIGNAL_RUN_FIRST | G_SIGNAL_ACTION,
			G_STRUCT_OFFSET (GsqQuerierClass, resolve), NULL, NULL,
			g_cclosure_marshal_VOID__VOID, G_TYPE_NONE, 0);
	
	signals[SIGNAL_INFO_UPDATE] = g_signal_new ("info-update",
			G_OBJECT_CLASS_TYPE (klass), G_SIGNAL_RUN_FIRST | G_SIGNAL_ACTION,
			G_STRUCT_OFFSET (GsqQuerierClass, info_update), NULL, NULL,
			g_cclosure_marshal_VOID__VOID, G_TYPE_NONE, 0);
	
	signals[SIGNAL_PLAYERS_UPDATE] = g_signal_new ("players-update",
			G_OBJECT_CLASS_TYPE (klass), G_SIGNAL_RUN_FIRST | G_SIGNAL_ACTION,
			G_STRUCT_OFFSET (GsqQuerierClass, players_update), NULL, NULL,
			g_cclosure_marshal_VOID__VOID, G_TYPE_NONE, 0);
	
	signals[SIGNAL_LOG] = g_signal_new ("log",
			G_OBJECT_CLASS_TYPE (klass), G_SIGNAL_RUN_FIRST | G_SIGNAL_ACTION,
			G_STRUCT_OFFSET (GsqQuerierClass, log), NULL, NULL,
			g_cclosure_marshal_VOID__STRING, G_TYPE_NONE, 1, G_TYPE_STRING);
	
	signals[SIGNAL_TIMEOUT] = g_signal_new ("timeout",
			G_OBJECT_CLASS_TYPE (klass), G_SIGNAL_RUN_FIRST | G_SIGNAL_ACTION,
			G_STRUCT_OFFSET (GsqQuerierClass, timeout), NULL, NULL,
			g_cclosure_marshal_VOID__VOID, G_TYPE_NONE, 0);
	
	signals[SIGNAL_ERROR] = g_signal_new ("error",
			G_OBJECT_CLASS_TYPE (klass), G_SIGNAL_RUN_FIRST | G_SIGNAL_ACTION,
			G_STRUCT_OFFSET (GsqQuerierClass, error), NULL, NULL,
			g_cclosure_marshal_VOID__STRING, G_TYPE_NONE, 1, G_TYPE_STRING);
	
	signals[SIGNAL_PLAYER_ONLINE] = g_signal_new ("player-online",
			G_OBJECT_CLASS_TYPE (klass), G_SIGNAL_RUN_FIRST | G_SIGNAL_ACTION,
			G_STRUCT_OFFSET (GsqQuerierClass, player_online), NULL, NULL,
			g_cclosure_marshal_VOID__POINTER, G_TYPE_NONE, 1, G_TYPE_POINTER);
	
	signals[SIGNAL_PLAYER_OFFLINE] = g_signal_new ("player-offline",
			G_OBJECT_CLASS_TYPE (klass), G_SIGNAL_RUN_FIRST | G_SIGNAL_ACTION,
			G_STRUCT_OFFSET (GsqQuerierClass, player_offline), NULL, NULL,
			g_cclosure_marshal_VOID__POINTER, G_TYPE_NONE, 1, G_TYPE_POINTER);
	
	klass->info_update = gsq_querier_info_updated;
	klass->players_update = gsq_querier_players_updated;
	
	g_timeout_add_seconds (1, gsq_querier_check_timeout, NULL);
	
	GInetAddress *iaddr;
	GSocketAddress *saddr;
	GSource *source;
	GError *error = NULL;
	
	ip4sock = g_socket_new (G_SOCKET_FAMILY_IPV4, G_SOCKET_TYPE_DATAGRAM,
			G_SOCKET_PROTOCOL_UDP, &error);
	if (ip4sock) {
		if (default_port) {
			iaddr = g_inet_address_new_any (G_SOCKET_FAMILY_IPV4);
			saddr = g_inet_socket_address_new (iaddr, default_port);
			g_socket_bind (ip4sock, saddr, FALSE, NULL);
			g_object_unref (saddr);
			g_object_unref (iaddr);
		}
		
		source = g_socket_create_source (ip4sock, G_IO_IN, NULL);
		g_source_set_callback (source, (GSourceFunc) gsq_socket_recveived,
				NULL, NULL);
		g_source_attach (source, NULL);
		g_source_unref (source);
	} else {
		g_log (GSQ_LOG_DOMAIN, G_LOG_LEVEL_ERROR,
			"Could not create a IPv4 socket: %s", error->message);
		g_error_free (error);
	}
	
	ip6sock = g_socket_new (G_SOCKET_FAMILY_IPV6, G_SOCKET_TYPE_DATAGRAM,
			G_SOCKET_PROTOCOL_UDP, &error);
	if (ip6sock) {
		if (default_port) {
			iaddr = g_inet_address_new_any (G_SOCKET_FAMILY_IPV6);
			saddr = g_inet_socket_address_new (iaddr, default_port);
			g_socket_bind (ip6sock, saddr, FALSE, NULL);
			g_object_unref (saddr);
			g_object_unref (iaddr);
		}
		
		source = g_socket_create_source (ip6sock, G_IO_IN, NULL);
		g_source_set_callback (source, (GSourceFunc) gsq_socket_recveived,
				NULL, NULL);
		g_source_attach (source, NULL);
		g_source_unref (source);
	} else {
		g_log (GSQ_LOG_DOMAIN, G_LOG_LEVEL_WARNING,
				"Could not create a IPv6 socket: %s", error->message);
		g_error_free (error);
	}
}

static void
gsq_querier_init (GsqQuerier *querier)
{
	querier->priv = G_TYPE_INSTANCE_GET_PRIVATE (querier, GSQ_TYPE_QUERIER,
			GsqQuerierPrivate);
	querier->priv->extra = g_hash_table_new_full (g_str_hash, g_str_equal,
			NULL, g_free);
	querier->priv->cancellable = g_cancellable_new ();
	querier->priv->timer = g_timer_new ();
	querier->priv->fields = g_array_new (FALSE, FALSE, sizeof (GsqField));
	servers = g_list_append (servers, querier);
}


GsqQuerier*
gsq_querier_new (const gchar *address)
{
	return g_object_new (GSQ_TYPE_QUERIER, "address", address, NULL);
}


void
gsq_querier_emit_info_update (GsqQuerier *querier)
{
	g_signal_emit (querier, signals[SIGNAL_INFO_UPDATE], 0);
}

void
gsq_querier_emit_player_update (GsqQuerier *querier)
{
	g_signal_emit (querier, signals[SIGNAL_PLAYERS_UPDATE], 0);
}

void
gsq_querier_emit_log (GsqQuerier *querier, const gchar *msg)
{
	g_signal_emit (querier, signals[SIGNAL_LOG], 0, msg);
}


const gchar *
gsq_querier_get_address (GsqQuerier *querier)
{
	g_return_val_if_fail (GSQ_IS_QUERIER (querier), NULL);
	return querier->priv->address;
}

GInetAddress *
gsq_querier_get_iaddr (GsqQuerier *querier)
{
	g_return_val_if_fail (GSQ_IS_QUERIER (querier), NULL);
	return querier->priv->iaddr;
}

void
gsq_querier_set_name (GsqQuerier *querier, const gchar *value)
{
	gsq_safefree (querier->name);
	querier->name = g_strdup (value);
}

void
gsq_querier_set_game (GsqQuerier *querier, const gchar *value)
{
	gsq_safefree (querier->game);
	querier->game = g_strdup (value);
}

void
gsq_querier_set_map (GsqQuerier *querier, const gchar *value)
{
	gsq_safefree (querier->map);
	querier->map = g_strdup (value);
}

void
gsq_querier_set_version (GsqQuerier *querier, const gchar *value)
{
	gsq_safefree (querier->version);
	querier->version = g_strdup (value);
}


glong
gsq_querier_get_ping (GsqQuerier *querier)
{
	g_return_val_if_fail (GSQ_IS_QUERIER (querier), 0);
	return querier->priv->ping;
}

void
gsq_querier_set_default_port (guint16 port)
{
	default_port = port;
}

guint16
gsq_querier_get_default_port ()
{
	return default_port;
}

void
gsq_querier_set_debug_mode (gboolean enable)
{
	debug_mode = enable;
}

gboolean
gsq_querier_get_debug_mode ()
{
	return debug_mode;
}

void
gsq_querier_set_pdata (GsqQuerier *querier, gpointer pdata)
{
	g_return_if_fail (GSQ_IS_QUERIER (querier));
	querier->priv->pdata = pdata;
}

gpointer
gsq_querier_get_pdata (GsqQuerier *querier)
{
	g_return_val_if_fail (GSQ_IS_QUERIER (querier), NULL);
	return querier->priv->pdata;
}

guint16
gsq_querier_get_port (GsqQuerier *querier)
{
	g_return_val_if_fail (GSQ_IS_QUERIER (querier), 0);
	return querier->priv->port;
}

GArray *
gsq_querier_get_fields (GsqQuerier *querier)
{
	g_return_val_if_fail (GSQ_IS_QUERIER (querier), NULL);
	return querier->priv->fields;
}

GList *
gsq_querier_get_players (GsqQuerier *querier)
{
	g_return_val_if_fail (GSQ_IS_QUERIER (querier), NULL);
	return querier->priv->players;
}

void
gsq_querier_set_extra (GsqQuerier *querier, const gchar *key, const gchar *value)
{
	g_hash_table_replace (querier->priv->extra, (gchar *) key, g_strdup (value));
}

gchar *
gsq_querier_get_extra (GsqQuerier *querier, const gchar *key)
{
	return g_hash_table_lookup (querier->priv->extra, key);
}


static void
gsq_querier_resolved (GResolver *resolver, GAsyncResult *result, GsqQuerier *querier)
{
	GList *list = g_resolver_lookup_by_name_finish (resolver, result, NULL);
	querier->priv->working = FALSE;
	if (!list) {
		g_signal_emit (querier, signals[SIGNAL_ERROR], 0, "Host is unknown");
		return;
	}
	
	querier->priv->iaddr = g_object_ref (list->data);
	g_signal_emit (querier, signals[SIGNAL_RESOLVE], 0);
	g_resolver_free_addresses (list);
	gsq_querier_update (querier);
}

static void
gsq_querier_resolve (GsqQuerier *querier)
{
	guint16 port;
	gchar *host = gsq_parse_address (querier->priv->address, &port, NULL);
	
	querier->priv->port = port == 0 ? 27015 : port;
	
	GResolver *resolver = g_resolver_get_default ();
	g_resolver_lookup_by_name_async (resolver, host, querier->priv->cancellable,
			(GAsyncReadyCallback) gsq_querier_resolved, querier);
	g_free (host);
	g_object_unref (resolver);
}


void
gsq_querier_send (GsqQuerier *querier, guint16 port, const gchar *data, gsize length)
{
	GSocketAddress *addr = g_inet_socket_address_new (querier->priv->iaddr, port);
	
	if (gsq_querier_get_debug_mode ()) {
		g_printf ("** '%s' sent to port %d %"  G_GSIZE_FORMAT " bytes:\n",
				gsq_querier_get_address (querier), port, length);
		gsq_print_dump (data, length);
	}
	
	if (g_socket_address_get_family (addr) == G_SOCKET_FAMILY_IPV4)
		g_socket_send_to (ip4sock, addr, data, length, NULL, NULL);
	else
		g_socket_send_to (ip6sock, addr, data, length, NULL, NULL);
	
	g_object_unref (addr);
}


static void
gsq_querier_query (GsqQuerier *querier)
{
	gsq_source_query (querier);
}

void
gsq_querier_update (GsqQuerier *querier)
{
	g_return_if_fail (GSQ_IS_QUERIER (querier));
	
	if (querier->priv->working)
		return;
	
	if (querier->priv->iaddr) {
		querier->priv->working = TRUE;
		g_timer_reset (querier->priv->timer);
		gsq_querier_query (querier);
	} else {
		querier->priv->working = TRUE;
		gsq_querier_resolve (querier);
	}
}


void
gsq_querier_process (GsqQuerier *querier, const gchar *data, gssize size)
{
	if (!gsq_source_process (querier, data, size))
		gsq_querier_clear (querier);
}


void
gsq_querier_add_field (GsqQuerier *querier, const gchar *name, GType type)
{
	GsqField field;
	field.name = g_strdup (name);
	field.type = type;
	g_array_append_val (querier->priv->fields, field);
}


void
gsq_querier_add_player (GsqQuerier *querier, const gchar *name, gint kills,
		const gchar *time)
{
	GsqPlayer *player = g_slice_new (GsqPlayer);
	player->name = g_strdup (name);
	player->kills = kills;
	player->time = g_strdup (time);
	querier->priv->newplayers = g_list_prepend (querier->priv->newplayers, player);
}


static void
gsq_querier_info_updated (GsqQuerier *querier)
{
	gdouble time = g_timer_elapsed (querier->priv->timer, NULL);
	querier->priv->ping = (glong) floor (time * 1000);
	querier->priv->working = FALSE;
}


GsqPlayer *
gsq_querier_find_player (GsqQuerier *querier, const gchar *name)
{
	g_return_val_if_fail (GSQ_IS_QUERIER (querier), NULL);
	
	if (!name)
		return NULL;
	
	GList *iter = querier->priv->players;
	while (iter) {
		GsqPlayer *player = iter->data;
		if (strcmp (player->name, name) == 0)
			return player;
		iter = iter->next;
	}
	
	return NULL;
}


static inline gboolean
gsq_querier_remove_player_by_name (GList **players, const gchar *name)
{
	GList *iter = *players;
	
	for (; iter; iter = iter->next) {
		GsqPlayer *player = iter->data;
		if (strcmp (player->name, name) == 0) {
			*players = g_list_delete_link (*players, iter);
			gsq_querier_free_player (player);
			return TRUE;
		}
	}
	
	return FALSE;
}


static void
gsq_querier_players_updated (GsqQuerier *querier)
{
	GsqPlayer *player;
	GList *iter, *oldlist;
	
	oldlist = querier->priv->players;
	querier->priv->players = g_list_reverse (querier->priv->newplayers);
	querier->priv->newplayers = NULL;
	
	for (iter = querier->priv->players; iter; iter = iter->next) {
		player = iter->data;
		if (!gsq_querier_remove_player_by_name (&oldlist, player->name))
			g_signal_emit (querier, signals[SIGNAL_PLAYER_ONLINE], 0, player);
	}
	
	for (iter = oldlist; iter; iter = iter->next) {
		player = iter->data;
		g_signal_emit (querier, signals[SIGNAL_PLAYER_OFFLINE], 0, player);
	}
	
	gsq_querier_free_players (oldlist);
}


static gboolean
gsq_socket_recveived (GSocket *socket, GIOCondition condition, gpointer udata)
{
	gssize length;
	gchar data[4096];
	GSocketAddress *saddr = NULL;
	
	length = g_socket_receive_from (socket, &saddr, data, 4095, NULL, NULL);
	if (length == -1)
		return TRUE;
	data[length] = 0;
	
	GInetAddress *iaddr = g_inet_socket_address_get_address ((GInetSocketAddress *) saddr);
	guint16 port = g_inet_socket_address_get_port ((GInetSocketAddress *) saddr);
	
	if (debug_mode) {
		gchar *ip = g_inet_address_to_string (iaddr);
		g_printf ("** received %" G_GSIZE_FORMAT " bytes from %s:%d:\n",
				length, ip, port);
		gsq_print_dump (data, length);
		g_free (ip);
	}
	
	GList *iter = servers;
	while (iter) {
		GsqQuerier *querier = iter->data;
		if (querier->priv->port == port && querier->priv->iaddr &&
				g_inet_address_equal (querier->priv->iaddr, iaddr)) {
			gsq_querier_process (querier, data, length);
			break;
		}
		iter = iter->next;
	}
	
	g_object_unref (saddr);
	
	return TRUE;
}


guint16
gsq_querier_get_ipv4_local_port ()
{
	GSocketAddress *addr = g_socket_get_local_address (ip4sock, NULL);
	return g_inet_socket_address_get_port (G_INET_SOCKET_ADDRESS (addr));
}

guint16
gsq_querier_get_ipv6_local_port ()
{
	GSocketAddress *addr = g_socket_get_local_address (ip6sock, NULL);
	return g_inet_socket_address_get_port (G_INET_SOCKET_ADDRESS (addr));
}
