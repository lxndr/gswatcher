/* 
 * watcher.c
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



#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <glib/gprintf.h>
#include "proto-source.h"
#include "watcher.h"
#include "utils.h"


enum {
	SIGNAL_NAME_CHANGE,
	SIGNAL_MAP_CHANGE,
	LAST_SIGNAL
};

struct _GsqWatcherPrivate {
	guint name, map;
};


static guint signals[LAST_SIGNAL] = { 0 };


static void gsq_watcher_info_updated (GsqQuerier *querier);


G_DEFINE_TYPE (GsqWatcher, gsq_watcher, GSQ_TYPE_QUERIER);


static void
gsq_watcher_finalize (GObject *object)
{
	if (G_OBJECT_CLASS (gsq_watcher_parent_class)->finalize)
		G_OBJECT_CLASS (gsq_watcher_parent_class)->finalize (object);
}


static void
gsq_watcher_class_init (GsqWatcherClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);
	object_class->finalize = gsq_watcher_finalize;
	
	GsqQuerierClass *querier_class = GSQ_QUERIER_CLASS (klass);
	querier_class->info_update = gsq_watcher_info_updated;
	
	g_type_class_add_private (object_class, sizeof (GsqWatcherPrivate));
	
	signals[SIGNAL_NAME_CHANGE] = g_signal_new ("name-change",
			G_OBJECT_CLASS_TYPE (klass), G_SIGNAL_RUN_FIRST | G_SIGNAL_ACTION,
			G_STRUCT_OFFSET (GsqWatcherClass, name_change), NULL, NULL,
			g_cclosure_marshal_VOID__VOID, G_TYPE_NONE, 0);
	
	signals[SIGNAL_MAP_CHANGE] = g_signal_new ("map-change",
			G_OBJECT_CLASS_TYPE (klass), G_SIGNAL_RUN_FIRST | G_SIGNAL_ACTION,
			G_STRUCT_OFFSET (GsqWatcherClass, map_change), NULL, NULL,
			g_cclosure_marshal_VOID__VOID, G_TYPE_NONE, 0);
}

static void
gsq_watcher_init (GsqWatcher *watcher)
{
	watcher->priv = G_TYPE_INSTANCE_GET_PRIVATE (watcher, GSQ_TYPE_WATCHER,
			GsqWatcherPrivate);
}


GsqQuerier*
gsq_watcher_new (const gchar *address)
{
	return g_object_new (GSQ_TYPE_WATCHER, "address", address, NULL);
}


static void
gsq_watcher_info_updated (GsqQuerier *querier)
{
	GsqWatcherPrivate *priv = ((GsqWatcher *) querier)->priv;
	guint hash;
	
	hash = g_str_hash (querier->name);
	if (priv->name != hash) {
		g_signal_emit (querier, signals[SIGNAL_NAME_CHANGE], 0);
		priv->name = hash;
	}
	
	hash = g_str_hash (querier->map);
	if (priv->map != hash) {
		g_signal_emit (querier, signals[SIGNAL_MAP_CHANGE], 0);
		priv->map = hash;
	}
	
	if (GSQ_QUERIER_CLASS (gsq_watcher_parent_class)->info_update)
		GSQ_QUERIER_CLASS (gsq_watcher_parent_class)->info_update (querier);
}
