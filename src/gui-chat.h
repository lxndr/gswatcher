/* 
 * gui-chat.c
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



#ifndef __GSTOOL__CHAT_H__
#define __GSTOOL__CHAT_H__

#include <gtk/gtk.h>
#include "client.h"


GtkWidget *gui_chat_create ();
void gui_chat_init (GsClient *client);
void gui_chat_free (GsClient *client);
void gui_chat_setup (GsClient *client);
void gui_chat_send (GsClient *client, const gchar *cmd);
void gui_chat_log (GsClient *client, const gchar *name, gint team, const gchar *msg);


#endif
