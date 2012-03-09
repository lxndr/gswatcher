/* 
 * platform.c
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



#include <glib/gstdio.h>
#include "platform.h"


gchar *
gs_get_system_font ()
{
#if defined (G_OS_UNIX) && !defined (G_OS_MACOSX)
	/* in case if it's gnome 3 */
	GSettings *sysconfig = g_settings_new ("org.gnome.desktop.interface");
	gchar *font = g_settings_get_string (sysconfig, "monospace-font-name");
	if (font) {
		g_object_unref (sysconfig);
		return font;
	}
	g_object_unref (sysconfig);
#endif
	
	return g_strdup (GS_DEFAULT_MONOSPACE_FONT);
}
