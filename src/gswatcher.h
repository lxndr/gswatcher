/* 
 * gswatcher.h
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



#ifndef __GSWATCHER_H__
#define __GSWATCHER_H__

#include <glib.h>
#include <glib-object.h>
#include <gio/gio.h>
#include "client.h"


typedef struct _GsBuddy GsBuddy;
struct _GsBuddy {
	gchar *name;
	GDateTime *lastseen;
	gchar *lastaddr;
	GList *servers;
	gboolean notify;
};



void gs_application_shutdown ();

void gs_application_set_interval (gdouble interval);
gdouble gs_application_get_interval ();

void gs_application_set_pause (gboolean pause);
gboolean gs_application_get_pause ();

void gs_application_set_default_port (guint16 port);
guint16 gs_application_get_default_port ();

GsClient *gs_application_add_server (const gchar *address);
void gs_application_remove_server (GsClient *client);
void gs_application_remove_server_ask (GsClient *client);
GsClient *gs_application_find_server (const gchar *address);
GList *gs_application_server_list ();
void gs_application_save_server_list ();

GsBuddy *gs_application_add_buddy (const gchar *name, gint64 lastseen,
		const gchar *lastaddr, gboolean notify);
void gs_application_change_buddy (const gchar *name, gboolean notify);
void gs_application_remove_buddy (const gchar *name);
GsBuddy *gs_application_find_buddy (const gchar *name);

const gchar *gs_application_get_icon_dir ();
const gchar *gs_application_get_sound_dir ();
void gs_application_save_preferences ();



#endif
