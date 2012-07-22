/* 
 * gui-server-list.c
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
#include "platform.h"
#include "gui-server-list.h"
#include "gui-console.h"
#include "gui-log.h"
#include "gui-chat.h"
#include "gui-window.h"
#include "gui-player-list.h"
#include "gui-info.h"


enum {
	ROW_FAVORITE_TITLE,
	ROW_FAVORITE,
	ROW_SEPARATOR,
	ROW_OTHER_TITLE,
	ROW_OTHER
};

enum {
	COLUMN_SERVER,
	COLUMN_TYPE,
	COLUMN_SPACE,
	COLUMN_NAME,
	COLUMN_NAME_WEIGHT,
	COLUMN_FLAG,
	COLUMN_ICON,
	COLUMN_ICON_VISIBLE,
	COLUMN_GAME,
	COLUMN_MAP,
	COLUMN_PLAYERS,
	COLUMN_PLAYERS_COLOR,
	COLUMN_PLAYERS_NUMBER,
	COLUMN_PING,
	COLUMN_PING_COLOR,
	COLUMN_ONLINE,
	COLUMN_NUMBER
};


static GtkWidget *entry;
static GtkWidget *scrolled;
static GtkWidget *listview;
static GtkTreeViewColumn *gamecolumn;
static GtkListStore *liststore;
static GuiGameColumnMode game_column_mode;
static GsClient *selected = NULL;
static gboolean visible = FALSE;


static gint
gui_slist_sort_string_func (GtkTreeModel *model, GtkTreeIter *a, GtkTreeIter *b,
		gpointer udata)
{
	gint id, at, bt, ret;
	gchar *as, *bs;
	GtkSortType order;
	
	gtk_tree_sortable_get_sort_column_id (GTK_TREE_SORTABLE (model), &id, &order);
	gtk_tree_model_get (model, a, COLUMN_TYPE, &at, id, &as, -1);
	gtk_tree_model_get (model, b, COLUMN_TYPE, &bt, id, &bs, -1);
	
	if (at == bt)
		ret = g_utf8_collate (as ? as : "", bs ? bs : "");
	else
		ret = order == GTK_SORT_ASCENDING ? at - bt : bt - at;
	
	g_free (as);
	g_free (bs);
	
	return ret;
}

static gint
gui_slist_sort_integer_func (GtkTreeModel *model, GtkTreeIter *a, GtkTreeIter *b,
		gpointer udata)
{
	gint id, at, bt;
	gint ai, bi;
	GtkSortType order;
	
	gtk_tree_sortable_get_sort_column_id (GTK_TREE_SORTABLE (model), &id, &order);
	gtk_tree_model_get (model, a, COLUMN_TYPE, &at, id, &ai, -1);
	gtk_tree_model_get (model, b, COLUMN_TYPE, &bt, id, &bi, -1);
	
	if (at != bt)
		return order == GTK_SORT_ASCENDING ? at - bt : bt - at;
	return ai - bi;
}

static gboolean
gui_slist_separator_func (GtkTreeModel *model, GtkTreeIter *iter, gpointer udata)
{
	int type;
	gtk_tree_model_get (model, iter, COLUMN_TYPE, &type, -1);
	if (type == ROW_SEPARATOR)
		return TRUE;
	return FALSE;
}

static gboolean
gui_slist_selection_func (GtkTreeSelection *selection, GtkTreeModel *model,
		GtkTreePath *path, gboolean path_currently_selected, gpointer udata)
{
	GtkTreeIter iter;
	int type;
	
	gtk_tree_model_get_iter (model, &iter, path);
	gtk_tree_model_get (model, &iter, COLUMN_TYPE, &type, -1);
	
	if (type == ROW_FAVORITE || type == ROW_OTHER)
		return TRUE;
	return FALSE;
}


static void
gui_slist_select_another (GtkTreeIter *iter)
{
	GtkTreeModel *model = GTK_TREE_MODEL (liststore);
	GtkTreeSelection *selection = gtk_tree_view_get_selection (
			GTK_TREE_VIEW (listview));
	GtkTreeIter tmp;
	gint type, type2;
	
	gtk_tree_model_get (model, iter, COLUMN_TYPE, &type, -1);
	
	tmp = *iter;
	if (gtk_tree_model_iter_next (model, &tmp)) {
		gtk_tree_model_get (model, &tmp, COLUMN_TYPE, &type2, -1);
		if (type == ROW_FAVORITE || type == ROW_OTHER) {
			gtk_tree_selection_select_iter (selection, &tmp);
		} else {
			gtk_tree_selection_unselect_iter (selection, iter);
		}
		return;
	}
	
	tmp = *iter;
	if (gtk_tree_model_iter_previous (model, &tmp)) {
		gtk_tree_model_get (model, &tmp, COLUMN_TYPE, &type2, -1);
		if (type == ROW_FAVORITE || type == ROW_OTHER) {
			gtk_tree_selection_select_iter (selection, &tmp);
		} else {
			gtk_tree_selection_unselect_iter (selection, iter);
		}
		return;
	}
}


void
gui_slist_remove (GsClient *client)
{
	gui_slist_select_another (&client->sliter);
	gtk_list_store_remove (liststore, &client->sliter);
}


static void
gui_slist_selection_changed (GtkTreeSelection *selection, gpointer udata)
{
	GtkTreeModel *model;
	GtkTreeIter iter;
	
	selected = NULL;
	if (gtk_tree_selection_get_selected (selection, &model, &iter))
		gtk_tree_model_get (model, &iter, COLUMN_SERVER, &selected, -1);
	
	const gchar *addr = selected ? gsq_querier_get_address (selected->querier) : "";
	gtk_entry_set_text (GTK_ENTRY (entry), addr);
	gui_info_setup (selected);
	gui_plist_setup (selected);
	gui_plist_update (selected);
	gui_console_setup (selected);
	gui_log_setup (selected);
	gui_chat_setup (selected);
}


static GdkPixbuf *
load_icon (const gchar *name, GError **error)
{
	static GHashTable *icons = NULL;
	GdkPixbuf *pixbuf;
	
	if (!(name && *name))
		return NULL;
	
	if (!icons)
		icons = g_hash_table_new_full (g_str_hash, g_str_equal, g_free,
				g_object_unref);
	
	if ((pixbuf = g_hash_table_lookup (icons, name)))
		return pixbuf;
	
	gchar *path = g_build_filename (app->icon_dir, name, NULL);
	pixbuf = gdk_pixbuf_new_from_file (path, error);
	if (!pixbuf) {
		g_free (path);
		return NULL;
	}
	g_free (path);
	
	g_hash_table_insert (icons, g_strdup (name), pixbuf);
	return pixbuf;
}

static GdkPixbuf *
get_flag_icon (const gchar *code)
{
	if (!(code && *code))
		return NULL;
	if (strcmp (code, "--") == 0)
		return NULL;
	
	GError *error = NULL;
	gchar *path = g_strdup_printf ("flags%s%s.png", G_DIR_SEPARATOR_S, code);
	GdkPixbuf *pixbuf = load_icon (path, &error);
	
	if (!pixbuf) {
		g_warning ("Could not load %s flag icon: %s", code, error->message);
		g_error_free (error);
	}
	
	g_free (path);
	
	return pixbuf;
}

static GdkPixbuf *
get_game_icon (const gchar *code)
{
	if (!(code && *code))
		return NULL;
	
	GError *error = NULL;
	gchar *path = g_strdup_printf ("games%s%s.png", G_DIR_SEPARATOR_S, code);
	GdkPixbuf *pixbuf = load_icon (path, &error);
	
	if (!pixbuf) {
		g_warning ("Could not load %s game icon: %s", code, error->message);
		g_error_free (error);
	}
	
	g_free (path);
	
	return pixbuf;
}


static gchar *
format_date_time (gint64 time)
{
	gint64 day = time / (24 * 60 * 60);
	gint64 hour = time / 60 / 60 % 24;
	gint64 minute = time / 60 % 60;
	gint64 second = time % 60;
	
	GString *s = g_string_sized_new (8);
	
	if (day > 0)
		g_string_append_printf (s, "%" G_GINT64_FORMAT " ", day);
	g_string_append_printf (s, "%" G_GINT64_FORMAT ":%.2" G_GINT64_FORMAT, hour, minute);
	if (hour == 0)
		g_string_append_printf (s, ":%.2" G_GINT64_FORMAT, second);
	
	return g_string_free (s, FALSE);
}


static void
server_error (GsqQuerier *querier, const gchar *msg, GsClient *client)
{
	gtk_list_store_set (liststore, &client->sliter,
			COLUMN_PING, msg,
			COLUMN_PING_COLOR, "red",
			COLUMN_ONLINE, FALSE,
			-1);
}

static void
server_timed_out (GsqQuerier *querier, GsClient *client)
{
	GTimeVal tv;
	g_get_current_time (&tv);
	guint64 time = tv.tv_sec - gsq_watcher_get_offline_time (GSQ_WATCHER (querier));
	
	if (time < 15) {
		gtk_list_store_set (liststore, &client->sliter,
				COLUMN_PING, _("Timeout"),
				COLUMN_PING_COLOR, "red",
				COLUMN_PLAYERS_COLOR, NULL,
				COLUMN_ONLINE, FALSE,
				-1);
	} else {
		gchar *tmp = format_date_time (time);
		
		gtk_list_store_set (liststore, &client->sliter,
				COLUMN_PING, tmp,
				COLUMN_PING_COLOR, "red",
				COLUMN_ONLINE, FALSE,
				-1);
		
		g_free (tmp);
	}
	
	if (client == selected)
		gui_info_setup (client);
}

static void
server_resolved (GsqQuerier *querier, GsClient *client)
{
	gtk_list_store_set (liststore, &client->sliter,
			COLUMN_FLAG, get_flag_icon (client->country_code),
			-1);
	
	if (client == selected)
		gui_info_update (client);
}

static void
game_detected (GsqQuerier *querier, GsClient *client)
{
	gtk_list_store_set (liststore, &client->sliter,
			COLUMN_ICON, get_game_icon (client->querier->gameid->str),
			-1);
	
	if (client == selected)
		gui_plist_setup (client);
}


static void
server_info_updated (GsqQuerier *querier, GsClient *client)
{
	gui_slist_update (client);
}


static void
server_players_updated (GsqQuerier *querier, GsClient *cl)
{
	if (cl == selected)
		gui_plist_update (cl);
}


void
gui_slist_add (GsClient *cl)
{
	g_return_if_fail (GS_IS_CLIENT (cl));
	
	gtk_list_store_append (liststore, &cl->sliter);
	gtk_list_store_set (liststore, &cl->sliter,
			COLUMN_SERVER, cl,
			COLUMN_SPACE, TRUE,
			COLUMN_ICON_VISIBLE, game_column_mode == GUI_GAME_COLUMN_ICON,
			COLUMN_PING, _("Querying"),
			COLUMN_PING_COLOR, "grey",
			COLUMN_ONLINE, FALSE,
			-1);
	
	gui_slist_update (cl);
	
	GtkTreeSelection *sel = gtk_tree_view_get_selection (GTK_TREE_VIEW (listview));
	if (gtk_tree_selection_count_selected_rows (sel) == 0)
		gtk_tree_selection_select_iter (sel, &cl->sliter);
	
	g_signal_connect (cl->querier, "error", G_CALLBACK (server_error), cl);
	g_signal_connect (cl->querier, "timeout", G_CALLBACK (server_timed_out), cl);
	g_signal_connect (cl->querier, "resolve", G_CALLBACK (server_resolved), cl);
	g_signal_connect (cl->querier, "gameid-changed", G_CALLBACK (game_detected), cl);
	g_signal_connect (cl->querier, "info-update", G_CALLBACK (server_info_updated), cl);
	g_signal_connect (cl->querier, "players-update", G_CALLBACK (server_players_updated), cl);
}


GsClient *
gui_slist_get_selected ()
{
	return selected;
}


void
gui_slist_set_hscrollbar (gboolean hscrollbar)
{
	gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolled),
			hscrollbar ? GTK_POLICY_AUTOMATIC : GTK_POLICY_NEVER,
			GTK_POLICY_AUTOMATIC);
}


void
gui_slist_set_game_column_mode (GuiGameColumnMode mode)
{
	gtk_tree_view_column_set_visible (gamecolumn, mode != GUI_GAME_COLUMN_ICON);
	game_column_mode = mode;
	gui_slist_update_list (app->server_list);
	
	GList *server_iter = app->server_list;
	while (server_iter) {
		GsClient *client = server_iter->data;
		gtk_list_store_set (liststore, &client->sliter,
				COLUMN_ICON_VISIBLE, game_column_mode == GUI_GAME_COLUMN_ICON,
				-1);
		server_iter = server_iter->next;
	}
}

GuiGameColumnMode
gui_slist_get_game_column_mode ()
{
	return game_column_mode;
}


void
gui_slist_set_visible (gboolean seen)
{
	if (seen && !visible) {
		visible = TRUE;
		gui_slist_update_list (app->server_list);
	}
	visible = seen;
}


void
gui_slist_update (GsClient *cl)
{
	if (!visible)
		return;
	
	/* game name */
	gchar *gamename = gs_client_get_game_name (cl,
			game_column_mode == GUI_GAME_COLUMN_FULL);
	
	/* map name */
	const gchar *mapname = cl->querier->map->str;
	
	/* number of players */
	gchar *players = g_strdup_printf ("%d / %d",
			cl->querier->numplayers,
			cl->querier->maxplayers);
	
	/* player count color */
	gint num = cl->querier->numplayers;
	gint max = cl->querier->maxplayers;
	const gchar *player_color = NULL;
	if (num != 0 && max > 0)
		player_color = num >= max ? "dark red" : "dark green";
	
	/* ping */
	gchar *ping = g_strdup_printf ("%ld", gsq_querier_get_ping (cl->querier));
	
	gtk_list_store_set (liststore, &cl->sliter,
			COLUMN_TYPE, cl->favorite ? ROW_FAVORITE : ROW_OTHER,
			COLUMN_NAME, cl->querier->name->str,
			COLUMN_GAME, *gamename ? gamename : _("unknown"),
			COLUMN_MAP, *mapname ? mapname : _("unknown"),
			COLUMN_PLAYERS, players,
			COLUMN_PLAYERS_COLOR, player_color,
			COLUMN_PLAYERS_NUMBER, num,
			-1);
	
	if (gsq_querier_get_ping (cl->querier))
		gtk_list_store_set (liststore, &cl->sliter,
				COLUMN_PING, ping,
				COLUMN_PING_COLOR, NULL,
				COLUMN_ONLINE, TRUE,
				-1);
	
	g_free (ping);
	g_free (players);
	g_free (gamename);
	
	if (cl == selected)
		gui_info_update (cl);
}

