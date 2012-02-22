#ifndef __GSTOOL__LOG_H__
#define __GSTOOL__LOG_H__

#include <gtk/gtk.h>

GtkWidget *gui_log_create ();
GtkWidget *gui_log_create_bar ();
void gui_log_init (GsClient *client);
void gui_log_set (GsClient *client);
void gui_log_print (GsClient *client, const gchar *msg);

#endif