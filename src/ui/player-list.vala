using Gtk;

namespace Gsw.Ui {

[GtkTemplate (ui = "/org/lxndr/gswatcher/ui/player-list.ui")]
class PlayerList : Widget {
  public Querier querier { get; set; }

  [GtkChild]
  private unowned ColumnView view;

  class construct {
    set_layout_manager_type (typeof (BinLayout));
  }

  construct {
    notify["querier"].connect (upate_columns);
    upate_columns ();
  }

  public override void dispose () {
    get_first_child ().unparent ();
    base.dispose ();
  }

  private void upate_columns () {
    while (view.columns.get_n_items () > 0)
      view.remove_column ((ColumnViewColumn) view.columns.get_item (0));

    if (querier == null)
      return;

    var n_columns = querier.plist_fields.get_n_items ();

    for (var i = 0; i < n_columns; i++) {
      var field = (PlayerField) querier.plist_fields.get_item (i);
      var column = create_column (field);
      view.append_column (column);
    }
  }

  private ColumnViewColumn create_column (PlayerField field) {
    var factory = new SignalListItemFactory ();
    factory.setup.connect ((factory, listitem) => setup_listitem (listitem, field));
    factory.bind.connect ((factory, listitem) => bind_listitem (listitem, field));

    var col = new ColumnViewColumn (field.title, factory);
    col.expand = field.main;
    return col;
  }

  private void setup_listitem (ListItem listitem, PlayerField field) {
    var label = new Label ("");

    switch (field.kind) {
      case PlayerFieldType.NUMBER:
      case PlayerFieldType.TIME:
        label.halign = Align.END;
        break;
      default:
        label.halign = Align.START;
        break;
    }

    listitem.child = label;
  }

  private void bind_listitem (ListItem listitem, PlayerField field) {
    var player = (Player) listitem.item;
    var label = (Label) listitem.child;

    player.change.connect(() => {
      switch (field.kind) {
        case PlayerFieldType.TIME:
          var duration = double.parse (player.get (field.field));
          label.label = format_time_duration ((int64) (duration * 1000000));
          break;
        default:
          label.label = player.get (field.field);
          break;
      }
    });

    player.change ();
  }
}

}
