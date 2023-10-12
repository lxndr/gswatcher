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

[GtkTemplate (ui = "/io/github/lxndr/gswatcher/ui/server-details.ui")]
class ServerDetails : Widget {
  public Client? client { get; set; }

  class construct {
    typeof (ServerDetailsEntry).ensure ();

    set_layout_manager_type (typeof (BinLayout));
  }

  protected override void dispose () {
    get_first_child ().unparent ();
    base.dispose ();
  }
}

}
