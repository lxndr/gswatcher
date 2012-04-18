/* 
 * proto-quake3.c: Quake3 based query protocol implementation
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



#include <math.h>
#include <string.h>
#include <stdlib.h>
#include <glib/gi18n.h>
#include <glib/gprintf.h>
#include "utils.h"
#include "querier-private.h"
#include "proto-quake3.h"
#include "utils.h"


typedef struct _Q3Game {
	gchar *proto_id;
	gchar *id;
	gchar *name;
	gchar *mode_key;
	guint16 port;
} Q3Game;

static const Q3Game games[] = {
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



void
gsq_quake3_query (GsqQuerier *querier)
{
	static gchar *query = "\xFF\xFF\xFF\xFFgetstatus\n";
	guint16 port = gsq_querier_get_gport (querier);
	if (port > 0) {
		gsq_querier_send (querier, port, query, 14);
	} else {
		/* try default ports */
		gint i, prev = 0;
		for (i = 0; games[i].proto_id; i++) {
			if (games[i].port != prev) {
				gsq_querier_send (querier, games[i].port, query, 14);
				prev = games[i].port;
			}
		}
	}
}


static const Q3Game *
find_spec (const gchar *id)
{
	if (!id)
		return NULL;
	
	gint i;
	for (i = 0; games[i].proto_id; i++)
		if (strcmp (games[i].proto_id, id) == 0)
			return &games[i];
	return NULL;
}


static void
clear_name (gchar *name)
{
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
get_sinfo (GsqQuerier *querier, gchar *data, gsize length, guint16 qport)
{
	data = g_strndup (data, length);
	gchar *p = data;
	
	/* parse data */
	if (*p++ != '\\')
		return FALSE;
	
	GHashTable *values = g_hash_table_new (g_str_hash, g_str_equal);
	gchar *f, *v;
	
	while (TRUE) {
		/* key */
		f = strchr (p, '\\');
		if (f)
			*f = '\0';
		else
			break;
		
		/* value */
		v = f + 1;
		f = strchr (v, '\\');
		if (f)
			*f = '\0';
		
		g_hash_table_replace (values, p, v);
		
		if (f)
			p = f + 1;
		else
			break;
	}
	
	/* check game port */
	if (!*gsq_querier_get_gameid (querier)) {
		gchar *id = g_hash_table_lookup (values, "gamename");
		const Q3Game *spec = find_spec (id);
		if (spec) {
			gsq_querier_set_gameid (querier, spec->id);
			gsq_querier_set_game (querier, spec->name);
		} else {
			gsq_querier_set_gameid (querier, id);
			gsq_querier_set_game (querier, id);
		}
		
		guint16 port = gsq_querier_get_gport (querier);
		if ((port > 0 && port != qport) && (spec && spec->port != qport)) {
			g_hash_table_destroy (values);
			g_free (data);
			return FALSE;
		}
		
		gsq_querier_add_field (querier, N_("Score"), G_TYPE_INT);
		gsq_querier_add_field (querier, N_("Ping"), G_TYPE_INT);
	}
	
	/* server name */
	gchar *name = gsq_lookup_value (values, "sv_hostname", "hostname", NULL);
	clear_name (name);
	gsq_querier_set_name (querier, name);
	/* map name */
	gchar *map = g_hash_table_lookup (values, "mapname");
	gsq_querier_set_map (querier, map);
	/* maximum players */
	gchar *maxplayers = g_hash_table_lookup (values, "sv_maxclients");
	gsq_querier_set_maxplayers (querier, atoi (maxplayers));
	/* game version */
	gchar *version = gsq_lookup_value (values, "version", "gameversion",
			"shortversion", NULL);
	gsq_querier_set_version (querier, version);
	/* password */
	gchar *password = gsq_lookup_value (values, "pswrd", "g_needpass", NULL);
	gsq_querier_set_extra (querier, "password",
			gsq_str2bool (password) ? "true" : "false");
	
	g_hash_table_destroy (values);
	gsq_querier_emit_info_update (querier);
	g_free (data);
	return TRUE;
}

static gboolean
get_plist (GsqQuerier *querier, gchar *p, gssize len)
{
	GError *error = NULL;
	static GRegex *re = NULL;
	if (!re) {
		re = g_regex_new ("^(-?\\d+) (\\d+) \"(.*)\"$", G_REGEX_OPTIMIZE |
				G_REGEX_NEWLINE_LF | G_REGEX_MULTILINE, 0, &error);
		if (!re)
			g_error (error->message);
	}
	
	GMatchInfo *minfo;
	gint numplayers = 0;
	g_regex_match_full (re, p, len, 0, 0, &minfo, &error);
	while (g_match_info_matches (minfo)) {
		gchar *score = g_match_info_fetch (minfo, 1);
		gchar *ping = g_match_info_fetch (minfo, 2);
		gchar *name = g_match_info_fetch (minfo, 3);
		clear_name (name);
		gsq_querier_add_player (querier, name, atoi (score), atoi (ping));
		g_free (score);
		g_free (ping);
		g_free (name);
		
		numplayers++;
		g_match_info_next (minfo, &error);
	}
	g_match_info_free (minfo);
	
	if (error) {
		g_warning ("Error while matching: %s\n", error->message);
		g_error_free (error);
	}
	
	gsq_querier_set_numplayers (querier, numplayers);
	gsq_querier_emit_player_update (querier);
	return TRUE;
}

gboolean
gsq_quake3_process (GsqQuerier *querier, guint16 qport,
		const gchar *data, gssize size)
{
	if (size < 20) /* very silly, the size of a respond with empty data */
		return FALSE;
	
	if (strncmp (data, "\xFF\xFF\xFF\xFFstatusResponse\n", 19) != 0)
		return FALSE;
	
	gchar *p = (gchar *) data + 19;
	gchar *f = strchr (p, '\n');
	if (!f)
		return FALSE;
	if (f > data + size)
		return FALSE;
	
	return get_sinfo (querier, p, f - p, qport) &&
			get_plist (querier, f + 1, size - (f - data + 1));
}
