/* 
 * gui-notification.c
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



#include <gdk/gdk.h>
#include <gtk/gtk.h>
#include <glib/gprintf.h>
#include <string.h>
#include "gswatcher.h"
#include "platform.h"
#include "gui-notification.h"


#if defined (G_OS_UNIX)
	#ifndef G_OS_MACOSX
		#include <gdk/gdkx.h>
	#endif
	#include <unistd.h>
#elif defined (G_OS_WIN32)
	#include <windows.h>
#endif


typedef gpointer NotifyNotification;
gboolean (*notify_init) (const char *app_name);
NotifyNotification (* notify_notification_new) (const char *summary,
		const char *body, const char *icon, GtkWidget *attach);
void (* notify_notification_set_timeout) (NotifyNotification notification,
		gint timeout);
gboolean (* notify_notification_show) (NotifyNotification notification,
		GError **error);


static gboolean enabled = TRUE;
static gboolean internal = FALSE;
static gint timeout = 5;



/*                            Sound notification                              */

static gchar *sound_file = NULL;
static gchar *sound_default_playbins[] = {
#ifdef G_OS_WIN32
		"gst-launch-1.0.exe --quiet playbin uri='file:///$(file)'",
		"gst-launch-0.10.exe --quiet playbin2 uri='file:///$(file)'",
#else
		"gst-launch-1.0 --quiet playbin uri=file://$(file)",
		"gst-launch-0.10 --quiet playbin2 uri=file://$(file)",
#endif
		"mplayer -quiet -novideo -noar -noconsolecontrols -nojoystick -nolirc -nomouseinput $(file)",
		"vlc $(file)",
		NULL
	};
static gchar *sound_specific_playbin = NULL;
static gchar **sound_argv = NULL;
static GPid sound_pid = 0;
static guint sound_pid_watch = 0;


static gboolean
eval_playbin_cmd_cb (const GMatchInfo *minfo, GString *result, gpointer udata)
{
	gchar *match = g_match_info_fetch (minfo, 0);
	if (strcmp (match, "$(file)") == 0) {
#ifdef G_OS_WIN32
		gchar *fname = gs_quote_win32_file_name (sound_file);
#else
		gchar *fname = g_shell_quote (sound_file);
#endif
		g_string_append (result, fname);
		g_free (fname);
	} else {
		g_string_append_printf (result, "%s", match);
	}
	g_free (match);
	return FALSE;
}

static gchar *
eval_playbin_cmd (const gchar *cmd)
{
	GRegex *reg = g_regex_new ("\\$\\(.+\\)", 0, 0, NULL);
	gchar *command = g_regex_replace_eval (reg, cmd, -1, 0, 0,
			eval_playbin_cmd_cb, NULL, NULL);
	g_regex_unref (reg);
	return command;
}


static gboolean
check_playbin_cmd (const gchar *cmd)
{
	GError *error = NULL;
	gint argc;
	
	gchar *cmdline = eval_playbin_cmd (cmd);
	if (!g_shell_parse_argv (cmdline, &argc, &sound_argv, &error)) {
		g_warning ("%s", error->message);
		g_error_free (error);
		g_free (cmdline);
		return FALSE;
	}
	g_free (cmdline);
	
	if (argc <= 1) {
		g_strfreev (sound_argv);
		sound_argv = NULL;
		return FALSE;
	}
	
	gchar *path = g_find_program_in_path (sound_argv[0]);
	if (!path) {
		g_strfreev (sound_argv);
		sound_argv = NULL;
		return FALSE;
	}
	
	g_free (sound_argv[0]);
	sound_argv[0] = path;
	return TRUE;
}

static void
update_playbin_cmd ()
{
	if (sound_specific_playbin) {
		check_playbin_cmd (sound_specific_playbin);
	} else {
		gint i;
		for (i = 0; i < 3; i++)
			if (check_playbin_cmd (sound_default_playbins[i]))
				break;
	}
}

void
gs_notification_set_playbin (const gchar *cmd)
{
	if (sound_specific_playbin) {
		g_free (sound_specific_playbin);
		sound_specific_playbin = NULL;
	}
	
	if (cmd)
		sound_specific_playbin = g_strdup (cmd);
	
	update_playbin_cmd ();
}


void
gs_notification_set_sound (const gchar *sound)
{
	if (sound_file)
		g_free (sound_file);
	
	if (sound)
		sound_file = g_strdup (sound);
	else
		sound_file = g_build_filename (gs_application_get_sound_dir (), "alert01.ogg", NULL);
	
	update_playbin_cmd ();
}

const gchar *
gs_notification_get_sound ()
{
	return sound_file;
}


