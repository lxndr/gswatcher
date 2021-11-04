namespace Gsw {

enum PlayerFieldType {
  STRING,
  NUMBER,
  DURATION;

  public static PlayerFieldType parse_nick (string nick) {
    var enumc = (EnumClass) typeof (PlayerFieldType).class_ref ();
    var eval = enumc.get_value_by_nick (nick);

    return eval == null
      ? PlayerFieldType.STRING
      : (PlayerFieldType) eval.value;
  }
}

class PlayerField : Object {
  public string title { get; set; }
  public string field { get; set; }
  public PlayerFieldType kind { get; set; default = STRING; }
  public bool main { get; set; default = false; }
}

class Player : Gee.HashMap<string, string> {
  public signal void change ();

  public override void @set (string key, string value) {
    if (value != @get (key)) {
      base.set (key, value);
    }
  }
}

class PlayerList : Object, ListModel {
  private Gee.List<Player> _list;

  construct {
    _list = new Gee.ArrayList<Player> ();
  }

  public Object? get_item (uint position) {
    return _list.get ((int) position);
  }

  public Type get_item_type () {
    return typeof (Player);
  }

  public uint get_n_items () {
    return _list.size;
  }

  public void apply (Gee.List<Player> players) {
    var old_size = _list.size;
    var new_size = players.size;
    var min_size = int.min (old_size, new_size);

    if (old_size > new_size) {
      while (_list.size > min_size)
        _list.remove_at (min_size);
      items_changed ((uint) (new_size - 1), (uint) (old_size - new_size), 0);
    }

    for (var idx = 0; idx < min_size; idx++) {
      _list[idx].set_all (players[idx]);
      _list[idx].change ();
    }

    if (new_size > old_size) {
      for (var idx = min_size; idx < new_size; idx++) {
        _list.add (players[idx]);
      }

      items_changed (old_size - 1, 0, new_size - old_size);
    }
  }
}

}
