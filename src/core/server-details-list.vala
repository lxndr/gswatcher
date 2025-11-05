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
  private Gee.List<ServerDetailsEntry> list = new Gee.ArrayList<ServerDetailsEntry> ();

  public Object? get_item (uint position) {
    return list.get ((int) position);
  }

  public Type get_item_type () {
    return typeof (ServerDetailsEntry);
  }

  public uint get_n_items () {
    return (uint) list.size;
  }

  public void apply (Gee.Map<string, string> map) {
    for (var i = list.size - 1; i >= 0; i--) {
      var key = list[i].key;

      if (!map.has_key (key)) {
        list.remove_at (i);
        items_changed (i, 1, 0);
      }
    }

    foreach (var item in map) {
      var detail = list.first_match (it => it.key == item.key);

      if (detail == null) {
        list.add (new ServerDetailsEntry (item.key, item.value));
        items_changed (list.size, 0, 1);
      } else {
        detail.value = item.value;
      }
    }
  }
}

}
