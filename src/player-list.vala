namespace Gsw {

enum PlayerFieldType {
  STRING,
  NUMBER,
  TIME
}

class PlayerField : Object {
  public string title { get; set; }
  public string field { get; set; }
  public PlayerFieldType kind {get; set; }
  public bool main { get; set; }
}

class Player : Gee.HashMap<string, string> {}

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
    var old_size = get_n_items ();
    var new_size = players.size;

    _list.clear ();

    foreach (var player in players)
      _list.add (player);

    if (new_size > old_size)
      items_changed (old_size - 1, 0, new_size - old_size);
    else if (old_size > new_size)
      items_changed (new_size - 1, old_size - new_size, 0);
  }
}

}
