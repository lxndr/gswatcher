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
	GArray *packets;
	GHashTable *sinfo;
	GHashTable *plist;
	gint8 number;
} Private;


const gchar *query = "\xFE\xFD\x09GS3Q";
const gsize query_length = 7;
const guint16 default_ports[] = {6500, 25565, 29900, 64000, 64100};


void
gsq_gamespy3_free (GsqQuerier *querier)
{
}


void
gsq_gamespy3_query (GsqQuerier *querier)
{
	guint16 port = gsq_querier_get_qport (querier);
	if (port > 0) {
		gsq_querier_send (querier, port, query, query_length);
	} else {
		/* try default ports */
		gint i;
		port = gsq_querier_get_gport (querier);
		if (port > 0)
			gsq_querier_send (querier, port, query, query_length);
		for (i = 0; i < G_N_ELEMENTS (default_ports); i++)
			gsq_querier_send (querier, default_ports[i], query, query_length);
	}
}


static inline gboolean
get_sinfo (GHashTable *sinfo, gchar **data, gchar *end)
{
	gchar *p = *data;
	
	while (*p && p < end) {
		gchar *key = gsq_get_cstring (&p);
		if (p >= end)
			break;
		
		gchar *value = gsq_get_cstring (&p);
		g_hash_table_replace (sinfo, key, value);
	}
	
	*data = p + 1;
	return TRUE;
}


static inline gboolean
get_plist (GHashTable *plist, gchar **data, gchar *end)
{
	gchar *p = *data;
	
	while (*p && p < end) {
		gchar *field = gsq_get_cstring (&p);
		guint8 number = gsq_get_uint8 (&p);
		
		GPtrArray *a = g_hash_table_lookup (plist, field);
		if (!a) {
			a = g_ptr_array_new ();
			g_hash_table_insert (plist, field, a);
		}
		g_ptr_array_set_size (a, number);
		
		while (*p && p < end) {
			gchar *value = gsq_get_cstring (&p);
			g_ptr_array_add (a, value);
		}
		
		p++;
	}
	
	*data = p + 1;
	return TRUE;
}


static gboolean
process_packet (Private *priv, gchar *p, gchar *end)
{
	while (p < end) {
		guint8 type = *p++;
		
		/* server info */
		if (type == 0) {
			if (!get_sinfo (priv->sinfo, &p, end))
				return FALSE;
		/* player list */
		} else if (type == 1) {
			if (!get_plist (priv->plist, &p, end))
				return FALSE;
		} else {
			return TRUE;
		}
		
		if (end < p)
			return TRUE;
	}
	
	return TRUE;
}


static gboolean
got_all_packets (Private *priv)
{
	if (priv->number == 0 || priv->packets->len < priv->number)
		return FALSE;
	
	gint i;
	for (i = 0; i < priv->packets->len; i++)
		if (g_array_index (priv->packets, Packet, i).length == 0)
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
	g_string_safe_assign (querier->version,
			gsq_lookup_value (priv->sinfo, "gamever", "version", NULL));
	querier->password = gsq_str2bool (
			gsq_lookup_value (priv->sinfo, "password", "s7", NULL));
	
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
			g_snprintf (team_name, 8, "Team %s", (gchar *) g_ptr_array_index (team_list, i));
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
					g_ptr_array_index (team_list, i),
					gsq_str2int (g_ptr_array_index (ping_list, i)));
		
	} else {
		if (!player_list)
			return FALSE;
		for (i = 0; i < player_list->len; i++)
			gsq_querier_add_player (querier, g_ptr_array_index (player_list, i));
	}
	
	return TRUE;
}


static void
clear_packets (Private *priv)
{
	gint i;
	
	for (i = 0; i < priv->packets->len; i++) {
		Packet *pkt = &g_array_index (priv->packets, Packet, i);
		if (pkt->data)
			g_slice_free1 (pkt->length, pkt->data);
	}
	
	g_array_set_size (priv->packets, 0);
}


