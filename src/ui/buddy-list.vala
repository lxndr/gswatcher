using Gtk;

namespace Gsw.Ui {

class NewBuddy : Buddy {
  public NewBuddy () {
    Object (name : _("Click here to add new buddy"));
  }

  public signal void add (string name);
}

[GtkTemplate (ui = "/io/github/lxndr/gswatcher/ui/buddy-list.ui")]
class BuddyList : Widget {
  public Gsw.BuddyList buddy_list { get; set; }

  [GtkChild]
  private unowned SortListModel sort_model;

  [GtkChild]
  private unowned GLib.ListStore combined_list_store;

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
      var buddy = (Buddy) buddy_list.selection.selected_item;

      if (buddy != null)
        buddy_list.remove (buddy);
    }, null);
  }

  construct {
    var new_buddy_list_item = new NewBuddy ();
    new_buddy_list_item.add.connect (name => add (name));

    var new_item_list = new GLib.ListStore (typeof (NewBuddy));
    new_item_list.append (new_buddy_list_item);
    combined_list_store.append (sort_model);
    combined_list_store.append (new_item_list);

    view.sort_by_column (name_column, SortType.ASCENDING);

    selection.items_changed.connect ((position, removed, added) => {
      if (added > 0)
        view.scroll_to (position, null, SELECT | FOCUS, null);
    });
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
  private bool is_editor_row (Buddy? buddy) {
    if (buddy == null)
      return false;

    return buddy is NewBuddy;
  }

  [GtkCallback]
  private void handle_editing_change (Object obj, ParamSpec pspec) {
    // NOTE: `this` is GtkListItem
    var list_item = (ListItem) this;
    var buddy = (NewBuddy) list_item.item;
    var label = (Gtk.EditableLabel) obj;

    if (label.editing) {
      label.text = "";
    } else {
      var default_text = _("Click here to add new buddy");
      var text = label.text;

      if (text != "" && text != default_text)
        buddy.add (text);

      label.text = default_text;
    }
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
