using Gtk;

namespace Gsw.Ui {

[GtkTemplate (ui = "/org/lxndr/gswatcher/ui/buddy-list.ui")]
class BuddyList : Widget {
  public Gsw.BuddyList buddy_list { get; set; }

  [GtkChild]
  private unowned GLib.ListStore combined_list_store;

  [GtkChild]
  private unowned MapListModel mapped_list;

  [GtkChild]
  private unowned SingleSelection selection;

  [GtkChild]
  private unowned ColumnView view;

  [GtkChild]
  private unowned ColumnViewColumn name_column;

  public signal void add (string name);
  public signal bool change (Buddy buddy, string name);
  public signal void remove (Buddy buddy);

  class construct {
    set_layout_manager_type (typeof (BinLayout));
    set_css_name ("buddy-list");

    add_binding (Gdk.Key.Delete, 0, (widget, args) => {
      var buddy_list = (BuddyList) widget;
      var buddy_list_item = (BuddyListItem) buddy_list.selection.selected_item;
      var buddy = buddy_list_item.buddy;

      if (buddy != null)
        buddy_list.remove (buddy);
    }, null);
  }

  construct {
    mapped_list.set_map_func (item => new BuddyListItem ((Buddy) item));

    var new_buddy_list_item = new NewBuddyListItem ();
    new_buddy_list_item.added.connect (name => add (name));

    var new_item_list = new GLib.ListStore (typeof (BuddyListItem));
    new_item_list.append (new_buddy_list_item);
    combined_list_store.append (mapped_list);
    combined_list_store.append (new_item_list);

    view.sort_by_column (name_column, SortType.ASCENDING);
  }

  protected override void dispose () {
    get_first_child ().unparent ();
    base.dispose ();
  }

  [GtkCallback]
  private bool invert_boolean (bool b) {
    return !b;
  }

  [GtkCallback]
  private void handle_editing_change (Object obj, ParamSpec pspec) {
    // NOTE: `this` is GtkListItem
    var list_item = (ListItem) this;
    var item = (BuddyListItem) list_item.item;
    var label = (Gtk.EditableLabel) obj;

    if (label.editing) {
      item.handle_start_editing ();
    } else {
      item.handle_stop_editing (label.text);
    }
  }

  [GtkCallback]
  private void handle_notifications_change (Object obj, ParamSpec pspec) {
    // NOTE: `this` is GtkListItem
    var list_item = (ListItem) this;
    var item = (BuddyListItem) list_item.item;
    var check = (Gtk.CheckButton) obj;
    item.buddy.notifications = check.active;
  }
}

}