gboolean
gsq_gamespy3_process (GsqQuerier *querier, guint16 qport, const gchar *data,
		gssize size)
{
	guint8 type = *data;
	if (strncmp (data + 1, "GS3Q", 4) != 0)
		return FALSE;
	
	/* challange */
	if (type == 9) {
		if (size < 8)
			return FALSE;
		gchar query[15];
		memcpy (query, "\xFE\xFD\x00GS3Q\xFF\xFF\xFF\xFF\xFF\xFF\xFF\x01", 15);
		gint32 challenge = GINT32_TO_BE (atoi (data + 5));
		if (challenge)
			memcpy (query + 7, &challenge, 4);
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
			priv->packets = g_array_sized_new (FALSE, TRUE, sizeof (Packet), 2);
			priv->sinfo = g_hash_table_new (g_str_hash, g_str_equal);
			priv->plist = g_hash_table_new (g_str_hash, g_str_equal);
			gsq_querier_set_pdata (querier, priv);
		}
		
		/* number of packet */
		gint8 number = data[14];
		if (number & 0x80) {
			number = number & 0x7F;
			priv->number = number + 1;
		}
		
		/* store data */
		if (number >= priv->packets->len)
			g_array_set_size (priv->packets, number + 1);
		Packet *pkt = &g_array_index (priv->packets, Packet, number);
		pkt->length = size - 15;
		pkt->data = g_slice_copy (pkt->length, data + 15);
		
		/* process packet's data */
		if (!process_packet (priv, pkt->data, pkt->data + pkt->length))
			return FALSE;
		
		if (gsq_querier_get_protocol (querier) == NULL) {
			const gchar *gameid = gsq_lookup_value (priv->sinfo,
					"gamename", "game_id", NULL);			
			if (gameid && strcmp (gameid, "battlefield2") == 0) {
				g_string_assign (querier->gameid, "bf2");
				g_string_assign (querier->gamename, "Battlefield 2");
				gsq_querier_add_field (querier, N_("Score"), G_TYPE_INT);
				gsq_querier_add_field (querier, N_("Deaths"), G_TYPE_INT);
				gsq_querier_add_field (querier, N_("Skill"), G_TYPE_INT);
				gsq_querier_add_field (querier, N_("Team"), G_TYPE_STRING);
				gsq_querier_add_field (querier, N_("Ping"), G_TYPE_INT);
			} else if (gameid && (strcmp (gameid, "stella") == 0 || strcmp (gameid, "stellad") == 0)) {
				gchar *gamevariant = g_hash_table_lookup (priv->sinfo, "gamevariant");
				if (gamevariant && strcmp (gamevariant, "bf2142") == 0) {
					g_string_assign (querier->gameid, "bf2142");
					g_string_assign (querier->gamename, "Battlefield 2142");
					gsq_querier_add_field (querier, N_("Score"), G_TYPE_INT);
					gsq_querier_add_field (querier, N_("Deaths"), G_TYPE_INT);
					gsq_querier_add_field (querier, N_("Skill"), G_TYPE_INT);
					gsq_querier_add_field (querier, N_("Team"), G_TYPE_STRING);
					gsq_querier_add_field (querier, N_("Ping"), G_TYPE_INT);
				} else {
					g_string_assign (querier->gameid, gameid);
					g_string_assign (querier->gamename, gameid);
				}
			} else if (gameid && strcmp (gameid, "MINECRAFT") == 0) {
				g_string_assign (querier->gameid, "mc");
				g_string_assign (querier->gamename, "Minecraft");
			} else {
				if (g_hash_table_lookup (priv->sinfo, "p268435717")) {
					g_string_assign (querier->gameid, "ut3");
					g_string_assign (querier->gamename, "Unreal Tournament 3");
					gsq_querier_add_field (querier, N_("Score"), G_TYPE_INT);
					gsq_querier_add_field (querier, N_("Deaths"), G_TYPE_INT);
					gsq_querier_add_field (querier, N_("Team"), G_TYPE_STRING);
					gsq_querier_add_field (querier, N_("Ping"), G_TYPE_INT);
				} else {
					g_string_assign (querier->gameid, gameid);
					g_string_assign (querier->gamename, gameid);
				}
			}
		}
		
		if (got_all_packets (priv)) {
			gint i;
			for (i = 0; i < priv->number; i++) {
				Packet *pkt = &g_array_index (priv->packets, Packet, i);
				process_packet (priv, pkt->data, pkt->data + pkt->length);
			}
			
			fill_info (querier, priv);
			clear_packets (priv);
			gsq_querier_emit_info_update (querier);
			gsq_querier_emit_player_update (querier);
		}
		
		return TRUE;
	}
	
	return FALSE;
}
