/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*- */
/* 
 * gui-window.h
 * Copyright (C) Alexander AB 2012 <lxndr87@users.sourceforge.net>
 * 
 * gswatcher is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * GSWatcher is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License along
 * with this program.  If not, see <http://www.gnu.org/licenses/>.
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