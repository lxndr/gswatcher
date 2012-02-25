/* 
 * console.h
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



#ifndef __GSQUERY__CONSOLE_H__
#define __GSQUERY__CONSOLE_H__

#include <glib.h>
#include <glib-object.h>
#include <gio/gio.h>

G_BEGIN_DECLS


GQuark gsq_console_error_quark (void);
#define GSQ_CONSOLE_ERROR gsq_console_error_quark ()

typedef enum {
	GSQ_CONSOLE_ERROR_PASS,
	GSQ_CONSOLE_ERROR_DATA,
	GSQ_CONSOLE_ERROR_AUTH,
	GSQ_CONSOLE_ERROR_TIMEOUT
} GsqConsoleErrorEnum;



#define GSQ_TYPE_CONSOLE			(gsq_console_get_type ())
#define GSQ_CONSOLE(obj)			(G_TYPE_CHECK_INSTANCE_CAST ((obj), GSQ_TYPE_CONSOLE, GsqConsole))
#define GSQ_IS_CONSOLE(obj)			(G_TYPE_CHECK_INSTANCE_TYPE ((obj), GSQ_TYPE_CONSOLE))
#define GSQ_CONSOLE_CLASS(klass)	(G_TYPE_CHECK_CLASS_CAST ((klass), GSQ_TYPE_CONSOLE, GsqConsoleClass))
#define GSQ_IS_CONSOLE_CLASS(klass)	(G_TYPE_CHECK_CLASS_TYPE ((klass), GSQ_TYPE_CONSOLE))
#define GSQ_CONSOLE_GET_CLASS(obj)	(G_TYPE_INSTANCE_GET_CLASS ((obj), GSQ_TYPE_CONSOLE, GsqConsoleClass))

typedef struct _GsqConsole			GsqConsole;
typedef struct _GsqConsolePrivate	GsqConsolePrivate;
typedef struct _GsqConsoleClass		GsqConsoleClass;

struct _GsqConsole {
	GObject parent_instance;
	GsqConsolePrivate *priv;
};

struct _GsqConsoleClass {
	GObjectClass parent_class;
	
	void (*connect) (GsqConsole *console);
	void (*disconnect) (GsqConsole *console);
};

GType gsq_console_get_type (void) G_GNUC_CONST;
GsqConsole* gsq_console_new (const gchar *address);

void gsq_console_set_password (GsqConsole *console, const gchar *password);
const gchar *gsq_console_get_password (GsqConsole *console);

void gsq_console_set_timeout (GsqConsole *console, guint timeout);
guint gsq_console_get_timeout (GsqConsole *console);

void gsq_console_send (GsqConsole *console, const gchar *command,
		GAsyncReadyCallback callback, gpointer udata);
gchar *gsq_console_send_finish (GsqConsole *console, GAsyncResult *result,
		GError **error);

gboolean gsq_console_is_connected (GsqConsole *console);
guint gsq_console_queue_length (GsqConsole *console);


G_END_DECLS

#endif
