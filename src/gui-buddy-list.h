/*  
 *  gui-buddy-list.h
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





#ifndef __GSTOOL__BUDDY_LIST_H__
#define __GSTOOL__BUDDY_LIST_H__

#include <gtk/gtk.h>
#include "gstool.h"

GtkWidget *gui_blist_create ();
void gui_blist_add (GsBuddy *buddy);
void gui_blist_update (GsBuddy *buddy);

#endif
