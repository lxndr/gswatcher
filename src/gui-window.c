/* 
 * gui-window.c
 * Copyright (C) Alexander AB 2012 <lxndr87@users.sourceforge.net>
 * 
 * gstool is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * GSTool is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License along
 * with this program.  If not, see <http://www.gnu.org/licenses/>.
 */


#include <glib/gprintf.h>
#include "gui-window.h"
#include "gstool.h"
#include "gui-server-list.h"
#include "gui-info.h"
#include "gui-player-list.h"
#include "gui-buddy-list.h"
#include "gui-console.h"
#include "gui-log.h"
#include "gui-preferences.h"


GtkWidget *window = NULL;
static GtkWidget *infobox_toolbar = NULL;
static GtkWidget *console_toolbar = NULL;
static GtkWidget *log_toolbar = NULL;
static GtkWidget *chat_toolbar = NULL;
static GtkWidget *infobox;
static GtkWidget *slist_paned, *plist_paned;
static GtkWidget *plist;
static GtkWidget *layout_button;
static GtkWidget *pause_button;

static GtkStatusIcon *trayicon;
static GtkWidget *traymenu;
static GtkWidget *trayshow;
static GtkWidget *traypause;



static gboolean
gs_trayicon_button_press (GtkStatusIcon *trayicon, GdkEventButton *event,
		gpointer udata)
{
	if (event->type == GDK_BUTTON_PRESS && event->button == 1) {
		if (gtk_widget_get_visible (GTK_WIDGET (window)))
			gui_window_hide ();
		else
			gui_window_show ();
	}
	
	if (event->type == GDK_BUTTON_PRESS && event->button == 3) {
		gtk_menu_popup (GTK_MENU (traymenu), NULL, NULL,
#ifdef G_OS_WIN32
				NULL, NULL,
#else
				gtk_status_icon_position_menu, trayicon,
#endif
				event->button, event->time);
	}
	
	return FALSE;
}


static void
gs_window_tools_switched (GtkNotebook *notebook, GtkWidget *page, guint pagenum,
		gpointer udata)
{
	switch (pagenum) {
	case 0:
		gtk_notebook_set_action_widget (notebook, infobox_toolbar, GTK_PACK_END);
		break;
	case 1:
		gtk_notebook_set_action_widget (notebook, console_toolbar, GTK_PACK_END);
		break;
	case 2:
		gtk_notebook_set_action_widget (notebook, log_toolbar, GTK_PACK_END);
		break;
	case 3:
		gtk_notebook_set_action_widget (notebook, chat_toolbar, GTK_PACK_END);
		break;
	}
}


static void
gs_window_about_clicked (GtkButton *button)
{
	const gchar *authors[] = {
		"lxndr <lxndr87@users.sourceforge.net>",
		NULL
	};
	
	gtk_show_about_dialog (GTK_WINDOW (window),
			"program-name", "Game Server Tool",
			"authors", authors,
#if GTK_MAJOR_VERSION == 3
			"license-type", GTK_LICENSE_GPL_3_0,
#endif
			"logo-icon-name", "gstool",
			"version", GS_VERSION,
			"website", "http://gswtool.sourceforge.net",
			"website-label", _("Offical Web Page"),
			NULL);
}


static void
gs_window_quit_clicked (GtkWidget *widget, gpointer udata)
{
	gtk_main_quit ();
}


static void
gs_window_layout_toggled (GtkToggleButton *togglebutton, gpointer udata)
{
	if (gtk_toggle_button_get_active (togglebutton)) {
		gui_slist_set_hscrollbar (TRUE);
		gtk_container_remove (GTK_CONTAINER (infobox), g_object_ref (plist));
		gtk_paned_pack2 (GTK_PANED (plist_paned), plist, FALSE, FALSE);
		g_object_unref (plist);
	} else {
		gui_slist_set_hscrollbar (TRUE);
		gtk_container_remove (GTK_CONTAINER (plist_paned), g_object_ref (plist));
		gtk_box_pack_start (GTK_BOX (infobox), plist, TRUE, TRUE, 0);
		g_object_unref (plist);
	}
}


