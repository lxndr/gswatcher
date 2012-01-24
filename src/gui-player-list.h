#ifndef __GSTOOL__PLAEYR_LIST_H__
#define __GSTOOL__PLAYER_LIST_H__

#include <gtk/gtk.h>
#include "client.h"

GtkWidget *gui_plist_create ();
GtkWidget *gui_plist_create_bar ();
void gui_plist_setup (GsClient *client);
void gui_plist_update (GsClient *client);


#endif