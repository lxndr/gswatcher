/* 
 * querier.h: querier functions
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



#ifndef __GSQUERY__QUERIER_H__
#define __GSQUERY__QUERIER_H__

#include <glib.h>
#include <glib-object.h>
#include <gio/gio.h>

G_BEGIN_DECLS


/* Server querier */

#define GSQ_TYPE_QUERIER			(gsq_querier_get_type ())
#define GSQ_QUERIER(obj)			(G_TYPE_CHECK_INSTANCE_CAST ((obj), GSQ_TYPE_QUERIER, GsqQuerier))
#define GSQ_IS_QUERIER(obj)			(G_TYPE_CHECK_INSTANCE_TYPE ((obj), GSQ_TYPE_QUERIER))
#define GSQ_QUERIER_CLASS(klass)	(G_TYPE_CHECK_CLASS_CAST ((klass), GSQ_TYPE_QUERIER, GsqQuerierClass))
#define GSQ_IS_QUERIER_CLASS(klass)	(G_TYPE_CHECK_CLASS_TYPE ((klass), GSQ_TYPE_QUERIER))
#define GSQ_QUERIER_GET_CLASS(obj)	(G_TYPE_INSTANCE_GET_CLASS ((obj), GSQ_TYPE_QUERIER, GsqQuerierClass))

typedef struct _GsqQuerier			GsqQuerier;
typedef struct _GsqQuerierPrivate	GsqQuerierPrivate;
typedef struct _GsqQuerierClass	GsqQuerierClass;
typedef struct _GsqField			GsqField;
typedef struct _GsqPlayer			GsqPlayer;

struct _GsqQuerier {
	GObject parent_instance;
	GsqQuerierPrivate *priv;
};

struct _GsqQuerierClass {
	GObjectClass parent_class;
	
	void (*resolve) (GsqQuerier *querier);
	void (*detect) (GsqQuerier *querier);
	void (*info_update) (GsqQuerier *querier);
	void (*players_update) (GsqQuerier *querier);
	void (*log) (GsqQuerier *querier, const gchar *msg);
	void (*timeout) (GsqQuerier *querier);
	void (*error) (GsqQuerier *querier, const gchar *msg);
	void (*gameid_changed) (GsqQuerier *querier);
	void (*map_changed) (GsqQuerier *quereir);
	void (*player_online) (GsqQuerier *querier, GsqPlayer *player);
	void (*player_offline) (GsqQuerier *querier, GsqPlayer *player);
};

struct _GsqField {
	gchar *name;
	GType type;
};

struct _GsqPlayer {
	gchar *name;
	GValue *values;
};


GType gsq_querier_get_type (void) G_GNUC_CONST;
GsqQuerier* gsq_querier_new (const gchar *address);

void gsq_querier_set_name (GsqQuerier *querier, const gchar *name);
const gchar *gsq_querier_get_name (GsqQuerier *querier);
void gsq_querier_set_gameid (GsqQuerier *querier, const gchar *gameid);
const gchar *gsq_querier_get_gameid (GsqQuerier *querier);
void gsq_querier_set_game (GsqQuerier *querier, const gchar *game);
const gchar *gsq_querier_get_game (GsqQuerier *querier);
void gsq_querier_set_mode (GsqQuerier *querier, const gchar *mode);
const gchar *gsq_querier_get_mode (GsqQuerier *querier);
void gsq_querier_set_map (GsqQuerier *querier, const gchar *map);
const gchar *gsq_querier_get_map (GsqQuerier *querier);
void gsq_querier_set_version (GsqQuerier *querier, const gchar *version);
const gchar *gsq_querier_get_version (GsqQuerier *querier);
void gsq_querier_set_numplayers (GsqQuerier *querier, gint numplayers);
gint gsq_querier_get_numplayers (GsqQuerier *querier);
void gsq_querier_set_maxplayers (GsqQuerier *querier, gint maxplayers);
gint gsq_querier_get_maxplayers (GsqQuerier *querier);
void gsq_querier_set_password (GsqQuerier *querier, gboolean password);
gboolean gsq_querier_get_password (GsqQuerier *querier);

GInetAddress *gsq_querier_get_iaddr (GsqQuerier *querier);
void gsq_querier_update (GsqQuerier *querier);

const gchar *gsq_querier_get_address (GsqQuerier *querier);
guint16 gsq_querier_get_gport (GsqQuerier *querier);
guint16 gsq_querier_get_qport (GsqQuerier *querier);
const gchar *gsq_querier_get_protocol (GsqQuerier *querier);
gchar *gsq_querier_get_extra (GsqQuerier *querier, const gchar *key);
glong gsq_querier_get_ping (GsqQuerier *querier);

GArray *gsq_querier_get_fields (GsqQuerier *querier);
GList *gsq_querier_get_players (GsqQuerier *querier);
GsqPlayer *gsq_querier_find_player (GsqQuerier *querier, const gchar *name);


/* Base functions */

typedef enum _GsqDebugFlag {
	GSQ_DEBUG_NONE			= 1 << 0,
	GSQ_DEBUG_INCOMING_DATA	= 1 << 1,
	GSQ_DEBUG_OUTGOING_DATA	= 1 << 2,
	GSQ_DEBUG_EVENT			= 1 << 3
} GsqDebugFlag;


gboolean gsq_init (guint16 default_port);
void gsq_fini ();

guint16 gsq_get_local_ipv4_port ();
guint16 gsq_get_local_ipv6_port ();

void gsq_set_debug_flags (GsqDebugFlag flags);
GsqDebugFlag gsq_get_debug_flags ();

typedef void (*GsqQueryFunc) (GsqQuerier *querier);
typedef gboolean (*GsqProcessFunc) (GsqQuerier *querier, guint16 qport,
		const gchar *data, gssize size);
typedef void (*GsqFreeFunc) (GsqQuerier *querier);

void gsq_register_protocol (const gchar *name, GsqQueryFunc query_fn,
		GsqProcessFunc process_fn, GsqFreeFunc free_fn);
void gsq_unregister_protocol (const gchar *name);


G_END_DECLS

#endif
