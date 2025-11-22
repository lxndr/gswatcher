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
using GLib;

public class ServerInfo : Object {
  public string? server_name { get; set; }
  public string? server_name_markup { get; set; }
  public string? server_type { get; set; }
  public string? server_os { get; set; }
  public string? game_id { get; set; }
  public string? game_name { get; set; }
  public string? game_mode { get; set; }
  public string? game_version { get; set; }
  public string? map { get; set; }
  public int num_players { get; set; default = -1; }
  public int max_players { get; set; default = -1; }
  public bool @private { get; set; }
  public bool secure { get; set; }

  public string? server_name_clean {
    owned get {
      if (server_name_markup != null) {
        return strip_pango_markup (server_name_markup);
      }

      return server_name;
    }
  }
}

}
