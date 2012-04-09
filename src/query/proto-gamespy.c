/* 
 * proto-gamespy.c: GameSpy query protocol implementation
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
 * 
 * Supported games:  --------------------+- def. port  ---+- query port  ---
 *   - Red Orchestra                     |          7757  |     gport + 10
 *   - Killing Floor                     |          7707  |     gport + 10
 *   - Unreal Tournament                 |          7777  |     gport +  1
 *   - Tactical Ops: Assault on Terror   |          7777  |     gport +  1
 *   - Battlefield 1942                  |         14567  |          23000
 * 
 */



#include <string.h>
#include <stdlib.h>
#include <glib/gprintf.h>
#include <glib/gi18n.h>
#include "querier-private.h"
#include "proto-gamespy.h"
#include "utils.h"


typedef struct _Packet {
	gchar *data;
	gsize length;
} Packet;

typedef struct _Private {
	gint id;
	gint max;
	GArray *packets;
	GHashTable *values;
} Private;


void
gsq_gamespy_query (GsqQuerier *querier)
{
	static const gchar *query = "\\basic\\\\info\\\\players\\";
	static const gsize query_length = 22;
	
	guint16 port = gsq_querier_get_qport (querier);
	if (port > 0) {
		gsq_querier_send (querier, port, query, query_length);
	} else {
		/* try default ports */
		port = gsq_querier_get_gport (querier);
		if (port > 0) {
			gsq_querier_send (querier, port + 1, query, query_length);
			gsq_querier_send (querier, port + 10, query, query_length);
		} else {
			gsq_querier_send (querier, 7778, query, query_length);
			gsq_querier_send (querier, 7717, query, query_length);
			gsq_querier_send (querier, 7767, query, query_length);
		}
		gsq_querier_send (querier, 23000, query, query_length);
	}
}


void
gsq_gamespy_free (GsqQuerier *querier)
{
	gint i;
	Private *priv = gsq_querier_get_pdata (querier);
	g_hash_table_destroy (priv->values);
	for (i = 0; i < priv->packets->len; i++) {
		Packet *pkt = &g_array_index (priv->packets, Packet, i);
		g_slice_free1 (pkt->length, pkt->data);
	}
	g_array_free (priv->packets, TRUE);
	g_slice_free (Private, priv);
}


static void
gamespy_reset (Private *priv)
{
	gint i;
	for (i = 0; i < priv->packets->len; i++) {
		Packet *pkt = &g_array_index (priv->packets, Packet, i);
		if (pkt->data)
			g_slice_free1 (pkt->length, pkt->data);
	}
	g_array_set_size (priv->packets, 0);
	priv->id = 0;
	priv->max = 0;
}


static void
gamespy_add_fields (GsqQuerier *querier)
{
	gchar *game_id = gsq_querier_get_gameid (querier);
	
	gsq_querier_add_field (querier, N_("Frags"), G_TYPE_INT);
	if (strcmp (game_id, "kf") == 0) {
		gsq_querier_add_field (querier, N_("Ping"), G_TYPE_INT);
	} else if (strcmp (game_id, "bf1942") == 0) {
		gsq_querier_add_field (querier, N_("Deaths"), G_TYPE_INT);
		gsq_querier_add_field (querier, N_("Score"), G_TYPE_INT);
		gsq_querier_add_field (querier, N_("Ping"), G_TYPE_INT);
		gsq_querier_add_field (querier, N_("Team"), G_TYPE_STRING);
	} else if (strcmp (game_id, "ut") == 0 || strcmp (game_id, "to-aot") == 0 ||
			strcmp (game_id, "ro") == 0) {
		gsq_querier_add_field (querier, N_("Ping"), G_TYPE_INT);
		gsq_querier_add_field (querier, N_("Team"), G_TYPE_STRING);
	} else {
		gsq_querier_add_field (querier, N_("Ping"), G_TYPE_INT);
	}
}


inline gchar *
get_team_name_to (gint num)
{
	switch (num) {
	case 0: return N_("Special Forces");
	case 1: return N_("Terrorists");
	default: return N_("Spectators");
	}
}


