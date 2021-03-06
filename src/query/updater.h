/* 
 * updater.h
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



#ifndef __GSQUERY__UPDATER_H__
#define __GSQUERY__UPDATER_H__

#include <glib.h>
#include <glib-object.h>
#include "querier.h"

G_BEGIN_DECLS


#define GSQ_TYPE_UPDATER			(gsq_updater_get_type ())
#define GSQ_UPDATER(obj)			(G_TYPE_CHECK_INSTANCE_CAST ((obj), GSQ_TYPE_UPDATER, GsqUpdater))
#define GSQ_IS_UPDATER(obj)			(G_TYPE_CHECK_INSTANCE_TYPE ((obj), GSQ_TYPE_UPDATER))
#define GSQ_UPDATER_CLASS(klass)	(G_TYPE_CHECK_CLASS_CAST ((klass), GSQ_TYPE_UPDATER, GsqUpdaterClass))
#define GSQ_IS_UPDATER_CLASS(klass)	(G_TYPE_CHECK_CLASS_TYPE ((klass), GSQ_TYPE_UPDATER))
#define GSQ_UPDATER_GET_CLASS(obj)	(G_TYPE_INSTANCE_GET_CLASS ((obj), GSQ_TYPE_UPDATER, GsqUpdaterClass))

typedef struct _GsqUpdater			GsqUpdater;
typedef struct _GsqUpdaterPrivate	GsqUpdaterPrivate;
typedef struct _GsqUpdaterClass		GsqUpdaterClass;

struct _GsqUpdater {
	GObject parent_instance;
	GsqUpdaterPrivate *priv;
};

struct _GsqUpdaterClass {
	GObjectClass parent_class;
};

GType gsq_updater_get_type (void) G_GNUC_CONST;
GsqUpdater* gsq_updater_new ();

void gsq_updater_add (GsqUpdater *updater, GsqQuerier *querier);
void gsq_updater_take (GsqUpdater *updater, GsqQuerier *querier);
void gsq_updater_remove (GsqUpdater *updater, GsqQuerier *querier);
GList *gsq_updater_list (GsqUpdater *updater);

void gsq_updater_set_interval (GsqUpdater *updater, gfloat interval);
gfloat gsq_updater_get_interval (GsqUpdater *updater);

void gsq_updater_set_pause (GsqUpdater *updater, gboolean pause);
gboolean gsq_updater_get_pause (GsqUpdater *updater);


G_END_DECLS

#endif
