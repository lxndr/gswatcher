using Gtk;

namespace Gsw.Ui {

  [GtkTemplate (ui = "/org/lxndr/gswatcher/ui/server-list.ui")]
  class ServerList : Widget {
    public QuerierManager querier_manager { get; set; }
    public Gtk.MapListModel? server_item_list { get; private set; }

    [GtkChild]
    private unowned ColumnView view;

    [GtkChild]
    private unowned ColumnViewColumn name_column;

    class construct {
      typeof (ServerListItem).ensure ();
      typeof (NewServerListItem).ensure ();

      set_layout_manager_type (typeof (BinLayout));
    }

    construct {
      create_server_item_list ();
      notify["querier_manager"].connect (create_server_item_list);

      view.sort_by_column (name_column, SortType.ASCENDING);
    }

    protected override void dispose () {
      get_first_child ().unparent ();
      base.dispose ();
    }

    private void create_server_item_list () {
      if (querier_manager == null) {
        server_item_list = null;
      } else {
        server_item_list = new Gtk.MapListModel(querier_manager.querier_list, (querier) => {
          return new ServerListItem ((Querier) querier);
        });
      }
    }
  }

}
