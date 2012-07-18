/* 
 * querier.c: querier implementation
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



#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <glib/gprintf.h>
#include <gobject/gvaluecollector.h>
#include "proto-gamespy.h"
#include "proto-gamespy3.h"
#include "proto-quake3.h"
#include "proto-source.h"
#include "querier.h"
#include "querier-private.h"
#include "utils.h"


enum {
	PROP_0,
	PROP_ADDRESS,
	PROP_PROTOCOL,
	PROP_RESET_ADDRESS
};

enum {
	SIGNAL_RESOLVE,
	SIGNAL_DETECT,
	SIGNAL_INFO_UPDATE,
	SIGNAL_PLAYER_UPDATE,
	SIGNAL_LOG,
	SIGNAL_TIMEOUT,
	SIGNAL_ERROR,
	SIGNAL_MAP_CHANGED,
	SIGNAL_GAMEID_CHANGED,
	SIGNAL_PLAYER_ONLINE,
	SIGNAL_PLAYER_OFFLINE,
	LAST_SIGNAL
};


typedef struct _GsqProtocol {
	gchar *name;
	GsqQueryFunc query;
	GsqProcessFunc process;
	GsqFreeFunc free;
} GsqProtocol;

struct _GsqQuerierPrivate {
	gchar *address;
	GsqProtocol *protocol;
	
	GString *name;
	GString *gameid;
	GString *game;
	GString *mode;
	GString *map;
	GString *version;
	gint numplayers;
	gint maxplayers;
	gboolean password;
	glong ping;
	GHashTable *extra;
	
	guint gameid_hash;
	guint map_hash;
	
	gboolean working;
	GTimer *timer;
	GCancellable *cancellable;
	GInetAddress *iaddr;
	guint16 gport;
	guint16 qport;
	gpointer pdata;
	gboolean reset_address;
	gboolean update_sinfo;
	gboolean update_plist;
	GArray *fields;
	GList *players;
	GList *newplayers;
};


static GSocket *ip4sock = NULL, *ip6sock = NULL;
static GList *servers = NULL;
static GList *protocols = NULL;
static GsqDebugFlag debug_flags = GSQ_DEBUG_NONE;
static guint signals[LAST_SIGNAL] = { 0 };

static void gsq_querier_resolve (GsqQuerier *querier);
static void gsq_querier_query (GsqQuerier *querier);
static void gsq_querier_players_updated (GsqQuerier *querier);
static gboolean gsq_socket_recveived (GSocket *socket,
		GIOCondition condition, gpointer udata);


G_DEFINE_TYPE (GsqQuerier, gsq_querier, G_TYPE_OBJECT);


static void
gsq_querier_free_player (GsqQuerier *querier, GsqPlayer *player)
{
	gint i, count = querier->priv->fields->len;
	g_free (player->name);
	for (i = 0; i < count; i++)
		g_value_unset (&player->values[i]);
	g_slice_free1 (count * sizeof (GValue), player->values);
	g_slice_free (GsqPlayer, player);
}


static void
gsq_querier_free_players (GsqQuerier *querier, GList *players)
{
	while (players) {
		gsq_querier_free_player (querier, players->data);
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
	
	gsq_querier_set_name (querier, NULL);
	gsq_querier_set_gameid (querier, NULL);
	gsq_querier_set_game (querier, NULL);
	gsq_querier_set_mode (querier, NULL);
	gsq_querier_set_map (querier, NULL);
	gsq_querier_set_version (querier, NULL);
	priv->numplayers = 0;
	priv->maxplayers = 0;
	g_hash_table_remove_all (priv->extra);
	gsq_querier_free_fields (priv->fields);
}


/* clears all the data.
	called on error, timeout, finalization */
