#ifndef __GSTOOL__GUI_INFO_H__
#define __GSTOOL__GUI_INFO_H__

#include <gtk/gtk.h>
#include "client.h"


GtkWidget *gui_info_create ();
void gui_info_update (GsClient *client);


#endif