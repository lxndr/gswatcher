#ifndef __GSQUERY__QUERIER_H__
#define __GSQUERY__QUERIER_H__

#include <glib.h>
#include <glib-object.h>
#include <gio/gio.h>

G_BEGIN_DECLS


#define GSQ_TYPE_QUERIER			(gsq_querier_get_type ())
#define GSQ_QUERIER(obj)			(G_TYPE_CHECK_INSTANCE_CAST ((obj), GSQ_TYPE_QUERIER, GsqQuerier))
#define GSQ_IS_QUERIER(obj)			(G_TYPE_CHECK_INSTANCE_TYPE ((obj), GSQ_TYPE_QUERIER))
#define GSQ_QUERIER_CLASS(klass)	(G_TYPE_CHECK_CLASS_CAST ((klass), GSQ_TYPE_QUERIER, GsqQuerierClass))
#define GSQ_IS_QUERIER_CLASS(klass)	(G_TYPE_CHECK_CLASS_TYPE ((klass), GSQ_TYPE_QUERIER))
#define GSQ_QUERIER_GET_CLASS(obj)	(G_TYPE_INSTANCE_GET_CLASS ((obj), GSQ_TYPE_QUERIER, GsqQuerierClass))

typedef struct _GsqQuerier			GsqQuerier;
typedef struct _GsqQuerierPrivate	GsqQuerierPrivate;
typedef struct _GsqQuerierClass	GsqQuerierClass;
typedef struct _GsqPlayer			GsqPlayer;

struct _GsqQuerier {
	GObject parent_instance;
	GsqQuerierPrivate *priv;
	
	gchar *name;
	gchar *game;
	gchar *map;
	gint numplayers;
	gint maxplayers;
	gchar *version;
};

struct _GsqQuerierClass {
	GObjectClass parent_class;
	
	void (*resolve) (GsqQuerier *querier);
	void (*info_update) (GsqQuerier *querier);
	void (*players_update) (GsqQuerier *querier);
	void (*log) (GsqQuerier *querier, const gchar *msg);
	void (*timeout) (GsqQuerier *querier);
	void (*error) (GsqQuerier *querier, const gchar *msg);
	void (*player_online) (GsqQuerier *querier, GsqPlayer *player);
	void (*player_offline) (GsqQuerier *querier, GsqPlayer *player);
};

struct _GsqPlayer {
	gchar *name;
	gint kills;
	gchar *time;
};


GType gsq_querier_get_type (void) G_GNUC_CONST;
GsqQuerier* gsq_querier_new (const gchar *address);

GInetAddress *gsq_querier_get_iaddr (GsqQuerier *querier);
void gsq_querier_update (GsqQuerier *querier);

const gchar *gsq_querier_get_address (GsqQuerier *querier);
guint16 gsq_querier_get_port (GsqQuerier *querier);
gchar *gsq_querier_get_extra (GsqQuerier *querier, const gchar *key);
glong gsq_querier_get_ping (GsqQuerier *querier);

GList *gsq_querier_get_players (GsqQuerier *querier);
GsqPlayer *gsq_querier_find_player (GsqQuerier *querier, const gchar *name);

void gsq_querier_set_debug_mode (gboolean enable);
gboolean gsq_querier_get_debug_mode ();


G_END_DECLS

#endif
