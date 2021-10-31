using Gtk;

namespace Gsw.Ui {

private ListModel? create_itemlist (QuerierManager? querier_manager) {
  MapListModel? model = null;

  if (querier_manager != null) {
    model = new MapListModel (querier_manager.querier_list, (querier) => {
      return new ServerListItem ((Querier) querier);
    });
  }

  return model;
}

[GtkTemplate (ui = "/org/lxndr/gswatcher/ui/server-list.ui")]
class ServerList : Widget {
  public QuerierManager? querier_manager { get; set; }
  public MapListModel? item_list { get; set; }

  [GtkChild]
  private unowned SingleSelection selection;

  [GtkChild]
  private unowned ColumnView view;

  [GtkChild]
  private unowned ColumnViewColumn name_column;

  signal void remove (Server server);
  signal void selection_change (Querier querier);

  class construct {
    typeof (ServerListItem).ensure ();
    typeof (NewServerListItem).ensure ();

    set_layout_manager_type (typeof (BinLayout));

    add_binding (
      Gdk.Key.Delete, 0,
      (widget, args) => {
        var server_list = (ServerList) widget;
        var server = (Server) server_list.selection.selected_item;

        if (server != null)
          server_list.remove (server);

        return true;
      },
      null
    );
  }

  construct {
    bind_property (
      "querier-manager", this, "item-list", BindingFlags.SYNC_CREATE,
      (binding, from_value, ref to_value) => {
        var querier_manager = (QuerierManager) from_value.get_object ();
        var item_list = create_itemlist (querier_manager);
        to_value.set_object (item_list);
        return true;
      },
      null
    );

    view.sort_by_column (name_column, SortType.ASCENDING);
  }

  public override void dispose () {
    get_first_child ().unparent ();
    base.dispose ();
  }

  [GtkCallback]
  private void handle_selection_change (Object object, ParamSpec pspec) {
    var server_listitem = (ServerListItem) selection.get_selected_item ();
    selection_change (server_listitem.querier);
  }
}

}
