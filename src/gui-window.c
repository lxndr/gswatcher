/* 
 * gui-window.c
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



#include <glib/gprintf.h>
#include "gui-window.h"
#include "gswatcher.h"
#include "gui-server-list.h"
#include "gui-info.h"
#include "gui-player-list.h"
#include "gui-buddy-list.h"
#include "gui-console.h"
#include "gui-log.h"
#include "gui-chat.h"
#include "gui-preferences.h"


GtkWidget *window = NULL;
static gint win_left = 10, win_top = 10, win_width = 800, win_height = 600;
static gboolean win_maximized = FALSE;
static GtkWidget *notebook;
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
check_slist_visibility ()
{
	gint tab_page = gtk_notebook_get_current_page (GTK_NOTEBOOK (notebook));
	GdkWindow *gdk_window = gtk_widget_get_window (window);
	gboolean iconified = gdk_window != NULL &&
			(gdk_window_get_state (gdk_window) & GDK_WINDOW_STATE_ICONIFIED);
	gboolean visible = gtk_widget_get_visible (window);
	gui_slist_set_visible (tab_page == 0 && !iconified && visible);
}


static void
tab_button_toggled (GtkToggleButton *button, gpointer udata)
{
	if (gtk_toggle_button_get_active (button))
		gtk_notebook_set_current_page (GTK_NOTEBOOK (notebook),
				GPOINTER_TO_INT (udata));
	check_slist_visibility ();
}


static void
gs_window_about_clicked (GtkButton *button)
{
	const gchar *authors[] = {
		"Alexander AB <lxndr87@users.sourceforge.net>",
		NULL
	};
	
	gtk_show_about_dialog (GTK_WINDOW (window),
			"program-name", "Game Server Watcher",
			"authors", authors,
#if GTK_MAJOR_VERSION == 3
			"license-type", GTK_LICENSE_GPL_3_0,
#endif
			"logo-icon-name", "gswatcher",
			"version", GS_VERSION,
			"website", GS_HOMEPAGE,
			"website-label", _("Offical Web Page"),
			NULL);
}


static void
gs_window_quit_clicked (GtkWidget *widget, gpointer udata)
{
	gs_application_shutdown ();
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
	gboolean pause = !gs_application_get_pause ();
	gs_application_set_pause (pause);
	
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

static gboolean
gs_window_configure_event (GtkWidget *gtk_window, GdkEvent *event, gpointer udata)
{
	if (!gtk_widget_get_visible (gtk_window))
		return FALSE;
	
	GdkWindow *gdk_window = gtk_widget_get_window (gtk_window);
	win_maximized = gdk_window != NULL &&
			(gdk_window_get_state (gdk_window) & GDK_WINDOW_STATE_MAXIMIZED);
	
    if (!win_maximized) {
		gtk_window_get_position (GTK_WINDOW (gtk_window), &win_left, &win_top);
		gtk_window_get_size (GTK_WINDOW (gtk_window), &win_width, &win_height);
	}
	
	return FALSE;
}

static gboolean
gs_window_state_event (GtkWidget *widget, GdkEvent *event, gpointer udata)
{
	GdkEventWindowState *e = (GdkEventWindowState *) event;
	GdkWindowState mask = GDK_WINDOW_STATE_WITHDRAWN | GDK_WINDOW_STATE_ICONIFIED;
	if ((e->changed_mask & mask) != (e->new_window_state & mask))
		check_slist_visibility ();
	return FALSE;
}


void
gui_window_show ()
{
	g_signal_handlers_block_by_func (trayshow, gs_window_show_toggled, NULL);
	gtk_check_menu_item_set_active (GTK_CHECK_MENU_ITEM (trayshow), TRUE);
	g_signal_handlers_unblock_by_func (trayshow, gs_window_show_toggled, NULL);
	
	if (!gtk_widget_get_visible( GTK_WIDGET (window))) {
		gtk_window_resize (GTK_WINDOW (window), win_width, win_height);
		gtk_window_move (GTK_WINDOW (window), win_left, win_top);
	}
	gtk_window_present_with_time (GTK_WINDOW (window), gtk_get_current_event_time ());
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
gui_window_load_geometry (JsonObject *geom)
{
	JsonNode* node;
	
	if ((node = json_object_get_member (geom, "left")))
		win_left = json_node_get_int (node);
	if ((node = json_object_get_member (geom, "top")))
		win_top = json_node_get_int (node);
	gtk_window_move (GTK_WINDOW (window), win_left, win_top);
	
	if ((node = json_object_get_member (geom, "width")))
		win_width = json_node_get_int (node);
	if ((node = json_object_get_member (geom, "height")))
		win_height = json_node_get_int (node);
	gtk_window_resize (GTK_WINDOW (window), win_width, win_height);
	
	if (json_object_get_boolean_member (geom, "maximized"))
		gtk_window_maximize (GTK_WINDOW (window));
	
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (layout_button),
			json_object_get_boolean_member (geom, "layout"));
	if ((node = json_object_get_member (geom, "slist-size")))
		gtk_paned_set_position (GTK_PANED (slist_paned), json_node_get_int (node));
	if ((node = json_object_get_member (geom, "plist-size")))
		gtk_paned_set_position (GTK_PANED (plist_paned), json_node_get_int (node));
}

JsonObject *
gui_window_save_geometry ()
{
	JsonObject *obj = json_object_new ();
	
	json_object_set_int_member (obj, "left", win_left);
	json_object_set_int_member (obj, "top", win_top);
	json_object_set_int_member (obj, "width", win_width);
	json_object_set_int_member (obj, "height", win_height);
	json_object_set_boolean_member (obj, "maximized", win_maximized);
	
	json_object_set_boolean_member (obj, "layout",
			gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (layout_button)));
	json_object_set_int_member (obj, "slist-size",
			gtk_paned_get_position (GTK_PANED (slist_paned)));
	json_object_set_int_member (obj, "plist-size",
			gtk_paned_get_position (GTK_PANED (plist_paned)));

	return obj;
}


static GtkWidget *
gui_window_create_tablabel (const gchar *icon, const gchar *title)
{
	GtkWidget *i = gtk_image_new_from_icon_name (icon, GTK_ICON_SIZE_MENU);
	GtkWidget *l = gtk_label_new (title);
	
	GtkWidget *b = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 4);
	gtk_box_pack_start (GTK_BOX (b), i, FALSE, TRUE, 0);
	gtk_box_pack_start (GTK_BOX (b), l, TRUE, TRUE, 0);
	gtk_widget_show_all (b);
	return b;
}


GtkWidget *
gui_window_create (GtkApplication *app)
{
/* tray icon */
	trayicon = gtk_status_icon_new_from_icon_name ("gswatcher");
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
	
	GtkWidget *trayquit = gtk_menu_item_new_with_label (_("Exit"));
	
	g_signal_connect (trayquit, "activate",
			G_CALLBACK (gs_window_quit_clicked), NULL);
	
	traymenu = gtk_menu_new ();
	traymenu = g_object_ref_sink (traymenu);
	gtk_menu_shell_append (GTK_MENU_SHELL (traymenu), trayshow);
	gtk_menu_shell_append (GTK_MENU_SHELL (traymenu), traypause);
	gtk_menu_shell_append (GTK_MENU_SHELL (traymenu), separator);
	gtk_menu_shell_append (GTK_MENU_SHELL (traymenu), trayquit);
	gtk_widget_show_all (traymenu);
	
