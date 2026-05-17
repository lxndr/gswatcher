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

/**
 * An observable list of players.
 */
public class PlayerList : Object, ListModel {
  private Gee.List<Player> _list = new Gee.ArrayList<Player> ();

  /**
   * Gets the player at the specified index.
   *
   * @param index The index of the player to retrieve.
   * @return The player at the specified index.
   */
  public new Player @get (int index) {
    return _list[index];
  }

  /**
   * Returns an iterator over the players in the list.
   *
   * @return An iterator over the players in the list.
   */
  public Gee.Iterator<Player> iterator () {
    return _list.iterator ();
  }

  /**
   * Applies the given list of players to the current list, updating existing players and adding or removing players as necessary.
   *
   * @param players The list of players to apply to the current list.
   */
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

  /*
   * ListModel implementation
   */

  public Object? get_item (uint position) {
    if (position >= _list.size) {
      return null;
    }

    return _list[(int) position];
  }

  public Type get_item_type () {
    return _list.element_type;
  }

  public uint get_n_items () {
    return _list.size;
  }  
}

}
