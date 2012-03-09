/* 
 * gstool.h
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



#ifndef __GSTOOL_H__
#define __GSTOOL_H__

#include <glib.h>
#include <glib-object.h>
#include <gio/gio.h>
#include "client.h"


#define GS_TYPE_APPLICATION				(gs_application_get_type ())
#define GS_APPLICATION(obj)				(G_TYPE_CHECK_INSTANCE_CAST ((obj), GS_TYPE_APPLICATION, GsApplication))
#define GS_IS_APPLICATION(obj)			(G_TYPE_CHECK_INSTANCE_TYPE ((obj), GS_TYPE_APPLICATION))
#define GS_APPLICATION_CLASS(klass)		(G_TYPE_CHECK_CLASS_CAST ((klass), GS_TYPE_APPLICATION, GsApplicationClass))
#define GS_IS_APPLICATION_CLASS(klass)	(G_TYPE_CHECK_CLASS_TYPE ((klass), GS_TYPE_APPLICATION))
#define GS_APPLICATION_GET_CLASS(obj)	(G_TYPE_INSTANCE_GET_CLASS ((obj), GS_TYPE_APPLICATION, GsApplicationClass))


typedef struct _GsApplication		GsApplication;
typedef struct _GsApplicationClass	GsApplicationClass;
typedef struct _GsBuddy GsBuddy;

struct _GsApplication {
	GtkApplication parent_instance;
	
	gchar *buddy_list_path;
	gchar *server_list_path;
	gchar *preferences_path;
	gchar *icon_dir;
	gchar *pixmap_dir;
	gchar *sound_dir;
#ifdef ENABLE_NLS
	gchar *locale_dir;
#endif
	
	GHashTable *buddy_list;
	gdouble update_rate;
	gboolean pause;
	guint timer;
	GList *server_list;
	GList *server_iter;
	gchar **specific_servers;
};

struct _GsApplicationClass {
	GtkApplicationClass parent_class;
};

struct _GsBuddy {
	gchar *name;
	GDateTime *lastseen;
	gchar *lastaddr;
	GList *servers;
	gboolean notify;
};


extern GsApplication *app;


GType gs_application_get_type (void) G_GNUC_CONST;
GsApplication* gs_application_new ();
void gs_application_shutdown (GsApplication *app);

void gs_application_set_update_rate (GsApplication *app, gdouble rate);
gdouble gs_application_get_update_rate (GsApplication *app);

void gs_application_set_pause (GsApplication *app, gboolean pause);
gboolean gs_application_get_pause (GsApplication *app);

GsClient *gs_application_add_server (GsApplication *app, const gchar *address);
void gs_application_remove_server (GsApplication *app, GsClient *client);
GsClient *gs_application_find_server (GsApplication *app, const gchar *address);
GList *gs_application_server_list (GsApplication *app);
void gs_application_save_server_list (GsApplication *app);

GsBuddy *gs_application_add_buddy (GsApplication *app, const gchar *name,
		gint64 lastseen, const gchar *lastaddr, gboolean notify);
void gs_application_change_buddy (GsApplication *app, const gchar *name,
		gboolean notify);
void gs_application_remove_buddy (GsApplication *app, const gchar *name);
GsBuddy *gs_application_find_buddy (GsApplication *app, const gchar *name);

void gs_application_save_preferences (GsApplication *app);


#endif
