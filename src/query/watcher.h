/* 
 * watcher.h
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



#ifndef __GSQUERY__WATCHER_H__
#define __GSQUERY__WATCHER_H__

#include <glib.h>
#include <glib-object.h>
#include "querier.h"

G_BEGIN_DECLS


#define GSQ_TYPE_WATCHER			(gsq_watcher_get_type ())
#define GSQ_WATCHER(obj)			(G_TYPE_CHECK_INSTANCE_CAST ((obj), GSQ_TYPE_WATCHER, GsqWatcher))
#define GSQ_IS_WATCHER(obj)			(G_TYPE_CHECK_INSTANCE_TYPE ((obj), GSQ_TYPE_WATCHER))
#define GSQ_WATCHER_CLASS(klass)	(G_TYPE_CHECK_CLASS_CAST ((klass), GSQ_TYPE_WATCHER, GsqWatcherClass))
#define GSQ_IS_WATCHER_CLASS(klass)	(G_TYPE_CHECK_CLASS_TYPE ((klass), GSQ_TYPE_WATCHER))
#define GSQ_WATCHER_GET_CLASS(obj)	(G_TYPE_INSTANCE_GET_CLASS ((obj), GSQ_TYPE_WATCHER, GsqWatcherClass))

typedef struct _GsqWatcher			GsqWatcher;
typedef struct _GsqWatcherPrivate	GsqWatcherPrivate;
typedef struct _GsqWatcherClass		GsqWatcherClass;

struct _GsqWatcher {
	GsqQuerier parent_instance;
	GsqWatcherPrivate *priv;
};

struct _GsqWatcherClass {
	GsqQuerierClass parent_class;
	
	void (*name_change) (GsqWatcher *watcher);
	void (*map_change) (GsqWatcher *watcher);
};


GType gsq_watcher_get_type (void) G_GNUC_CONST;
GsqQuerier* gsq_watcher_new (const gchar *address);

G_END_DECLS

#endif
