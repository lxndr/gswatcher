/* 
 * utils.c
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