/* toolbar */
	GtkStyleContext *context;
	GtkWidget *image;
	
	GtkWidget *slist_button = gtk_radio_button_new_with_label (
			NULL, _("Server list"));
	image = gtk_image_new_from_icon_name ("gtk-connect", GTK_ICON_SIZE_MENU);
	gtk_button_set_image (GTK_BUTTON (slist_button), image);
	gtk_toggle_button_set_mode (GTK_TOGGLE_BUTTON (slist_button), FALSE);
	context = gtk_widget_get_style_context (GTK_WIDGET (slist_button));
	gtk_style_context_add_class (context, "raised");
	g_signal_connect (slist_button, "toggled",
			G_CALLBACK (tab_button_toggled), GINT_TO_POINTER (0));
	
	GtkWidget *blist_button = gtk_radio_button_new_with_label_from_widget (
			GTK_RADIO_BUTTON (slist_button), _("Buddy list"));
	image = gtk_image_new_from_icon_name ("gtk-orientation-portrait", GTK_ICON_SIZE_MENU);
	gtk_button_set_image (GTK_BUTTON (blist_button), image);
	gtk_toggle_button_set_mode (GTK_TOGGLE_BUTTON (blist_button), FALSE);
	context = gtk_widget_get_style_context (GTK_WIDGET (blist_button));
	gtk_style_context_add_class (context, "raised");
	g_signal_connect (blist_button, "toggled",
			G_CALLBACK (tab_button_toggled), GINT_TO_POINTER (1));
	
	GtkWidget *prefs_button = gtk_radio_button_new_with_label_from_widget (
			GTK_RADIO_BUTTON (slist_button), _("Preferences"));
	image = gtk_image_new_from_icon_name ("preferences-system", GTK_ICON_SIZE_MENU);
	gtk_button_set_image (GTK_BUTTON (prefs_button), image);
	gtk_toggle_button_set_mode (GTK_TOGGLE_BUTTON (prefs_button), FALSE);
	context = gtk_widget_get_style_context (GTK_WIDGET (prefs_button));
	gtk_style_context_add_class (context, "raised");
	g_signal_connect (prefs_button, "toggled",
			G_CALLBACK (tab_button_toggled), GINT_TO_POINTER (2));
	
	GtkWidget *tab_box = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 4);
	gtk_box_set_homogeneous (GTK_BOX (tab_box), TRUE);
	gtk_box_pack_start (GTK_BOX (tab_box), slist_button, FALSE, TRUE, 0);
	gtk_box_pack_start (GTK_BOX (tab_box), blist_button, FALSE, TRUE, 0);
	gtk_box_pack_start (GTK_BOX (tab_box), prefs_button, FALSE, TRUE, 0);
	
	layout_button = gtk_toggle_button_new_with_label (_("_Layout"));
	g_object_set (G_OBJECT (layout_button),
			"use-underline", TRUE,
			NULL);
	context = gtk_widget_get_style_context (GTK_WIDGET (layout_button));
	gtk_style_context_add_class (context, "raised");
	g_signal_connect (layout_button, "toggled",
			G_CALLBACK (gs_window_layout_toggled), NULL);
	
	pause_button = gtk_toggle_button_new_with_label (_("_Pause"));
	image = gtk_image_new_from_icon_name ("media-playback-pause", GTK_ICON_SIZE_MENU);
	g_object_set (G_OBJECT (pause_button),
			"use-underline", TRUE,
			"image", image,
			NULL);
	context = gtk_widget_get_style_context (GTK_WIDGET (pause_button));
	gtk_style_context_add_class (context, "raised");
	g_signal_connect (pause_button, "toggled",
			G_CALLBACK (gs_window_pause_toggled), NULL);
	
	GtkWidget *about_button = gtk_button_new_with_label (_("About"));
	image = gtk_image_new_from_icon_name ("help-about", GTK_ICON_SIZE_MENU);
	g_object_set (G_OBJECT (about_button),
			"image", image,
			NULL);
	context = gtk_widget_get_style_context (GTK_WIDGET (about_button));
	gtk_style_context_add_class (context, "raised");
	g_signal_connect (about_button, "clicked",
			G_CALLBACK (gs_window_about_clicked), NULL);
	
	GtkWidget *quit_button = gtk_button_new_with_label (_("Quit"));
	image = gtk_image_new_from_icon_name ("application-exit", GTK_ICON_SIZE_MENU);
	g_object_set (G_OBJECT (quit_button),
			"image", image,
			NULL);
	context = gtk_widget_get_style_context (GTK_WIDGET (quit_button));
	gtk_style_context_add_class (context, "raised");
	g_signal_connect (quit_button, "clicked",
			G_CALLBACK (gs_window_quit_clicked), NULL);
	
	GtkWidget *control_box = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 4);
	gtk_box_set_homogeneous (GTK_BOX (tab_box), TRUE);
	gtk_box_pack_start (GTK_BOX (control_box), layout_button, FALSE, TRUE, 0);
	gtk_box_pack_start (GTK_BOX (control_box), pause_button, FALSE, TRUE, 0);
	gtk_box_pack_start (GTK_BOX (control_box), about_button, FALSE, TRUE, 0);
	gtk_box_pack_start (GTK_BOX (control_box), quit_button, FALSE, TRUE, 0);
	
	GtkWidget *toolbar_box = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 4);
	g_object_set (G_OBJECT (toolbar_box),
			"border-width", 3,
			NULL);
	gtk_box_pack_start (GTK_BOX (toolbar_box), tab_box, FALSE, TRUE, 0);
	gtk_box_pack_end (GTK_BOX (toolbar_box), control_box, FALSE, TRUE, 0);
	
	GtkToolItem *toolbar_item = gtk_tool_item_new ();
	gtk_tool_item_set_expand (toolbar_item, TRUE);
	gtk_container_add (GTK_CONTAINER (toolbar_item), toolbar_box);
	
	GtkWidget *toolbar = gtk_toolbar_new ();
	context = gtk_widget_get_style_context (GTK_WIDGET (toolbar));
	gtk_style_context_add_class (context, GTK_STYLE_CLASS_PRIMARY_TOOLBAR);
	gtk_toolbar_insert (GTK_TOOLBAR (toolbar), toolbar_item, 0);
	
	gtk_widget_show_all (toolbar);
	
