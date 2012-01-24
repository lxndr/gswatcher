#ifndef __GSTOOL__CONSOLE_H__
#define __GSTOOL__CONSOLE_H__

#include <gtk/gtk.h>
#include "client.h"


typedef enum _GsLogType {
	GS_CONSOLE_RESPOND,
	GS_CONSOLE_INFO,
	GS_CONSOLE_COMMAND,
	GS_CONSOLE_ERROR
} GsLogType;


GtkWidget *gs_console_create ();
GtkWidget *gs_console_create_bar ();
void gs_console_init (GsClient *client);
void gs_console_free (GsClient *client);
void gs_console_set (GsClient *client);
void gs_console_send (GsClient *client, const gchar *cmd);
void gs_console_log (GsClient *client, GsLogType type, const gchar *msg);

void gui_console_set_use_system_font (gboolean sysfont);
gboolean gui_console_get_use_system_font ();

void gui_console_set_font (const gchar *name);
const gchar *gui_console_get_font ();


#endif