void
gui_slist_update_list (GList *ls)
{
	while (ls) {
		GsClient *cl = ls->data;
		gui_slist_update (cl);
		ls = ls->next;
	}
}


static void
gs_address_activated (GtkEntry *entry, gpointer udata)
{
	const gchar *address = gtk_entry_get_text (entry);
	gs_application_add_server (app, address);
}


static void
gs_address_icon_clicked (GtkEntry *entry, GtkEntryIconPosition icon,
		GdkEvent *event, gpointer udata)
{
	const gchar *address = gtk_entry_get_text (entry);
	
	if (icon == GTK_ENTRY_ICON_PRIMARY) {
		// copy to the clipboard
		GtkClipboard *clipboard = gtk_widget_get_clipboard (GTK_WIDGET (entry),
				GDK_SELECTION_CLIPBOARD);
		gtk_clipboard_set_text (clipboard, address, -1);
	} else {
		// add new server
		const gchar *address = gtk_entry_get_text (entry);
		gs_application_add_server (app, address);
	}
}


static void
gs_address_changed (GtkEntry *entry, gpointer udata)
{
	const gchar *address = gtk_entry_get_text (entry);
	
	gboolean enabled = *address != 0;
	gtk_entry_set_icon_sensitive (entry, GTK_ENTRY_ICON_PRIMARY, enabled);
	gtk_entry_set_icon_activatable (entry, GTK_ENTRY_ICON_PRIMARY, enabled);
	
	enabled = *address && !gs_application_find_server (app, address);
	gtk_entry_set_icon_sensitive (entry, GTK_ENTRY_ICON_SECONDARY, enabled);
	gtk_entry_set_icon_activatable (entry, GTK_ENTRY_ICON_SECONDARY, enabled);
}


