/* 
 * gui-preferences.c
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



#include <glib/gi18n.h>
#include <gtk/gtk.h>
#include "gui-server-list.h"
#include "gui-notification.h"
#include "gui-preferences.h"
#include "gui-console.h"
#include "platform.h"


static GObject *ctl_interval, *ctl_port;
static GObject *ctl_game_column, *ctl_nenable, *ctl_nsound, *ctl_sysfont,
		*ctl_fontlabel, *ctl_font, *ctl_logaddress, *ctl_connect;


void G_MODULE_EXPORT
gui_prefs_interval_changed (GtkAdjustment *adjustment, gpointer udata)
{
	gdouble value = gtk_adjustment_get_value (adjustment);
	gs_application_set_interval (value);
	gs_application_save_preferences ();
}


void G_MODULE_EXPORT
gui_prefs_gamecolumn_changed (GtkComboBox *widget, gpointer udata)
{
	gui_slist_set_game_column_mode (gtk_combo_box_get_active (widget));
	gs_application_save_preferences ();
}


void G_MODULE_EXPORT
gui_prefs_port_changed (GtkAdjustment *adjustment, gpointer udata)
{
	guint16 value = (guint16) gtk_adjustment_get_value (adjustment);
	gs_application_set_default_port (value);
	gs_application_save_preferences ();
}


void G_MODULE_EXPORT
gui_prefs_enable_toggled (GtkToggleButton *togglebutton, gpointer udata)
{
	gboolean value = gtk_toggle_button_get_active (togglebutton);
	gs_notification_set_enable (value);
	gs_application_save_preferences ();
}


void G_MODULE_EXPORT
gui_prefs_sound_changed (GtkFileChooserButton *widget, gpointer udata)
{
	gchar *fname = gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (widget));
	gs_notification_set_sound (fname);
	g_free (fname);
	gs_application_save_preferences ();
}


void G_MODULE_EXPORT
gui_prefs_sound_test_clicked (GtkButton *button, gpointer udata)
{
	gs_notification_sound ();
}


void G_MODULE_EXPORT
gui_prefs_sysfont_toggled (GtkToggleButton *togglebutton, gpointer udata)
{
	gboolean value = gtk_toggle_button_get_active (togglebutton);
	gtk_widget_set_sensitive (GTK_WIDGET (ctl_font), !value);
	gtk_widget_set_sensitive (GTK_WIDGET (ctl_fontlabel), !value);
	gui_console_set_use_system_font (value);
	gs_application_save_preferences ();
}


void G_MODULE_EXPORT
gui_prefs_font_changed (GtkFontButton *widget, gpointer udata)
{
	gchar *fontname = gtk_font_chooser_get_font (GTK_FONT_CHOOSER (ctl_font));
	gui_console_set_font (fontname);
	g_free (fontname);
	gs_application_save_preferences ();
}


void G_MODULE_EXPORT
gui_prefs_logaddress_changed (GtkEntry *entry, gpointer user_data)
{
	const gchar *address = gtk_entry_get_text (entry);
	gs_client_set_logaddress (address);
	gs_application_save_preferences ();
}


void G_MODULE_EXPORT
gui_prefs_connect_changed (GtkEntry *entry, gpointer user_data)
{
	const gchar *command = gtk_entry_get_text (entry);
	gs_client_set_connect_command (command);
	gs_application_save_preferences ();
}


GtkWidget *
gui_prefs_create ()
{
	GObject *root = NULL;
	GError *error = NULL;
	GtkBuilder *builder = gtk_builder_new ();
	if (gtk_builder_add_from_resource (builder, "/org/gswatcher/ui/preferences.ui", &error) == 0) {
		g_warning (error->message);
		g_object_unref (builder);
		g_error_free (error);
		return NULL;
	}
	
	root               = gtk_builder_get_object (builder, "root");
	ctl_interval       = gtk_builder_get_object (builder, "interval");
	ctl_game_column    = gtk_builder_get_object (builder, "game-column");
	ctl_port           = gtk_builder_get_object (builder, "port");
	ctl_nenable        = gtk_builder_get_object (builder, "notification-enable");
	ctl_nsound         = gtk_builder_get_object (builder, "notification-sound");
	ctl_sysfont        = gtk_builder_get_object (builder, "system-font");
	ctl_fontlabel      = gtk_builder_get_object (builder, "font-label");
	ctl_font           = gtk_builder_get_object (builder, "font");
	ctl_logaddress     = gtk_builder_get_object (builder, "log-address");
	ctl_connect        = gtk_builder_get_object (builder, "connect-command");
	
	gtk_builder_connect_signals (builder, NULL);
	root = g_object_ref (root);
	g_object_unref (builder);
	return GTK_WIDGET (root);
}



void
gui_prefs_init ()
{
	struct {
		gpointer obj, fn;
	} *signal, signals[] = {
		{ctl_interval,     gui_prefs_interval_changed},
		{ctl_game_column,  gui_prefs_gamecolumn_changed},
		{ctl_port,         gui_prefs_port_changed},
		{ctl_nenable,      gui_prefs_enable_toggled},
		{ctl_nsound,       gui_prefs_sound_changed},
		{ctl_sysfont,      gui_prefs_sysfont_toggled},
		{ctl_font,         gui_prefs_font_changed},
		{ctl_font,         gui_prefs_logaddress_changed},
		{ctl_connect,      gui_prefs_connect_changed},
		{NULL,             NULL}
	};
	
	for (signal = signals; signal->obj; signal++)
		g_signal_handlers_block_by_func (signal->obj, signal->fn, NULL);
	
	gtk_adjustment_set_value (GTK_ADJUSTMENT (ctl_interval),
			gs_application_get_interval ());
	
	gtk_combo_box_set_active (GTK_COMBO_BOX (ctl_game_column),
			gui_slist_get_game_column_mode ());
	
	gtk_adjustment_set_value (GTK_ADJUSTMENT (ctl_port),
			gs_application_get_default_port ());
	
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (ctl_nenable),
			gs_notification_get_enable ());
	
	gtk_file_chooser_set_filename (GTK_FILE_CHOOSER (ctl_nsound),
			gs_notification_get_sound ());
	
	const gchar *fontname = gui_console_get_font ();
	gtk_font_chooser_set_font (GTK_FONT_CHOOSER (ctl_font),
			fontname ? fontname : GS_DEFAULT_MONOSPACE_FONT);
	
	const gchar *address = gs_client_get_logaddress ();
	gtk_entry_set_text (GTK_ENTRY (ctl_logaddress), address ? address : "");
	
	const gchar *command = gs_client_get_connect_command ();
	gtk_entry_set_text (GTK_ENTRY (ctl_connect), command ? command : "");
	
	gboolean use = gui_console_get_use_system_font ();
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (ctl_sysfont), use);
	gtk_widget_set_sensitive (GTK_WIDGET (ctl_font), !use);
	gtk_widget_set_sensitive (GTK_WIDGET (ctl_fontlabel), !use);
	
	for (signal = signals; signal->obj; signal++)
		g_signal_handlers_unblock_by_func (signal->obj, signal->fn, NULL);
}
