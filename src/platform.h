/* 
 * platform.h
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



#ifndef __GSTOOL__PLATFORM_H__
#define __GSTOOL__PLATFORM_H__

#include <glib.h>
#include <gtk/gtk.h>


void gs_initialize_directories (const gchar *exe);

const gchar *gs_get_buddy_list_path ();
const gchar *gs_get_server_list_path ();
const gchar *gs_get_preferences_path ();

const gchar *gs_get_locale_dir ();
const gchar *gs_get_icon_dir ();
const gchar *gs_get_sound_dir ();
const gchar *gs_get_pixmap_dir ();


#if GTK_MAJOR_VERSION == 2
gboolean gtk_tree_model_iter_previous (GtkTreeModel *model, GtkTreeIter *iter);
#endif

#ifdef G_OS_WIN32
	#define GS_DEFAULT_MONOSPACE_FONT "Courier New 10"
#else
	#define GS_DEFAULT_MONOSPACE_FONT "Monospace 9"
#endif


gchar *
gs_get_system_font ();


#endif
