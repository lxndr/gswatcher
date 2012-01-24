#ifndef __GSTOOL__NOTIFICATION_H__
#define __GSTOOL__NOTIFICATION_H__

#include <glib.h>

void gs_notification_init (gboolean notifier);

void gs_notification_set_enable (gboolean enable);
gboolean gs_notification_get_enable ();

void gs_notification_set_sound (const gchar *sound);
const gchar *gs_notification_get_sound ();

void gs_notification_set_timeout (gint time);
gint notification_get_timeout ();

void gs_notification_message (const gchar *summary, const gchar *body);

#endif