static void
gs_window_show_toggled (GtkCheckMenuItem *menuitem, gpointer udata)
{
	gboolean active = gtk_check_menu_item_get_active (menuitem);
	if (active)
		gui_window_show ();
	else
		gui_window_hide ();
}


static void
gs_window_pause_toggled (GtkWidget *widget, gpointer udata)
{
	gboolean pause = !gs_is_pause ();
	gs_pause (pause);
	
	g_signal_handlers_block_by_func (traypause, gs_window_pause_toggled, NULL);
	gtk_check_menu_item_set_active (GTK_CHECK_MENU_ITEM (traypause), pause);
	g_signal_handlers_unblock_by_func (traypause, gs_window_pause_toggled, NULL);
	
	g_signal_handlers_block_by_func (pause_button, gs_window_pause_toggled, NULL);
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (pause_button), pause);
	g_signal_handlers_unblock_by_func (pause_button, gs_window_pause_toggled, NULL);
}


static gboolean
gs_window_delete_event (GtkWidget *widget, GdkEvent *event, gpointer udata)
{
	gui_window_hide ();
	return TRUE;
}


void
gui_window_show ()
{
	g_signal_handlers_block_by_func (trayshow, gs_window_show_toggled, NULL);
	gtk_check_menu_item_set_active (GTK_CHECK_MENU_ITEM (trayshow), TRUE);
	g_signal_handlers_unblock_by_func (trayshow, gs_window_show_toggled, NULL);
	gtk_widget_show (window);
}

void
gui_window_hide ()
{
	g_signal_handlers_block_by_func (trayshow, gs_window_show_toggled, NULL);
	gtk_check_menu_item_set_active (GTK_CHECK_MENU_ITEM (trayshow), FALSE);
	g_signal_handlers_unblock_by_func (trayshow, gs_window_show_toggled, NULL);
	gtk_widget_hide (window);
}


void
gui_window_load_geometry (GJsonNode *geometry)
{
	if (g_json_object_has (geometry, "width") &&
			g_json_object_has (geometry, "height")) {
		gtk_window_resize (GTK_WINDOW (window),
				g_json_object_get_integer (geometry, "width"),
				g_json_object_get_integer (geometry, "height"));
	}
	
	if (g_json_object_has (geometry, "slist-size"))
		gtk_paned_set_position (GTK_PANED (slist_paned),
				g_json_object_get_integer (geometry, "slist-size"));
	if (g_json_object_has (geometry, "plist-size"))
		gtk_paned_set_position (GTK_PANED (plist_paned),
				g_json_object_get_integer (geometry, "plist-size"));
	
	if (g_json_object_has (geometry, "layout"))
		gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (layout_button),
				g_json_object_get_integer (geometry, "layout"));
}

GJsonNode *
gui_window_save_geometry ()
{
	GJsonNode *node = g_json_object_new ();
	
	gint width, height;
	gtk_window_get_size (GTK_WINDOW (window), &width, &height);
	g_json_object_set_integer (node, "width", width);
	g_json_object_set_integer (node, "height", height);
	
	gint slist_size = gtk_paned_get_position (GTK_PANED (slist_paned));
	g_json_object_set_integer (node, "slist-size", slist_size);
	
	gint plist_size = gtk_paned_get_position (GTK_PANED (plist_paned));
	g_json_object_set_integer (node, "plist-size", plist_size);
	
	gboolean layout = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (layout_button));
	g_json_object_set_integer (node, "layout", layout);
	
	return node;
}


static GtkWidget *
gui_window_create_tablabel (const gchar *icon, const gchar *title)
{
	GtkWidget *i = gtk_image_new_from_stock (icon, GTK_ICON_SIZE_MENU);
	GtkWidget *l = gtk_label_new (title);
	
	GtkWidget *b = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 4);
	gtk_box_pack_start (GTK_BOX (b), i, FALSE, TRUE, 0);
	gtk_box_pack_start (GTK_BOX (b), l, TRUE, TRUE, 0);
	gtk_widget_show_all (b);
	return b;
}