static void
gamespy_fill (GsqQuerier *querier, GHashTable *values)
{
	gchar *gameid = gsq_querier_get_gameid (querier);
	
	/* server info */
	gsq_querier_set_name (querier,
			g_hash_table_lookup (values, "hostname"));
	gsq_querier_set_map (querier,
			gsq_lookup_value (values, "maptitle", "mapname", NULL));
	gsq_querier_set_version (querier,
			g_hash_table_lookup (values, "gamever"));
	gsq_querier_set_numplayers (querier, gsq_str2int (
			g_hash_table_lookup (values, "numplayers")));
	gsq_querier_set_maxplayers (querier, gsq_str2int (
			g_hash_table_lookup (values, "maxplayers")));
	gsq_querier_emit_info_update (querier);
	
	/* player list */
	gint i;
	gchar key[32], key2[32];
	for (i = 0;; i++) {
		g_sprintf (key, "player_%d", i);
		g_sprintf (key2, "playername_%d", i);
		gchar *player = gsq_lookup_value (values, key, key2, NULL);
		if (!player)
			break;
		
		g_sprintf (key, "frags_%d", i);
		g_sprintf (key2, "kills_%d", i);
		gchar *frags = gsq_lookup_value (values, key, key2, NULL);
		if (!frags)
			break;
		
		g_sprintf (key, "ping_%d", i);
		gchar *ping = g_hash_table_lookup (values, key);
		if (!ping)
			break;
		
		if (strcmp (gameid, "kf") == 0) {
			gsq_querier_add_player (querier, player, atoi (frags), atoi (ping));
		} else if (strcmp (gameid, "bf1942") == 0) {
			g_sprintf (key, "deaths_%d", i);
			gchar *deaths = g_hash_table_lookup (values, key);
			if (!deaths)
				break;
			
			g_sprintf (key, "score_%d", i);
			gchar *score = g_hash_table_lookup (values, key);
			if (!score)
				break;
			
			g_sprintf (key, "team_%d", i);
			gchar *team = g_hash_table_lookup (values, key);
			if (!team)
				break;
			
			gsq_querier_add_player (querier, player, atoi (frags), atoi (deaths),
					atoi (score), atoi (ping), team);
		} else if (strcmp (gameid, "ut") == 0 || strcmp (gameid, "to-aot") == 0 ||
				strcmp (gameid, "ro") == 0) {
			g_sprintf (key, "team_%d", i);
			gchar *team = g_hash_table_lookup (values, key);
			if (!team)
				break;
			
			gsq_querier_add_player (querier, player, atoi (frags), atoi (ping), team);
		} else {
			gsq_querier_add_player (querier, player, atoi (frags), atoi (ping));
		}
	}
	
	gsq_querier_emit_player_update (querier);
}


