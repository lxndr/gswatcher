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
 * An observable list of game servers represented as `Client` objects.
 */
public class ServerList : Object, ListModel {
  private Gee.List<Client> _list = new Gee.ArrayList<Client> ();

  /**
   * A signal emitted when a client is added to the list.
   */
  public signal void added (Client client);

  /**
   * A signal emitted when a client is removed from the list.
   */
  public signal void removed (Client client);

  /**
   * Gets the client at the specified index.
   *
   * @param index The index of the client to retrieve.
   * @return The client at the specified index.
   */
  public new Client @get (int index) {
    return _list[index];
  }

  /**
   * Returns an iterator over the clients in the list.
   *
   * @return An iterator over the clients in the list.
   */
  public Gee.Iterator<Client> iterator () {
    return _list.iterator ();
  }

  /**
   * Adds a client with the specified address to the list.
   *
   * If a client with the same address already exists, it will be returned instead of adding a new one.
   * The `added` and `items_changed` signals will be emitted if a new client is added to the list.
   *
   * @param address The address of the client to add.
   * @return The added client.
   */
  public virtual Client add (string address) {
    var existing_client = find_by_address (address);

    if (existing_client != null) {
      return existing_client;
    }

    var client = new Client (new Server (address));
    _list.add (client);
    items_changed (_list.size - 1, 0, 1);
    added (client);
    return client;
  }

  /**
   * Removes the specified client from the list.
   *
   * If the client is found and removed, the `removed` and `items_changed` signals will be emitted.
   *
   * @param client The client to remove.
   */
  public virtual void remove (Client client) {
    var idx = _list.index_of (client);

    if (idx > -1) {
      _list.remove_at (idx);
      items_changed (idx, 1, 0);
      removed (client);
    }
  }

  /**
   * Finds a client in the list by its server address.
   *
   * @param address The server address to search for.
   * @return The client with the specified server address, or `null` if no such client exists.
   */
  public Client? find_by_address (string address) {
    return _list.first_match (client => client.server.address == address);
  }

  /**
   * Checks if a client with the specified server address exists in the list.
   *
   * @param address The server address to check for.
   * @return `true` if a client with the specified server address exists, `false` otherwise.
   */
  public bool exists (string address) {
    return find_by_address (address) != null;
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
