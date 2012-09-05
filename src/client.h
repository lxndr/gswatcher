/* 
 * client.h: a layer between GsqQuerier and the GUI
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



#ifndef __GSWATCHER__CLIENT_H__
#define __GSWATCHER__CLIENT_H__


#include <gtk/gtk.h>
#include "query/watcher.h"
#include "query/console-source.h"
#include "query/console-telnet.h"


typedef enum _GuiConsoleSettings {
	GUI_CONSOLE_NONE = 1 << 0,
	GUI_CONSOLE_HOST = 1 << 1,
	GUI_CONSOLE_PORT = 1 << 2,
	GUI_CONSOLE_USER = 1 << 3,
	GUI_CONSOLE_PASS = 1 << 4,
	GUI_CONSOLE_ALL = GUI_CONSOLE_HOST | GUI_CONSOLE_PORT |
			GUI_CONSOLE_USER | GUI_CONSOLE_PASS
} GuiConsoleSettings;



#define GS_TYPE_CLIENT				(gs_client_get_type ())
#define GS_CLIENT(obj)				(G_TYPE_CHECK_INSTANCE_CAST ((obj), GS_TYPE_CLIENT, GsClient))
#define GS_CLIENT_CLASS(klass)		(G_TYPE_CHECK_CLASS_CAST ((klass), GS_TYPE_CLIENT, GsClientClass))
#define GS_IS_CLIENT(obj)			(G_TYPE_CHECK_INSTANCE_TYPE ((obj), GS_TYPE_CLIENT))
#define GS_IS_CLIENT_CLASS(klass)	(G_TYPE_CHECK_CLASS_TYPE ((klass), GS_TYPE_CLIENT))
#define GS_CLIENT_GET_CLASS(obj)	(G_TYPE_INSTANCE_GET_CLASS ((obj), GS_TYPE_CLIENT, GsClientClass))


typedef struct _GsClient GsClient;
typedef struct _GsClientClass GsClientClass;

struct _GsClient {
	GObject parent_object;
	
	/* querier */
	GsqQuerier *querier;
	gboolean favorite;
	gboolean password;
	gchar *version;
	const gchar *country_code;
	const gchar *country;
	GtkTreeIter sliter;			/* server list iterator */
	
	/* console */
	GsqConsole *console;
	GuiConsoleSettings console_settings;
	gchar *console_password;
	guint16 console_port;
	GtkTextBuffer *console_buffer;
	GtkListStore *console_history;
	
	/* log */
	gchar *log_address;
	GtkTextBuffer *log_buffer;
	gboolean log_auto;
	gboolean log_auto_scroll;
	gboolean log_output;
	gboolean log_wrapping;
	
	/* chat */
	GtkTextBuffer *chat_buffer;
};

struct _GsClientClass {
	GObjectClass parent_class;
};


GType gs_client_get_type (void) G_GNUC_CONST;
GsClient* gs_client_new (const gchar *address);

void gs_client_set_favorite (GsClient *client, gboolean favorite);
gboolean gs_client_get_favorite (GsClient *client);

gchar *gs_client_get_game_name (GsClient* client, gboolean extra);

void gs_client_set_console_port (GsClient *client, guint16 port);
guint16 gs_client_get_console_port (GsClient *client);
void gs_client_set_console_password (GsClient *client, const gchar *password);
const gchar *gs_client_get_console_password (GsClient *client);
void gs_client_send_command (GsClient* client, const gchar *cmd);

void gs_client_set_logaddress (const gchar *address);
gchar *gs_client_get_logaddress ();
void gs_client_enable_log (GsClient *client, gboolean enable);

void gs_client_send_message (GsClient *client, const gchar *msg);

void gs_client_set_connect_command (const gchar *command);
gchar *gs_client_get_connect_command ();
void gs_client_connect_to_game (GsClient *client);


#endif
