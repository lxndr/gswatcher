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
