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
 * An observable list of buddies.
 */
public class BuddyList : Object, ListModel {
  private Gee.List<Buddy> _list = new Gee.ArrayList<Buddy> ();

  /**
   * A signal emitted when a buddy is added to the list.
   */
  public signal void added (Buddy buddy);

  /**
   * A signal emitted when a buddy is removed from the list.
   */
  public signal void removed (Buddy buddy);

  /**
   * Gets the buddy at the specified index.
   *
   * @param index The index of the buddy to retrieve.
   * @return The buddy at the specified index.
   */
  public new Buddy @get (int index) {
    return _list[index];
  }

  /**
   * Returns an iterator over the buddies in the list.
   *
   * @return An iterator over the buddies in the list.
   */
  public Gee.Iterator<Buddy> iterator () {
    return _list.iterator ();
  }

  /**
   * Adds a buddy with the specified name to the list.
   *
   * If a buddy with the same name already exists, it will be returned instead of adding a new one.
   * The `added` and `items_changed` signals will be emitted if a new buddy is added to the list.
   *
   * @param name The name of the buddy to add.
   * @return The added buddy.
   */
  public virtual Buddy add (string name) {
    var existing_buddy = find_by_name (name);

    if (existing_buddy != null) {
      return existing_buddy;
    }

    var buddy = new Buddy (name);
    _list.add (buddy);
    items_changed (_list.size - 1, 0, 1);
    added (buddy);
    return buddy;
  }

  /**
   * Removes the specified buddy from the list.
   *
   * If the buddy is found and removed, the `removed` and `items_changed` signals will be emitted.
   *
   * @param buddy The buddy to remove.
   */
  public virtual void remove (Buddy buddy) {
    var idx = _list.index_of (buddy);

    if (idx > -1) {
      _list.remove_at (idx);
      items_changed (idx, 1, 0);
      removed (buddy);
    }
  }

  /**
   * Finds a buddy in the list by their name.
   *
   * @param name The name to search for.
   * @return The buddy with the specified name, or `null` if no such buddy exists.
   */
  public Buddy? find_by_name (string name) {
    return _list.first_match (item => item.name == name);
  }

  /**
   * Checks if a buddy with the specified name exists in the list.
   *
   * @param name The name to check for.
   * @return `true` if a buddy with the specified name exists, `false` otherwise.
   */
  public bool exists (string name) {
    return find_by_name (name) != null;
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
