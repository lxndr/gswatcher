/* 
 * proto-source-console.c: Source Engine(TM) remote console implementation
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



#include <string.h>
#include "utils.h"
#include "console-private.h"
#include "proto-source-console.h"


static gboolean
source_send_packet (GSocket *socket, gint32 id, gint32 type, const gchar *cmd,
		GCancellable *cancellable, GError **error)
{
	id = GINT32_TO_LE (id);
	type = GINT32_TO_LE (type);
	
	gsize len = strlen (cmd) + 1;
	gint32 size = GINT32_TO_LE (len + 9);
	
	return	g_socket_send_all (socket, (gchar *) &size, 4, cancellable, error) &&
			g_socket_send_all (socket, (gchar *) &id, 4, cancellable, error) &&
			g_socket_send_all (socket, (gchar *) &type, 4, cancellable, error) &&
			g_socket_send_all (socket, cmd, len, cancellable, error) &&
			g_socket_send_all (socket, "", 1, cancellable, error);
}

#define source_send_password(sock, pass, cancel, error)	\
			source_send_packet (sock, 0, 3, pass, cancel, error)
#define source_send_command(sock, cmd, cancel, error)	\
			source_send_packet (sock, 0, 2, cmd, cancel, error)


gboolean
gsq_source_console_init (GsqConsole *console, GSocket *socket,
		GCancellable *cancellable, GError **error)
{
	return source_send_password (socket, gsq_console_get_password (console),
			cancellable, error);
}

gboolean
gsq_source_console_send (GsqConsole *console, GSocket *socket, const gchar *command,
		GCancellable *cancellable, GError **error)
{
	return source_send_command (socket, command, cancellable, error);
}


static gboolean
source_recv_packet (GSocket *socket, gint32 *id, gint32 *type, gchar *buffer,
		GCancellable *cancellable, GError **error)
{
	gint32 size;
	
	if (!g_socket_receive_all (socket, (gchar *) &size, 4, cancellable, error))
		return FALSE;
	size = GINT32_FROM_LE (size);
	if (size < 10 || size > 4106) {
		g_set_error_literal (error, 0, 0, "The server has sent an incorrect package");
		return FALSE;
	}
	
	if (!g_socket_receive_all (socket, (gchar *) id, 4, cancellable, error))
		return FALSE;
	*id = GINT32_FROM_LE (*id);
	
	if (!g_socket_receive_all (socket, (gchar *) type, 4, cancellable, error))
		return FALSE;
	*type = GINT32_FROM_LE (*type);
	if (!(*type == 0 || *type == 2)) {
		g_set_error_literal (error, G_IO_ERROR, G_IO_ERROR_INVALID_DATA,
				"The server has sent an incorrect package");
		return FALSE;
	}
	
	return g_socket_receive_all (socket, buffer, size - 8, cancellable, error);
}

gboolean
gsq_source_console_process (GsqConsole *console, GSocket *socket,
		GCancellable *cancellable, GError **error)
{
	gint32 id, type;
	char buffer[4096 + 2];
	
	if (!source_recv_packet (socket, &id, &type, buffer, cancellable, error))
		return FALSE;
	
	if (type == 2) {
		if (id == -1) {
			g_set_error (error, 0, 0, "Authentication attempt failed");
			return FALSE;
		} else {
			gsq_console_set_connected (console);
		}
	} else if (type == 0) {
		if (gsq_console_is_connected (console)) {
			gchar *str = gsq_utf8_repair (buffer);
			g_signal_emit_by_name (console, "respond", str);
			g_free (str);
		}
	}
	
	return TRUE;
}
