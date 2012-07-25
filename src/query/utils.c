/* 
 * utils.c
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



#include <stdlib.h>
#include <string.h>
#include <glib/gprintf.h>
#include "utils.h"


gchar *
gsq_utf8_repair (const gchar *text)
{
	g_return_val_if_fail (text != NULL, NULL);
	
	gchar *p = (gchar *) text, *invalid;
	GString *str = g_string_sized_new (256);
	
	while (!g_utf8_validate (p, -1, (const gchar **) &invalid)) {
		g_string_append_len (str, p, invalid - p);
		g_string_append (str, "\357\277\275");
		p = invalid + 1;
	}
	
	g_string_append (str, p);
	return g_string_free (str, FALSE);
}


void
gsq_print_dump (const gchar *data, gsize length)
{
	g_return_if_fail (data != NULL);
	
	gint len, i;
	gchar *p = (gchar *) data;
	
	while (p - data < length) {
		if (length - (p - data) >= 16)
			len = 16;
		else
			len = length - (p - data);
		
		g_printf ("\t");
		for (i = 0; i < len; i++)
			g_printf ("%02X ", (guint8) p[i]);
		for (; i < 16; i++)
			g_printf ("   ");
		g_printf ("   ");
		for (i = 0; i < len; i++)
			g_printf ("%c", g_ascii_isprint (p[i]) ? p[i] : '.');
		g_printf ("\n");
		
		p += len;
	}
}


gchar *
gsq_parse_address (const gchar *addr, guint16 *port, guint16 *port2)
{
	g_return_val_if_fail (addr != NULL, NULL);
	
	gchar *p = (gchar *) addr, *host = NULL, *f;
	if (port)
		*port = 0;
	if (port2)
		*port2 = 0;
	
	// host name
	if (*p == '[') {	// most likely IPv6
		p++;
		f = strchr (p, ']');
		if (f) {
			host = g_strndup (p, f - p);
			p = f + 1;
			if (*p == ':')
				p++;
		} else {
			// error: missing ']'
		}
	} else {
		f = strchr (p, ':');
		if (f) {
			if (f[1] == ':') {	// '::' looks like IPv6 with no port(s)
				return g_strdup (addr);
			} else {			// either Name or IPv4 with port(s)
				host = g_strndup (p, f - p);
				p = f + 1;
			}
		} else {		// anything with no port(s)
			return g_strdup (addr);
		}
	}
	
	// port(s)
	f = strchr (p, ':');
	if (f) {	// there is port2
		if (port)
			*port = atoi (p);
		if (port2)
			*port2 = atoi (f + 1);
	} else {	// port only
		if (port)
			*port = atoi (p);
	}
	
	return host;
}


gchar *
gsq_lookup_value (GHashTable *values, ...)
{
	va_list va;
	va_start (va, values);
	
	gchar *key, *value = NULL;
	while ((key = va_arg (va, gchar *))) {
		value = g_hash_table_lookup (values, key);
		if (value)
			break;
	}
	
	va_end (va);
	return value;
}


gboolean
gsq_str2bool (const gchar *str)
{
	if (!str)
		return FALSE;
	gchar *s = g_ascii_strdown (str, -1);
	gboolean ret = strcmp (s, "true") == 0 || strcmp (s, "1") == 0 ||
			strcmp (s, "yes") == 0 || strcmp (s, "on") == 0;
	g_free (s);
	return ret;
}
