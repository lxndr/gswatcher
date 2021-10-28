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

      add_binding (Gdk.Key.D, 0, (widget, args) => {
        print ("Shortcut triggered\n");
        var server_list = (BuddyList) widget;
        server_list.remove ((Buddy) server_list.selection.selected_item);
      }, null);
    }

    construct {
      view.sort_by_column (name_column, SortType.DESCENDING);
    }

    protected override void dispose () {
      get_first_child ().unparent ();
      base.dispose ();
    }
  }

}