gboolean
gsq_gamespy_process (GsqQuerier *querier, guint16 qport,
		const gchar *data, gssize size)
{
	Private *priv = gsq_querier_get_pdata (querier);
	if (!priv) {
		/* initialize private data */
		priv = g_slice_new0 (Private);
		priv->packets = g_array_sized_new (FALSE, TRUE, sizeof (Packet), 4);
		priv->values = g_hash_table_new (g_str_hash, g_str_equal);
		gsq_querier_set_pdata (querier, priv);
	}
	
	/* copy data */
	Packet pkt;
	pkt.length = size + 1;
	pkt.data = g_slice_alloc (pkt.length);
	memcpy (pkt.data, data, size);
	pkt.data[size] = '\0';
	
	/* remove special keys */
	g_hash_table_remove (priv->values, "queryid");
	g_hash_table_remove (priv->values, "final");
	
	/* parse data */
	gchar *f, *v, *k = pkt.data;
	if (*k++ != '\\')
		goto error;
	
	while (TRUE) {
		/* key */
		f = strchr (k, '\\');
		if (f)
			*f = '\0';
		else
			break;
		
		/* value */
		v = f + 1;
		f = strchr (v, '\\');
		if (f)
			*f = '\0';
		
		g_hash_table_replace (priv->values, k, v);
		
		if (f)
			k = f + 1;
		else
			break;
	}
	
	/* found out if this is a new packet set */
	gchar *queryid = g_hash_table_lookup (priv->values, "queryid");
	if (!queryid)
		goto error;
	f = strchr (queryid, '.');
	if (!f)
		goto error;
	gint id = atoi (queryid);
	gint num = atoi (f + 1);
	
	if (id != priv->id) {
		gamespy_reset (priv);
		priv->id = id;
	}
	
	/* found out how many packets there is in the set */
	if (g_hash_table_lookup (priv->values, "final"))
		priv->max = num;
	
	gchar *gameid = gsq_querier_get_gameid (querier);
	if (!*gameid) {
		/* while protocol is being detected, we only need the first packet */
		if (num != 1)
			goto error;
		
		/* determine game */
		gchar *gamename = g_hash_table_lookup (priv->values, "gamename");
		gchar *gametype = g_hash_table_lookup (priv->values, "gametype");
		
		if (gamename && strcmp (gamename, "redorchestra") == 0) {
			if (gametype && (strcmp (gametype, "KFGameType") == 0 ||
					strcmp (gametype, "SRGameType") == 0)) {
				/* Killing Floor */
				gsq_querier_set_gameid (querier, "kf");
				gsq_querier_set_game (querier, "Killing Floor");
			} else {
				/* Red Orchestra OR game based on it */
				gsq_querier_set_gameid (querier, "ro");
				gsq_querier_set_game (querier, "Red Orchestra");
				if (gametype)
					gsq_querier_set_mode (querier, gametype);
			}
		} else if (gamename && strcmp (gamename, "ut") == 0) {
			if (gametype && strncmp (gametype, "TO", 2) == 0) {
				/* Tactical Ops */
				gsq_querier_set_gameid (querier, "to-aot");
				gsq_querier_set_game (querier, "Tactical Ops: Assault on Terror");
				gchar version[8];
				g_sprintf (version, "%c.%c.%c",
						gametype[2], gametype[3], gametype[4]);
				gsq_querier_set_version (querier, version);
			} else {
				/* Unreal Tournament */
				gsq_querier_set_gameid (querier, "ut");
				gsq_querier_set_game (querier, "Unreal Tournament");
				gsq_querier_set_mode (querier, gametype);
			}
		} else if (gamename && (strcmp (gamename, "bfield1942") == 0 ||
				strcmp (gamename, "bfield1942d") == 0)) {
			/* Battlefield 1942 */
			gsq_querier_set_gameid (querier, "bf1942");
			gsq_querier_set_game (querier, "Battlefield 1942");
		} else {
			/* Something else */
			gsq_querier_set_gameid (querier, gamename);
			gsq_querier_set_game (querier, gamename);
		}
		
		gamespy_add_fields (querier);
		
		/* check if this packet belongs to the querier */
		gchar *hostport = g_hash_table_lookup (priv->values, "hostport");
		guint16 port;
		if (!(hostport && (port = atoi (hostport))))
			goto error;
		guint16 gport = gsq_querier_get_gport (querier);
		if (gport > 0) {
			if (gport != port)
				goto error;
		} else {
			gameid = gsq_querier_get_gameid (querier);
			if (!((strcmp (gameid, "kf") == 0 && port == 7707) ||
					(strcmp (gameid, "ro") == 0 && port == 7757) ||
					(strcmp (gameid, "to-aot") == 0 && port == 7777) ||
					(strcmp (gameid, "ut") == 0 && port == 7777) ||
					(strcmp (gameid, "bf1942") == 0 && port == 14567)))
				goto error;
		}
	}
	
	/* check if we got all the packets */
	g_array_append_val (priv->packets, pkt);
	if (priv->max > 0 && priv->packets->len == priv->max) {
		gamespy_fill (querier, priv->values);
		gamespy_reset (priv);
	}
	
	return TRUE;
	
error:
	g_slice_free1 (pkt.length, pkt.data);
	return FALSE;
}
