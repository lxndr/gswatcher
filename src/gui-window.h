/* 
 * gui-window.h
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



#ifndef __GUI__WINDOW_H__
#define __GUI__WINDOW_H__


#include <glib/gi18n-lib.h>
#include <gtk/gtk.h>
#include "json.h"
#include "gstool.h"


extern GtkWidget *window;


// window
GtkWidget *gui_window_create ();
void gui_window_destroy ();

void gui_window_show ();
void gui_window_hide ();

void gui_window_load_geometry (GJsonNode *geometry);
GJsonNode *gui_window_save_geometry ();


#endif
