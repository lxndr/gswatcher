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
	guint16 length;
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
	guint32 port = gsq_querier_get_gport (querier);
	if (port == 0)
		port = 27015;
	
	gsq_querier_send (querier, port, sinfo_query, 25);
	
	Private *priv = gsq_querier_get_pdata (querier);
	const gchar *query = priv && *priv->plist_query ?
			priv->plist_query : plist_query;
	gsq_querier_send (querier, port, query, 9);
}


static inline guint8
get_byte (gchar **p)
{
	gint8 v = * (guint8 *) *p;
	*p += 1;
	return v;
}

static inline guint16
get_short (gchar **p)
{
	gint16 v = * (guint16 *) *p;
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

static guint64
get_longlong (gchar **p)
{
	guint64 v = * (guint64 *) *p;
	*p += 8;
	return GUINT64_FROM_LE (v);
}

static inline gfloat
get_float (gchar **p)
{
	gfloat v = * (gfloat *) *p;
	*p += 4;
	return GFLOAT_FROM_LE (v);
}

static inline gchar *
get_string (gchar **p)
{
	gchar *s = *p;
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
	gchar *dir, *desc, *tags, tmp[256];
	gchar *server_os, *server_type;
	
	guint8 pver = get_byte (&p);						// Protocol version
	gsq_querier_set_name (querier, get_string (&p));	// Server name
	gsq_querier_set_map (querier, get_string (&p));		// Map name
	dir = get_string (&p);								// Game folder
	desc = get_string (&p);								// Game description
	guint16 appid = get_short (&p);						// Application ID
	gsq_querier_set_numplayers (querier, get_byte (&p));// Number of players
	gsq_querier_set_maxplayers (querier, get_byte (&p));// Maximum players
	get_byte (&p);										// Number of bots
	gchar dedicated = get_byte (&p);					// Dedicated
	gchar os = get_byte (&p);							// OS
	gsq_querier_set_password (querier, get_byte (&p));	// Password
	gboolean secure = get_byte (&p);					// Secure
	gsq_querier_set_version (querier, get_string (&p));	// Game version
	guint8 edf = get_byte (&p);							// extra data field
	if (edf & 0x80)
		get_short (&p);									// server's game port
	if (edf & 0x10)
		get_longlong (&p);								// server's SteamID
	if (edf & 0x40) {
		get_short (&p);									// spectator port
		get_string (&p);								// spectator name
	}
	if (edf & 0x20)
		tags = get_string (&p);							// tags
	if (edf & 0x01)
		get_longlong (&p);								// server's Game ID
	
	switch (os) {
		case 'l': server_os = "Linux"; break;
		case 'w': server_os = "Windows"; break;
		default:  server_os = "Unknown";
	}
	
	switch (dedicated) {
		case 'd': server_type = "Dedicated"; break;
		case 'l': server_type = "Listen"; break;
		case 'p': server_type = "SourceTV"; break;
		default:  server_type = "";
	}
	
	gsq_querier_set_extra (querier, "os", server_os);
	gsq_querier_set_extra (querier, "type", server_type);
	g_snprintf (tmp, 128, "%d", pver);
	gsq_querier_set_extra (querier, "protocol-version", tmp);
	g_snprintf (tmp, 128, "%d", appid);
	gsq_querier_set_extra (querier, "appid", tmp);
	gsq_querier_set_extra (querier, "secure", secure ? "true" : "false");
	
	gchar *game_id = NULL, *game_name = desc, *game_mode = NULL;
	switch (appid) {
		case 10:
			if (strcmp (dir, "cstrike") == 0) {
				game_id = "cs";
				game_name = "Counter-Strike";
			} else {
				game_id = "hl";
				game_name = "Half-Life";
			}
			break;
		case 20:
			game_id = "tf";
			game_name = "Team Fortress";
			break;
		case 30:
			game_id = "dod";
			game_name = "Day of Defeat";
			break;
		case 80:
			game_id = "czero";
			game_name = "Counter-Strike: Condition Zero";
			break;
		case 240:
			game_id = "css";
			game_name = "Counter-Strike: Source";
			break;
		case 300:
			game_id = "dods";
			game_name = "Day of Defeat: Source";
			break;
		case 320:
			game_id = "hl2dm";
			game_name = "Half-Life 2: Deathmatch";
			break;
		case 440:
			game_id = "tf2";
			game_name = "Team Fortress 2";
			break;
		case 500:
			game_id = "l4d";
			game_name = "Left 4 Dead";
			game_mode = desc + 6;
			break;
		case 550:
			game_id = "l4d2";
			game_name = "Left 4 Dead 2";
			if (strstr (tags, "coop"))
				game_mode = "Co-op";
			else if (strstr (tags, "realism"))
				game_mode = "Realism";
			else if (strstr (tags, "survival"))
				game_mode = "Survival";
			else if (strstr (tags, "versus"))
				game_mode = "Versus";
			else if (strstr (tags, "scavenge"))
				game_mode = "Scavenge";
			break;
		case 630:
			game_id = "as";
			game_name = "Alien Swarm";
			break;
		case 17500:
			game_id = "zp";
			game_name = "Zombie Panic!";
			break;
		case 17520:
			game_id = "syn";
			game_name = "Synergy";
			break;
		case 17700:
			game_id = "ins";
			game_name = "Insurgency";
			break;
		case 17710:
			game_id = "nd";
			game_name = "Nuclear dawn";
			break;
	}
	
	gsq_querier_set_gameid (querier, game_id);
	gsq_querier_set_game (querier, game_name);
	gsq_querier_set_mode (querier, game_mode);
	gsq_querier_emit_info_update (querier);
}

static void
get_server_info_gold (GsqQuerier *querier, gchar *p)
{
	gchar *dir, tmp[64];
	
	get_string (&p);									// Game server IP and port
	gsq_querier_set_name (querier, get_string (&p));	// Server name
	gsq_querier_set_map (querier, get_string (&p));		// Map name
	dir = get_string (&p);								// Game directory
	get_string (&p);									// Game description
	gsq_querier_set_numplayers (querier, get_byte (&p));// Number of players
	gsq_querier_set_maxplayers (querier, get_byte (&p));// Maximum players
	guint8 pver = get_byte (&p);						// Network version
	gchar dedicated = get_byte (&p);					// Dedicated
	gchar os = get_byte (&p);							// OS
	gsq_querier_set_password (querier, get_byte (&p));	// Password
	get_byte (&p);										// IsMod
	gboolean secure = get_byte (&p);					// Secure
	
	if (strcmp (dir, "cstrike") == 0) {
		gsq_querier_set_gameid (querier, "cs");
		gsq_querier_set_game (querier, "Counter-Strike");
	}
	
	g_snprintf (tmp, 64, "%d", pver);
	gsq_querier_set_extra (querier, "protocol-version", tmp);
	gsq_querier_set_extra (querier, "secure", secure ? "true" : "false");
	
	gchar *server_os, *server_type;
	
	switch (os) {
		case 'l': server_os = "Linux"; break;
		case 'w': server_os = "Windows"; break;
		default:  server_os = "Unknown";
	}
	
	switch (dedicated) {
		case 'd': server_type = "Dedicated"; break;
		case 'l': server_type = "Listen"; break;
		case 'p': server_type = "SourceTV"; break;
		default:  server_type = "";
	}
	
	gsq_querier_set_extra (querier, "os", server_os);
	gsq_querier_set_extra (querier, "type", server_type);
	
	gsq_querier_emit_info_update (querier);
}

static void
get_player_list (GsqQuerier *querier, gchar *p)
{
	gint8 i, count = get_byte (&p);
	gchar time[16];
	
	for (i = 0; i < count; i++) {
		get_byte (&p);
		gchar *nickname = get_string (&p);			// Nickname
		gint kills = get_long (&p);					// Kills
		format_time (time, 16, get_float (&p));		// Time
		gsq_querier_add_player (querier, nickname, kills, time);
	}
	
	gsq_querier_emit_player_update (querier);
}


static gboolean
source_process2 (GsqQuerier *querier, gchar *p, gssize size)
{
	Private *priv = gsq_querier_get_pdata (querier);
	gint type = get_byte (&p);
	
	if ((type == 'A' || type == 'I' || type == 'm' || type == 'D' || type == 'R') &&
			gsq_querier_get_fields (querier)->len == 0) {
		gsq_querier_add_field (querier, "Kills", G_TYPE_INT);
		gsq_querier_add_field (querier, "Time", G_TYPE_STRING);
	}
	
	switch (type) {
	case 'A':
		if (size == 9) {
			memcpy (priv->plist_query, "\xFF\xFF\xFF\xFF\x55", 5);
			memcpy (priv->plist_query + 5, p, 4);
			
			/* request player list right away */
			guint32 port = gsq_querier_get_gport (querier);
			if (port == 0)
				port = 27015;
			gsq_querier_send (querier, port, priv->plist_query, 9);
			return TRUE;
		}
		break;
	case 'I':
		get_server_info (querier, p);
		priv->newprotocol = TRUE;
		return TRUE;
	case 'm':
		if (!priv->newprotocol)
			get_server_info_gold (querier, p);
		return TRUE;
	case 'D':
		get_player_list (querier, p);
		return TRUE;
	case 'R':
		gsq_querier_emit_log (querier, p);
		return TRUE;
	}
	
	return FALSE;
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
	
	/* check if this packet belongs to the querier */
	guint16 port = gsq_querier_get_gport (querier);
	if (!(port == qport || (port == 0 && qport == 27015)))
		return FALSE;
	
	gchar *p = (gchar *) data;
	gint format = get_long (&p);
	
	if (format == -1) {
		if (!source_process2 (querier, p, size))
			return FALSE;
	} else if (format == -2) {
		gint reqid, maxpackets, numpacket;
		gboolean compressed;
		guint16 length;
		
		reqid = get_long (&p);
		compressed = reqid >> 7;
		
		if (*((gint32 *) (p + 1)) == -1) {
			/* GoldSource way */
			guint8 c = get_byte (&p);
			maxpackets = c & 0x0F;
			numpacket = c >> 4;
			length = size - (p - data);
		} else {
			/* Source way */
			maxpackets = get_byte (&p);
			numpacket = get_byte (&p);
			length = get_short (&p);
		}
		
		/* perform some checks */
		if (compressed) {
			g_warning ("Compressed split packets are not supported");
			return FALSE;
		}
		
		if (maxpackets < 2 || numpacket >= maxpackets)
			return FALSE;
		if (length > 1248)
			return FALSE;
		
		if (priv->reqid != reqid) {
			free_packets (priv->packets);
			priv->reqid = reqid;
		}
		
		if (priv->packets->len == 0)
			g_array_set_size (priv->packets, maxpackets);
		else if (maxpackets != priv->packets->len)
			return FALSE;
		
		Packet *pkt = &g_array_index (priv->packets, Packet, numpacket);
		pkt->length = length;
		pkt->data = g_slice_copy (length, p);
		
		if (got_all_packets (priv->packets, maxpackets)) {
			gsize datasize;
			p = assemble_packets (priv->packets, &datasize);
			gboolean ret = source_process2 (querier, p + 4, size);
			g_slice_free1 (datasize, p);
			free_packets (priv->packets);
			priv->reqid = 0;
			return ret;
		}
	}
	
	return TRUE;
}
