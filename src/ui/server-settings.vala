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

using Gtk;

namespace Gsw.Ui {

[GtkTemplate (ui = "/io/github/lxndr/gswatcher/ui/server-settings.ui")]
class ServerSettings : Widget {
  private Client? _client;
  private Gee.List<Binding> bindings = new Gee.ArrayList<Binding> ();

  [GtkChild]
  private unowned SpinButton console_port_spin;

  [GtkChild]
  private unowned Entry console_password_entry;

  [GtkChild]
  private unowned Switch notify_server_online_switch;

  [GtkChild]
  private unowned Switch notify_server_offline_switch;

  [GtkChild]
  private unowned Switch notify_player_slot_available_switch;

  class construct {
    set_layout_manager_type (typeof (BinLayout));
  }

  protected override void dispose () {
    get_first_child ().unparent ();
    base.dispose ();
  }

  public Client? client {
    get {
      return _client;
    }

    set {
      if (_client != null) {
        foreach (var binding in bindings) {
          binding.unbind ();
        }

        bindings.clear ();
      }

      _client = value;

      if (_client != null) {
        bindings.add_all_array ({
          _client.bind_property ("console-port", console_port_spin, "value", BIDIRECTIONAL | SYNC_CREATE),
          _client.bind_property ("console-password", console_password_entry, "text", BIDIRECTIONAL | SYNC_CREATE),
          _client.bind_property ("notify-server-online", notify_server_online_switch, "active", BIDIRECTIONAL | SYNC_CREATE),
          _client.bind_property ("notify-server-offline", notify_server_offline_switch, "active", BIDIRECTIONAL | SYNC_CREATE),
          _client.bind_property ("notify-player-slot-available", notify_player_slot_available_switch, "active", BIDIRECTIONAL | SYNC_CREATE),
        });
      }
    }
  }
}

}
