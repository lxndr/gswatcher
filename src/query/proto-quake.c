/* 
 * proto-quake.c: Quake 2/3 based query protocol implementation
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



#include <math.h>
#include <string.h>
#include <stdlib.h>
#include <glib/gi18n.h>
#include <glib/gprintf.h>
#include "utils.h"
#include "querier-private.h"
#include "proto-quake.h"
#include "utils.h"


typedef struct _GameSpec {
	gchar *proto_id;
	gchar *id;
	gchar *name;
	gchar *mode_key;
	guint16 port;
} GameSpec;


static const GameSpec q2games[] = {
	{"arena",                      "arena",   "Alien Arena",                    NULL,         27910},
	{"baseq2",                     "q2",      "Quake 2",                        NULL,         27910},
	{"action",                     "aq2",     "Action Quake 2",                 NULL,         27910},
	{NULL,                         NULL,      NULL,                             NULL,             0}
};

static const GameSpec q3games[] = {
	{"base",                       "q3",      "Quake 3",                        NULL,         27960},
	{"baseq3",                     "q3",      "Quake 3",                        NULL,         27960},
	{"osp",                        "q3osp",   "Quake3: OSP",                    NULL,         27960},
	{"q3ut4",                      "urt",     "Urban Terror",                   NULL,         27960},
	{"baseoa",                     "oarena",  "Open Arena",                     NULL,         27960},
	{"ZEQ2",                       "zeq2",    "ZEQ2",                           NULL,         27960},
	{"excessiveplus",              "e+",      "Quake 3 (E+)",                   NULL,         27960},
	{"Nexuiz",                     "nexuiz",  "Nexuiz",                         NULL,         26000},
	{"Warsow",                     "warsow",  "Warsow",                         "gametype",   44400},
	{"Call of Duty",               "cod",     "Call of Duty",                   "g_gametype", 28960},
	{"CoD:United Offensive",       "cod2-uo", "Call of Duty: United Offensive", "g_gametype", 28960},
	{"Call of Duty 2",             "cod2",    "Call of Duty 2",                 "g_gametype", 28960},
	{"Call of Duty 4",             "cod4",    "Call of Duty 4",                 "g_gametype", 28960},
	{"Call of Duty: World at War", "cod-ww",  "Call of Duty: World at War",     "g_gametype", 28960},
	{NULL,                         NULL,      NULL,                             NULL,             0}
};



static void
quake_query (GsqQuerier *querier, const gchar *line, gsize length, const GameSpec *games)
{
	guint16 port = gsq_querier_get_qport (querier);
	if (port == 0)
		port = gsq_querier_get_gport (querier);
	
	if (port > 0) {
		gsq_querier_send (querier, port, line, length);
	} else {
		/* try default ports */
		gint prev = 0;
		while (games->id) {
			if (games->port != prev) {
				gsq_querier_send (querier, games->port, line, length);
				prev = games->port;
			}
			games++;
		}
	}
}


void
gsq_quake2_query (GsqQuerier *querier)
{
	/* a final null byte has to be sent thus there's 11 bytes */
	quake_query (querier, "\xFF\xFF\xFF\xFFstatus", 11, q2games);
}

void
gsq_quake3_query (GsqQuerier *querier)
{
	quake_query (querier, "\xFF\xFF\xFF\xFFgetstatus\n", 14, q3games);
}


static const GameSpec *
find_spec (const gchar *id, const GameSpec *games)
{
	if (!id)
		return NULL;
	
	while (games->id) {
		if (strcmp (games->proto_id, id) == 0)
			return games;
		games++;
	}
	
	return NULL;
}


static void
clear_name (gchar *name)
{
	if (!name)
		return;
	
	gchar *s = name;
	gchar *d = name;
	
	while (TRUE) {
		if (*s == '^') {
			if (s[1] != 0x00)
				s++;
		} else if (*s >= 0x20 || *s == 0x0b || *s == 0x00) {
			*d = *s;
			if (*s == 0x00)
				break;
			d++;
		}
		
		s++;
	}
}