GtkWidget *
gui_window_create ()
{
/* tray icon */
	trayicon = gtk_status_icon_new_from_icon_name ("gstool");
	g_object_set (G_OBJECT (trayicon),
			"title", _("Game Server Tool"),
			"tooltip-text", _("Game Server Tool"),
			NULL);
	g_signal_connect (trayicon, "button-press-event",
			G_CALLBACK (gs_trayicon_button_press), NULL);
	
/* tray menu */
	trayshow = gtk_check_menu_item_new_with_label (_("_Show"));
	g_object_set (G_OBJECT (trayshow),
			"use-underline", TRUE,
			NULL);
	g_signal_connect (trayshow, "toggled",
			G_CALLBACK (gs_window_show_toggled), NULL);
	
	traypause = gtk_check_menu_item_new_with_label (_("_Pause"));
	g_object_set (G_OBJECT (traypause),
			"use-underline", TRUE,
			NULL);
	g_signal_connect (traypause, "toggled",
			G_CALLBACK (gs_window_pause_toggled), NULL);
	
	GtkWidget *separator = gtk_separator_menu_item_new ();
	
	GtkWidget *trayquit = gtk_image_menu_item_new_from_stock (GTK_STOCK_QUIT, NULL);
	g_signal_connect (trayquit, "activate",
			G_CALLBACK (gs_window_quit_clicked), NULL);
	
	traymenu = gtk_menu_new ();
	traymenu = g_object_ref_sink (traymenu);
	gtk_menu_shell_append (GTK_MENU_SHELL (traymenu), trayshow);
	gtk_menu_shell_append (GTK_MENU_SHELL (traymenu), traypause);
	gtk_menu_shell_append (GTK_MENU_SHELL (traymenu), separator);
	gtk_menu_shell_append (GTK_MENU_SHELL (traymenu), trayquit);
	gtk_widget_show_all (traymenu);
	
/* controls */
	layout_button = gtk_toggle_button_new_with_label (_("_Layout"));
	g_object_set (G_OBJECT (layout_button), "use-underline", TRUE, NULL);
	g_signal_connect (layout_button, "toggled",
			G_CALLBACK (gs_window_layout_toggled), NULL);
	
	GtkWidget *pause_icon = gtk_image_new_from_stock (GTK_STOCK_MEDIA_PAUSE,
			GTK_ICON_SIZE_MENU);
	pause_button = gtk_toggle_button_new_with_label (_("_Pause"));
	g_object_set (G_OBJECT (pause_button),
			"use-underline", TRUE,
			"image", pause_icon,
			NULL);
	g_signal_connect (pause_button, "toggled",
			G_CALLBACK (gs_window_pause_toggled), NULL);
	
	GtkWidget *about = gtk_button_new_from_stock (GTK_STOCK_ABOUT);
	g_signal_connect (about, "clicked", G_CALLBACK (gs_window_about_clicked), NULL);
	
	GtkWidget *quit = gtk_button_new_from_stock (GTK_STOCK_QUIT);
	g_signal_connect (quit, "clicked", G_CALLBACK (gs_window_quit_clicked), NULL);
	
	GtkWidget *toolbox = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 4);
	gtk_box_pack_start (GTK_BOX (toolbox), layout_button, FALSE, TRUE, 0);
	gtk_box_pack_start (GTK_BOX (toolbox), pause_button, FALSE, TRUE, 0);
	gtk_box_pack_start (GTK_BOX (toolbox), about, FALSE, TRUE, 0);
	gtk_box_pack_start (GTK_BOX (toolbox), quit, FALSE, TRUE, 0);
	gtk_widget_show_all (toolbox);
	
/* infobox */
	GtkWidget *sinfo = gui_info_create ();
	plist = gui_plist_create ();
	
	infobox = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 4);
	gtk_box_pack_start (GTK_BOX (infobox), sinfo, TRUE, TRUE, 0);
	gtk_box_pack_start (GTK_BOX (infobox), plist, TRUE, TRUE, 0);
	gtk_widget_show (infobox);
	
	GtkWidget *infobox_label = gtk_label_new (_("Info"));
	gtk_widget_show (infobox_label);
	
	infobox_toolbar = gui_plist_create_bar ();
	g_object_ref_sink (infobox_toolbar);
	
