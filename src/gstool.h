#ifndef __GSTOOL_H__
#define __GSTOOL_H__

#include <glib.h>
#include "client.h"


typedef struct _GsBuddy {
	gchar *name;
	GDateTime *lastseen;
	gchar *lastaddr;
	GList *servers;
	gboolean notify;
} GsBuddy;


GsBuddy *gs_add_buddy (const gchar *name, gint64 lastseen, const gchar *lastaddr,
		gboolean notify);
void gs_change_buddy (const gchar *name, gboolean notify);
void gs_remove_buddy (const gchar *name);
GsBuddy *gs_find_buddy (const gchar *name);

GsClient *gs_add_server (const gchar *address);
void gs_remove_server (GsClient *client);
GsClient *gs_find_server (const gchar *address);
GList *gs_get_server_list ();
void gs_save_server_list ();

void gs_set_update_rate (gdouble rate);
gdouble gs_get_update_rate ();

void gs_pause (gboolean pause);
gboolean gs_is_pause ();

void gs_save_preferences ();

#endif
