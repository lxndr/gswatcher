#ifndef __GSQUERY__SOURCE_CONSOLE_H__
#define __GSQUERY__SOURCE_CONSOLE_H__

#include "console.h"

gboolean gsq_source_console_init (GsqConsole *console, GSocket *socket,
		GCancellable *cancellable, GError **error);
gboolean gsq_source_console_send (GsqConsole *console, GSocket *socket,
		const gchar *command, GCancellable *cancellable, GError **error);
gboolean gsq_source_console_process (GsqConsole *console, GSocket *socket,
		GCancellable *cancellable, GError **error);

#endif
