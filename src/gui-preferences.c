#include <glib/gi18n.h>
#include <gtk/gtk.h>
#include "gui-server-list.h"
#include "gui-notification.h"
#include "gui-preferences.h"
#include "gui-console.h"
#include "platform.h"


static GtkAdjustment *ctl_urate;
static GtkWidget *ctl_nenable, *ctl_game_column, *ctl_nsound, *ctl_sysfont,
		*ctl_fontlabel, *ctl_font;


static void
gui_prefs_updaterate_changed (GtkAdjustment *adjustment, gpointer udata)
{
	gdouble value = gtk_adjustment_get_value (adjustment);
	gs_set_update_rate (value);
	gs_save_preferences ();
}


static void
gui_prefs_gamecolumn_changed (GtkComboBox *widget, gpointer udata)
{
	gui_slist_set_game_column_mode (gtk_combo_box_get_active (widget));
	gs_save_preferences ();
}
														

static void
gui_prefs_enable_toggled (GtkToggleButton *togglebutton, gpointer udata)
{
	gboolean value = gtk_toggle_button_get_active (togglebutton);
	gs_notification_set_enable (value);
	gs_save_preferences ();
}


static void
gs_prefs_sound_changed (GtkFileChooserButton *widget, gpointer udata)
{
	gchar *fname = gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (widget));
	gs_notification_set_sound (fname);
	g_free (fname);
	gs_save_preferences ();
}


static void
gui_prefs_sysfont_toggled (GtkToggleButton *togglebutton, gpointer udata)
{
	gboolean value = gtk_toggle_button_get_active (togglebutton);
	gtk_widget_set_sensitive (ctl_font, !value);
	gtk_widget_set_sensitive (ctl_fontlabel, !value);
	gui_console_set_use_system_font (value);
	gs_save_preferences ();
}


static void
gui_prefs_font_changed (GtkFontButton *widget, gpointer udata)
{
	gchar *fontname = gtk_font_chooser_get_font (GTK_FONT_CHOOSER (ctl_font));
	gui_console_set_font (fontname);
	g_free (fontname);
	gs_save_preferences ();
}


