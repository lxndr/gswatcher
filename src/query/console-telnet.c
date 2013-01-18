/* 
 * console-telnet.c: remote console via telnet protocol
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



#include "console-telnet.h"


static void gsq_console_telnet_finalize (GObject *object);
static void gsq_console_telnet_connected (GsqConsole *console);
static gboolean gsq_console_telnet_command (GsqConsole *console,
		const gchar *command, GError **error);
static gboolean gsq_console_telnet_received (GsqConsole *console,
		const gchar *data, gsize length, GError **error);
static void gsq_console_telnet_reset (GsqConsole *console);


G_DEFINE_TYPE (GsqConsoleTelnet, gsq_console_telnet, GSQ_TYPE_CONSOLE);


static void
gsq_console_telnet_class_init (GsqConsoleTelnetClass *class)
{
	GObjectClass *gobject_class = G_OBJECT_CLASS (class);
	gobject_class->finalize = gsq_console_telnet_finalize;
	
	GsqConsoleClass *console_class = GSQ_CONSOLE_CLASS (class);
	console_class->connected = gsq_console_telnet_connected;
	console_class->command = gsq_console_telnet_command;
	console_class->received = gsq_console_telnet_received;
	console_class->reset = gsq_console_telnet_reset;
}


static void
gsq_console_telnet_init (GsqConsoleTelnet *con)
{
}


static void
gsq_console_telnet_finalize (GObject *object)
{
	G_OBJECT_CLASS (gsq_console_telnet_parent_class)->finalize (object);
}


GsqConsole *
gsq_console_telnet_new (const gchar *host, guint16 port)
{
	g_return_val_if_fail (host != NULL, NULL);
	return g_object_new (GSQ_TYPE_CONSOLE_TELNET,
			"host", host, "port", port, NULL);
}


static void
gsq_console_telnet_connected (GsqConsole *console)
{
	g_print ("telnet activated\n");
}


static gboolean
gsq_console_telnet_command (GsqConsole *console, const gchar *command,
		GError **error)
{
	return FALSE;
}


static gboolean
gsq_console_telnet_received (GsqConsole *console, const gchar *data,
		gsize length, GError **error)
{
	return FALSE;
}


static void
gsq_console_telnet_reset (GsqConsole *console)
{
	
}
