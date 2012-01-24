#ifndef __GSTOOL__PLATFORM_H__
#define __GSTOOL__PLATFORM_H__

#include <glib.h>
#include <gtk/gtk.h>


void gs_initialize_directories (const gchar *exe);

const gchar *gs_get_buddy_list_path ();
const gchar *gs_get_server_list_path ();
const gchar *gs_get_preferences_path ();

const gchar *gs_get_locale_dir ();
const gchar *gs_get_icon_dir ();
const gchar *gs_get_sound_dir ();


#if GTK_MAJOR_VERSION == 2
gboolean gtk_tree_model_iter_previous (GtkTreeModel *model, GtkTreeIter *iter);
#endif

#ifdef G_OS_WIN32
	#define GS_DEFAULT_MONOSPACE_FONT "Courier New 10"
#else
	#define GS_DEFAULT_MONOSPACE_FONT "Monospace 9"
#endif


gchar *
gs_get_system_font ();


#endif