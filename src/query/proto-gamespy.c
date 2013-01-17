/* 
 * proto-gamespy.c: GameSpy query protocol implementation
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

#define MAX_PACKETS 32


typedef struct _Private {
	gint id;
	gint max;
	GPtrArray *packets;
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
	Private *priv = gsq_querier_get_pdata (querier);
	g_hash_table_unref (priv->values);
	g_ptr_array_free (priv->packets, TRUE);
	g_slice_free (Private, priv);
}


static void
gamespy_reset (Private *priv)
{
	g_hash_table_remove_all (priv->values);
	g_ptr_array_set_size (priv->packets, 0);
	priv->id = 0;
	priv->max = 0;
}


static void
add_fields (GsqQuerier *querier)
{
	const gchar *game_id = querier->gameid->str;
	
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


static gboolean
got_all_packets (Private *priv)
{
	gint i;
	
	if (priv->max == 0 || priv->packets->len < priv->max)
		return FALSE;
	
	for (i = 0; i < priv->packets->len; i++)
		if (g_ptr_array_index (priv->packets, i) == NULL)
			return FALSE;
	
	return TRUE;
}


static void
fill_info (GsqQuerier *querier, GHashTable *values)
{
	const gchar *gameid = querier->gameid->str;
	
	/* server info */
	g_string_assign (querier->name,
			g_hash_table_lookup (values, "hostname"));
	g_string_assign (querier->map,
			gsq_lookup_value (values, "maptitle", "mapname", NULL));
	g_string_assign (querier->version,
			g_hash_table_lookup (values, "gamever"));
	querier->numplayers = gsq_str2int (
			g_hash_table_lookup (values, "numplayers"));
	querier->maxplayers = gsq_str2int (
			g_hash_table_lookup (values, "maxplayers"));
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


static gboolean
parse_packet (GHashTable *info, gchar *data)
{
	gchar *f, *v, *k = data;
	if (*k++ != '\\')
		return FALSE;
	
	while (TRUE) {
		/* key */
		if ((f = strchr (k, '\\')))
			*f = '\0';
		else
			return FALSE;
		
		/* value */
		v = f + 1;
		if ((f = strchr (v, '\\')))
			*f = '\0';
		
		g_hash_table_insert (info, k, v);
		
		if (f)
			k = f + 1;
		else
			break;
	}
	
	return TRUE;
}


static gint
get_queryid (const gchar *data, gssize length, gint *qid, gint *num, gboolean *final)
{
	static GRegex *re = NULL;
	GError *error = NULL;
	
	if (!re) {
		re = g_regex_new ("(\\\\final\\\\)?\\\\queryid\\\\(\\d+)\\.(\\d+)(\\\\final\\\\)?$",
				G_REGEX_OPTIMIZE, 0, &error);
		if (!re) {
			g_warning (error->message);
			g_error_free (error);
			return -1;
		}
	}
	
	GMatchInfo *match;
	gint ret = -1, p;
	
	if (g_regex_match_full (re, data, length, length < 24 ? 0 : length - 24, 0, &match, &error)) {
		g_match_info_fetch_pos (match, 0, &ret, NULL);
		/* query id */
		g_match_info_fetch_pos (match, 2, &p, NULL);
		*qid = atoi (&data[p]);
		/* number */
		g_match_info_fetch_pos (match, 3, &p, NULL);
		gint d = atoi (&data[p]);
		if (d > 0 && d <= MAX_PACKETS)
			*num = d - 1;
		else
			ret = -1;
		/* final */
		g_match_info_fetch_pos (match, 1, &p, NULL);
		if (p == -1)
			g_match_info_fetch_pos (match, 4, &p, NULL);
		*final = p != -1;
	} else {
		if (error) {
			g_warning (error->message);
			g_error_free (error);
		}
	}
	
	g_match_info_free (match);
	return ret;
}


