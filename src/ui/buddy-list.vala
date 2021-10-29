using Gtk;

namespace Gsw.Ui {

  [GtkTemplate (ui = "/org/lxndr/gswatcher/ui/buddy-list.ui")]
  class BuddyList : Widget {
    public Gsw.BuddyList buddy_list { get; set; }

    [GtkChild]
    private unowned SingleSelection selection;

    [GtkChild]
    private unowned ColumnView view;

    [GtkChild]
    private unowned ColumnViewColumn name_column;

    public signal void remove (Buddy buddy);

    class construct {
      set_layout_manager_type (typeof (BinLayout));

      add_binding (Gdk.Key.Delete, 0, (widget, args) => {
        var buddy_list = (BuddyList) widget;
        var buddy = (Buddy) buddy_list.selection.selected_item;

        if (buddy != null) {
          buddy_list.remove (buddy);
        }
      }, null);
    }

    construct {
      view.sort_by_column (name_column, SortType.ASCENDING);
    }

    protected override void dispose () {
      get_first_child ().unparent ();
      base.dispose ();
    }

    [GtkCallback]
    public void notify_toggled (CheckButton check) {
      // TODO: how to tell what check button toggled?
      print ("toggled %d\n", (int) check.active);
    }
  }

}
