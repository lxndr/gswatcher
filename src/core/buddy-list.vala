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

public class BuddyList : Object, ListModel {
  private Gee.List<Buddy> list = new Gee.ArrayList<Buddy> ();

  public Object? get_item (uint position) {
    return list[(int) position];
  }

  public Type get_item_type () {
    return typeof (Buddy);
  }

  public uint get_n_items () {
    return list.size;
  }

  public uint size {
    get {
      return get_n_items ();
    }
  }

  public new Buddy get (uint index) {
    return (Buddy) get_item (index);
  }

  public virtual Buddy add (string name) {
    var buddy = find_by_name (name);

    if (buddy != null)
      return buddy;

    buddy = new Buddy (name);
    list.add (buddy);
    items_changed (list.size - 1, 0, 1);
    return buddy;
  }

  public virtual void remove (Buddy buddy) {
    var idx = list.index_of (buddy);

    if (idx > -1) {
      list.remove_at (idx);
      items_changed (idx, 1, 0);
    }
  }

  public Buddy? find_by_name (string name) {
    return list.first_match (item => item.name == name);
  }

  public bool exists (string name) {
    return find_by_name (name) != null;
  }
}

}
