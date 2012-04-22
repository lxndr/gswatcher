/* 
 * platform.h
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



#ifndef __GSTOOL__PLATFORM_H__
#define __GSTOOL__PLATFORM_H__

#include <glib.h>
#include <gtk/gtk.h>


#ifdef G_OS_WIN32
	#define GS_DEFAULT_MONOSPACE_FONT "Courier New 10"
#else
	#define GS_DEFAULT_MONOSPACE_FONT "Monospace 9"
#endif


gchar *gs_get_system_font ();
gchar *gs_quote_win32_file_name (const gchar *fname);

#endif
