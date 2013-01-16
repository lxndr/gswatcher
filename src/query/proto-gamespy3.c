/* 
 * proto-gamespy3.c: GameSpy 3 query protocol implementation
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
 *   - Battlefield 2                     |         16567  |          29900
 *   - Battlefield 2142                  |         17567  |          29900
 *   - Unreal Tournament 3               |          7777  |           6500
 *   - Crysis                            |         64000  |               
 *   - Crysis 2                          |         64100  |               
 *   - Minecraft                         |         25565  |          gport
 *   - Arma 2                            |          2302  |          gport
 * 
 */



#include <string.h>
#include <stdlib.h>
#include <glib/gi18n.h>
#include <glib/gprintf.h>
#include "querier-private.h"
#include "proto-gamespy3.h"
#include "utils.h"


typedef struct _Packet {
	gchar *data;
	gsize length;
} Packet;

typedef struct _Private {
	guint8 qid;
	GArray *packets;
	gint8 number;
	GHashTable *sinfo;
	GHashTable *plist;
	gchar *teams[2];
} Private;


static const guint16 default_ports[] = {2302, 6500, 25565, 29900, 64000, 64100};



static void
free_plist_values (GPtrArray *array)
{
	g_ptr_array_free (array, TRUE);
}


static void
free_packet_func (gpointer data)
{
	Packet *pkt = data;
	
	if (pkt->length > 0) {
		g_slice_free1 (pkt->length, pkt->data);
		pkt->data = NULL;
		pkt->length = 0;
	}
}


static void
clear_private_data (Private *priv)
{
	GHashTableIter iter;
	GPtrArray *values;
	
	/* clear server info */
	g_hash_table_remove_all (priv->sinfo);
	
	/* clear player list */
	g_hash_table_iter_init (&iter, priv->plist);
	while (g_hash_table_iter_next (&iter, NULL, (gpointer *) &values))
		g_ptr_array_set_size (values, 0);
	
	g_array_set_size (priv->packets, 0);
	priv->number = 0;
}


void
gsq_gamespy3_free (GsqQuerier *querier)
{
	Private *priv = gsq_querier_get_pdata (querier);
	
	g_hash_table_unref (priv->sinfo);
	g_hash_table_unref (priv->plist);
	
	g_array_free (priv->packets, TRUE);
	g_slice_free (Private, priv);
}


void
gsq_gamespy3_query (GsqQuerier *querier)
{
	Private *priv = gsq_querier_get_pdata (querier);
	gchar query[7];
	memcpy (query, "\xFE\xFD\x09GS3Q", 7);
	
	if (priv) {
		priv->qid++;
		query[6] = priv->qid;
	} else {
		query[6] = g_random_int_range (0, G_MAXUINT8);
	}
	
	guint16 port = gsq_querier_get_qport (querier);
	if (port > 0) {
		gsq_querier_send (querier, port, query, 7);
	} else {
		/* try default ports */
		gint i;
		port = gsq_querier_get_gport (querier);
		if (port > 0)
			gsq_querier_send (querier, port, query, 7);
		for (i = 0; i < G_N_ELEMENTS (default_ports); i++)
			gsq_querier_send (querier, default_ports[i], query, 7);
	}
}


static gchar *
get_sinfo (GHashTable *sinfo, const gchar *data, const gchar *end)
{
	gchar *p = (gchar *) data;
	
	while (p < end && *p) {
		gchar *key = gsq_get_cstring (&p);
		if (p >= end)
			break;
		
		gchar *value = gsq_get_cstring (&p);
		g_hash_table_insert (sinfo, key, value);
	}
	
	return p + 1;
}


static gchar *
get_plist (GHashTable *plist, const gchar *data, const gchar *end)
{
	gchar *p = (gchar *) data;
	
	while (p < end && *p) {
		gchar *field = gsq_get_cstring (&p);
		if (p >= end)
			break;
		
		guint8 number = gsq_get_uint8 (&p);
		
		/*  */
		GPtrArray *a = g_hash_table_lookup (plist, field);
		if (a == NULL) {
			a = g_ptr_array_new ();
			g_hash_table_insert (plist, g_strdup (field), a);
		}
		
		/* g_assert (number <= a->len); */
		g_return_val_if_fail (number <= a->len, NULL);	/* server can mess around */
		g_ptr_array_set_size (a, number);				/* start with */
		
		while (p < end && *p) {
			gchar *value = gsq_get_cstring (&p);
			g_ptr_array_add (a, value);
		}
		
		/* Unreal Tournament 3 can put empty strings after each block */
		while (p < end && *p == 0)
			p++;
	}
	
	return p + 1;
}


