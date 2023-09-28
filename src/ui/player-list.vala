using Gtk;

namespace Gsw.Ui {

[GtkTemplate (ui = "/ru/lxndr/gswatcher/ui/player-list.ui")]
class PlayerList : Widget {
  private Client? _client;

  [GtkChild]
  private unowned ColumnView view;

  class construct {
    set_layout_manager_type (typeof (BinLayout));
  }

  public override void dispose () {
    get_first_child ().unparent ();
    base.dispose ();
  }

  public Client? client {
    get {
      return _client;
    }

    set {
      if (_client != null) {
        _client.plist_fields.items_changed.disconnect (plist_fields_change);
        plist_fields_change (0, _client.plist_fields.get_n_items (), 0);
      }

      _client = value;

      if (_client != null) {
        _client.plist_fields.items_changed.connect (plist_fields_change);
        plist_fields_change (0, 0, _client.plist_fields.get_n_items ());
      }
    }
  }

  private void plist_fields_change (uint position, uint removed, uint added) {
    for (var i = 0; i < removed; i++) {
      if (view.columns.get_n_items () > position)
        view.remove_column ((ColumnViewColumn) view.columns.get_item (position));
    }

    for (var i = 0; i < added; i++) {
      var index = position + i;
      var field = (PlayerField) client.plist_fields.get_item (index);
      var column = create_column (field, i == 0);
      view.insert_column (index, column);
    }
  }

  private ColumnViewColumn create_column (PlayerField field, bool expand) {
    var factory = new SignalListItemFactory ();
    factory.setup.connect ((factory, listitem) => player_setup (listitem as ListItem, field));
    factory.bind.connect ((factory, listitem) => player_bind (listitem as ListItem, field));

    var col = new ColumnViewColumn (field.title, factory);
    // TODO: add player list sorting
    col.expand = expand;
    return col;
  }

  private void player_setup (ListItem listitem, PlayerField field) {
    var label = new Label ("");

    switch (field.kind) {
      case PlayerFieldType.NUMBER:
      case PlayerFieldType.DURATION:
        label.halign = Align.END;
        break;
      default:
        label.halign = Align.START;
        break;
    }

    listitem.child = label;
  }

  private void player_bind (ListItem listitem, PlayerField field) {
    var player = (Player) listitem.item;
    var label = (Label) listitem.child;

    player.change.connect(() => {
      switch (field.kind) {
        case PlayerFieldType.DURATION:
          var str_val = player.get (field.field);
          double val = 0;

          if (str_val == null) {
            label.label = "";
          } else if (double.try_parse (str_val, out val, null)) {
            label.label = format_time_duration ((int64) (val * 1000000));
          } else {
            log (Config.LOG_DOMAIN, LEVEL_WARNING, "could not parse double value '%s'", str_val);
            label.label = "";
          }

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
