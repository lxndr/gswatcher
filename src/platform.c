#include <glib/gstdio.h>
#include "platform.h"

static gchar *buddy_list_path = NULL;
static gchar *server_list_path = NULL;
static gchar *preferences_path = NULL;
#ifdef ENABLE_NLS
static gchar *locale_dir = NULL;
#endif
static gchar *icon_dir = NULL;
static gchar *sound_dir = NULL;


void
gs_initialize_directories (const gchar *exe)
{
	GFile *fexe = g_file_new_for_commandline_arg (exe);
	gchar *exepath = g_file_get_path (fexe);
	gchar *bin_dir = g_path_get_dirname (exepath);
	gchar *root_dir = g_path_get_dirname (bin_dir);
	g_free (bin_dir);
	g_free (exepath);
	g_object_unref (fexe);
	
	gchar *config_dir = g_build_filename (g_get_user_config_dir (), "gstool", NULL);
	g_mkdir_with_parents (config_dir, 0750);
	
	buddy_list_path = g_build_filename (config_dir, "buddylist.json", NULL);
	server_list_path = g_build_filename (config_dir, "serverlist.json", NULL);
	preferences_path = g_build_filename (config_dir, "preferences.json", NULL);
	
	g_free (config_dir);
	
#ifdef GS_DATADIR
	gchar *data_dir = GS_DATADIR;
#else
	gchar *data_dir = g_build_filename (root_dir, "share", "gstool", NULL);
#endif
	
	icon_dir = g_build_filename (data_dir, "icons", NULL);
	sound_dir = g_build_filename (data_dir, "sounds", NULL);
	
#ifdef ENABLE_NLS
#ifdef GS_LOCALEDIR
	locale_dir = GS_LOCALEDIR;
#else
	locale_dir = g_build_filename (root_dir, "share", "locale", NULL);
#endif
#endif
	
#ifndef GS_DATADIR
	g_free (data_dir);
#endif
	g_free (root_dir);
}

const gchar *
gs_get_buddy_list_path ()
{
	return buddy_list_path;
}

const gchar *
gs_get_server_list_path ()
{
	return server_list_path;
}

const gchar *
gs_get_preferences_path ()
{
	return preferences_path;
}

#ifdef ENABLE_NLS
const gchar *
gs_get_locale_dir ()
{
	return locale_dir;
}
#endif

const gchar *
gs_get_icon_dir ()
{
	return icon_dir;
}

const gchar *
gs_get_sound_dir ()
{
	return sound_dir;
}



#if GTK_MAJOR_VERSION == 2

gboolean
gtk_tree_model_iter_previous (GtkTreeModel *model, GtkTreeIter *iter)
{
  gboolean retval;
  GtkTreePath *path;

  path = gtk_tree_model_get_path (model, iter);
  if (path == NULL)
    return FALSE;

  retval = gtk_tree_path_prev (path) &&
           gtk_tree_model_get_iter (model, iter, path);
  if (retval == FALSE)
    iter->stamp = 0;

  gtk_tree_path_free (path);

  return retval;
}

#endif


gchar *
gs_get_system_font ()
{
	// in case if it's gnome 3
	GSettings *sysconfig = g_settings_new ("org.gnome.desktop.interface");
	gchar *font = g_settings_get_string (sysconfig, "monospace-font-name");
	if (font) {
		g_object_unref (sysconfig);
		return font;
	}
	g_object_unref (sysconfig);
	
	return g_strdup (GS_DEFAULT_MONOSPACE_FONT);
}
