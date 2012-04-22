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
	GHashTable *info;
	gint8 max;
} Private;


const gchar *query = "\xFE\xFD\x09GS3Q";
const gsize query_length = 7;
const guint16 default_ports[] = {29900, 6500, 64000, 64100};


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
		for (i = 0; i < 4; i++)
			gsq_querier_send (querier, default_ports[i], query, query_length);
	}
}


static inline gboolean
get_sinfo (GHashTable *info, gchar **data, gchar *end)
{
	gchar *p = *data, *key = p, *value;
	
	while (*key != 0) {
		value = key + strlen (key) + 1;
		if (end < value)
			return FALSE;
		
		p = value + strlen (value) + 1;
		if (end < p)
			return FALSE;
		
		g_hash_table_replace (info, key, value);
		key = p;
	}
	
	*data = key + 1;
	return TRUE;
}

static gboolean
process_packet (Private *priv, gchar *p, gchar *end)
{
	while (p < end) {
		guint8 type = *p++;
		
		/* server info */
		if (type == 0) {
			if (!get_sinfo (priv->info, &p, end))
				return FALSE;
			
		/* player list */
		} else if (type == 1) {
			
		/* team list */
		} else if (type == 2) {
			
		/* odd stuff */
		} else {
			return FALSE;
		}
		
		if (end < p)
			return TRUE;
	}
	
	return TRUE;
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
			priv = g_slice_new (Private);
			priv->packets = g_array_sized_new (FALSE, FALSE, sizeof (Packet), 2);
			priv->info = g_hash_table_new (g_str_hash, g_str_equal);
			gsq_querier_set_pdata (querier, priv);
		}
		
		/* number of packet */
		gint8 number = data[15];
		if (number & 0x80) {
			priv->max = number;
			number = number & 0x7F;
		}
		
		Packet pkt;
		pkt.length = size - 15;
		pkt.data = g_slice_copy (pkt.length, data + 15);
		g_array_insert_val (priv->packets, number, pkt);
		
		if (!process_packet (priv, pkt.data, pkt.data + pkt.length))
			return FALSE;
		
		if (gsq_querier_get_protocol (querier) == NULL) {
			gchar *gameid = gsq_lookup_value (priv->info,
					"gamename", "p268435717", NULL);
			if (strcmp (gameid, "unreal3") == 0) {
				gsq_querier_set_gameid (querier, "ut3");
				gsq_querier_set_game (querier, "Unreal Tournament 3");
				gsq_querier_add_field (querier, N_("Score"), G_TYPE_INT);
				gsq_querier_add_field (querier, N_("Deaths"), G_TYPE_INT);
				gsq_querier_add_field (querier, N_("Team"), G_TYPE_STRING);
				gsq_querier_add_field (querier, N_("Ping"), G_TYPE_INT);
			} else if (strcmp (gameid, "battlefield2") == 0) {
				gsq_querier_set_gameid (querier, "bf2");
				gsq_querier_set_game (querier, "Battlefield 2");
				gsq_querier_add_field (querier, N_("Score"), G_TYPE_INT);
				gsq_querier_add_field (querier, N_("Deaths"), G_TYPE_INT);
				gsq_querier_add_field (querier, N_("Skill"), G_TYPE_INT);
				gsq_querier_add_field (querier, N_("Team"), G_TYPE_STRING);
				gsq_querier_add_field (querier, N_("Ping"), G_TYPE_INT);
			} else if ((strcmp (gameid, "stella") == 0 || strcmp (gameid, "stellad") == 0) &&
					strcmp ("bf2142", g_hash_table_lookup (priv->info, "gamevariant")) == 0) {
				gsq_querier_set_gameid (querier, "bf2142");
				gsq_querier_set_game (querier, "Battlefield 2142");
				gsq_querier_add_field (querier, N_("Score"), G_TYPE_INT);
				gsq_querier_add_field (querier, N_("Deaths"), G_TYPE_INT);
				gsq_querier_add_field (querier, N_("Skill"), G_TYPE_INT);
				gsq_querier_add_field (querier, N_("Team"), G_TYPE_STRING);
				gsq_querier_add_field (querier, N_("Ping"), G_TYPE_INT);				
			}
		}
		
//		if (got_all_apckets (priv)) {
//			gsq_gamespy3_fill (querier, priv);
			// TODO
//		}
		
		return TRUE;
	}
	
	return FALSE;
}