static void
playbin_died (GPid pid, gint status, gpointer data)
{
	sound_pid = 0;
	sound_pid_watch = 0;
}

void
gs_notification_sound ()
{
	GError *error = NULL;
	
	if (sound_pid) {
		g_source_remove (sound_pid_watch);
		g_spawn_close_pid (sound_pid);
		sound_pid_watch = 0;
		sound_pid = 0;
	}
	
	if (sound_argv == NULL)
		return;
	
	if (!g_spawn_async (NULL, sound_argv, NULL,
			G_SPAWN_DO_NOT_REAP_CHILD | G_SPAWN_STDOUT_TO_DEV_NULL,
			NULL, NULL, &sound_pid, &error)) {
		g_warning (error->message);
		g_error_free (error);
		return;
	}
	
	sound_pid_watch = g_child_watch_add (sound_pid, playbin_died, NULL);
}



// Internal notifier   =========================================================

typedef struct {
	gchar *summary;
	gchar *body;
} Notification;

static GtkWidget *window = NULL, *title, *body;
static gboolean supports_alpha = FALSE;
static GList *notifications = NULL;
static guint srcid = 0;

static gboolean gs_notifier_show_step (gpointer data);


static void
gs_notifier_place ()
{
	GdkScreen *screen = gtk_widget_get_screen (window);
	gint width = gdk_screen_get_width (screen);
	gint height = gdk_screen_get_height (screen);
	
	GdkRectangle size;
	gtk_widget_get_allocation (window, &size);
	
	GdkRectangle area = {0, 0, width, height};
	
#if defined (G_WINDOWING_X11)
	GdkAtom atom = gdk_atom_intern ("_NET_WORKAREA", TRUE);
	if (atom) {
		GdkAtom actual_type;
		gint actual_format, actual_length;
		long *workareas;
		
		GdkWindow *root_window = gdk_get_default_root_window ();
		gboolean ret = gdk_property_get (root_window, atom, GDK_NONE, 0, 4 * 32,
				FALSE, &actual_type, &actual_format, &actual_length,
				(guchar **) &workareas);
		if (ret == TRUE && actual_type != GDK_NONE && actual_format != 0) {
			int disp_screen = GDK_SCREEN_XNUMBER (screen);
			area.x		= workareas[disp_screen * 4];
			area.y		= workareas[disp_screen * 4 + 1];
			area.width	= workareas[disp_screen * 4 + 2];
			area.height	= workareas[disp_screen * 4 + 3];
			g_free (workareas);
		}
	}
#elif defined (G_WINDOWING_WIN32)
	APPBARDATA appbar;
	memset (&appbar, 0, sizeof (APPBARDATA));
	appbar.cbSize = sizeof (APPBARDATA);
	SHAppBarMessage (ABM_GETTASKBARPOS, &appbar);
	
	area.width = appbar.uEdge == ABE_RIGHT ? appbar.rc.left : width;
	area.height = appbar.uEdge == ABE_BOTTOM ? appbar.rc.top : height;
#endif
	
	gtk_window_move (GTK_WINDOW (window),
			area.width + area.x - size.width - 3,
			area.height + area.y - size.height - 3);
}


static void
gs_notifier_draw_window (cairo_t *cr, gint width, gint height)
{
	cairo_set_line_width (cr, 1.5);
	cairo_rectangle (cr, 1, 1, width - 2, height - 2);
	cairo_set_source_rgb (cr, 1, 1, 1);
	cairo_fill_preserve (cr);
	cairo_set_source_rgb (cr, 0, 0, 0);
	cairo_stroke (cr);
	
	cairo_set_line_width (cr, 1);
	cairo_rectangle (cr, 3, 3, width - 6, height - 6);
	
	cairo_pattern_t *pat = cairo_pattern_create_linear (4, 4, width - 8, height - 8);
	cairo_pattern_add_color_stop_rgb (pat, 0, .95, .95, 1);
	cairo_pattern_add_color_stop_rgb (pat, 1, .85, .85, 1);
	cairo_set_source (cr, pat);
	cairo_fill_preserve (cr);
	cairo_pattern_destroy (pat);
	
	cairo_set_line_width (cr, 1);
	cairo_set_source_rgb (cr, 0.3, 0.3, 0.3);
	cairo_stroke (cr);
}


static void
gs_notifier_draw_rounded_rectagle (cairo_t *cr, gdouble x, gdouble y,
		gdouble width, gdouble height, gdouble radius)
{
	gdouble degrees = 3.14159265 / 180.0;
	
	cairo_new_sub_path (cr);
		cairo_arc (cr, x + width - radius, y + radius, radius, -90 * degrees, 0 * degrees);
		cairo_arc (cr, x + width - radius, y + height - radius, radius, 0 * degrees, 90 * degrees);
		cairo_arc (cr, x + radius, y + height - radius, radius, 90 * degrees, 180 * degrees);
		cairo_arc (cr, x + radius, y + radius, radius, 180 * degrees, 270 * degrees);
	cairo_close_path (cr);
}


