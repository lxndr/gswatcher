/*
  Game Server Watcher
  Copyright (C) 2025  Alexander Burobin

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

string format_player_list (Gee.List<Player> plist) {
  var b = new StringBuilder ("[");
  var first = true;

  foreach (var player in plist) {
    if (!first) {
      b.append (", ");
    }

    b.append (format_string_map (player));
    first = false;
  }

  b.append ("]");
  return b.str;
}
