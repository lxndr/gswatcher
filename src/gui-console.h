/* 
 * gui-console.h
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



#ifndef __GUI__CONSOLE_H__
#define __GUI__CONSOLE_H__

#include <gtk/gtk.h>
#include "client.h"


typedef enum _GsLogType {
	GUI_CONSOLE_RESPOND,
	GUI_CONSOLE_INFO,
	GUI_CONSOLE_COMMAND,
	GUI_CONSOLE_ERROR
} GsLogType;


GtkWidget *gs_console_create ();
GtkWidget *gs_console_create_bar ();
void gs_console_init (GsClient *client);
void gs_console_free (GsClient *client);
void gui_console_setup (GsClient *client);
void gs_console_send (GsClient *client, const gchar *cmd);
void gs_console_log (GsClient *client, GsLogType type, const gchar *msg);

void gui_console_set_use_system_font (gboolean sysfont);
gboolean gui_console_get_use_system_font ();

void gui_console_set_font (const gchar *name);
const gchar *gui_console_get_font ();


#endif
