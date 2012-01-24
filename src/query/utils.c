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