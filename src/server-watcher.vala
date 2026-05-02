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

[CCode (has_type_id = false)]
private enum ServerOnlineStatus {
  UNKNOWN,
  ONLINE,
  OFFLINE,
}

private struct ServerWatcherRecord {
  ulong online_change_handler_id;
  ulong num_players_handler_id;
  ulong max_players_handler_id;
  ServerOnlineStatus prev_online;
  bool prev_has_slots;
}

public class ServerWatcher : Object {
  public ServerList client_list { get; construct; }
  private Gee.Map<Client, ServerWatcherRecord?> server_records;

  public signal void online (Client client);
  public signal void offline (Client client);
  public signal void player_slot_available (Client client);

  construct {
    server_records = new Gee.HashMap<Client, ServerWatcherRecord?> ();

    client_list.items_changed.connect ((pos, removed, added) => {
      for (var idx = pos; idx < pos + removed; idx++) {
        on_server_removed (client_list[idx]);
      }

      for (var idx = pos; idx < pos + added; idx++) {
        on_server_added (client_list[idx]);
      }
    });
  }

  public ServerWatcher (ServerList client_list) {
    Object (client_list : client_list);
  }

  ~ServerWatcher () {
    clear_server_records ();
  }

  private void on_server_added (Client client) {
    var online_change_handler_id = client.notify["online"].connect (() => on_online_change (client));
    var num_players_handler_id = client.sinfo.notify["num-players"].connect (() => on_player_count_change (client));
    var max_players_handler_id = client.sinfo.notify["max-players"].connect (() => on_player_count_change (client));

    var record = ServerWatcherRecord () {
      online_change_handler_id = online_change_handler_id,
      num_players_handler_id = num_players_handler_id,
      max_players_handler_id = max_players_handler_id,
      prev_has_slots = false,
      prev_online = UNKNOWN,
    };

    server_records.set (client, record);
  }

  private void on_server_removed (Client client) {
    ServerWatcherRecord? record;
    server_records.unset (client, out record);
    disconnect_all_client_signals (client, record);
  }

  private void on_online_change (Client client) {
    var record = server_records.get (client);

    // first online status change is skipped

    if (client.online) {
      if (record.prev_online == OFFLINE) {
        online (client);
      }
    } else {
      if (record.prev_online == ONLINE) {
        offline (client);
      }
    }

    record.prev_online = client.online ? ServerOnlineStatus.ONLINE : ServerOnlineStatus.OFFLINE;
    server_records.set (client, record);
  }

  private void on_player_count_change (Client client) {
    var record = server_records.get (client);

    var has_slots = client.sinfo.num_players < client.sinfo.max_players;

    if (!record.prev_has_slots && has_slots) {
      player_slot_available (client);
    }

    record.prev_has_slots = has_slots;
    server_records.set (client, record);
  }

  private void clear_server_records () {
    foreach (var entry in server_records.entries) {
      var client = entry.key;
      var record = entry.value;
      disconnect_all_client_signals (client, record);
    }
  }
  
  private void disconnect_all_client_signals (Client client, ServerWatcherRecord record) {
    client.disconnect (record.online_change_handler_id);
    client.sinfo.disconnect (record.num_players_handler_id);
    client.sinfo.disconnect (record.max_players_handler_id);
  }
}

}
