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
#include "client.h"


typedef struct _GsBuddy {
	gchar *name;
	GDateTime *lastseen;
	gchar *lastaddr;
	GList *servers;
	gboolean notify;
} GsBuddy;


GsBuddy *gs_add_buddy (const gchar *name, gint64 lastseen, const gchar *lastaddr,
		gboolean notify);
void gs_change_buddy (const gchar *name, gboolean notify);
void gs_remove_buddy (const gchar *name);
GsBuddy *gs_find_buddy (const gchar *name);

GsClient *gs_add_server (const gchar *address);
void gs_remove_server (GsClient *client);
GsClient *gs_find_server (const gchar *address);
GList *gs_get_server_list ();
void gs_save_server_list ();

void gs_set_update_rate (gdouble rate);
gdouble gs_get_update_rate ();

void gs_pause (gboolean pause);
gboolean gs_is_pause ();

void gs_save_preferences ();

#endif