static void
gsq_querier_reset (GsqQuerier *querier)
{
	GsqQuerierPrivate *priv = querier->priv;
	
	if (priv->reset_address) {
		if (priv->iaddr) {
			g_object_unref (priv->iaddr);
			priv->iaddr = NULL;
		}
		priv->gport = 0;
		priv->qport = 0;
	}
	
	if (priv->pdata) {
		if (priv->protocol->free)
			priv->protocol->free (querier);
		priv->pdata = NULL;
	}
	priv->protocol = NULL;
	priv->working = FALSE;
	priv->ping = 0;
	
	priv->newplayers = NULL;
	gsq_querier_players_updated (querier);
	
	/* this is called last because gsq_querier_free_player needs to know
		number of fields, gsq_querier_clear deletes it */
	gsq_querier_clear (querier);
}


static void
gsq_querier_finalize (GObject *object)
{
	GsqQuerier *querier = GSQ_QUERIER (object);
	servers = g_list_remove (servers, querier);
	
	GsqQuerierPrivate *priv = querier->priv;
	g_cancellable_cancel (priv->cancellable);
	g_object_unref (priv->cancellable);
	g_timer_destroy (priv->timer);
	gsq_querier_reset (querier);
	g_string_free (priv->name, TRUE);
	g_string_free (priv->gameid, TRUE);
	g_string_free (priv->game, TRUE);
	g_string_free (priv->mode, TRUE);
	g_string_free (priv->map, TRUE);
	g_string_free (priv->version, TRUE);
	if (priv->address)
		g_free (priv->address);
	g_hash_table_destroy (priv->extra);
	g_array_free (priv->fields, TRUE);
	
	if (G_OBJECT_CLASS (gsq_querier_parent_class)->finalize)
		G_OBJECT_CLASS (gsq_querier_parent_class)->finalize (object);
}


