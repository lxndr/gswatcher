#ifndef __GSTOOL__CHAT_H__
#define __GSTOOL__CHAT_H__

#include <gtk/gtk.h>
#include "client.h"


GtkWidget *gui_chat_create ();
void gui_chat_init (GsClient *client);
void gui_chat_free (GsClient *client);
void gui_chat_set (GsClient *client);
void gui_chat_send (GsClient *client, const gchar *cmd);
void gui_chat_log (GsClient *client, const gchar *name, gint team, const gchar *msg);


#endif
