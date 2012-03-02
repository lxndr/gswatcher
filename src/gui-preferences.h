/* 
 * gui-preferences.h
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



#ifndef __GSTOOL__PREFERENCES_H__
#define __GSTOOL__PREFERENCES_H__

#include <gtk/gtk.h>
#include "gstool.h"


GtkWidget *gs_prefs_create ();

void gui_prefs_set_update_rate (gdouble rate);
void gui_prefs_set_game_column_mode (GuiGameColumnMode mode);
void gui_prefs_set_port (guint16 port);
void gui_prefs_set_connect_command (const gchar *command);
void gui_prefs_set_enable_notifications (gboolean enable);
void gui_prefs_set_notification_sound (const gchar *filename);
void gui_prefs_set_use_system_font (gboolean use);
void gui_prefs_set_font (const gchar *fontname);
void gui_prefs_set_logaddress (const gchar *address);


#endif