static void
gsq_querier_set_property (GObject *object, guint prop_id, const GValue *value,
		GParamSpec *pspec)
{
	GsqQuerier *querier = GSQ_QUERIER (object);
	GsqQuerierPrivate *priv = querier->priv;
	
	switch (prop_id) {
	case PROP_ADDRESS:
		if (priv->address)
			g_free (priv->address);
		priv->address = g_strdup (g_value_get_string (value));
		gsq_querier_set_name (querier, priv->address);
		break;
	case PROP_RESET_ADDRESS:
		priv->reset_address = g_value_get_boolean (value);
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
	GsqQuerierPrivate *priv = querier->priv;
	
	switch (prop_id) {
	case PROP_ADDRESS:
		g_value_set_string (value, priv->address);
		break;
	case PROP_PROTOCOL:
		g_value_set_string (value, gsq_querier_get_protocol (querier));
		break;
	case PROP_RESET_ADDRESS:
		g_value_set_boolean (value, priv->reset_address);
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
			g_param_spec_string ("address", "Address",
			"Host and port of the server",
			NULL, G_PARAM_WRITABLE | G_PARAM_CONSTRUCT_ONLY));
	
	g_object_class_install_property (object_class, PROP_PROTOCOL,
			g_param_spec_string ("protocol", "Protocol",
			"A protocol the querier is using",
			NULL, G_PARAM_READABLE | G_PARAM_WRITABLE));
	
	g_object_class_install_property (object_class, PROP_RESET_ADDRESS,
			g_param_spec_boolean ("reset-address", "Reset address",
			"Resolve address again on querier reset",
			FALSE, G_PARAM_READABLE | G_PARAM_WRITABLE));
	
	signals[SIGNAL_RESOLVE] = g_signal_new ("resolve",
			G_OBJECT_CLASS_TYPE (klass), G_SIGNAL_RUN_FIRST | G_SIGNAL_ACTION,
			G_STRUCT_OFFSET (GsqQuerierClass, resolve), NULL, NULL,
			g_cclosure_marshal_VOID__VOID, G_TYPE_NONE, 0);
	
	signals[SIGNAL_DETECT] = g_signal_new ("detect",
			G_OBJECT_CLASS_TYPE (klass), G_SIGNAL_RUN_FIRST | G_SIGNAL_ACTION,
			G_STRUCT_OFFSET (GsqQuerierClass, detect), NULL, NULL,
			g_cclosure_marshal_VOID__VOID, G_TYPE_NONE, 0);
	
	signals[SIGNAL_INFO_UPDATE] = g_signal_new ("info-update",
			G_OBJECT_CLASS_TYPE (klass), G_SIGNAL_RUN_FIRST | G_SIGNAL_ACTION,
			G_STRUCT_OFFSET (GsqQuerierClass, info_update), NULL, NULL,
			g_cclosure_marshal_VOID__VOID, G_TYPE_NONE, 0);
	
	signals[SIGNAL_PLAYER_UPDATE] = g_signal_new ("players-update",
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
	
	signals[SIGNAL_GAMEID_CHANGED] = g_signal_new ("gameid-changed",
			G_OBJECT_CLASS_TYPE (klass), G_SIGNAL_RUN_FIRST | G_SIGNAL_ACTION,
			G_STRUCT_OFFSET (GsqQuerierClass, gameid_changed), NULL, NULL,
			g_cclosure_marshal_VOID__VOID, G_TYPE_NONE, 0);
	
	signals[SIGNAL_MAP_CHANGED] = g_signal_new ("map-changed",
			G_OBJECT_CLASS_TYPE (klass), G_SIGNAL_RUN_FIRST | G_SIGNAL_ACTION,
			G_STRUCT_OFFSET (GsqQuerierClass, map_changed), NULL, NULL,
			g_cclosure_marshal_VOID__VOID, G_TYPE_NONE, 0);
	
	signals[SIGNAL_PLAYER_ONLINE] = g_signal_new ("player-online",
			G_OBJECT_CLASS_TYPE (klass), G_SIGNAL_RUN_FIRST | G_SIGNAL_ACTION,
			G_STRUCT_OFFSET (GsqQuerierClass, player_online), NULL, NULL,
			g_cclosure_marshal_VOID__POINTER, G_TYPE_NONE, 1, G_TYPE_POINTER);
	
	signals[SIGNAL_PLAYER_OFFLINE] = g_signal_new ("player-offline",
			G_OBJECT_CLASS_TYPE (klass), G_SIGNAL_RUN_FIRST | G_SIGNAL_ACTION,
			G_STRUCT_OFFSET (GsqQuerierClass, player_offline), NULL, NULL,
			g_cclosure_marshal_VOID__POINTER, G_TYPE_NONE, 1, G_TYPE_POINTER);
	
	klass->players_update = gsq_querier_players_updated;
}

static void
gsq_querier_init (GsqQuerier *querier)
{
	querier->priv = G_TYPE_INSTANCE_GET_PRIVATE (querier, GSQ_TYPE_QUERIER,
			GsqQuerierPrivate);
	GsqQuerierPrivate *priv = querier->priv;
	
	priv->name = g_string_sized_new (32);
	priv->gameid = g_string_sized_new (2);
	priv->game = g_string_sized_new (8);
	priv->mode = g_string_sized_new (4);
	priv->map = g_string_sized_new (16);
	priv->version = g_string_sized_new (16);
	priv->extra = g_hash_table_new_full (g_str_hash, g_str_equal, NULL, g_free);
	priv->cancellable = g_cancellable_new ();
	priv->timer = g_timer_new ();
	priv->fields = g_array_new (FALSE, FALSE, sizeof (GsqField));
	
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
	querier->priv->update_sinfo = TRUE;
}

void
gsq_querier_emit_player_update (GsqQuerier *querier)
{
	querier->priv->update_plist = TRUE;
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

const gchar *
gsq_querier_get_protocol (GsqQuerier *querier)
{
	g_return_val_if_fail (GSQ_IS_QUERIER (querier), NULL);
	return querier->priv->protocol ? querier->priv->protocol->name : NULL;
}

GInetAddress *
gsq_querier_get_iaddr (GsqQuerier *querier)
{
	g_return_val_if_fail (GSQ_IS_QUERIER (querier), NULL);
	return querier->priv->iaddr;
}

void
gsq_querier_set_name (GsqQuerier *querier, const gchar *name)
{
	g_return_if_fail (GSQ_IS_QUERIER (querier));
	g_string_truncate (querier->priv->name, 0);
	g_string_insert_len (querier->priv->name, -1,
			name && *name ? name : querier->priv->address, -1);
}

const gchar *
gsq_querier_get_name (GsqQuerier *querier)
{
	g_return_val_if_fail (GSQ_IS_QUERIER (querier), NULL);
	return querier->priv->name->str;
}

void
gsq_querier_set_gameid (GsqQuerier *querier, const gchar *gameid)
{
	g_return_if_fail (GSQ_IS_QUERIER (querier));
	GsqQuerierPrivate *priv = querier->priv;
	
	g_string_truncate (priv->gameid, 0);
	if (gameid != NULL)
		g_string_insert_len (priv->gameid, -1, gameid, -1);
}

const gchar *
gsq_querier_get_gameid (GsqQuerier *querier)
{
	g_return_val_if_fail (GSQ_IS_QUERIER (querier), NULL);
	return querier->priv->gameid->str;
}

void
gsq_querier_set_game (GsqQuerier *querier, const gchar *game)
{
	g_return_if_fail (GSQ_IS_QUERIER (querier));
	g_string_truncate (querier->priv->game, 0);
	if (game != NULL)
		g_string_insert_len (querier->priv->game, -1, game, -1);
}

const gchar *
gsq_querier_get_game (GsqQuerier *querier)
{
	g_return_val_if_fail (GSQ_IS_QUERIER (querier), NULL);
	return querier->priv->game->str;
}

void
gsq_querier_set_mode (GsqQuerier *querier, const gchar *mode)
{
	g_return_if_fail (GSQ_IS_QUERIER (querier));
	g_string_truncate (querier->priv->mode, 0);
	if (mode != NULL)
		g_string_insert_len (querier->priv->mode, -1, mode, -1);
}

const gchar *
gsq_querier_get_mode (GsqQuerier *querier)
{
	g_return_val_if_fail (GSQ_IS_QUERIER (querier), NULL);
	return querier->priv->mode->str;
}

void
gsq_querier_set_map (GsqQuerier *querier, const gchar *map)
{
	g_return_if_fail (GSQ_IS_QUERIER (querier));
	GsqQuerierPrivate *priv = querier->priv;
	
	g_string_truncate (priv->map, 0);
	if (map != NULL)
		g_string_insert_len (priv->map, -1, map, -1);
}

const gchar *
gsq_querier_get_map (GsqQuerier *querier)
{
	g_return_val_if_fail (GSQ_IS_QUERIER (querier), NULL);
	return querier->priv->map->str;
}

void
gsq_querier_set_version (GsqQuerier *querier, const gchar *version)
{
	g_return_if_fail (GSQ_IS_QUERIER (querier));
	g_string_truncate (querier->priv->version, 0);
	if (version != NULL)
		g_string_insert_len (querier->priv->version, -1, version, -1);
}

const gchar *
gsq_querier_get_version (GsqQuerier *querier)
{
	g_return_val_if_fail (GSQ_IS_QUERIER (querier), NULL);
	return querier->priv->version->str;
}

void
gsq_querier_set_numplayers (GsqQuerier *querier, gint numplayers)
{
	g_return_if_fail (GSQ_IS_QUERIER (querier));
	querier->priv->numplayers = numplayers;
}

gint
gsq_querier_get_numplayers (GsqQuerier *querier)
{
	g_return_val_if_fail (GSQ_IS_QUERIER (querier), 0);
	return querier->priv->numplayers;
}

void
gsq_querier_set_maxplayers (GsqQuerier *querier, gint maxplayers)
{
	g_return_if_fail (GSQ_IS_QUERIER (querier));
	querier->priv->maxplayers = maxplayers;
}

gint
gsq_querier_get_maxplayers (GsqQuerier *querier)
{
	g_return_val_if_fail (GSQ_IS_QUERIER (querier), 0);
	return querier->priv->maxplayers;
}

void
gsq_querier_set_password (GsqQuerier *querier, gboolean password)
{
	g_return_if_fail (GSQ_IS_QUERIER (querier));
	querier->priv->password = password;
}

gboolean
gsq_querier_get_password (GsqQuerier *querier)
{
	g_return_val_if_fail (GSQ_IS_QUERIER (querier), FALSE);
	return querier->priv->password;
}


glong
gsq_querier_get_ping (GsqQuerier *querier)
{
	g_return_val_if_fail (GSQ_IS_QUERIER (querier), 0);
	return querier->priv->ping;
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
gsq_querier_get_gport (GsqQuerier *querier)
{
	g_return_val_if_fail (GSQ_IS_QUERIER (querier), 0);
	return querier->priv->gport;
}

guint16
gsq_querier_get_qport (GsqQuerier *querier)
{
	g_return_val_if_fail (GSQ_IS_QUERIER (querier), 0);
	return querier->priv->qport;
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
	
	if (debug_flags & GSQ_DEBUG_EVENT)
		g_printf ("** resolved %s\n", querier->priv->address);
	
	gsq_querier_update (querier);
}

static void
gsq_querier_resolve (GsqQuerier *querier)
{
	GsqQuerierPrivate *priv = querier->priv;
	
	if (debug_flags & GSQ_DEBUG_EVENT)
		g_printf ("** resolving %s\n", priv->address);
	
	gchar *host = gsq_parse_address (priv->address, &priv->gport, &priv->qport);
	
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
	
	if (debug_flags & GSQ_DEBUG_OUTGOING_DATA) {
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
	GsqQuerierPrivate *priv = querier->priv;
	
	if (debug_flags & GSQ_DEBUG_EVENT)
		g_printf ("querying %s\n", priv->address);
	
	if (priv->protocol) {
		priv->protocol->query (querier);
	} else {
		GList *proto_iter = protocols;
		while (proto_iter) {
			GsqProtocol *proto = proto_iter->data;
			proto->query (querier);
			proto_iter = g_list_next (proto_iter);
		}
	}
}

void
gsq_querier_update (GsqQuerier *querier)
{
	g_return_if_fail (GSQ_IS_QUERIER (querier));
	
	if (querier->priv->working)
		return;
	
	querier->priv->working = TRUE;
	if (querier->priv->iaddr) {
		g_timer_reset (querier->priv->timer);
		gsq_querier_query (querier);
	} else {
		gsq_querier_resolve (querier);
	}
}


void
gsq_querier_add_field (GsqQuerier *querier, const gchar *name, GType type)
{
	g_return_if_fail (GSQ_IS_QUERIER (querier));
	g_return_if_fail (name != NULL);
	g_return_if_fail (querier->priv->players == NULL);
	g_return_if_fail (querier->priv->newplayers == NULL);
	
	GsqField field;
	field.name = g_strdup (name);
	field.type = type;
	g_array_append_val (querier->priv->fields, field);
}


void
gsq_querier_add_player (GsqQuerier *querier, const gchar *name, ...)
{
	guint i;
	va_list va;
	gchar *error = NULL;
	GsqQuerierPrivate *priv = querier->priv;
	
	GsqPlayer *player = g_slice_new (GsqPlayer);
	player->name = g_strdup (name);
	player->values = g_slice_alloc0 (sizeof (GValue) * priv->fields->len);
	
	va_start (va, name);
	for (i = 0; i < priv->fields->len; i++) {
		GType type = g_array_index (priv->fields, GsqField, i).type;
		G_VALUE_COLLECT_INIT (&player->values[i], type, va, 0, &error);
		if (error) {
			g_error ("Error while collecting a value: %s", error);
			g_free (error);
		}
	}
	va_end (va);
	
	priv->newplayers = g_list_prepend (priv->newplayers, player);
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
gsq_querier_remove_player_by_name (GsqQuerier *querier, GList **players,
		const gchar *name)
{
	GList *iter = *players;
	
	for (; iter; iter = iter->next) {
		GsqPlayer *player = iter->data;
		if (strcmp (player->name, name) == 0) {
			*players = g_list_delete_link (*players, iter);
			gsq_querier_free_player (querier, player);
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
		if (!gsq_querier_remove_player_by_name (querier, &oldlist, player->name))
			g_signal_emit (querier, signals[SIGNAL_PLAYER_ONLINE], 0, player);
	}
	
	for (iter = oldlist; iter; iter = iter->next) {
		player = iter->data;
		g_signal_emit (querier, signals[SIGNAL_PLAYER_OFFLINE], 0, player);
	}
	
	gsq_querier_free_players (querier, oldlist);
}


static GsqProtocol *
gsq_querier_detect (GsqQuerier *querier, guint16 qport,
		const gchar *data, gssize size)
{
	GsqQuerierPrivate *priv = querier->priv;
	
	GList *proto_iter = protocols;
	while (proto_iter) {
		GsqProtocol *protocol = proto_iter->data;
		if (protocol->process (querier, qport, data, size)) {
			priv->protocol = protocol;
			priv->qport = qport;
			if (debug_flags & GSQ_DEBUG_EVENT)
				g_printf ("detected '%s' as '%s'\n", priv->address, protocol->name);
			g_signal_emit (querier, signals[SIGNAL_DETECT], 0);
			return protocol;
		} else {
			/* protocol does not recognize this packet
				we have to clean all the variables
				protocol functions are able to change */
			gsq_querier_clear (querier);
			if (priv->pdata) {
				if (protocol->free)
					protocol->free (querier);
				priv->pdata = NULL;
			}
		}
		proto_iter = proto_iter->next;
	}
	
	return NULL;
}


static gboolean
gsq_socket_recveived (GSocket *socket, GIOCondition condition, gpointer udata)
{
	gssize length;
	gchar data[4096];
	GSocketAddress *saddr = NULL;
	
	length = g_socket_receive_from (socket, &saddr, data, 4095, NULL, NULL);
	/* certain servers may send very short spammy messages.
			assume that such messages can never have useful information */
	if (length < 5) {
		if (saddr)
			g_object_unref (saddr);
		return G_SOURCE_CONTINUE;
	}
	data[length] = 0;
	
	GInetAddress *iaddr = g_inet_socket_address_get_address ((GInetSocketAddress *) saddr);
	guint16 port = g_inet_socket_address_get_port ((GInetSocketAddress *) saddr);
	
	if (debug_flags & GSQ_DEBUG_INCOMING_DATA) {
		gchar *ip = g_inet_address_to_string (iaddr);
		g_printf ("** received %" G_GSIZE_FORMAT " bytes from %s:%d:\n",
				length, ip, port);
		gsq_print_dump (data, length);
		g_free (ip);
	}
	
	GList *iter = servers;
	while (iter) {
		GsqQuerier *querier = iter->data;
		GsqQuerierPrivate *priv = querier->priv;
		if (priv->iaddr && (priv->qport == 0 || priv->qport == port) &&
				g_inet_address_equal (priv->iaddr, iaddr)) {
			priv->update_sinfo = FALSE;
			priv->update_plist = FALSE;
			
			if (priv->protocol) {
				/* server may send odd data, ignore it */
				priv->protocol->process (querier, port, data, length);
			} else {
				/* this function sets priv->protocol,
					that's how we know if it succeeds */
				gsq_querier_detect (querier, port, data, length);
			}
			
			if (priv->protocol) {
				if (priv->update_sinfo) {
					gdouble time = g_timer_elapsed (querier->priv->timer, NULL);
					querier->priv->ping = (glong) floor (time * 1000);
					querier->priv->working = FALSE;
					
					g_signal_emit (querier, signals[SIGNAL_INFO_UPDATE], 0);
					
					guint hash = g_string_hash (priv->gameid);
					if (hash != priv->gameid_hash) {
						g_signal_emit (querier, signals[SIGNAL_GAMEID_CHANGED], 0);
						priv->gameid_hash = hash;
					}
					
					hash = g_string_hash (priv->map);
					if (hash != priv->map_hash) {
						g_signal_emit (querier, signals[SIGNAL_MAP_CHANGED], 0);
						priv->map_hash = hash;
					}
				}
				if (priv->update_plist)
					g_signal_emit (querier, signals[SIGNAL_PLAYER_UPDATE], 0);
				break;
			}
		}
		iter = iter->next;
	}
	
	g_object_unref (saddr);
	
	return G_SOURCE_CONTINUE;
}



/* Base functions */

static gboolean
check_timeout (gpointer udata)
{
	GList *iter = servers;
	while (iter) {
		GsqQuerier *querier = iter->data;
		if (querier->priv->iaddr && querier->priv->working) {
			gdouble time = g_timer_elapsed (querier->priv->timer, NULL);
			if (time >= 10) {
				g_signal_emit (querier, signals[SIGNAL_TIMEOUT], 0);
				gsq_querier_reset (querier);
			} else if (time >= 3) {
				gsq_querier_query (querier);
			}
		}
		iter = iter->next;
	}
	return TRUE;
}


gboolean
gsq_init (guint16 default_port)
{
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
		g_warning ("Could not create a IPv4 socket: %s", error->message);
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
		g_warning ("Could not create a IPv6 socket: %s", error->message);
		g_error_free (error);
	}
	
	gsq_register_protocol ("source",
			gsq_source_query,
			gsq_source_process,
			gsq_source_free);
	gsq_register_protocol ("quake3",
			gsq_quake3_query,
			gsq_quake3_process,
			NULL);
	gsq_register_protocol ("gamespy",
			gsq_gamespy_query,
			gsq_gamespy_process,
			gsq_gamespy_free);
	gsq_register_protocol ("gamespy3",
			gsq_gamespy3_query,
			gsq_gamespy3_process,
			gsq_gamespy3_free);
	
	g_timeout_add_seconds (1, check_timeout, NULL);
	
	return ip4sock != NULL && ip6sock != NULL;
}

void
gsq_fini ()
{
	g_socket_close (ip4sock, NULL);
	g_object_unref (ip4sock);
	g_socket_close (ip6sock, NULL);
	g_object_unref (ip6sock);
}

guint16
gsq_get_local_ipv4_port ()
{
	GSocketAddress *addr = g_socket_get_local_address (ip4sock, NULL);
	return g_inet_socket_address_get_port (G_INET_SOCKET_ADDRESS (addr));
}

guint16
gsq_get_local_ipv6_port ()
{
	GSocketAddress *addr = g_socket_get_local_address (ip6sock, NULL);
	return g_inet_socket_address_get_port (G_INET_SOCKET_ADDRESS (addr));
}


void
gsq_set_debug_flags (GsqDebugFlag flags)
{
	debug_flags = flags;
}

GsqDebugFlag
gsq_get_debug_flags ()
{
	return debug_flags;
}


static GsqProtocol *
gsq_find_protocol (const gchar *name)
{
	GsqProtocol *proto;
	GList *iter = protocols;
	
	while (iter) {
		proto = iter->data;
		if (strcmp (proto->name, name) == 0)
			return proto;
		iter = g_list_next (iter);
	}
	
	return NULL;
}

void
gsq_register_protocol (const gchar *name, GsqQueryFunc query_fn,
		GsqProcessFunc process_fn, GsqFreeFunc free_fn)
{
	GsqProtocol *proto = gsq_find_protocol (name);
	if (proto) {
		protocols = g_list_remove (protocols, proto);
		g_free (proto->name);
		g_free (proto);
	}
	
	proto = g_new (GsqProtocol, 1);
	proto->name = g_strdup (name);
	proto->query = query_fn;
	proto->process = process_fn;
	proto->free = free_fn;
	protocols = g_list_append (protocols, proto);
}

void
gsq_unregister_protocol (const gchar *name)
{
	GsqProtocol *proto = gsq_find_protocol (name);
	if (proto) {
		protocols = g_list_remove (protocols, proto);
		g_free (proto->name);
		g_free (proto);
	} else {
		g_warning ("Protocol `%s' not found", name);
	}
}
