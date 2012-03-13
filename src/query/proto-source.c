/* 
 * proto-source.c: Source Engine(TM) query protocol implementation
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
#include <glib/gprintf.h>
#include "utils.h"
#include "querier-private.h"
#include "proto-source.h"


typedef struct _Packet {
	gchar *data;
	gsize length;
} Packet;

typedef struct _Private {
	gchar plist_query[9];
	gint reqid;
	GArray *packets;
	gboolean newprotocol;
} Private;


const gchar *sinfo_query = "\xFF\xFF\xFF\xFFTSource Engine Query\x00";
const gchar *plist_query = "\xFF\xFF\xFF\xFF\x55\xFF\xFF\xFF\xFF";


static void
free_packets (GArray *packets)
{
	guint i;
	
	for (i = 0; i < packets->len; i++) {
		Packet *packet = &g_array_index (packets, Packet, i);
		if (packet->data)
			g_slice_free1 (packet->length, packet->data);
	}
	
	g_array_set_size (packets, 0);
}


void
gsq_source_free (GsqQuerier *querier)
{
	Private *priv = gsq_querier_get_pdata (querier);
	if (priv->packets) {
		free_packets (priv->packets);
		g_array_free (priv->packets, TRUE);
	}
	
	g_slice_free (Private, priv);
}


void
gsq_source_query (GsqQuerier *querier)
{
	Private *priv = gsq_querier_get_pdata (querier);
	
	guint32 port = gsq_querier_get_port (querier);
	if (port == 0) port = 27015;
	
	gsq_querier_send (querier, port, sinfo_query, 25);
	
	if (priv && *priv->plist_query)
		gsq_querier_send (querier, port, priv->plist_query, 9);
	else
		gsq_querier_send (querier, port, plist_query, 9);
}


static inline gint8
get_byte (gchar **p)
{
	gint8 v = * (gint8 *) *p;
	*p += 1;
	return v;
}

static inline gint16
get_short (gchar **p)
{
	gint16 v = * (gint16 *) *p;
	*p += 2;
	return GINT16_FROM_LE (v);
}

static inline gint32
get_long (gchar **p)
{
	gint32 v = * (gint32 *) *p;
	*p += 4;
	return GINT32_FROM_LE (v);
}

/* it may be usefull in the future */
/*
static guint64
get_longlong (gchar **p)
{
	guint64 v = * (guint64 *) *p;
	*p += 8;
	return GUINT64_FROM_LE (v);
}
*/

static inline gfloat
get_float (gchar **p)
{
	gfloat v = * (gfloat *) *p;
	*p += 4;
	return GFLOAT_FROM_LE (v);
}

static char *
get_string (gchar **p, gchar *buf, gsize maxlen)
{
	gchar *s = *p;
	if (buf)
		*p += g_strlcpy (buf, s, maxlen);
	else
		*p += strlen (s);
	(*p)++;
	return s;
}

static void
format_time (gchar *dst, gsize maxlen, gfloat time)
{
	gint t = (gint) floor (time);
	if (time >= 0)
		g_snprintf (dst, maxlen, "%d:%02d:%02d",
				t / (60 * 60), (t / 60) % 60, t % 60);
	else
		g_strlcpy (dst, "?:??:??", maxlen);
}


