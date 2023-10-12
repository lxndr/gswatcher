/*
  Game Server Watcher
  Copyright (C) 2023  Alexander Burobin

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

class BuddyWatcher : Object {
  public ServerList client_list { get; construct; }
  public BuddyList buddy_list { get; construct; }
  public uint update_interval { get; construct; default = 5000; }
  private uint update_timer;

  public signal void online (Buddy buddy, Client client);

  construct {
    buddy_list.items_changed.connect ((pos, removed, added) => {
      for (var idx = pos; idx < pos + added; idx++)
        on_buddy_added (buddy_list[pos]);
    });

    update_timer = Timeout.add (update_interval, on_timer_tick);
  }

  public BuddyWatcher (ServerList client_list, BuddyList buddy_list) {
    Object (client_list : client_list, buddy_list : buddy_list);
  }

  ~BuddyWatcher () {
    Source.remove (update_timer);
  }

  private void on_buddy_added (Buddy buddy) {
    foreach (var client in client_list) {
      foreach (var player in client.plist) {
        if (player.has_key ("name") && player["name"] == buddy.name) {
          on_maybe_new_online (buddy, client);
        }
      }
    }
  }

  private bool on_timer_tick () {
    foreach (var client in client_list) {
      foreach (var player in client.plist) {
        var player_name = get_player_name (player);

        if (player_name == null)
          continue;

        foreach (var buddy in buddy_list) {
          if (buddy.name == player_name) {
            on_maybe_new_online (buddy, client);
          }
        }
      }
    }

    return Source.CONTINUE;
  }

  private void on_maybe_new_online (Buddy buddy, Client client) {
    var expires_at = buddy.lastseen == null
      ? new DateTime.now_local ()
      : buddy.lastseen.add (TimeSpan.MILLISECOND * update_interval * 2);

    buddy.lastaddr = client.server.address;
    buddy.lastseen = new DateTime.now_local ();

    if (buddy.lastseen.compare (expires_at) >= 0)
      online (buddy, client);
  }

  private string? get_player_name (Player player) {
    if (player.has_key ("name"))
      return player["name"];

    if (player.has_key ("player"))
      return player["player"];

    return null;
  }
}

}
