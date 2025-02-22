/*
  Game Server Watcher
  Copyright (C) 2025  Alexander Burobin

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

public class ServerList : Object, ListModel {
  private Gee.List<Client> list = new Gee.ArrayList<Client> ();

  public Object? get_item (uint position) {
    return list[(int) position];
  }

  public Type get_item_type () {
    return typeof (Client);
  }

  public uint get_n_items () {
    return list.size;
  }

  public uint size {
    get {
      return get_n_items ();
    }
  }

  public new Client get (uint index) {
    return (Client) get_item (index);
  }

  public virtual Client add (string address) {
    var client = new Client (new Server (address));
    list.add (client);
    items_changed (list.size - 1, 0, 1);
    return client;
  }

  public virtual void remove (Client client) {
    var idx = list.index_of (client);

    if (idx > -1) {
      list.remove_at (idx);
      items_changed (idx, 1, 0);
    }
  }

  public Client? find_by_name (string address) {
    return list.first_match (client => client.server.address == address);
  }

  public bool exists (string address) {
    return find_by_name (address) != null;
  }
}

}
