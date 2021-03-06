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

GQuark gsq_querier_error_quark (void);
#define GSQ_QUERIER_ERROR gsq_querier_error_quark ()

typedef enum {
	GSQ_QUERIER_ERROR_TIMEOUT,
	GSQ_QUERIER_ERROR_CUSTOM
} GsqQuerierErrorEnum;


#define GSQ_TYPE_QUERIER			(gsq_querier_get_type ())
#define GSQ_QUERIER(obj)			(G_TYPE_CHECK_INSTANCE_CAST ((obj), GSQ_TYPE_QUERIER, GsqQuerier))
#define GSQ_IS_QUERIER(obj)			(G_TYPE_CHECK_INSTANCE_TYPE ((obj), GSQ_TYPE_QUERIER))
#define GSQ_QUERIER_CLASS(klass)	(G_TYPE_CHECK_CLASS_CAST ((klass), GSQ_TYPE_QUERIER, GsqQuerierClass))
#define GSQ_IS_QUERIER_CLASS(klass)	(G_TYPE_CHECK_CLASS_TYPE ((klass), GSQ_TYPE_QUERIER))
#define GSQ_QUERIER_GET_CLASS(obj)	(G_TYPE_INSTANCE_GET_CLASS ((obj), GSQ_TYPE_QUERIER, GsqQuerierClass))

typedef struct _GsqQuerier			GsqQuerier;
typedef struct _GsqQuerierPrivate	GsqQuerierPrivate;
typedef struct _GsqQuerierClass		GsqQuerierClass;
typedef struct _GsqField			GsqField;
typedef struct _GsqPlayer			GsqPlayer;

struct _GsqQuerier {
	GObject parent_instance;
	GsqQuerierPrivate *priv;
	
	GString *name;
	GString *gameid;
	GString *gamename;
	GString *gamemode;
	GString *map;
	GString *version;
	gint numplayers;
	gint maxplayers;
	gboolean password;
};

struct _GsqQuerierClass {
	GObjectClass parent_class;
	
	void (*resolve) (GsqQuerier *querier);
	void (*detect) (GsqQuerier *querier);
	void (*info_update) (GsqQuerier *querier);
	void (*players_update) (GsqQuerier *querier);
	void (*log) (GsqQuerier *querier, const gchar *msg);
	void (*error) (GsqQuerier *querier, const GError *error);
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


GType           gsq_querier_get_type           () G_GNUC_CONST;

GsqQuerier *    gsq_querier_new                (const gchar    *address,
                                                guint16         gport,
                                                guint16         qport,
                                                const gchar    *protocol);

GsqQuerier *    gsq_querier_new_parse          (const gchar    *address);

void            gsq_querier_set_protocol       (GsqQuerier     *querier,
                                                const gchar    *protocol);

const gchar *   gsq_querier_get_protocol       (GsqQuerier     *querier) G_GNUC_CONST;

void            gsq_querier_set_protocol_auto  (GsqQuerier     *querier,
                                                gboolean        value);

gboolean        gsq_querier_get_protocol_auto  (GsqQuerier     *querier) G_GNUC_CONST;

void            gsq_querier_set_gport          (GsqQuerier     *querier,
                                                guint16         port);

guint16         gsq_querier_get_gport          (GsqQuerier     *querier) G_GNUC_CONST;

void            gsq_querier_set_gport_auto     (GsqQuerier     *querier,
                                                gboolean        value);

gboolean        gsq_querier_get_gport_auto     (GsqQuerier     *querier) G_GNUC_CONST;

void            gsq_querier_set_qport          (GsqQuerier     *querier,
                                                guint16         port);

guint16         gsq_querier_get_qport          (GsqQuerier     *querier) G_GNUC_CONST;

void            gsq_querier_set_qport_auto     (GsqQuerier     *querier,
                                                gboolean        value);

gboolean        gsq_querier_get_qport_auto     (GsqQuerier     *querier) G_GNUC_CONST;

const gchar *   gsq_querier_get_address        (GsqQuerier     *querier) G_GNUC_CONST;

GInetAddress *  gsq_querier_get_iaddr          (GsqQuerier     *querier) G_GNUC_CONST;

gchar *         gsq_querier_get_extra          (GsqQuerier     *querier,
                                                const gchar    *key) G_GNUC_CONST;

glong           gsq_querier_get_ping           (GsqQuerier     *querier) G_GNUC_CONST;

gboolean        gsq_querier_is_detected        (GsqQuerier     *querier) G_GNUC_CONST;

GArray *        gsq_querier_get_fields         (GsqQuerier     *querier) G_GNUC_CONST;

GList *         gsq_querier_get_players        (GsqQuerier     *querier) G_GNUC_CONST;

GsqPlayer *     gsq_querier_find_player        (GsqQuerier     *querier,
                                                const gchar    *name) G_GNUC_CONST;

void            gsq_querier_update             (GsqQuerier     *querier);



/* debug functions */

typedef enum _GsqDebugFlag {
    GSQ_DEBUG_NONE            = 1 << 0,
    GSQ_DEBUG_INCOMING_DATA   = 1 << 1,
    GSQ_DEBUG_OUTGOING_DATA   = 1 << 2,
    GSQ_DEBUG_EVENT           = 1 << 3
} GsqDebugFlag;


void            gsq_set_debug_flags            (GsqDebugFlag    flags);
GsqDebugFlag    gsq_get_debug_flags            ();


/* local port information */
guint16         gsq_get_default_local_port     () G_GNUC_CONST;

void            gsq_set_default_local_port     (guint16         port);

guint16         gsq_get_local_ipv4_port        ();

guint16         gsq_get_local_ipv6_port        ();


/* protocol registry */
typedef void (*GsqQueryFunc) (GsqQuerier *querier);
typedef gboolean (*GsqProcessFunc) (GsqQuerier *querier, guint16 qport,
		const gchar *data, gssize size);
typedef void (*GsqFreeFunc) (GsqQuerier *querier);

void            gsq_register_protocol          (const gchar    *name,
                                                GsqQueryFunc    query_fn,
                                                GsqProcessFunc  process_fn,
                                                GsqFreeFunc     free_fn);

void            gsq_unregister_protocol        (const gchar    *name);


G_END_DECLS

#endif