static gboolean
process_packet (Private *priv, const gchar *data, const gchar *end)
{
	gchar *p = (gchar *) data;
	
	while (p < end) {
		switch (*p++) {
		case 0:		/* server info */
			p = get_sinfo (priv->sinfo, p, end);
			break;
		case 1:		/* player list */
			p = get_plist (priv->plist, p, end);
			if (p == NULL)
				return TRUE;
			break;
		case 2:		/* team info */
			return TRUE;
		default:
			return FALSE;
		}
	}
	
	return TRUE;
}


static gboolean
got_all_packets (Private *priv)
{
	gint i;
	
	if (priv->number == 0 || priv->packets->len < priv->number)
		return FALSE;
	
	for (i = 0; i < priv->packets->len; i++)
		if (g_array_index (priv->packets, Packet, i).data == NULL)
			return FALSE;
	
	return TRUE;
}


static gboolean
fill_info (GsqQuerier *querier, Private *priv)
{
	gint i;
	
	/* server info */
	g_string_safe_assign (querier->name,
			gsq_lookup_value (priv->sinfo, "hostname", NULL));
	g_string_safe_assign (querier->map,
			gsq_lookup_value (priv->sinfo, "p1073741825", "mapname", "map", NULL));
	querier->numplayers = gsq_str2int (
			gsq_lookup_value (priv->sinfo, "numplayers", NULL));
	querier->maxplayers = gsq_str2int (
			gsq_lookup_value (priv->sinfo, "maxplayers", NULL));
	querier->password = gsq_str2bool (
			gsq_lookup_value (priv->sinfo, "password", "s7", NULL));
	
	if (strcmp (querier->gameid->str, "ut3") == 0) {
		const gchar *mode = gsq_lookup_value (priv->sinfo, "s32779", NULL);
		switch (gsq_str2int (mode)) {
		case 1: mode = "DM"; break;
		case 2: mode = "WAR"; break;
		case 3: mode = "VCTF"; break;
		case 4: mode = "TDM"; break;
		case 5: mode = "DUEL"; break;
		default: mode = "";
		}
		
		g_string_assign (querier->gamemode, mode);
		g_string_safe_assign (querier->version,
				gsq_lookup_value (priv->sinfo, "EngineVersion", NULL));
	} else if (strcmp (querier->gameid->str, "bf2") == 0) {
		const gchar *version = gsq_lookup_value (priv->sinfo, "gamever", NULL);
		const gchar *os = gsq_lookup_value (priv->sinfo, "bf2_os", NULL);
		const gboolean dedicated = gsq_str2bool (
				gsq_lookup_value (priv->sinfo, "bf2_dedicated", NULL));
		g_string_printf (querier->version, "%s (%s%s)", version, os,
				dedicated ? ", Dedicated" : "");
		priv->teams[0] = gsq_lookup_value (priv->sinfo, "bf2_team1", NULL);
		priv->teams[1] = gsq_lookup_value (priv->sinfo, "bf2_team2", NULL);
		
	} else if (strcmp (querier->gameid->str, "bf2142") == 0) {
		g_string_safe_assign (querier->version,
				gsq_lookup_value (priv->sinfo, "gamever", NULL));
		priv->teams[0] = gsq_lookup_value (priv->sinfo, "bf2142_team1", NULL);
		priv->teams[1] = gsq_lookup_value (priv->sinfo, "bf2142_team2", NULL);
		
	} else {
		g_string_safe_assign (querier->version,
				gsq_lookup_value (priv->sinfo, "gamever", "version", NULL));
	}
	
	/* player list */
	GPtrArray *player_list = g_hash_table_lookup (priv->plist, "player_");
	GPtrArray *score_list = g_hash_table_lookup (priv->plist, "score_");
	GPtrArray *deaths_list = g_hash_table_lookup (priv->plist, "deaths_");
	GPtrArray *skill_list = g_hash_table_lookup (priv->plist, "skill_");
	GPtrArray *team_list = g_hash_table_lookup (priv->plist, "team_");
	GPtrArray *ping_list = g_hash_table_lookup (priv->plist, "ping_");
	
	if (strcmp (querier->gameid->str, "ut3") == 0) {
		if (!(player_list && score_list && score_list->len == player_list->len &&
				deaths_list && deaths_list->len == player_list->len &&
				team_list && team_list->len == player_list->len &&
				ping_list && ping_list->len == player_list->len))
			return FALSE;
		gchar team_name[8];
		for (i = 0; i < player_list->len; i++) {
			g_snprintf (team_name, 8, "Team %d",
					gsq_str2int (g_ptr_array_index (team_list, i)) + 1);
			gsq_querier_add_player (querier, g_ptr_array_index (player_list, i),
					gsq_str2int (g_ptr_array_index (score_list, i)),
					gsq_str2int (g_ptr_array_index (deaths_list, i)),
					team_name,
					gsq_str2int (g_ptr_array_index (ping_list, i)));
		}
		
	} else if (strcmp (querier->gameid->str, "bf2142") == 0 || strcmp (querier->gameid->str, "bf2") == 0) {
		if (!(player_list && score_list && score_list->len == player_list->len &&
				deaths_list && deaths_list->len == player_list->len &&
				skill_list && skill_list->len == player_list->len &&
				team_list && team_list->len == player_list->len &&
				ping_list && ping_list->len == player_list->len))
			return FALSE;
		for (i = 0; i < player_list->len; i++)
			gsq_querier_add_player (querier, g_ptr_array_index (player_list, i),
					gsq_str2int (g_ptr_array_index (score_list, i)),
					gsq_str2int (g_ptr_array_index (deaths_list, i)),
					gsq_str2int (g_ptr_array_index (skill_list, i)),
					priv->teams[gsq_str2int (g_ptr_array_index (team_list, i)) - 1],
					gsq_str2int (g_ptr_array_index (ping_list, i)));
		
	} else if (strcmp (querier->gameid->str, "arma2") == 0) {
		if (!(player_list && score_list && score_list->len == player_list->len &&
				score_list && score_list->len == player_list->len &&
				deaths_list && deaths_list->len == player_list->len))
			return FALSE;
		for (i = 0; i < player_list->len; i++)
			gsq_querier_add_player (querier, g_ptr_array_index (player_list, i),
					gsq_str2int (g_ptr_array_index (score_list, i)),
					gsq_str2int (g_ptr_array_index (deaths_list, i)));
		
	} else {
		if (!player_list)
			return FALSE;
		for (i = 0; i < player_list->len; i++)
			gsq_querier_add_player (querier, g_ptr_array_index (player_list, i));
	}
	
	return TRUE;
}