static void
get_server_info (GsqQuerier *querier, gchar *p)
{
	gchar desc[128], dir[128], ver[128], tmp[128];
	
	gint8 pver = get_byte (&p);				// Protocol version
	get_string (&p, tmp, 128);				// Server name
	gsq_querier_set_name (querier, tmp);
	get_string (&p, tmp, 128);				// Map name
	gsq_querier_set_map (querier, tmp);
	get_string (&p, dir, 128);				// Game folder
	get_string (&p, desc, 128);				// Game description
	gint16 appid = get_short (&p);			// Application ID
	querier->numplayers = get_byte (&p);	// Number of players
	querier->maxplayers = get_byte (&p);	// Maximum players
	get_byte (&p);							// Number of bots
	gchar dedicated = get_byte (&p);		// Dedicated
	gchar os = get_byte (&p);				// OS
	gboolean pass = get_byte (&p);			// Password
	gboolean secure = get_byte (&p);		// Secure
	get_string (&p, ver, 128);				// Game version
	gsq_querier_set_version (querier, ver);
	
	g_snprintf (tmp, 128, "%d", pver);
	gsq_querier_set_extra (querier, "protocol-version", tmp);
	g_snprintf (tmp, 128, "%d", appid);
	gsq_querier_set_extra (querier, "appid", tmp);
	gsq_querier_set_extra (querier, "password", pass ? "true" : "false");
	gsq_querier_set_extra (querier, "secure", secure ? "true" : "false");
	
	switch (os) {
		case 'l': gsq_querier_set_extra (querier, "os", "Linux"); break;
		case 'w': gsq_querier_set_extra (querier, "os", "Windows"); break;
		default:  gsq_querier_set_extra (querier, "os", "Unknown");
	}
	
	switch (dedicated) {
		case 'd': gsq_querier_set_extra (querier, "type", "Dedicated"); break;
		case 'l': gsq_querier_set_extra (querier, "type", "Listen"); break;
		case 'p': gsq_querier_set_extra (querier, "type", "SourceTV"); break;
		default:  gsq_querier_set_extra (querier, "type", "");
	}
	
	switch (appid) {
		case 10:
			if (strcmp (dir, "valve") == 0)
				gsq_querier_set_game (querier, "hl");
			else
				gsq_querier_set_game (querier, "cs");
			break;
		case 20: gsq_querier_set_game (querier, "tf"); break;
		case 30: gsq_querier_set_game (querier, "dod"); break;
		case 80: gsq_querier_set_game (querier, "czero"); break;
		case 240: gsq_querier_set_game (querier, "css"); break;
		case 300: gsq_querier_set_game (querier, "dods"); break;
		case 320: gsq_querier_set_game (querier, "hl2dm"); break;
		case 440: gsq_querier_set_game (querier, "tf2"); break;
		case 500:
			gsq_querier_set_game (querier, "l4d");
			gsq_querier_set_extra (querier, "mode", desc + 6);
			break;
		case 550: gsq_querier_set_game (querier, "l4d2"); break;
		case 630: gsq_querier_set_game (querier, "as"); break;
		case 17500: gsq_querier_set_game (querier, "zp"); break;
		case 17520: gsq_querier_set_game (querier, "syn"); break;
		case 17700: gsq_querier_set_game (querier, "ins"); break;
		case 17710: gsq_querier_set_game (querier, "nd"); break;
		default: gsq_querier_set_game (querier, ""); break;
	}
	
	gsq_querier_emit_info_update (querier);
}

static void
get_server_info_gold (GsqQuerier *querier, gchar *p)
{
	gchar tmp[128];
	
	get_string (&p, NULL, 0);				// Game server IP and port
	get_string (&p, tmp, 128);				// Server name
	gsq_querier_set_name (querier, tmp);
	get_string (&p, tmp, 128);				// Map name
	gsq_querier_set_map (querier, tmp);
	get_string (&p, NULL, 0);				// Game directory
	get_string (&p, NULL, 0);				// Game description
	querier->numplayers = get_byte (&p);	// Number of players
	querier->maxplayers = get_byte (&p);	// Maximum players
	gint8 pver = get_byte (&p);				// Network version
	gchar dedicated = get_byte (&p);		// Dedicated
	gchar os = get_byte (&p);				// OS
	gboolean pass = get_byte (&p);			// Password
	get_byte (&p);							// IsMod
	gboolean secure = get_byte (&p);		// Secure
	
	g_snprintf (tmp, 128, "%d", pver);
	gsq_querier_set_extra (querier, "protocol-version", tmp);
	gsq_querier_set_extra (querier, "password", pass ? "true" : "false");
	gsq_querier_set_extra (querier, "secure", secure ? "true" : "false");
	
	switch (os) {
		case 'l': gsq_querier_set_extra (querier, "os", "Linux"); break;
		case 'w': gsq_querier_set_extra (querier, "os", "Windows"); break;
		default:  gsq_querier_set_extra (querier, "os", "Unknown");
	}
	
	switch (dedicated) {
		case 'd': gsq_querier_set_extra (querier, "type", "Dedicated"); break;
		case 'l': gsq_querier_set_extra (querier, "type", "Listen"); break;
		case 'p': gsq_querier_set_extra (querier, "type", "SourceTV"); break;
		default:  gsq_querier_set_extra (querier, "type", "");
	}
	
	gsq_querier_set_game (querier, "");
	gsq_querier_emit_info_update (querier);
}

