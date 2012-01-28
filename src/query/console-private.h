/* 
 * console-private.h: private functions for pritocol implementations
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



#ifndef __GSQUERY__CONSOLE_PRIVATE_H__
#define __GSQUERY__CONSOLE_PRIVATE_H__

#include <gio/gio.h>
#include "console.h"

gboolean g_socket_send_all (GSocket *socket, const gchar *buffer, gsize size,
		GCancellable *cancellable, GError **error);
gboolean g_socket_receive_all (GSocket *socket, gchar *buffer, gsize size,
		GCancellable *cancellable, GError **error);

GSocket *gsq_console_get_socket (GsqConsole *console);
void gsq_console_set_connected (GsqConsole *console);

#endif