static gboolean
slist_key_pressed (GtkWidget *widget, GdkEventKey *event, gpointer udata)
{
	if (event->keyval == GDK_KEY_Delete) {
		gs_application_remove_server_ask (app, selected);
	} else if (event->state == GDK_CONTROL_MASK &&
			(event->keyval == GDK_KEY_C || event->keyval == GDK_KEY_c ||
			event->keyval == GDK_KEY_Cyrillic_PE || event->keyval == GDK_KEY_Cyrillic_pe)) {
		gs_client_connect_to_game (selected);
	} else if (event->state == GDK_CONTROL_MASK &&
			(event->keyval == GDK_KEY_F || event->keyval == GDK_KEY_f ||
			event->keyval == GDK_KEY_Cyrillic_I || event->keyval == GDK_KEY_Cyrillic_i)) {
		/* TODO favour */
	}
	
	return TRUE;
}


GtkWidget *
gui_slist_create ()
{
/* address */
	entry = gtk_entry_new ();
	g_object_set (G_OBJECT (entry),
			"primary-icon-stock", GTK_STOCK_COPY,
			"primary-icon-sensitive", FALSE,
			"secondary-icon-stock", GTK_STOCK_ADD,
			"secondary-icon-sensitive", FALSE,
			NULL);
	g_signal_connect (entry, "activate", G_CALLBACK (gs_address_activated), NULL);
	g_signal_connect (entry, "icon-release", G_CALLBACK (gs_address_icon_clicked), NULL);
	g_signal_connect (entry, "changed", G_CALLBACK (gs_address_changed), NULL);
	
/* server list model */
	liststore = gtk_list_store_new (COLUMN_NUMBER,
			// server, type
			G_TYPE_POINTER, G_TYPE_INT,
			// space
			G_TYPE_BOOLEAN,
			// name text, name weight, flag
			G_TYPE_STRING, G_TYPE_INT, GDK_TYPE_PIXBUF,
			// icon, game
			GDK_TYPE_PIXBUF, G_TYPE_BOOLEAN, G_TYPE_STRING,
			// map
			G_TYPE_STRING,
			// players, color, number
			G_TYPE_STRING, G_TYPE_STRING, G_TYPE_INT,
			// ping, ping color
			G_TYPE_STRING, G_TYPE_STRING,
			/* online */
			G_TYPE_BOOLEAN);
	gtk_tree_sortable_set_sort_func (GTK_TREE_SORTABLE (liststore),
			COLUMN_NAME, gui_slist_sort_string_func, NULL, NULL);
	gtk_tree_sortable_set_sort_column_id (GTK_TREE_SORTABLE (liststore),
			COLUMN_NAME, GTK_SORT_ASCENDING);
	gtk_tree_sortable_set_sort_func (GTK_TREE_SORTABLE (liststore),
			COLUMN_GAME, gui_slist_sort_string_func, NULL, NULL);
	gtk_tree_sortable_set_sort_func (GTK_TREE_SORTABLE (liststore),
			COLUMN_MAP, gui_slist_sort_string_func, NULL, NULL);
	gtk_tree_sortable_set_sort_func (GTK_TREE_SORTABLE (liststore),
			COLUMN_PLAYERS_NUMBER, gui_slist_sort_integer_func, NULL, NULL);
	gtk_tree_sortable_set_sort_func (GTK_TREE_SORTABLE (liststore),
			COLUMN_PING, gui_slist_sort_string_func, NULL, NULL);
	
/* categories */
	GtkTreeIter iter;
	gtk_list_store_append (liststore, &iter);
	gtk_list_store_set (liststore, &iter,
			COLUMN_TYPE, ROW_FAVORITE_TITLE,
			COLUMN_SPACE, FALSE,
			COLUMN_NAME, _("Favorite servers"),
			COLUMN_NAME_WEIGHT, PANGO_WEIGHT_BOLD,
			COLUMN_ONLINE, TRUE,
			-1);
	gtk_list_store_append (liststore, &iter);
	gtk_list_store_set (liststore, &iter,
			COLUMN_TYPE, ROW_SEPARATOR,
			-1);
	gtk_list_store_append (liststore, &iter);
	gtk_list_store_set (liststore, &iter,
			COLUMN_TYPE, ROW_OTHER_TITLE,
			COLUMN_SPACE, FALSE,
			COLUMN_NAME, _("Other servers"),
			COLUMN_NAME_WEIGHT, PANGO_WEIGHT_BOLD,
			COLUMN_ONLINE, TRUE,
			-1);
	
/* server list view */
	listview = gtk_tree_view_new ();
	g_object_set (G_OBJECT (listview),
			"visible", TRUE,
			"model", liststore,
			NULL);
	gtk_tree_view_set_row_separator_func (GTK_TREE_VIEW (listview),
			(GtkTreeViewRowSeparatorFunc) gui_slist_separator_func, NULL, NULL);
	g_signal_connect (listview, "key-press-event",
			G_CALLBACK (slist_key_pressed), NULL);
	
	GtkTreeViewColumn *column;
	GtkCellRenderer *cell;
	
/* name column */
	column = gtk_tree_view_column_new ();
	g_object_set (G_OBJECT (column),
			"title", C_("Server", "Name"),
			"clickable", TRUE,
			"expand", TRUE,
			"sort-column-id", COLUMN_NAME,
			"min-width", 180,
			NULL);
	cell = gtk_cell_renderer_pixbuf_new ();
	g_object_set (G_OBJECT (cell),
			"width", 10,
			NULL);
	gtk_cell_layout_pack_start (GTK_CELL_LAYOUT (column), cell, FALSE);
	gtk_cell_layout_set_attributes (GTK_CELL_LAYOUT (column), cell,
			"visible", COLUMN_SPACE,
			NULL);
	cell = gtk_cell_renderer_pixbuf_new ();
	gtk_cell_layout_pack_start (GTK_CELL_LAYOUT (column), cell, FALSE);
	gtk_cell_layout_set_attributes (GTK_CELL_LAYOUT (column), cell,
			"pixbuf", COLUMN_ICON,
			"visible", COLUMN_ICON_VISIBLE,
			"sensitive", COLUMN_ONLINE,
			NULL);
	cell = gtk_cell_renderer_text_new ();
	g_object_set (G_OBJECT (cell),
			"ellipsize", PANGO_ELLIPSIZE_END,
			"xpad", 0,
			NULL);
	gtk_cell_layout_pack_start (GTK_CELL_LAYOUT (column), cell, TRUE);
	gtk_cell_layout_set_attributes (GTK_CELL_LAYOUT (column), cell,
			"text", COLUMN_NAME,
			"weight", COLUMN_NAME_WEIGHT,
			"sensitive", COLUMN_ONLINE,
			NULL);
	cell = gtk_cell_renderer_pixbuf_new ();
	gtk_cell_layout_pack_start (GTK_CELL_LAYOUT (column), cell, FALSE);
	gtk_cell_layout_set_attributes (GTK_CELL_LAYOUT (column), cell,
			"pixbuf", COLUMN_FLAG,
			"sensitive", COLUMN_ONLINE,
			NULL);
	gtk_tree_view_column_set_sort_order (GTK_TREE_VIEW_COLUMN (column),
			GTK_SORT_ASCENDING);
	gtk_tree_view_insert_column (GTK_TREE_VIEW (listview), column, -1);
	
/* game column */
	gamecolumn = gtk_tree_view_column_new ();
	g_object_set (G_OBJECT (gamecolumn),
			"title", _("Game"),
			"clickable", TRUE,
			"sort-column-id", COLUMN_GAME,
			"sizing", GTK_TREE_VIEW_COLUMN_AUTOSIZE,
			NULL);
	cell = gtk_cell_renderer_pixbuf_new ();
	gtk_cell_layout_pack_start (GTK_CELL_LAYOUT (gamecolumn), cell, FALSE);
	gtk_cell_layout_set_attributes (GTK_CELL_LAYOUT (gamecolumn), cell,
			"pixbuf", COLUMN_ICON,
			"sensitive", COLUMN_ONLINE,
			NULL);
	cell = gtk_cell_renderer_text_new ();
	gtk_cell_layout_pack_start (GTK_CELL_LAYOUT (gamecolumn), cell, TRUE);
	gtk_cell_layout_set_attributes (GTK_CELL_LAYOUT (gamecolumn), cell,
			"text", COLUMN_GAME,
			"sensitive", COLUMN_ONLINE,
			NULL);
	gtk_tree_view_insert_column (GTK_TREE_VIEW (listview), gamecolumn, -1);
	
/* map */
	column = gtk_tree_view_column_new ();
	g_object_set (G_OBJECT (column),
			"title", _("Map"),
			"clickable", TRUE,
			"sort-column-id", COLUMN_MAP,
			"max-width", 225,
			NULL);
	cell = gtk_cell_renderer_text_new ();
	gtk_cell_layout_pack_start (GTK_CELL_LAYOUT (column), cell, TRUE);
	gtk_cell_layout_set_attributes (GTK_CELL_LAYOUT (column), cell,
			"text", COLUMN_MAP,
			"sensitive", COLUMN_ONLINE,
			NULL);
	gtk_tree_view_insert_column (GTK_TREE_VIEW (listview), column, -1);
	
/* palyers */
	column = gtk_tree_view_column_new ();
	g_object_set (G_OBJECT (column),
			"title", _("Players"),
			"clickable", TRUE,
			"sort-column-id", COLUMN_PLAYERS_NUMBER,
			NULL);
	cell = gtk_cell_renderer_text_new ();
	gtk_cell_layout_pack_start (GTK_CELL_LAYOUT (column), cell, TRUE);
	gtk_cell_layout_set_attributes (GTK_CELL_LAYOUT (column), cell,
			"text", COLUMN_PLAYERS,
			"foreground", COLUMN_PLAYERS_COLOR,
			"sensitive", COLUMN_ONLINE,
			NULL);
	gtk_tree_view_insert_column (GTK_TREE_VIEW (listview), column, -1);
	
/* ping */
	column = gtk_tree_view_column_new ();
	g_object_set (G_OBJECT (column),
			"title", _("Ping"),
			"clickable", TRUE,
			"sort-column-id", COLUMN_PING,
			"sizing", GTK_TREE_VIEW_COLUMN_AUTOSIZE,
			NULL);
	cell = gtk_cell_renderer_text_new ();
	gtk_cell_layout_pack_start (GTK_CELL_LAYOUT (column), cell, TRUE);
	gtk_cell_layout_set_attributes (GTK_CELL_LAYOUT (column), cell,
			"text", COLUMN_PING,
			"foreground", COLUMN_PING_COLOR,
			NULL);
	gtk_tree_view_insert_column (GTK_TREE_VIEW (listview), column, -1);
	
	
	GtkTreeSelection *sel = gtk_tree_view_get_selection (GTK_TREE_VIEW (listview));
	gtk_tree_selection_set_select_function (GTK_TREE_SELECTION (sel),
			(GtkTreeSelectionFunc) gui_slist_selection_func, NULL, NULL);
	g_signal_connect (sel, "changed",
			G_CALLBACK (gui_slist_selection_changed), NULL);
	
	scrolled = gtk_scrolled_window_new (NULL, NULL);
	g_object_set (G_OBJECT (scrolled),
			"visible", TRUE,
			"shadow-type", GTK_SHADOW_IN,
			"hscrollbar-policy", GTK_POLICY_NEVER,
			NULL);
	gtk_container_add (GTK_CONTAINER (scrolled), listview);
	
/* box */
	GtkWidget *box = gtk_box_new (GTK_ORIENTATION_VERTICAL, 4);
	gtk_box_pack_start (GTK_BOX (box), entry, FALSE, TRUE, 0);
	gtk_box_pack_start (GTK_BOX (box), scrolled, TRUE, TRUE, 0);
	gtk_widget_show_all (box);
	
	gui_slist_set_game_column_mode (GUI_GAME_COLUMN_ICON);
	
	return box;
}
