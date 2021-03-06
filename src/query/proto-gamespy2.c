/* 
 * proto-gamespy2.c: GameSpy 2 query protocol implementation
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
 *   - America's Army                    |          1716  |     gport +  1
 *   - Armed Assault                     |          2302  |           2302
 *   - Battlefield Vietman               |         23000  |          23000
 * 
 */



#include <string.h>
#include <stdlib.h>
#include <glib/gprintf.h>
#include <glib/gi18n.h>
#include "querier-private.h"
#include "utils.h"


typedef struct _Private {
	GHashTable *sinfo;
} Private;


static const guint16 default_qports[] = {1717, 2302, 23000};


void
gsq_gamespy2_query (GsqQuerier *querier)
{
	const gchar *sinfo_query = "\xFE\xFD\x00GS2s\xFF\x00\x00";
	const gchar *plist_query = "\xFE\xFD\x00GS2p\x00\xFF\x00";
	
	guint16 port = gsq_querier_get_qport (querier);
	if (port > 0) {
		gsq_querier_send (querier, port, sinfo_query, 10);
		gsq_querier_send (querier, port, plist_query, 10);
	} else {
		/* try default ports */
		gint i;
		port = gsq_querier_get_gport (querier);
		if (port > 0)
			gsq_querier_send (querier, port + 1, sinfo_query, 10);
		for (i = 0; i < G_N_ELEMENTS (default_qports); i++)
			gsq_querier_send (querier, default_qports[i], sinfo_query, 10);
	}
}


void
gsq_gamespy2_free (GsqQuerier *querier)
{
//	Private *priv = gsq_querier_get_pdata (querier);
//	g_hash_table_unref (priv->sinfo);
}


static void
parse_sinfo (GHashTable *sinfo, const gchar *data, gssize size)
{
	gchar *p = (gchar *) data;
	
	while (TRUE) {
		gchar *key = gsq_get_cstring (&p);
		if (!*key || p > data + size)
			break;
		gchar *value = gsq_get_cstring (&p);
		g_hash_table_insert (sinfo, key, value);
	}
}


static gboolean
detect_game (GsqQuerier *querier, GHashTable *sinfo)
{
	guint16 port;
	gint hostport = gsq_str2int (g_hash_table_lookup (sinfo, "hostport"));
	gchar *gameid = gsq_lookup_value (sinfo, "gamename", "game_id", NULL);
	
	if (strcmp (gameid, "armygame") == 0) {
		port = 1716;
		g_string_assign (querier->gameid, "aa");
		g_string_assign (querier->gamename, "America's Army");
		gsq_querier_add_field (querier, N_("Leader"), G_TYPE_INT);
		gsq_querier_add_field (querier, N_("Goal"), G_TYPE_INT);
		gsq_querier_add_field (querier, N_("Honor"), G_TYPE_INT);
		gsq_querier_add_field (querier, N_("Score"), G_TYPE_INT);
		gsq_querier_add_field (querier, N_("Ping"), G_TYPE_INT);
	} else if (strcmp (gameid, "bfvietnam") == 0) {
		port = 15567;
		g_string_assign (querier->gameid, "bfv");
		g_string_assign (querier->gamename, "Battlefield: Vietnam");
		gsq_querier_add_field (querier, N_("Kills"), G_TYPE_INT);
		gsq_querier_add_field (querier, N_("Deaths"), G_TYPE_INT);
		gsq_querier_add_field (querier, N_("Score"), G_TYPE_INT);
		gsq_querier_add_field (querier, N_("Ping"), G_TYPE_INT);
		gsq_querier_add_field (querier, N_("Team"), G_TYPE_STRING);
	} else if (strcmp (gameid, "armedass") == 0) {
		port = 2302;
		g_string_assign (querier->gameid, "arma");
		g_string_assign (querier->gamename, "Arma");
		gsq_querier_add_field (querier, N_("Score"), G_TYPE_INT);
		gsq_querier_add_field (querier, N_("Deaths"), G_TYPE_INT);
	} else if (strcmp (gameid, "arma2oapc") == 0) {
		return FALSE;
	} else {
		port = hostport;
		g_string_assign (querier->gameid, gameid);
		g_string_assign (querier->gamename, gameid);
	}
	
	if (hostport > 0) {
		guint16 gport = gsq_querier_get_gport (querier);
		if (gport > 0)
			port = gport;
		if (port != hostport)
			return FALSE;
		gsq_querier_set_gport (querier, gport);
	}
	
	return TRUE;
}


gboolean
gsq_gamespy2_process (GsqQuerier *querier, guint16 qport,
		const gchar *data, gssize size)
{
	if (memcmp (data, "\x00GS2", 4) != 0)
		return FALSE;
	
	if (data[4] == 's') {
		GHashTable *sinfo = g_hash_table_new (g_str_hash, g_str_equal);
		parse_sinfo (sinfo, data + 5, size);
		if (g_hash_table_size (sinfo) == 0) {
			g_hash_table_unref (sinfo);
			return FALSE;
		}
		
		if (!gsq_querier_is_detected (querier)) {
			if (!detect_game (querier, sinfo)) {
				g_hash_table_unref (sinfo);
				return FALSE;
			}
		}
		
		g_string_safe_assign (querier->name, g_hash_table_lookup (sinfo, "hostname"));
		g_string_safe_assign (querier->map, g_hash_table_lookup (sinfo, "mapname"));
		g_string_safe_assign (querier->version, g_hash_table_lookup (sinfo, "gamever"));
		querier->numplayers = gsq_str2int (g_hash_table_lookup (sinfo, "numplayers"));
		querier->maxplayers = gsq_str2int (g_hash_table_lookup (sinfo, "maxplayers"));
		querier->password = gsq_str2bool (g_hash_table_lookup (sinfo, "password"));
		
		g_hash_table_unref (sinfo);
		gsq_querier_emit_info_update (querier);
		return TRUE;
		
	} else if (data[4] == 'p') {
		gchar *p = (gchar *) data + 6;
		gint i, j, fcount = -1;
		gchar *field;
		gchar *values[7];
		guint8 pcount;
		
		/* number of players */
		pcount = gsq_get_uint8 (&p);
		
		/* fields */
		do {
			fcount++;
			field = gsq_get_cstring (&p);
		} while (*field && p < (data + size));
		
		gchar *gameid = querier->gameid->str;
		if (strcmp (gameid, "aa") == 0) {
			
		} else if (strcmp (gameid, "bfv") == 0) {
			for (i = 0; i < pcount; i++) {
				for (j = 0; j < fcount; j++)
					values[j] = gsq_get_cstring (&p);
				gsq_querier_add_player (querier, values[0],
						gsq_str2int (values[5]), gsq_str2int (values[2]),
						gsq_str2int (values[1]), gsq_str2int (values[3]),
						values[4]);
			}
			
		} else if (strcmp (gameid, "arma") == 0) {
			/* Armed Assault sends an empty string instead of team */
			for (i = 0; i < pcount; i++) {
				for (j = 0; j < fcount; j++)
					values[j] = gsq_get_cstring (&p);
				gsq_querier_add_player (querier, values[0],
						gsq_str2int (values[2]), gsq_str2int (values[3]));
			}
			
		} else {
			for (i = 0; i < pcount; i++) {
				for (j = 0; j < fcount; j++)
					values[j] = gsq_get_cstring (&p);
				gsq_querier_add_player (querier, values[0]);
			}
		}
		
		gsq_querier_emit_player_update (querier);
		return TRUE;
	}
	
	return FALSE;
}
