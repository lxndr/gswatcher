/* 
 * gui-server-list.h
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



#ifndef __GUI__SERVER_LIST_H__
#define __GUI__SERVER_LIST_H__

#include <gtk/gtk.h>
#include "gswatcher.h"


typedef enum _GuiGameColumnMode {
	GUI_GAME_COLUMN_ICON,		/* icon only */
	GUI_GAME_COLUMN_NAME,		/* game name */
	GUI_GAME_COLUMN_FULL		/* game name and game mode if known */
} GuiGameColumnMode;


GtkWidget * gui_slist_create ();
void gui_slist_add (GsClient *cl);
void gui_slist_remove (GsClient *cl);
void gui_slist_update (GsClient *cl);
void gui_slist_update_list (GList *ls);

GsClient *gui_slist_get_selected ();
void gui_slist_set_hscrollbar (gboolean hscrollbar);
void gui_slist_set_visible (gboolean seen);

void gui_slist_set_game_column_mode (GuiGameColumnMode mode);
GuiGameColumnMode gui_slist_get_game_column_mode ();


#endif
