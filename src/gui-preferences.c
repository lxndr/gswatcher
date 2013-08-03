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


void
gui_prefs_interval_changed (GtkAdjustment *adjustment, gpointer udata)
{
	gdouble value = gtk_adjustment_get_value (adjustment);
	gs_application_set_interval (value);
	gs_application_save_preferences ();
}


void
gui_prefs_gamecolumn_changed (GtkComboBox *widget, gpointer udata)
{
	gui_slist_set_game_column_mode (gtk_combo_box_get_active (widget));
	gs_application_save_preferences ();
}


void
gui_prefs_port_changed (GtkAdjustment *adjustment, gpointer udata)
{
	guint16 value = (guint16) gtk_adjustment_get_value (adjustment);
	gs_application_set_default_port (value);
	gs_application_save_preferences ();
}


void
gui_prefs_enable_toggled (GtkToggleButton *togglebutton, gpointer udata)
{
	gboolean value = gtk_toggle_button_get_active (togglebutton);
	gs_notification_set_enable (value);
	gs_application_save_preferences ();
}


void
gui_prefs_sound_changed (GtkFileChooserButton *widget, gpointer udata)
{
	gchar *fname = gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (widget));
	gs_notification_set_sound (fname);
	g_free (fname);
	gs_application_save_preferences ();
}


void
gui_prefs_sound_test_clicked (GtkButton *button, gpointer udata)
{
	gs_notification_sound ();
}


void
gui_prefs_sysfont_toggled (GtkToggleButton *togglebutton, gpointer udata)
{
	gboolean value = gtk_toggle_button_get_active (togglebutton);
	gtk_widget_set_sensitive (GTK_WIDGET (ctl_font), !value);
	gtk_widget_set_sensitive (GTK_WIDGET (ctl_fontlabel), !value);
	gui_console_set_use_system_font (value);
	gs_application_save_preferences ();
}


void
gui_prefs_font_changed (GtkFontButton *widget, gpointer udata)
{
	gchar *fontname = gtk_font_chooser_get_font (GTK_FONT_CHOOSER (ctl_font));
	gui_console_set_font (fontname);
	g_free (fontname);
	gs_application_save_preferences ();
}


void
gui_prefs_logaddress_changed (GtkEntry *entry, gpointer user_data)
{
	const gchar *address = gtk_entry_get_text (entry);
	gs_client_set_logaddress (address);
	gs_application_save_preferences ();
}


void
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
	
	root = g_object_ref (root);
	g_object_unref (builder);
	return GTK_WIDGET (root);
}


void
gui_prefs_set_interval (gdouble interval)
{
	g_signal_handlers_block_by_func (ctl_interval, gui_prefs_interval_changed, NULL);
	gtk_adjustment_set_value (GTK_ADJUSTMENT (ctl_interval), interval);
	g_signal_handlers_unblock_by_func (ctl_interval, gui_prefs_interval_changed, NULL);
}

void
gui_prefs_set_game_column_mode (GuiGameColumnMode mode)
{
	g_signal_handlers_block_by_func (ctl_game_column, gui_prefs_gamecolumn_changed, NULL);
	gtk_combo_box_set_active (GTK_COMBO_BOX (ctl_game_column), mode);
	g_signal_handlers_unblock_by_func (ctl_game_column, gui_prefs_gamecolumn_changed, NULL);
}

void
gui_prefs_set_port (guint16 port)
{
	g_signal_handlers_block_by_func (ctl_port, gui_prefs_port_changed, NULL);
	gtk_adjustment_set_value (GTK_ADJUSTMENT (ctl_port), port);
	g_signal_handlers_unblock_by_func (ctl_port, gui_prefs_port_changed, NULL);
}

void
gui_prefs_set_enable_notifications (gboolean enable)
{
	g_signal_handlers_block_by_func (ctl_nenable, gui_prefs_enable_toggled, NULL);
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (ctl_nenable), enable);
	g_signal_handlers_unblock_by_func (ctl_nenable, gui_prefs_enable_toggled, NULL);
}

void
gui_prefs_set_notification_sound (const gchar *filename)
{
	g_signal_handlers_block_by_func (ctl_nsound, gui_prefs_sound_changed, NULL);
	gtk_file_chooser_set_filename (GTK_FILE_CHOOSER (ctl_nsound), filename);
	g_signal_handlers_unblock_by_func (ctl_nsound, gui_prefs_sound_changed, NULL);
}

void
gui_prefs_set_use_system_font (gboolean use)
{
	g_signal_handlers_block_by_func (ctl_sysfont, gui_prefs_sysfont_toggled, NULL);
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (ctl_sysfont), use);
	gtk_widget_set_sensitive (GTK_WIDGET (ctl_font), !use);
	gtk_widget_set_sensitive (GTK_WIDGET (ctl_fontlabel), !use);
	g_signal_handlers_unblock_by_func (ctl_sysfont, gui_prefs_sysfont_toggled, NULL);
}

void
gui_prefs_set_font (const gchar *fontname)
{
	g_signal_handlers_block_by_func (ctl_font, gui_prefs_font_changed, NULL);
	gtk_font_chooser_set_font (GTK_FONT_CHOOSER (ctl_font),
			fontname ? fontname : GS_DEFAULT_MONOSPACE_FONT);
	g_signal_handlers_unblock_by_func (ctl_font, gui_prefs_font_changed, NULL);
}

void
gui_prefs_set_logaddress (const gchar *address)
{
	g_signal_handlers_block_by_func (ctl_font, gui_prefs_logaddress_changed, NULL);
	gtk_entry_set_text (GTK_ENTRY (ctl_logaddress), address ? address : "");
	g_signal_handlers_unblock_by_func (ctl_font, gui_prefs_logaddress_changed, NULL);
}

void
gui_prefs_set_connect_command (const gchar *command)
{
	g_signal_handlers_block_by_func (ctl_connect, gui_prefs_connect_changed, NULL);
	gtk_entry_set_text (GTK_ENTRY (ctl_connect), command ? command : "");
	g_signal_handlers_unblock_by_func (ctl_connect, gui_prefs_connect_changed, NULL);
}