static void
gs_notifier_draw_rounded_window (cairo_t *cr, gint width, gint height)
{
	cairo_set_line_width (cr, 1.5);
	gs_notifier_draw_rounded_rectagle (cr, 1, 1, width - 2, height - 2, 5);
	cairo_set_source_rgb (cr, 1, 1, 1);
	cairo_fill_preserve (cr);
	cairo_set_source_rgb (cr, 0.05, 0.05, 0.05);
	cairo_stroke (cr);
	
	cairo_set_line_width (cr, 1);
	gs_notifier_draw_rounded_rectagle (cr, 3, 3, width - 6, height - 6, 3);
	
	cairo_pattern_t *pat = cairo_pattern_create_linear (4, 4, width - 8, height - 8);
	cairo_pattern_add_color_stop_rgb (pat, 0, .95, .95, .95);
	cairo_pattern_add_color_stop_rgb (pat, 1, .85, .85, .85);
	cairo_set_source (cr, pat);
	cairo_fill_preserve (cr);
	cairo_pattern_destroy (pat);
	
	cairo_set_line_width (cr, 1);
	cairo_set_source_rgb (cr, 0.3, 0.3, 0.3);
	cairo_stroke (cr);
}


static gboolean
gs_notifier_draw (GtkWidget *widget, cairo_t *cr, gpointer udata)
{
	GtkAllocation allocation;
	gtk_widget_get_allocation (widget, &allocation);
	int width = allocation.width;
	int height = allocation.height;
	
	if (supports_alpha) {
		cairo_set_source_rgba (cr, 1, 1, 1, 0);
		cairo_set_operator (cr, CAIRO_OPERATOR_SOURCE);
		cairo_paint (cr);
		gs_notifier_draw_rounded_window (cr, width, height);
	} else {
		gs_notifier_draw_window (cr, width, height);
	}
	
	return FALSE;
}


#if GTK_MAJOR_VERSION == 2
static gboolean
gs_notifier_expose (GtkWidget *widget, GdkEvent *event, gpointer udata)
{
	cairo_t *cr = gdk_cairo_create (GDK_DRAWABLE (gtk_widget_get_window (widget)));
	gboolean ret = gs_notifier_draw (widget, cr, udata);
	cairo_destroy (cr);
	return ret;
}
#endif


static gboolean
gs_notifier_hide_step (gpointer data)
{
	if (notifications) {
		srcid = g_timeout_add (50, gs_notifier_show_step, NULL);
		return FALSE;
	}
	
#if GTK_CHECK_VERSION(3, 8, 0)
	gdouble opacity = gtk_widget_get_opacity (window) - 0.05;
	gtk_widget_set_opacity (window, opacity);
#else
	gdouble opacity = gtk_window_get_opacity (GTK_WINDOW (window)) - 0.05;
	gtk_window_set_opacity (GTK_WINDOW (window), opacity);
#endif
	if (opacity <= 0) {
		gtk_widget_hide (window);
		srcid = 0;
		return FALSE;
	}
	
	return TRUE;
}


static gboolean
gs_notifier_expose_step (gpointer data)
{
	Notification *notification = (Notification *) notifications->data;
	notifications = g_list_remove (notifications, notification);
	g_free (notification->summary);
	g_free (notification->body);
	g_slice_free (Notification, notification);
	
	if (!notifications) {
		srcid = g_timeout_add (50, gs_notifier_hide_step, NULL);
		return FALSE;
	}
	
	notification = (Notification *) notifications->data;
	gtk_label_set_text (GTK_LABEL (title), notification->summary);
	gtk_label_set_text (GTK_LABEL (body), notification->body);
	
	gs_notification_sound ();
	
	return TRUE;
}


static gboolean
gs_notifier_show_step (gpointer data)
{
#if GTK_CHECK_VERSION(3, 8, 0)
	gdouble opacity = gtk_widget_get_opacity (window) + 0.2;
	gtk_widget_set_opacity (window, opacity);
#else
	gdouble opacity = gtk_window_get_opacity (GTK_WINDOW (window)) + 0.2;
	gtk_window_set_opacity (GTK_WINDOW (window), opacity);
#endif
	if (opacity >= 1) {
		srcid = g_timeout_add (timeout * 1000, gs_notifier_expose_step, NULL);
		return FALSE;
	}
	return TRUE;
}


