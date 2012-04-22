/* 
 * gui-notification.h
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



#ifndef __GUI__NOTIFICATION_H__
#define __GUI__NOTIFICATION_H__

#include <glib.h>

void gs_notification_init (gboolean notifier);

void gs_notification_set_enable (gboolean enable);
gboolean gs_notification_get_enable ();

void gs_notification_set_playbin (const gchar *cmd);
void gs_notification_set_sound (const gchar *sound);
const gchar *gs_notification_get_sound ();
void gs_notification_sound ();

void gs_notification_set_timeout (gint time);
gint notification_get_timeout ();

void gs_notification_message (const gchar *summary, const gchar *body);

#endif
