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

public class HashableStringMap : Gee.HashMap<string, string>, Gee.Hashable<Gee.HashMap<string, string>> {
  private uint _hash = 0;
  private bool _hash_valid = false;

  public bool equal_to (Gee.HashMap<string, string> another_map) {
    if (another_map == null) {
      return false;
    }

    if (this.size != another_map.size) {
      return false;
    }

    foreach (var key in this.keys) {
      if (!another_map.has_key(key) || this.get(key) != another_map.get(key)) {
        return false;
      }
    }

    return true;
  }

  public uint hash () {
    if (!_hash_valid) {
      _hash = (uint) hash_string_map (this);
      _hash_valid = true;
    }

    return _hash;
  }

  public override void clear() {
    base.clear();
    invalidate_hash();
  }

  public override void set(string key, string value) {
    base.set(key, value);
    invalidate_hash();
  }

  public override bool unset(string key, out string value = null) {
    bool result = base.unset(key, out value);

    if (result) {
      invalidate_hash();
    }

    return result;
  }

  private void invalidate_hash() {
    _hash_valid = false;
  }
}

}