/* infobox */
	GtkWidget *sinfo = gui_info_create ();
	plist = gui_plist_create ();
	
	infobox = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 4);
	gtk_box_pack_start (GTK_BOX (infobox), sinfo, TRUE, TRUE, 0);
	gtk_box_pack_start (GTK_BOX (infobox), plist, TRUE, TRUE, 0);
	gtk_widget_show (infobox);
	
	GtkWidget *infobox_label = gtk_label_new (_("Information"));
	gtk_widget_show (infobox_label);
	
	infobox_toolbar = gui_info_create_bar ();
	g_object_ref_sink (infobox_toolbar);
	
/* console */
	GtkWidget *console = gui_console_create ();
	
	GtkWidget *console_label = gtk_label_new (_("Console"));
	gtk_widget_show (console_label);
	
	console_toolbar = gui_console_create_bar ();
	g_object_ref_sink (console_toolbar);
	
/* log */
	GtkWidget *log = gui_log_create ();
	
	GtkWidget *log_label = gtk_label_new (_("Log"));
	gtk_widget_show (log_label);
	
	log_toolbar = gui_log_create_bar ();
	g_object_ref_sink (log_toolbar);
	
/* chat */
	GtkWidget *chat = gui_chat_create ();
	
	GtkWidget *chat_label = gtk_label_new (_("Chat"));
	gtk_widget_show (chat_label);
	
