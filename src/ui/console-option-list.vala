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

using Gtk;

namespace Gsw.Ui {

[GtkTemplate (ui = "/io/github/lxndr/gswatcher/ui/console-option-list.ui")]
class ConsoleOptionList : Widget {
  private Server? _server;

  [GtkChild]
  private unowned Adjustment port_entry;

  [GtkChild]
  private unowned Entry password_entry;

  class construct {
    set_layout_manager_type (typeof (BinLayout));
  }

  protected override void dispose () {
    get_first_child ().unparent ();
    base.dispose ();
  }

  public Server server {
    get {
      return _server;
    }

    set {
      port_entry.value_changed.disconnect (port_changed);
      password_entry.changed.disconnect (password_changed);

      _server = value;

      if (_server != null) {
        port_entry.value = _server.console_port;
        port_entry.value_changed.connect (port_changed);

        password_entry.text = _server.console_password;
        password_entry.changed.connect (password_changed);
      }
    }
  }

  private void port_changed (Adjustment entry) {
    server.console_port = (uint16) entry.value;
  }

  private void password_changed (Editable entry) {
    server.console_password = entry.text;
  }
}

}