static gboolean
quake_process (GsqQuerier *querier, guint16 qport, const gchar *data, const GameSpec *games)
{
	gchar *plist, *sinfo = g_strdup (data);
	gchar *p, *f, *v;
	
	if (!(plist = strchr (sinfo, '\n'))) {
		g_free (sinfo);
		return FALSE;
	}
	*plist = 0;
	plist++;
	
	GHashTable *values = g_hash_table_new (g_str_hash, g_str_equal);
	
	/* parse server info */
	p = sinfo;
	if (*p++ != '\\') {
		g_hash_table_destroy (values);
		g_free (sinfo);
		return FALSE;
	}
	
	for (;;) {
		if ((v = strchr (p, '\\'))) {
			*v = 0;
			v++;
		} else {
			/* error: no value */
			g_hash_table_destroy (values);
			g_free (sinfo);
			return FALSE;
		}
		
		if ((f = strchr (v, '\\')))
			*f = 0;
		
		g_hash_table_replace (values, p, v);
		
		if (f)
			p = f + 1;
		else
			break;
	}
	
	if (g_hash_table_size (values) == 0) {
		g_hash_table_destroy (values);
		g_free (sinfo);
		return FALSE;
	}
	
	/* detection */
	if (!*querier->gameid->str) {
		gchar *id = gsq_lookup_value (values, "game", "gamename", NULL);
		const GameSpec *spec = find_spec (id, games);
		if (spec) {
			g_string_assign (querier->gameid, spec->id);
			g_string_assign (querier->gamename, spec->name);
		} else {
			g_string_assign (querier->gameid, id);
			g_string_assign (querier->gamename, id);
		}
		
		guint16 port = gsq_querier_get_gport (querier);
		if ((port > 0 && port != qport) && (spec && spec->port != qport)) {
			g_hash_table_destroy (values);
			g_free (sinfo);
			return FALSE;
		}
		
		gsq_querier_add_field (querier, N_("Score"), G_TYPE_INT);
		gsq_querier_add_field (querier, N_("Ping"), G_TYPE_INT);
	}
	
	/* server name */
	gchar *name = gsq_lookup_value (values, "hostname", "sv_hostname", NULL);
	if (name)
		clear_name (name);
	g_string_safe_assign (querier->name, name);
	/* map name */
	gchar *map = g_hash_table_lookup (values, "mapname");
	g_string_safe_assign (querier->map, map);
	/* maximum players */
	querier->maxplayers = gsq_str2int (gsq_lookup_value (values,
				"maxclients", "sv_maxclients", NULL));
	/* game version */
	gchar *version = gsq_lookup_value (values, "version", "gameversion", "shortversion", NULL);
	g_string_safe_assign (querier->version, version);
	/* password */
	querier->password = gsq_str2bool (gsq_lookup_value (values,
			"needpass", "pswrd", "g_needpass", NULL));
	
	g_hash_table_destroy (values);
	
	/* player list */
	gchar *score, *ping, *player;
	querier->numplayers = 0;
	p = plist;
	while (*p) {
		/* score */
		score = p;
		if (!(f = strchr (score, ' ')))
			break;
		*f = 0;
		/* ping */
		ping = f + 1;
		if (!(f = strstr (ping, " \"")))
			break;
		*f = 0;
		/* player */
		player = f + 2;
		if (!(f = strstr (player, "\"\n")))
			break;
		*f = 0;
		p = f + 2;
		
		clear_name (player);
		gsq_querier_add_player (querier, player, atoi (score), atoi (ping));
		querier->numplayers++;
	}
	
	gsq_querier_emit_info_update (querier);
	gsq_querier_emit_player_update (querier);
	g_free (sinfo);
	return TRUE;
}


gboolean
gsq_quake2_process (GsqQuerier *querier, guint16 qport, const gchar *data, gssize size)
{
	if (size < 11 || strncmp (data, "\xFF\xFF\xFF\xFFprint\n", 10))
		return FALSE;
	return quake_process (querier, qport, data + 10, q2games);
}


gboolean
gsq_quake3_process (GsqQuerier *querier, guint16 qport, const gchar *data, gssize size)
{
	if (size < 20 || strncmp (data, "\xFF\xFF\xFF\xFFstatusResponse\n", 19) != 0)
		return FALSE;
	return quake_process (querier, qport, data + 19, q3games);
}
