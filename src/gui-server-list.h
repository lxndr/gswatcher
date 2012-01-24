/*  
 *  gui-server-list.h
 *  Copyright (C) lxndr 2011 <lxndr87@users.sourceforge.net>
 *  
 *  gstool is free software: you can redistribute it and/or modify it
 *  under the terms of the GNU General Public License as published by the
 *  Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *  
 *  gstool is distributed in the hope that it will be useful, but
 *  WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *  See the GNU General Public License for more details.
 *  
 *  You should have received a copy of the GNU General Public License along
 *  with this program.  If not, see <http://www.gnu.org/licenses/>.
 */



#ifndef __GSTOOL__SERVER_LIST_H__
#define __GSTOOL__SERVER_LIST_H__

#include <gtk/gtk.h>
#include "gstool.h"


typedef enum _GuiGameColumnMode {
	GUI_GAME_COLUMN_ICON,		// icon only
	GUI_GAME_COLUMN_NAME,		// game name
	GUI_GAME_COLUMN_FULL		// game name and game mode if known
} GuiGameColumnMode;


GtkWidget * gui_slist_create ();
GsClient *gui_slist_get_selected ();
void gui_slist_add (GsClient *client);
void gui_slist_set_hscrollbar (gboolean hscrollbar);
void gui_slist_update_all ();

void gui_slist_set_game_column_mode (GuiGameColumnMode mode);
GuiGameColumnMode gui_slist_get_game_column_mode ();


#endif