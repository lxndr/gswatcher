/*
  Game Server Watcher
  Copyright (C) 2010-2026 Alexander Burobin

  This program is free software: you can redistribute it and/or modify
  it under the terms of the GNU Affero General Public License as
  published by the Free Software Foundation, either version 3 of the
  License, or (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU Affero General Public License for more details.

  You should have received a copy of the GNU Affero General Public License
  along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

using Gtk;

namespace Gsw.Ui {

[GtkTemplate (ui = "/io/github/lxndr/gswatcher/ui/buddy-list.ui")]
class BuddyList : Widget {
  public Gsw.BuddyList buddy_list { get; set; }

  [GtkChild]
  private unowned SingleSelection selection;

  [GtkChild]
  private unowned ColumnView view;

  [GtkChild]
  private unowned ColumnViewColumn name_column;

  [GtkChild]
  private unowned PopoverMenu context_popover_menu;

  public signal void remove (Buddy buddy);

  class construct {
    set_layout_manager_type (typeof (BinLayout));
    set_css_name ("buddy-list");

    install_action ("remove-selected", null, (widget) => {
      var buddy_list = (BuddyList) widget;
      buddy_list.on_remove_selected ();
    });

    add_binding_action (Gdk.Key.Delete, 0, "remove-selected", null);
  }

  construct {
    view.sort_by_column (name_column, SortType.ASCENDING);

    selection.items_changed.connect ((position, removed, added) => {
      if (view != null) {
        if (added > 0) {
#if GSW_GTK_4_12_SUPPORTED
          view.scroll_to (position, null, SELECT | FOCUS, null);
#endif
          view.grab_focus ();
        }

        if (removed > 0)
          view.grab_focus ();
      }
    });
  }

  protected override void dispose () {
    remove_all_children (this);
    base.dispose ();
  }

  [GtkCallback]
  private void on_right_click (int n_press, double x, double y) {
    activate_context_menu (x, y);
  }

  [GtkCallback]
  private void on_long_press (double x, double y) {
    activate_context_menu (x, y);
  }

  private void activate_context_menu (double x, double y) {
    var item_pos = get_column_view_item_for_y (view, y);

    if (item_pos == null) {
      return;
    }

    selection.set_selected (item_pos.pos);
    item_pos.row_widget.focus (TAB_FORWARD);

    var rect = Gdk.Rectangle ();
    rect.x = (int) x;
    rect.y = (int) y;
    rect.width = 1;
    rect.height = 1;

    context_popover_menu.set_pointing_to (rect);
    context_popover_menu.popup ();
  }

  private void on_remove_selected () {
    var buddy = (Buddy) selection.selected_item;

    if (buddy != null)
      remove (buddy);
  }

  [GtkCallback]
  private void handle_notifications_change (Object obj, ParamSpec pspec) {
    // NOTE: `this` is GtkListItem
    var list_item = (ListItem) this;
    var buddy = (Buddy) list_item.item;
    var check = (Gtk.CheckButton) obj;
    buddy.notifications = check.active;
  }

  [GtkCallback]
  private string format_lastseen (DateTime? time) {
    return time == null ? "" : time.format ("%x %X");
  }
}

}
