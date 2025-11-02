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

bool is_plist_fields_equal (Gee.List<PlayerField> fields1, Gee.List<PlayerField> fields2) {
  if (fields1.size != fields2.size) {
    return false;
  }

  for (var i = 0; i < fields1.size; i++) {
    if (fields1[i].title != fields2[i].title || fields1[i].field != fields2[i].field || fields1[i].kind != fields2[i].kind) {
      return false;
    }
  }

  return true;
}
