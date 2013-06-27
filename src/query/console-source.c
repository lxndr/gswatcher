/* 
 * console-source.c: source engine remote console
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



#include <string.h>
#include <stdio.h>
#include <glib/gprintf.h>
#include "console-source.h"



struct _GsqConsoleSourcePrivate {
	/* packet */
	gint32 size;
	gint32 id;
	gint32 type;
	
	/*  */
	GString *res;
};


static void gsq_console_source_finalize (GObject *object);
static void gsq_console_source_connected (GsqConsole *console);
static gboolean gsq_console_source_command (GsqConsole *console,
		const gchar *msg, GError **error);
static gboolean gsq_console_source_received (GsqConsole *console,
		const gchar *data, gsize length, GError **error);
static void gsq_console_source_reset (GsqConsole *console);


G_DEFINE_TYPE (GsqConsoleSource, gsq_console_source, GSQ_TYPE_CONSOLE);


static void
gsq_console_source_class_init (GsqConsoleSourceClass *class)
{
	GObjectClass *gobject_class = G_OBJECT_CLASS (class);
	gobject_class->finalize = gsq_console_source_finalize;
	
	GsqConsoleClass *console_class = GSQ_CONSOLE_CLASS (class);
	console_class->connected = gsq_console_source_connected;
	console_class->command = gsq_console_source_command;
	console_class->received = gsq_console_source_received;
	console_class->reset = gsq_console_source_reset;
	
	g_type_class_add_private (gobject_class, sizeof (GsqConsoleSourcePrivate));
}


static void
gsq_console_source_init (GsqConsoleSource *console)
{
	console->priv = G_TYPE_INSTANCE_GET_PRIVATE (console,
			GSQ_TYPE_CONSOLE_SOURCE, GsqConsoleSourcePrivate);
	gsq_console_set_chunk_size (GSQ_CONSOLE (console), 12);
	console->priv->res = g_string_sized_new (64);
}


static void
gsq_console_source_finalize (GObject *object)
{
	g_string_free (GSQ_CONSOLE_SOURCE (object)->priv->res, TRUE);
	
	G_OBJECT_CLASS (gsq_console_source_parent_class)->finalize (object);
}


GsqConsole *
gsq_console_source_new (const gchar *host, guint16 port)
{
	g_return_val_if_fail (host != NULL, NULL);
	return g_object_new (GSQ_TYPE_CONSOLE_SOURCE,
			"host", host, "port", port == 0 ? 27015 : port, NULL);
}


static gboolean
source_send_packet (GsqConsole *console, gint32 id, gint32 type,
		const gchar *command, GError **error)
{
	id = GINT32_TO_LE (id);
	type = GINT32_TO_LE (type);
	
	gsize len = strlen (command) + 1;
	gint32 size = GINT32_TO_LE (len + 9);
	
	return	gsq_console_send_data (console, (gchar *) &size, 4, error) &&
			gsq_console_send_data (console, (gchar *) &id, 4, error) &&
			gsq_console_send_data (console, (gchar *) &type, 4, error) &&
			gsq_console_send_data (console, command, len, error) &&
			gsq_console_send_data (console, "", 1, error);
}

#define source_send_password(sock, pass, error)	\
			source_send_packet (sock, 0, 3, pass, error)

static gboolean
gsq_console_source_command (GsqConsole *console, const gchar *msg, GError **error)
{
	return source_send_packet (console, 0, 2, msg, error) &&
		source_send_packet (console, 1, 2, "", error);
}


static void
gsq_console_source_connected (GsqConsole *console)
{
	source_send_password (console, gsq_console_get_password (console), NULL);
}


static gboolean
gsq_console_source_received (GsqConsole *console, const gchar *data,
		gsize length, GError **error)
{
	GsqConsoleSourcePrivate *pr = GSQ_CONSOLE_SOURCE (console)->priv;
	
	/* header */
	if (pr->size == 0) {
		pr->size = GINT32_FROM_LE (* (gint32 *) (data + 0));
		pr->id = GINT32_FROM_LE (* (gint32 *) (data + 4));
		pr->type = GINT32_FROM_LE (* (gint32 *) (data + 8));
		if (pr->size < 10 || pr->size > 4106 || !(pr->type == 0 || pr->type == 2)) {
			g_set_error_literal (error, GSQ_CONSOLE_ERROR,
					GSQ_CONSOLE_ERROR_DATA, "Odd data has been received");
			return FALSE;
		}
		
		gsq_console_set_chunk_size (console, pr->size - 8);
		
	/* data */
	} else {
		if (pr->type == 2) {
			if (pr->id == -1) {
				g_set_error_literal (error, GSQ_CONSOLE_ERROR, GSQ_CONSOLE_ERROR_AUTH,
						"Authentication attempt failed");
				return FALSE;
			} else {
				gsq_console_authenticate (console);
			}
		} else {
			if (gsq_console_is_authenticated (console)) {
				g_string_append (pr->res, data);
				if (pr->id == 1)
					gsq_console_finish_respond (console, pr->res->str);
			}
		}
		
		/* clear everything for next packet */
		gsq_console_source_reset (console);
	}
	
	return TRUE;
}


static void
gsq_console_source_reset (GsqConsole *console)
{
	GsqConsoleSourcePrivate *pr = GSQ_CONSOLE_SOURCE (console)->priv;
	gsq_console_set_chunk_size (console, 12);
	pr->size = 0;
	g_string_truncate (pr->res, 0);
}
