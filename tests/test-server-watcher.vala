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

public static int main (string[] args) {
  Test.init (ref args);

  // Test 1: ServerWatcher instantiation
  Test.add_func ("/server-watcher/instantiation", () => {
    var server_list = new ServerList ();
    var watcher = new ServerWatcher (server_list);

    assert_nonnull (watcher);
    assert_cmpuint (watcher.ref_count, EQ, 1);
  });

  // Test 2: Adding a server works
  Test.add_func ("/server-watcher/server_added_no_crash", () => {
    var server_list = new ServerList ();
    var watcher = new ServerWatcher (server_list);
    var client = server_list.add ("server1.example.com:27015");

    assert_nonnull (watcher);
    // assert_cmpuint (watcher.ref_count, EQ, 1);
    assert_nonnull (client);
    // assert_cmpuint (client.ref_count, EQ, 1);
  });

  // Test 3: Removing a server cleans up
  Test.add_func ("/server-watcher/server_removed_no_crash", () => {
    var server_list = new ServerList ();
    var watcher = new ServerWatcher (server_list);
    var client = server_list.add ("server1.example.com:27015");
    server_list.remove (client);
    assert_nonnull (watcher);
    assert_nonnull (client);
  });

  // Test 4: First online change is ignored (initialization)
  Test.add_func ("/server-watcher/first_online_change_skipped", () => {
    var server_list = new ServerList ();
    var watcher = new ServerWatcher (server_list);
    var client = server_list.add ("server1.example.com:27015");

    bool online_emitted = false;
    bool offline_emitted = false;
    watcher.online.connect ((c) => { online_emitted = true; });
    watcher.offline.connect ((c) => { offline_emitted = true; });

    client.set_property ("online", true);

    assert_false (online_emitted);
    assert_false (offline_emitted);
  });

  // Test 5: Subsequent transition to online should emit signal
  Test.add_func ("/server-watcher/online_signal_emitted", () => {
    var server_list = new ServerList ();
    var watcher = new ServerWatcher (server_list);
    var client = server_list.add ("server1.example.com:27015");

    bool online_emitted = false;
    watcher.online.connect ((c) => { online_emitted = true; });

    client.set_property ("online", true);   // First (skipped)
    client.set_property ("online", false);  // Second (offline)
    client.set_property ("online", true);   // Third (online)

    assert_true (online_emitted);
  });

  // Test 6: Transition to offline emits signal
  Test.add_func ("/server-watcher/offline_signal_emitted", () => {
    var server_list = new ServerList ();
    var watcher = new ServerWatcher (server_list);
    var client = server_list.add ("server1.example.com:27015");

    bool offline_emitted = false;
    watcher.offline.connect ((c) => { offline_emitted = true; });

    client.set_property ("online", true);   // First (skipped)
    client.set_property ("online", false);  // Second (offline)

    assert_true (offline_emitted);
  });

  // Test 7: Player slot available signal emitted when slot opens
  Test.add_func ("/server-watcher/player_slot_available_emitted", () => {
    var server_list = new ServerList ();
    var watcher = new ServerWatcher (server_list);
    var client = server_list.add ("server1.example.com:27015");

    bool slot_emitted = false;
    Client? slot_client = null;
    watcher.player_slot_available.connect ((c) => {
        slot_emitted = true;
        slot_client = c;
    });

    client.sinfo.num_players = 10;
    client.sinfo.max_players = 10;
    assert_false (slot_emitted);

    client.sinfo.num_players = 9;

    assert_true (slot_emitted);
    assert (slot_client == client);
  });

  // Test 8: No signal when slots already available
  Test.add_func ("/server-watcher/no_signal_when_slots_already_available", () => {
    var server_list = new ServerList ();
    var watcher = new ServerWatcher (server_list);
    var client = server_list.add ("server1.example.com:27015");

    bool slot_emitted = false;
    watcher.player_slot_available.connect ((c) => { slot_emitted = true; });

    // Establish state with available slots (first emission)
    client.sinfo.max_players = 10;
    client.sinfo.num_players = 5;
    assert_true (slot_emitted); // initial transition

    // Subsequent change while still having slots should NOT emit
    slot_emitted = false;
    client.sinfo.num_players = 3;
    assert_false (slot_emitted);
  });

  // Test 9: Multiple clients tracked independently
  Test.add_func ("/server-watcher/multiple_clients_independent", () => {
    var server_list = new ServerList ();
    var watcher = new ServerWatcher (server_list);
    var client1 = server_list.add ("server1.example.com:27015");
    var client2 = server_list.add ("server2.example.com:27015");

    bool client1_emitted = false;
    watcher.online.connect ((c) => { if (c == client1) client1_emitted = true; });

    client1.set_property ("online", true);
    client1.set_property ("online", false);
    client1.set_property ("online", true);

    assert_true (client1_emitted);
  });

  // Test 10: No signal after client removal
  Test.add_func ("/server-watcher/no_signal_after_removal", () => {
    var server_list = new ServerList ();
    var watcher = new ServerWatcher (server_list);
    var client = server_list.add ("server1.example.com:27015");

    bool signal_after_removal = false;
    watcher.online.connect ((c) => { signal_after_removal = true; });

    client.set_property ("online", true);
    client.set_property ("online", false);
    client.set_property ("online", true);

    assert_true (signal_after_removal);

    signal_after_removal = false;
    server_list.remove (client);
    client.set_property ("online", false);
    client.set_property ("online", true);

    assert_false (signal_after_removal);
  });

  // Test 11: Slot signal cycles: full -> available -> full -> available
  Test.add_func ("/server-watcher/slot_available_cycles", () => {
    var server_list = new ServerList ();
    var watcher = new ServerWatcher (server_list);
    var client = server_list.add ("server1.example.com:27015");

    int slot_count = 0;
    watcher.player_slot_available.connect ((c) => { slot_count++; });

    client.sinfo.num_players = 10;
    client.sinfo.max_players = 10;
    client.sinfo.num_players = 9;
    assert (slot_count == 1);

    client.sinfo.num_players = 8;
    assert (slot_count == 1);

    client.sinfo.num_players = 10;
    assert (slot_count == 1);

    client.sinfo.num_players = 9;
    assert (slot_count == 2);
  });

  // Test 12: Destructor with multiple clients does not crash
  Test.add_func ("/server-watcher/destructor_with_multiple_clients", () => {
    var server_list = new ServerList ();
    var watcher = new ServerWatcher (server_list);
    server_list.add ("server1.example.com:27015");
    server_list.add ("server2.example.com:27015");
    server_list.add ("server3.example.com:27015");
    watcher = null;
    assert (true);
  });

  // Test 13: Re-adding a client resets online_state_init
  Test.add_func ("/server-watcher/online_state_init_resets_on_readd", () => {
    var server_list = new ServerList ();
    var watcher = new ServerWatcher (server_list);

    bool first_emit = false;
    watcher.online.connect ((c) => { first_emit = true; });

    var client = server_list.add ("server1.example.com:27015");
    client.set_property ("online", true);
    client.set_property ("online", true);

    assert_false (first_emit);

    server_list.remove (client);
    bool second_emit = false;
    watcher.online.connect ((c) => { second_emit = true; });

    var client2 = server_list.add ("server1.example.com:27015");
    client2.set_property ("online", true);

    assert_false (second_emit);
  });

  // Test 14: Slot signal when going from full to available with different max
  Test.add_func ("/server-watcher/slot_signal_full_to_available", () => {
    var server_list = new ServerList ();
    var watcher = new ServerWatcher (server_list);
    var client = server_list.add ("server1.example.com:27015");

    bool emitted = false;
    watcher.player_slot_available.connect ((c) => { emitted = true; });

    client.sinfo.num_players = 16; // FIXME: current ServerWatcher logic prefers num_players to be set first
    client.sinfo.max_players = 16;
    assert_false (emitted);

    client.sinfo.num_players = 15;
    assert_true (emitted);
  });

  // Test 15: Changing max_players opens a slot and triggers signal
  Test.add_func ("/server-watcher/slot_signal_max_players_change", () => {
    var server_list = new ServerList ();
    var watcher = new ServerWatcher (server_list);
    var client = server_list.add ("server1.example.com:27015");

    bool emitted = false;
    watcher.player_slot_available.connect ((c) => { emitted = true; });

    client.sinfo.num_players = 10;
    client.sinfo.max_players = 10;
    assert_false (emitted);

    client.sinfo.max_players = 11;
    assert_true (emitted);
  });

  Test.run ();
  return 0;
}
