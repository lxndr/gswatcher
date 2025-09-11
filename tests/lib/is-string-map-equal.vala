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

bool is_string_map_equal (Gee.Map<string, string> map1, Gee.Map<string, string> map2) {
  if (map1.size != map2.size) {
    return false;
  }

  foreach (var key in map1.keys) {
    if (map1[key] != map2[key]) {
      return false;
    }
  }

  return true;
}
