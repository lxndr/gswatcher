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

public class ServerDetailsEntry : Object {
  public string key { get; construct; }
  public string value { get; set; }

  public ServerDetailsEntry (string key, string value) {
    Object (key : key, value : value);
  }
}

public class ServerDetailsList : Object, ListModel {
  private Gee.List<ServerDetailsEntry> _list = new Gee.ArrayList<ServerDetailsEntry> ();

  /**
   * Applies the given map of server details to the current list.
   *
   * Existing entries will be updated, new entries will be added, and entries that are no longer present in the map will be removed.
   *
   * @param map The map of server details to apply.
   */
  public void apply (Gee.Map<string, string> map) {
    for (var i = _list.size - 1; i >= 0; i--) {
      var key = _list[i].key;

      if (!map.has_key (key)) {
        _list.remove_at (i);
        items_changed (i, 1, 0);
      }
    }

    foreach (var item in map) {
      var detail = _list.first_match (it => it.key == item.key);

      if (detail == null) {
        _list.add (new ServerDetailsEntry (item.key, item.value));
        items_changed (_list.size - 1, 0, 1);
      } else {
        detail.value = item.value;
      }
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
