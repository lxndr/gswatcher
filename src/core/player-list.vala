/*
  Game Server Watcher
  Copyright (C) 2010-2026 Alexander Burobin

  This program is free software: you can redistribute it and/or modify
  it under the terms of the GNU Affero General Public License as
  published by the Free Software Foundation, either version 3 of the
  License, or (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU Affero General Public License for more details.

  You should have received a copy of the GNU Affero General Public License
  along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

namespace Gsw {

public class Player : Gee.HashMap<string, string> {
  public signal void change ();

  public override void @set (string key, string value) {
    if (value != @get (key)) {
      base.set (key, value);
    }
  }
}

public class PlayerList : Object, ListModel {
  private Gee.List<Player> _list = new Gee.ArrayList<Player> ();

  public Object? get_item (uint position) {
    return _list.get ((int) position);
  }

  public Type get_item_type () {
    return typeof (Player);
  }

  public uint get_n_items () {
    return _list.size;
  }

  public uint size {
    get {
      return get_n_items ();
    }
  }

  public new Player get (uint index) {
    return (Player) get_item (index);
  }

  public void apply (Gee.List<Player> players) {
    var old_size = _list.size;
    var new_size = players.size;

    if (old_size > new_size) {
      while (_list.size > new_size)
        _list.remove_at (new_size);

      items_changed (new_size, old_size - new_size, 0);
    }

    for (var idx = 0; idx < _list.size; idx++) {
      _list[idx].set_all (players[idx]);
      _list[idx].change ();
    }

    if (new_size > old_size) {
      for (var idx = old_size; idx < new_size; idx++)
        _list.add (players[idx]);

      items_changed (old_size, 0, new_size - old_size);
    }
  }
}

}