static gboolean
detect_game (GsqQuerier *querier, GHashTable *values)
{
	gchar *gamename = gsq_lookup_value (values, "gamename", "game_id", NULL);
	gchar *gametype = g_hash_table_lookup (values, "gametype");
	
	if (gamename && strcmp (gamename, "redorchestra") == 0) {
		if (gametype && (strcmp (gametype, "KFGameType") == 0 ||
				strcmp (gametype, "SRGameType") == 0)) {
			/* Killing Floor */
			g_string_assign (querier->gameid, "kf");
			g_string_assign (querier->gamename, "Killing Floor");
		} else {
			/* Red Orchestra OR game based on it */
			g_string_assign (querier->gameid, "ro");
			g_string_assign (querier->gamename, "Red Orchestra");
			if (gametype)
				g_string_assign (querier->gamemode, gametype);
		}
	} else if (gamename && strcmp (gamename, "ut") == 0) {
		if (gametype && strncmp (gametype, "TO", 2) == 0) {
			/* Tactical Ops */
			g_string_assign (querier->gameid, "to-aot");
			g_string_assign (querier->gamename, "Tactical Ops: Assault on Terror");
			gchar version[8];
			g_sprintf (version, "%c.%c.%c",
					gametype[2], gametype[3], gametype[4]);
			g_string_assign (querier->version, version);
		} else {
			/* Unreal Tournament */
			g_string_assign (querier->gameid, "ut");
			g_string_assign (querier->gamename, "Unreal Tournament");
			g_string_assign (querier->gamemode, gametype);
		}
	} else if (gamename && (strcmp (gamename, "bfield1942") == 0 ||
			strcmp (gamename, "bfield1942d") == 0)) {
		/* Battlefield 1942 */
		g_string_assign (querier->gameid, "bf1942");
		g_string_assign (querier->gamename, "Battlefield 1942");
	} else if (gamename && strcmp (gamename, "bfvietnam") == 0) {
		/* Battlefield Vietnam uses both GameSpy and GameSpy 2 protocols.
			We ignore GameSpy 1. */
		return FALSE;
	} else {
		/* Something else */
		g_string_assign (querier->gameid, gamename);
		g_string_assign (querier->gamename, gamename);
	}
	
	add_fields (querier);
	
	/* check if this packet belongs to the querier */
	gchar *hostport = g_hash_table_lookup (values, "hostport");
	guint16 port;
	if (!(hostport && (port = atoi (hostport))))
		return FALSE;
	guint16 gport = gsq_querier_get_gport (querier);
	if (gport > 0) {
		if (gport != port)
			return FALSE;
	} else {
		const gchar *gameid = querier->gameid->str;
		if (!((strcmp (gameid, "kf") == 0 && port == 7707) ||
				(strcmp (gameid, "ro") == 0 && port == 7757) ||
				(strcmp (gameid, "to-aot") == 0 && port == 7777) ||
				(strcmp (gameid, "ut") == 0 && port == 7777) ||
				(strcmp (gameid, "bf1942") == 0 && port == 14567)))
			return FALSE;
	}
	
	return TRUE;
}


gboolean
gsq_gamespy_process (GsqQuerier *querier, guint16 qport,
		const gchar *data, gssize size)
{
	if (size < 14 || *data != '\\')
		return FALSE;
	
	gint qid, num, i;
	gboolean final;
	size = get_queryid (data, size, &qid, &num, &final);
	if (size == -1)
		return FALSE;
	
	Private *priv = gsq_querier_get_pdata (querier);
	if (!priv) {
		priv = g_slice_new0 (Private);
		priv->packets = g_ptr_array_new_full (4, g_free);
		priv->values = g_hash_table_new (g_str_hash, g_str_equal);
		gsq_querier_set_pdata (querier, priv);
	}
	
	if (qid != priv->id) {
		gamespy_reset (priv);
		priv->id = qid;
	}
	
	if (final)
		priv->max = num + 1;
	
	if (!gsq_querier_is_detected (querier)) {
		if (num == 0) {
			gchar *tmp_data = g_strndup (data, size);
			GHashTable *tmp_table = g_hash_table_new (g_str_hash, g_str_equal);
			gboolean ret = parse_packet (tmp_table, tmp_data) &&
					detect_game (querier, tmp_table);
			g_hash_table_unref (tmp_table);
			g_free (tmp_data);
			if (ret == FALSE)
				return FALSE;
		}
	}
	
	/* store data */
	if (num >= priv->packets->len)
		g_ptr_array_set_size (priv->packets, num + 1);
	g_ptr_array_index (priv->packets, num) = g_strndup (data, size);
	
	if (got_all_packets (priv)) {
		for (i = 0; i < priv->max; i++)
			parse_packet (priv->values, g_ptr_array_index (priv->packets, i));
		fill_info (querier, priv->values);
		gamespy_reset (priv);
		gsq_querier_emit_info_update (querier);
		gsq_querier_emit_player_update (querier);
	}
	
	return TRUE;
}