/* tools */
	GtkWidget *tools = gtk_notebook_new ();
	gtk_notebook_append_page (GTK_NOTEBOOK (tools), infobox, infobox_label);
	gtk_notebook_append_page (GTK_NOTEBOOK (tools), console, console_label);
	gtk_notebook_append_page (GTK_NOTEBOOK (tools), log, log_label);
	gtk_notebook_append_page (GTK_NOTEBOOK (tools), chat, chat_label);
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
	GtkWidget *prefs = gui_prefs_create ();
	
	GtkWidget *prefs_label = gui_window_create_tablabel ("preferences-system", _("Preferences"));
	
/* notebook */
	notebook = gtk_notebook_new ();
	g_object_set (G_OBJECT (notebook),
			"show-tabs", FALSE,
			NULL);
	gtk_notebook_append_page (GTK_NOTEBOOK (notebook), plist_paned, slist_label);
	gtk_notebook_append_page (GTK_NOTEBOOK (notebook), blist, blist_label);
	gtk_notebook_append_page (GTK_NOTEBOOK (notebook), prefs, prefs_label);
	gtk_widget_show (notebook);
	
/* window */
	GtkWidget *box = gtk_box_new (GTK_ORIENTATION_VERTICAL, 0);
	g_object_set (G_OBJECT (box),
			"visible", TRUE,
			NULL);
	gtk_box_pack_start (GTK_BOX (box), toolbar, FALSE, TRUE, 0);
	gtk_box_pack_start (GTK_BOX (box), notebook, TRUE, TRUE, 0);
	
	window = gtk_application_window_new (app);
	g_object_set (G_OBJECT (window),
			"title",			"Game Server Watcher",
			"icon-name",		"gswatcher",
			"default-width",	win_width,
			"default-height",	win_height,
			"hide-titlebar-when-maximized", TRUE,
			NULL);
	gtk_container_add (GTK_CONTAINER (window), box);
	g_signal_connect (window, "delete-event", G_CALLBACK (gs_window_delete_event), NULL);
	g_signal_connect (window, "configure-event", G_CALLBACK (gs_window_configure_event), NULL);
	g_signal_connect (window, "window-state-event", G_CALLBACK (gs_window_state_event), NULL);
	
	return window;
}

void
gui_window_destroy ()
{
	g_object_unref (traymenu);
	g_object_unref (trayicon);
	g_object_unref (infobox_toolbar);
	gui_console_destroy_bar ();
	g_object_unref (log_toolbar);
	
	gui_console_destroy ();
	gtk_widget_destroy (window);
}
