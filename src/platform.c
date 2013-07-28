/* 
 * platform.c
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


gchar *
gs_quote_win32_file_name (const gchar *fname)
{
	g_return_val_if_fail (fname != NULL, NULL);
	
	GString *s = g_string_sized_new (64);
	
	while (*fname) {
		switch (*fname) {
		case '\'':
			g_string_append (s, "'\\''");
			break;
		case '\\':
			g_string_append (s, "\\\\");
			break;
		default:
			g_string_append_c (s, *fname);
		}
		++fname;
	}
	
	return g_string_free (s, FALSE);
}


/* http://stackoverflow.com/a/1024937 */
gchar *
gs_get_executable_path ()
{
#if defined (G_OS_WIN32)
#else
	GFile *link = g_file_new_for_path ("/proc/self/exe");
	GFileInfo *info = g_file_query_info (link, G_FILE_ATTRIBUTE_STANDARD_SYMLINK_TARGET,
			G_FILE_QUERY_INFO_NONE, NULL, NULL);
	gchar *path = g_strdup (g_file_info_get_symlink_target (info));
	g_object_unref (info);
	g_object_unref (link);
	return path;
#endif
}
