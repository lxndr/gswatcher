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

using Gsw;
using Gee;

private class NullLocationResolver : Object, GeoLocationResolver {
  public string? country_code_by_ip (string ip_address) {
    return null;
  }

  public string? location_by_ip (string ip_address) {
    return null;
  }
}

ServerList make_server_list () {
  var protocol_registry = new ProtocolRegistry ();
  var transport_registry = new TransportRegistry ();
  var game_db = new GameDb ();
  game_db.finish_loading ();

  var querier_manager = new QuerierManager (protocol_registry, transport_registry, game_db);

  return new ServerList (querier_manager, protocol_registry, transport_registry, game_db, new NullLocationResolver ());
}

public static int main (string[] args) {
  Test.init (ref args);

  Test.add_func ("/core/ServerList/get()", () => {
    var list = make_server_list ();
    var client1 = list.add ("localhost:27015");
    var client2 = list.add ("localhost:27016");

    assert (list.get (0) == client1);
    assert (list.get (1) == client2);
  });

  Test.add_func ("/core/ServerList/iterator()", () => {
    var list = make_server_list ();
    var client1 = list.add ("localhost:27015");
    var client2 = list.add ("localhost:27016");

    var iterator = list.iterator ();
    assert_true (iterator.next ());
    assert (iterator.get () == client1);
    assert_true (iterator.next ());
    assert (iterator.get () == client2);
    assert_false (iterator.next ());
  });

  Test.add_func ("/core/ServerList/add()", () => {
    var list = make_server_list ();

    list.items_changed.connect ((pos, removed, added) => {
      assert_cmpuint (list.get_n_items (), EQ, 1);
      assert_cmpuint (pos, EQ, 0);
      assert_cmpuint (removed, EQ, 0);
      assert_cmpuint (added, EQ, 1);
    });

    Client? added_signal_client = null;
    list.added.connect ((client) => {
      assert_nonnull (client);
      added_signal_client = client;
    });

    list.removed.connect ((client) => {
      assert_not_reached ();
    });

    var client = list.add ("localhost:27015");

    assert_cmpuint (list.get_n_items (), EQ, 1);
    assert_nonnull (client);
    assert (list[0] == client);
    assert (added_signal_client == client);
  });

  Test.add_func ("/core/ServerList/add()/duplicate", () => {
    var list = make_server_list ();
    var client1 = list.add ("localhost:27015");

    list.items_changed.connect ((pos, removed, added) => {
      assert_not_reached ();
    });

    list.added.connect ((client) => {
      assert_not_reached ();
    });

    list.removed.connect ((client) => {
      assert_not_reached ();
    });

    var client2 = list.add ("localhost:27015");

    assert_cmpuint (list.get_n_items (), EQ, 1);
    assert_nonnull (client1);
    assert (client1 == client2);
  });

  Test.add_func ("/core/ServerList/remove()", () => {
    var list = make_server_list ();
    var client = list.add ("localhost:27015");

    list.items_changed.connect ((pos, removed, added) => {
      assert_cmpuint (list.get_n_items (), EQ, 0);
      assert_cmpuint (pos, EQ, 0);
      assert_cmpuint (removed, EQ, 1);
      assert_cmpuint (added, EQ, 0);
    });

    list.added.connect ((client) => {
      assert_not_reached ();
    });

    Client? removed_signal_client = null;
    list.removed.connect ((client) => {
      assert_nonnull (client);
      removed_signal_client = client;
    });

    list.remove (client);

    assert_cmpuint (list.get_n_items (), EQ, 0);
    assert_nonnull (removed_signal_client);
    assert (removed_signal_client == client);
  });

  Test.add_func ("/core/ServerList/remove()/not_found", () => {
    var list = make_server_list ();
    var client = new Client (new Server ("localhost:27015"), list.querier_manager, list.protocol_registry, list.transport_registry, list.game_db, list.location_resolver);

    list.items_changed.connect ((pos, removed, added) => {
      assert_not_reached ();
    });

    list.added.connect ((client) => {
      assert_not_reached ();
    });

    list.removed.connect ((client) => {
      assert_not_reached ();
    });

    list.remove (client);

    assert_cmpuint (list.get_n_items (), EQ, 0);
  });

  Test.add_func ("/core/ServerList/find_by_address()", () => {
    var list = make_server_list ();
    var client1 = list.add ("localhost:27015");
    var client2 = list.add ("localhost:27016");

    var found_client1 = list.find_by_address ("localhost:27015");
    var found_client2 = list.find_by_address ("localhost:27016");

    assert_nonnull (found_client1);
    assert_nonnull (found_client2);
    assert (found_client1 == client1);
    assert (found_client2 == client2);
  });

  Test.add_func ("/core/ServerList/find_by_address()/not_found", () => {
    var list = make_server_list ();
    list.add ("localhost:27015");

    var found_client = list.find_by_address ("localhost:27016");
    assert_null (found_client);
  });

  Test.add_func ("/core/ServerList/exists()", () => {
    var list = make_server_list ();
    list.add ("localhost:27015");

    assert_true (list.exists ("localhost:27015"));
  });

  Test.add_func ("/core/ServerList/exists()/not_found", () => {
    var list = make_server_list ();
    list.add ("localhost:27015");

    assert_false (list.exists ("localhost:27016"));
  });

  /* ListModel implementation tests */

  Test.add_func ("/core/ServerList/get_item_type()", () => {
    var list = make_server_list ();
    assert (list.get_item_type () == typeof (Client));
  });

  Test.add_func ("/core/ServerList/get_n_items()", () => {
    var list = make_server_list ();
    assert_cmpuint (list.get_n_items (), EQ, 0);

    list.add ("localhost:27015");
    assert_cmpuint (list.get_n_items (), EQ, 1);

    list.add ("localhost:27016");
    assert_cmpuint (list.get_n_items (), EQ, 2);
  });

  Test.add_func ("/core/ServerList/get_item()", () => {
    var list = make_server_list ();
    var client1 = list.add ("localhost:27015");
    var client2 = list.add ("localhost:27016");

    assert (list.get_item (0) == client1);
    assert (list.get_item (1) == client2);
    assert_null (list.get_item (2));
  });

  Test.add_func ("/core/ServerList/items_changed/addition", () => {
    var list = make_server_list ();

    list.items_changed.connect ((pos, removed, added) => {
      assert_cmpuint (list.get_n_items (), EQ, 1);
      assert_cmpuint (pos, EQ, 0);
      assert_cmpuint (removed, EQ, 0);
      assert_cmpuint (added, EQ, 1);
    });

    list.add ("localhost:27015");
  });

  Test.add_func ("/core/ServerList/items_changed/removal", () => {
    var list = make_server_list ();
    var client = list.add ("localhost:27015");

    list.items_changed.connect ((pos, removed, added) => {
      assert_cmpuint (list.get_n_items (), EQ, 0);
      assert_cmpuint (pos, EQ, 0);
      assert_cmpuint (removed, EQ, 1);
      assert_cmpuint (added, EQ, 0);
    });

    list.remove (client);
  });

  Test.run ();
  return 0;
}