static void
get_player_list (GsqQuerier *querier, gchar *p)
{
	gint8 i, count = get_byte (&p);
	gchar time[16];
	
	for (i = 0; i < count; i++) {
		get_byte (&p);
		gchar *nickname = get_string (&p, NULL, 0);		// Nickname
		gint kills = get_long (&p);						// Kills
		format_time (time, 16, get_float (&p));			// Time
		gsq_querier_add_player (querier, nickname, kills, time);
	}
	
	gsq_querier_emit_player_update (querier);
}


static gboolean
source_process2 (GsqQuerier *querier, gchar *p)
{
	Private *priv = gsq_querier_get_pdata (querier);
	gint type = get_byte (&p);
	
	switch (type) {
	case 'A':
		memcpy (priv->plist_query, "\xFF\xFF\xFF\xFF\x55", 5);
		memcpy (priv->plist_query + 5, p, 4);
		gsq_source_query (querier);
		break;
	case 'I':
		get_server_info (querier, p);
		priv->newprotocol = TRUE;
		break;
	case 'm':
		if (!priv->newprotocol)
			get_server_info_gold (querier, p);
		break;
	case 'D':
		get_player_list (querier, p);
		break;
	case 'R':
		gsq_querier_emit_log (querier, p);
		break;
	default:
		return FALSE;
	}
	
	return TRUE;
}


static gboolean
got_all_packets (GArray *packets, guint max)
{
	guint i;
	
	if (packets->len != max)
		return FALSE;
	
	for (i = 0; i < packets->len; i++)
		if (g_array_index (packets, Packet, i).data == NULL)
			return FALSE;
	
	return TRUE;
}


static gchar *
assemble_packets (GArray *packets, gsize *datasize)
{
	guint i, offset = 0;
	
	*datasize = 0;
	for (i = 0; i < packets->len; i++)
		*datasize += g_array_index (packets, Packet, i).length;
	gchar *data = g_slice_alloc (*datasize);
	
	for (i = 0; i < packets->len; i++) {
		Packet *packet = &g_array_index (packets, Packet, i);
		memcpy (data + offset, packet->data, packet->length);
		offset += packet->length;
	}
	
	return data;
}


gboolean
gsq_source_process (GsqQuerier *querier, guint16 qport,
		const gchar *data, gssize size)
{
	Private *priv = gsq_querier_get_pdata (querier);
	if (!priv) {
		priv = g_slice_new0 (Private);
		priv->packets = g_array_new (FALSE, TRUE, sizeof (Packet));
		gsq_querier_set_pdata (querier, priv);
	}
	
	guint16 port = gsq_querier_get_port (querier);
	if (port == 0) port = 27015;
	
	gchar *p = (gchar *) data;
	gint format = get_long (&p);
	
	if (format == -1) {
		return source_process2 (querier, p);
	} else if (format == -2) {
		gint reqid = get_long (&p);
		gboolean compressed = reqid >= 0x10000000;
		gint maxpackets = get_byte (&p);
		gint numpacket = get_byte (&p);
		gint length = get_short (&p);
		
		if (compressed) {
			g_warning ("Compressed split packets are not supported");
			return FALSE;
		}
		
		if (priv->reqid != reqid) {
			free_packets (priv->packets);
			g_array_set_size (priv->packets, maxpackets);
			priv->reqid = reqid;
		}
		
		Packet *pkt = &g_array_index (priv->packets, Packet, numpacket);
		pkt->length = length;
		pkt->data = g_slice_copy (length, p);
		
		if (got_all_packets (priv->packets, maxpackets)) {
			gsize datasize;
			p = assemble_packets (priv->packets, &datasize);
			gboolean ret = source_process2 (querier, p + 4);
			g_slice_free1 (datasize, p);
			free_packets (priv->packets);
			priv->reqid = 0;
			return ret;
		}
		
		return TRUE;
	}
	
	return FALSE;
}