GtkWidget *
gs_prefs_create ()
{
	GtkWidget *box, *label, *tmp;
	
	PangoAttrList *bold = pango_attr_list_new ();
	PangoAttribute *attr = pango_attr_weight_new (PANGO_WEIGHT_BOLD);
	pango_attr_list_insert (bold, attr);
//	pango_attribute_destroy (attr);
	
	GtkWidget *grid = gtk_grid_new ();
	g_object_set (G_OBJECT (grid),
			"column-spacing", 8,
			"row-spacing",	2,
			"border-width", 8,
			NULL);
	
	// General section
	label = gtk_label_new (_("General"));
	gtk_label_set_attributes (GTK_LABEL (label), bold);
	
	tmp = gtk_separator_new (GTK_ORIENTATION_HORIZONTAL);
	
	box = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 8);
	gtk_box_pack_start (GTK_BOX (box), label, FALSE, TRUE, 0);
	gtk_box_pack_start (GTK_BOX (box), tmp, TRUE, TRUE, 0);
	gtk_grid_attach (GTK_GRID (grid), box, 0, 0, 2, 1);
	
	// Update rate
	GtkWidget *spin = gtk_spin_button_new_with_range (0.5, 15.0, 0.5);
	g_object_set (G_OBJECT (spin),
			"hexpand", TRUE,
			NULL);
	ctl_urate = gtk_spin_button_get_adjustment (GTK_SPIN_BUTTON (spin));
	g_signal_connect (ctl_urate, "value-changed",
			G_CALLBACK (gui_prefs_updaterate_changed), NULL);
	gtk_grid_attach (GTK_GRID (grid), spin, 1, 1, 1, 1);
	
	label = gtk_label_new (_("Update _rate:"));
	g_object_set (G_OBJECT (label),
			"use-underline", TRUE,
			"mnemonic-widget", spin,
			"margin-left", 8,
			"halign", GTK_ALIGN_START,
			NULL);
	gtk_grid_attach (GTK_GRID (grid), label, 0, 1, 1, 1);
	
	// Game column mode
	ctl_game_column = gtk_combo_box_text_new ();
	gtk_combo_box_text_append (GTK_COMBO_BOX_TEXT (ctl_game_column),
			"icon-only", _("Icon only"));
	gtk_combo_box_text_append (GTK_COMBO_BOX_TEXT (ctl_game_column),
			"short-text", _("Game name"));
	gtk_combo_box_text_append (GTK_COMBO_BOX_TEXT (ctl_game_column),
			"full-text", _("Game name and game mode if known"));
	gtk_combo_box_set_active (GTK_COMBO_BOX (ctl_game_column), 0);
	g_signal_connect (ctl_game_column, "changed",
			G_CALLBACK (gui_prefs_gamecolumn_changed), NULL);
	gtk_grid_attach (GTK_GRID (grid), ctl_game_column, 1, 2, 1, 1);
	
	label = gtk_label_new (_("_Game column:"));
	g_object_set (G_OBJECT (label),
			"use-underline", TRUE,
			"mnemonic-widget", ctl_game_column,
			"margin-left", 8,
			"halign", GTK_ALIGN_START,
			NULL);
	gtk_grid_attach (GTK_GRID (grid), label, 0, 2, 1, 1);
	
	// Notifications section
	label = gtk_label_new (_("Notifications"));
	gtk_label_set_attributes (GTK_LABEL (label), bold);
	
	tmp = gtk_separator_new (GTK_ORIENTATION_HORIZONTAL);
	
	box = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 8);
	g_object_set (G_OBJECT (box),
			"margin-top", 6,
			NULL);
	gtk_box_pack_start (GTK_BOX (box), label, FALSE, TRUE, 0);
	gtk_box_pack_start (GTK_BOX (box), tmp, TRUE, TRUE, 0);
	gtk_grid_attach (GTK_GRID (grid), box, 0, 3, 2, 1);
	
	// Enable
	ctl_nenable = gtk_check_button_new_with_label (_("E_nable"));
	g_object_set (G_OBJECT (ctl_nenable),
			"use-underline", TRUE,
			"margin-left", 8,
			NULL);
	g_signal_connect (ctl_nenable, "toggled", G_CALLBACK (gui_prefs_enable_toggled), NULL);
	gtk_grid_attach (GTK_GRID (grid), ctl_nenable, 0, 4, 2, 1);
	
	// Sound
	ctl_nsound = gtk_file_chooser_button_new (_("Choose notification sound"),
			GTK_FILE_CHOOSER_ACTION_OPEN);
	g_object_set (G_OBJECT (ctl_nsound),
			"hexpand", TRUE,
			NULL);
	g_signal_connect (ctl_nsound, "file-set", G_CALLBACK (gs_prefs_sound_changed), NULL);
	gtk_grid_attach (GTK_GRID (grid), ctl_nsound, 1, 5, 1, 1);
	
	label = gtk_label_new (_("_Sound:"));
	g_object_set (G_OBJECT (label),
			"use-underline", TRUE,
			"mnemonic-widget", ctl_nsound,
			"margin-left", 8,
			"halign", GTK_ALIGN_START,
			NULL);
	gtk_grid_attach (GTK_GRID (grid), label, 0, 5, 1, 1);
	
	// Console section
	label = gtk_label_new (_("Console"));
	gtk_label_set_attributes (GTK_LABEL (label), bold);
	
	tmp = gtk_separator_new (GTK_ORIENTATION_HORIZONTAL);
	
	box = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 8);
	g_object_set (G_OBJECT (box),
			"margin-top", 6,
			NULL);
	gtk_box_pack_start (GTK_BOX (box), label, FALSE, TRUE, 0);
	gtk_box_pack_start (GTK_BOX (box), tmp, TRUE, TRUE, 0);
	gtk_grid_attach (GTK_GRID (grid), box, 0, 6, 2, 1);
	
	// Use system font
	ctl_sysfont = gtk_check_button_new_with_label (_("Use s_ystem monospace font"));
	g_object_set (G_OBJECT (ctl_sysfont),
			"use-underline", TRUE,
			"margin-left", 8,
			NULL);
	g_signal_connect (ctl_sysfont, "toggled",
			G_CALLBACK (gui_prefs_sysfont_toggled), NULL);
	gtk_grid_attach (GTK_GRID (grid), ctl_sysfont, 0, 7, 2, 1);
	
	// Font
	ctl_font = gtk_font_button_new_with_font (GS_DEFAULT_MONOSPACE_FONT);
	g_object_set (G_OBJECT (ctl_font),
			"hexpand", TRUE,
			NULL);
	g_signal_connect (ctl_font, "font-set", G_CALLBACK (gui_prefs_font_changed), NULL);
	gtk_grid_attach (GTK_GRID (grid), ctl_font, 1, 8, 1, 1);
	
	ctl_fontlabel = gtk_label_new (_("_Font:"));
	g_object_set (G_OBJECT (ctl_fontlabel),
			"use-underline", TRUE,
			"mnemonic-widget", ctl_font,
			"margin-left", 8,
			"halign", GTK_ALIGN_START,
			NULL);
	gtk_grid_attach (GTK_GRID (grid), ctl_fontlabel, 0, 8, 1, 1);
	
	pango_attr_list_unref (bold);
	gtk_widget_show_all (grid);
	return grid;
}


void
gui_prefs_set_update_rate (gdouble rate)
{
	g_signal_handlers_block_by_func (ctl_urate, gui_prefs_updaterate_changed, NULL);
	gtk_adjustment_set_value (ctl_urate, rate);
	g_signal_handlers_unblock_by_func (ctl_urate, gui_prefs_updaterate_changed, NULL);
}

void
gui_prefs_set_game_column_mode (GuiGameColumnMode mode)
{
	g_signal_handlers_block_by_func (ctl_game_column, gui_prefs_gamecolumn_changed, NULL);
	gtk_combo_box_set_active (GTK_COMBO_BOX (ctl_game_column), mode);
	g_signal_handlers_unblock_by_func (ctl_game_column, gui_prefs_gamecolumn_changed, NULL);
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
	g_signal_handlers_block_by_func (ctl_nsound, gs_prefs_sound_changed, NULL);
	gtk_file_chooser_set_filename (GTK_FILE_CHOOSER (ctl_nsound), filename);
	g_signal_handlers_unblock_by_func (ctl_nsound, gs_prefs_sound_changed, NULL);
}

void
gui_prefs_set_use_system_font (gboolean use)
{
	g_signal_handlers_block_by_func (ctl_sysfont, gui_prefs_sysfont_toggled, NULL);
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (ctl_sysfont), use);
	gtk_widget_set_sensitive (ctl_font, !use);
	gtk_widget_set_sensitive (ctl_fontlabel, !use);
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