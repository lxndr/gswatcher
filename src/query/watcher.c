/* 
 * watcher.c
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



#include "utils.h"
#include "watcher.h"


enum {
	SIGNAL_NAME_CHANGED,
	SIGNAL_GAMEID_CHANGED,
	SIGNAL_MAP_CHANGED,
	LAST_SIGNAL
};


struct _GsqWatcherPrivate {
	guint name_hash;
	guint gameid_hash;
	guint map_hash;
	
	gint64 offline_time;
};



static guint signals[LAST_SIGNAL] = { 0 };


static void gsq_watcher_info_updated (GsqQuerier *querier);
static void gsq_watcher_timed_out (GsqQuerier *querier);


G_DEFINE_TYPE (GsqWatcher, gsq_watcher, GSQ_TYPE_QUERIER);


static void
gsq_watcher_class_init (GsqWatcherClass *class)
{
	GObjectClass *object_class = G_OBJECT_CLASS (class);
	
	g_type_class_add_private (object_class, sizeof (GsqWatcherPrivate));
	
	signals[SIGNAL_NAME_CHANGED] = g_signal_new ("name-changed",
			G_OBJECT_CLASS_TYPE (class), G_SIGNAL_RUN_FIRST | G_SIGNAL_ACTION,
			G_STRUCT_OFFSET (GsqWatcherClass, name_changed), NULL, NULL,
			g_cclosure_marshal_VOID__VOID, G_TYPE_NONE, 0);
	
	signals[SIGNAL_GAMEID_CHANGED] = g_signal_new ("gameid-changed",
			G_OBJECT_CLASS_TYPE (class), G_SIGNAL_RUN_FIRST | G_SIGNAL_ACTION,
			G_STRUCT_OFFSET (GsqWatcherClass, gameid_changed), NULL, NULL,
			g_cclosure_marshal_VOID__VOID, G_TYPE_NONE, 0);
	
	signals[SIGNAL_MAP_CHANGED] = g_signal_new ("map-changed",
			G_OBJECT_CLASS_TYPE (class), G_SIGNAL_RUN_FIRST | G_SIGNAL_ACTION,
			G_STRUCT_OFFSET (GsqWatcherClass, map_changed), NULL, NULL,
			g_cclosure_marshal_VOID__VOID, G_TYPE_NONE, 0);
	
	GsqQuerierClass *querier_class = GSQ_QUERIER_CLASS (class);
	querier_class->info_update = gsq_watcher_info_updated;
	querier_class->timeout = gsq_watcher_timed_out;
}


static void
gsq_watcher_init (GsqWatcher *watcher)
{
	watcher->priv = G_TYPE_INSTANCE_GET_PRIVATE (watcher,
			GSQ_TYPE_WATCHER, GsqWatcherPrivate);
}


GsqWatcher *
gsq_watcher_new (const gchar *address)
{
	g_return_val_if_fail (address != NULL && *address != '\0', NULL);
	
	guint16 gport, qport;
	gchar *host = gsq_parse_address (address, &gport, &qport);
	if (host == NULL)
		return NULL;
	
	return g_object_new (GSQ_TYPE_WATCHER, "address", host,
			"gport", gport, "gport-auto", gport == 0,
			"qport", qport, "qport-auto", qport == 0,
			NULL);
}


static void
gsq_watcher_info_updated (GsqQuerier *querier)
{
	GsqWatcherPrivate *priv = GSQ_WATCHER (querier)->priv;
	guint64 hash;
	
	priv->offline_time = 0;
	
	hash = g_string_hash (querier->name);
	if (hash != priv->name_hash) {
		g_signal_emit (querier, signals[SIGNAL_NAME_CHANGED], 0);
		priv->name_hash = hash;
	}
	
	hash = g_string_hash (querier->gameid);
	if (hash != priv->gameid_hash) {
		g_signal_emit (querier, signals[SIGNAL_GAMEID_CHANGED], 0);
		priv->gameid_hash = hash;
	}
	
	hash = g_string_hash (querier->map);
	if (hash != priv->map_hash) {
		g_signal_emit (querier, signals[SIGNAL_MAP_CHANGED], 0);
		priv->map_hash = hash;
	}
}


static void
gsq_watcher_timed_out (GsqQuerier *querier)
{
	GsqWatcherPrivate *priv = GSQ_WATCHER (querier)->priv;
	
	if (priv->offline_time == 0) {
		GTimeVal tv;
		g_get_current_time (&tv);
		priv->offline_time = tv.tv_sec;
	}
}


void
gsq_watcher_set_offline_time (GsqWatcher *watcher, guint64 time)
{
	g_return_if_fail (GSQ_IS_WATCHER (watcher));
	watcher->priv->offline_time = time;
}


guint64
gsq_watcher_get_offline_time (GsqWatcher *watcher)
{
	g_return_val_if_fail (GSQ_IS_WATCHER (watcher), 0);
	return watcher->priv->offline_time;
}


gboolean
gsq_watcher_is_online (GsqWatcher *watcher)
{
	g_return_val_if_fail (GSQ_IS_WATCHER (watcher), FALSE);
	return watcher->priv->offline_time == 0;
}