static gboolean
detect_game (GsqQuerier *querier, Private *priv, guint16 qport,
		const gchar *data, const gchar *end)
{
	if (*data != 0)
		return FALSE;
	get_sinfo (priv->sinfo, data + 1, end);
	
	gchar *hostname = gsq_lookup_value (priv->sinfo, "hostname", NULL);
	if (!hostname)
		return FALSE;
	
	guint16 hostport = gsq_str2int (g_hash_table_lookup (priv->sinfo, "hostport"));
	gchar *gameid = gsq_lookup_value (priv->sinfo, "gamename", "game_id", NULL);
	guint16 gport = gsq_querier_get_gport (querier);
	guint16 defport = 0;
	
	if (gameid == NULL) {
		if (g_hash_table_lookup (priv->sinfo, "p268435717")) {
			g_string_assign (querier->gameid, "ut3");
			g_string_assign (querier->gamename, "Unreal Tournament 3");
			gsq_querier_add_field (querier, N_("Score"), G_TYPE_INT);
			gsq_querier_add_field (querier, N_("Deaths"), G_TYPE_INT);
			gsq_querier_add_field (querier, N_("Team"), G_TYPE_STRING);
			gsq_querier_add_field (querier, N_("Ping"), G_TYPE_INT);
			defport = 7777;
		} else {
			goto fallback;
		}
	} else if (strcmp (gameid, "battlefield2") == 0) {
		g_string_assign (querier->gameid, "bf2");
		g_string_assign (querier->gamename, "Battlefield 2");
		gsq_querier_add_field (querier, N_("Score"), G_TYPE_INT);
		gsq_querier_add_field (querier, N_("Deaths"), G_TYPE_INT);
		gsq_querier_add_field (querier, N_("Skill"), G_TYPE_INT);
		gsq_querier_add_field (querier, N_("Team"), G_TYPE_STRING);
		gsq_querier_add_field (querier, N_("Ping"), G_TYPE_INT);
		defport = 16567;
	} else if (strcmp (gameid, "stella") == 0 || strcmp (gameid, "stellad") == 0) {
		gchar *gamevariant = g_hash_table_lookup (priv->sinfo, "gamevariant");
		if (gamevariant && strcmp (gamevariant, "bf2142") == 0) {
			g_string_assign (querier->gameid, "bf2142");
			g_string_assign (querier->gamename, "Battlefield 2142");
			gsq_querier_add_field (querier, N_("Score"), G_TYPE_INT);
			gsq_querier_add_field (querier, N_("Deaths"), G_TYPE_INT);
			gsq_querier_add_field (querier, N_("Skill"), G_TYPE_INT);
			gsq_querier_add_field (querier, N_("Team"), G_TYPE_STRING);
			gsq_querier_add_field (querier, N_("Ping"), G_TYPE_INT);
			defport = 17567;
		} else {
			goto fallback;
		}
	} else if (strcmp (gameid, "MINECRAFT") == 0) {
		g_string_assign (querier->gameid, "mc");
		g_string_assign (querier->gamename, "Minecraft");
		defport = 25565;
	} else if (strcmp (gameid, "arma2oapc") == 0) {
		g_string_assign (querier->gameid, "arma2");
		g_string_assign (querier->gamename, "Arma 2");
		g_string_assign (querier->gamemode,
				g_hash_table_lookup (priv->sinfo, "gametype"));
		gsq_querier_add_field (querier, N_("Score"), G_TYPE_INT);
		gsq_querier_add_field (querier, N_("Deaths"), G_TYPE_INT);
		defport = 2302;
		/* Arma 2 does not send hostport. qport and gport are same. */
		hostport = qport;
	} else {
fallback:
		g_string_assign (querier->gameid, gameid);
		g_string_assign (querier->gamename, gameid);
	}
	
	if (gport == 0)
		gport = defport;
	if (gport > 0 && hostport == gport) {
		gsq_querier_set_gport (querier, gport);
		return TRUE;
	}
	
	return FALSE;
}