/* console */
	GtkWidget *console = gs_console_create ();
	
	GtkWidget *console_label = gtk_label_new (_("Console"));
	gtk_widget_show (console_label);
	
	console_toolbar = gs_console_create_bar ();
	g_object_ref_sink (console_toolbar);
	
/* log */
/*	GtkWidget *log = gs_log_create ();
	
	GtkWidget *log_label = gtk_label_new (_("Log"));
	gtk_widget_show (log_label);
	
	log_toolbar = gs_log_create_bar ();
	g_object_ref_sink (log_toolbar);*/
	
/* chat */
	/* TODO */
	
/* tools */
	GtkWidget *tools = gtk_notebook_new ();
	gtk_notebook_append_page (GTK_NOTEBOOK (tools), infobox, infobox_label);
	gtk_notebook_append_page (GTK_NOTEBOOK (tools), console, console_label);
	// gtk_notebook_append_page (GTK_NOTEBOOK (tools), log, log_label);
	// gtk_notebook_append_page (GTK_NOTEBOOK (tools), chat, chat_label);
	gtk_notebook_set_action_widget (GTK_NOTEBOOK (tools), infobox_toolbar,
			GTK_PACK_END);
	g_object_set (G_OBJECT (tools),
			"tab-pos", GTK_POS_BOTTOM,
			"visible", TRUE,
			NULL);
	g_signal_connect (tools, "switch-page",
			G_CALLBACK (gs_window_tools_switched), NULL);
	
/* server list tab */
	GtkWidget *slist = gui_slist_create ();
	
	slist_paned = gtk_paned_new (GTK_ORIENTATION_VERTICAL);
	gtk_paned_pack1 (GTK_PANED (slist_paned), slist, TRUE, FALSE);
	gtk_paned_pack2 (GTK_PANED (slist_paned), tools, FALSE, FALSE);
	gtk_widget_show (slist_paned);
	
	plist_paned = gtk_paned_new (GTK_ORIENTATION_HORIZONTAL);
	gtk_paned_pack1 (GTK_PANED (plist_paned), slist_paned, TRUE, FALSE);
	gtk_widget_show (plist_paned);
	
	GtkWidget *slist_label = gui_window_create_tablabel (GTK_STOCK_CONNECT, _("Server list"));
	
/* buddy list tab */
	GtkWidget *blist = gui_blist_create ();
	
	GtkWidget *blist_label = gui_window_create_tablabel (GTK_STOCK_ORIENTATION_PORTRAIT, _("Buddy list"));
	
/* preference tab */
	GtkWidget *prefs = gs_prefs_create ();
	
	GtkWidget *prefs_label = gui_window_create_tablabel (GTK_STOCK_PREFERENCES, _("Preferences"));
	
/* notebook */
	GtkWidget *notebook = gtk_notebook_new ();
	gtk_notebook_append_page (GTK_NOTEBOOK (notebook), plist_paned, slist_label);
	gtk_notebook_append_page (GTK_NOTEBOOK (notebook), blist, blist_label);
	gtk_notebook_append_page (GTK_NOTEBOOK (notebook), prefs, prefs_label);
	gtk_notebook_set_action_widget (GTK_NOTEBOOK (notebook), toolbox, GTK_PACK_END);
	gtk_widget_show (notebook);
	
	window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
	g_object_set (G_OBJECT (window),
			"title",			"Game Server Tool",
			"icon-name",		"gstool",
			"default-width",	800,
			"default-height",	600,
			"visible",			TRUE,
			NULL);
	gtk_container_add (GTK_CONTAINER (window), notebook);
	g_signal_connect (window, "delete-event", G_CALLBACK (gs_window_delete_event), NULL);
	
	return window;
}

void
gui_window_destroy ()
{
	g_object_unref (traymenu);
	g_object_unref (trayicon);
	g_object_unref (infobox_toolbar);
	g_object_unref (console_toolbar);
//	g_object_unref (log_toolbar);
//	g_object_unref (chat_toolbar);
	gtk_widget_destroy (window);
}