static void
gs_notifier_message (const gchar *title_text, const gchar *body_text)
{
	Notification *notification = g_slice_new0 (Notification);
	notification->summary = g_strdup (title_text);
	notification->body = g_strdup (body_text);
	notifications = g_list_append (notifications, notification);
	
	if (!srcid) {
		gtk_label_set_text (GTK_LABEL (title), notification->summary);
		gtk_label_set_text (GTK_LABEL (body), notification->body);
		
#if GTK_CHECK_VERSION(3, 8, 0)
		gtk_widget_set_opacity (window, 0);
#else
		gtk_window_set_opacity (GTK_WINDOW (window), 0);
#endif
		gtk_widget_show (window);
		srcid = g_timeout_add (50, gs_notifier_show_step, NULL);
		
		gs_notification_sound ();
	}
}


static void
gs_notifier_init ()
{
	// title label
	PangoAttribute *attr = pango_attr_weight_new (PANGO_WEIGHT_BOLD);
	PangoAttrList *attrs = pango_attr_list_new ();
	pango_attr_list_insert (attrs, attr);
	
	title = gtk_label_new (NULL);
	gtk_label_set_attributes (GTK_LABEL (title), attrs);
	gtk_widget_show (title);
	
	// body label
	body = gtk_label_new (NULL);
	gtk_misc_set_alignment (GTK_MISC (body), 0.f, 0.f);
	gtk_widget_show (body);
	
	// window
	GtkWidget *vbox = gtk_box_new (GTK_ORIENTATION_VERTICAL, 12);
	gtk_box_pack_start (GTK_BOX (vbox), title, FALSE, TRUE, 0);
	gtk_box_pack_start (GTK_BOX (vbox), body, TRUE, TRUE, 0);
	gtk_widget_show (vbox);
	
	window = gtk_window_new (GTK_WINDOW_POPUP);
	g_object_set (G_OBJECT (window),
			"decorated", FALSE,
			"border-width", 10,
			"app-paintable", TRUE,
			NULL);
	gtk_container_add (GTK_CONTAINER (window), vbox);
	g_signal_connect (G_OBJECT (window), "size-allocate", G_CALLBACK (gs_notifier_place), NULL);
#if GTK_MAJOR_VERSION == 2
	g_signal_connect (window, "expose-event", G_CALLBACK (gs_notifier_expose), NULL);
#else
	g_signal_connect (window, "draw", G_CALLBACK (gs_notifier_draw), NULL);
#endif
	
	GdkScreen *screen = gtk_widget_get_screen (window);
	g_signal_connect (G_OBJECT (screen), "size-changed", G_CALLBACK (gs_notifier_place), NULL);
	
	GdkVisual *visual = gdk_screen_get_rgba_visual (screen);
	if (visual == NULL)
  		visual = gdk_screen_get_system_visual (screen);
	else 
		supports_alpha = TRUE;
	gtk_widget_set_visual (GTK_WIDGET (window), visual);
}



// =============================================================================

void
gs_notification_set_timeout (gint time)
{
	timeout = time;
}

gint
notification_get_timeout ()
{
	return timeout;
}


void
gs_notification_set_enable (gboolean enable)
{
	enabled = enable;
}

gboolean
gs_notification_get_enable ()
{
	return enabled;
}


void
gs_notification_message (const gchar *summary, const gchar *body)
{
	if (!enabled)
		return;
		
	if (internal) {
		gs_notifier_message (summary, body);
	} else {
		gs_notification_sound ();
		NotifyNotification n = notify_notification_new (summary, body, NULL, NULL);
		notify_notification_set_timeout (n, timeout * 1000);
		notify_notification_show (n, NULL);
		g_object_unref (n);
	}
}


void
gs_notification_init (gboolean notifier)
{
	sound_file = g_build_filename (gs_application_get_sound_dir (), "alert01.ogg", NULL);
	
#if defined (G_OS_UNIX) && !defined (G_OS_MACOSX)
	internal = notifier;
	
	if (!internal) {
		GModule *libnotify = g_module_open ("libnotify.so", G_MODULE_BIND_LAZY);
		if (libnotify) {
			g_module_symbol (libnotify, "notify_init", (gpointer *) &notify_init);
			if (notify_init ("gstool")) {
				g_module_symbol (libnotify, "notify_notification_new",
						(gpointer *) &notify_notification_new);
				g_module_symbol (libnotify, "notify_notification_set_timeout",
						(gpointer *) &notify_notification_set_timeout);
				g_module_symbol (libnotify, "notify_notification_show",
						(gpointer *) &notify_notification_show);
				return;
			} else {
				g_module_close (libnotify);
			}
		}
	}
#endif
	
	internal = TRUE;
	gs_notifier_init ();
}
