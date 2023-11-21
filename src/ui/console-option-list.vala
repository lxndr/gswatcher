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
class ConsoleOptionList : Popover {
  public Client? client { get; set; }

  [GtkChild]
  private unowned Adjustment port_entry;

  [GtkChild]
  private unowned Entry password_entry;

  public signal void values_changed (uint16 port, string password);

  [GtkCallback]
  private void on_password_activated () {
    popdown ();
  }

  [GtkCallback]
  private void on_closed () {
    values_changed ((uint16) port_entry.value, password_entry.text);
  }
}

}