gboolean
gsq_gamespy3_process (GsqQuerier *querier, guint16 qport, const gchar *data,
		gssize size)
{
	if (strncmp (data + 1, "GS3", 3) != 0)
		return FALSE;
	
	Private *priv = gsq_querier_get_pdata (querier);
	guint8 queryid = data[4];
	if (priv && priv->qid != queryid)
		return FALSE;
	
	guint8 type = *data;
	
	/* challenge */
	if (type == 9) {
		if (size < 7)
			return FALSE;
		gchar query[15];
		memcpy (query, "\xFE\xFD\x00GS3 \xFF\xFF\xFF\xFF\xFF\xFF\xFF\x01", 15);
		query[6] = queryid;
		gint32 challenge = GINT32_TO_BE (atoi (data + 5));
		if (challenge)
			memcpy (query + 7, &challenge, 4);
		if (priv)
			clear_private_data (priv);
		gsq_querier_send (querier, qport, query, 15);
		
	/* server info */
	} else if (type == 0) {
		/* splitnum\x00 */
		if (size < 18 || strncmp (data + 5, "splitnum", 9) != 0)
			return FALSE;
		
		/* create private data */
		Private *priv = gsq_querier_get_pdata (querier);
		if (priv == NULL) {
			priv = g_slice_new0 (Private);
			priv->qid = queryid;
			priv->packets = g_array_sized_new (FALSE, TRUE, sizeof (Packet), 2);
			g_array_set_clear_func (priv->packets, free_packet_func);
			priv->sinfo = g_hash_table_new (g_str_hash, g_str_equal);
			priv->plist = g_hash_table_new_full (g_str_hash, g_str_equal,
					g_free, (GDestroyNotify) free_plist_values);
			gsq_querier_set_pdata (querier, priv);
		}
		
		/* number of packet */
		gint8 number = data[14];
		if (number & 0x80) {
			number = number & 0x7F;
			priv->number = number + 1;
		}
		
		if (!gsq_querier_is_detected (querier))
			if (!detect_game (querier, priv, qport, data + 15, data + size))
				return FALSE;
		
		/* store data */
		if (number >= priv->packets->len)
			g_array_set_size (priv->packets, number + 1);
		Packet *pkt = &g_array_index (priv->packets, Packet, number);
		pkt->length = size - 15;
		pkt->data = g_slice_copy (pkt->length, data + 15);
		
		if (got_all_packets (priv)) {
			gint i;
			for (i = 0; i < priv->number; i++) {
				Packet *pkt = &g_array_index (priv->packets, Packet, i);
				process_packet (priv, pkt->data, pkt->data + pkt->length);
			}
			
			fill_info (querier, priv);
			gsq_querier_emit_info_update (querier);
			gsq_querier_emit_player_update (querier);
		}
		
		return TRUE;
	}
	
	return FALSE;
}
