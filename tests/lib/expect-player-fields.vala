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

[Compact (opaque = true)]
class ExpectPlayerFields {
  private string msg;
  private Gee.List<PlayerField> actual;

  public ExpectPlayerFields (string msg, Gee.List<PlayerField> actual) {
    this.msg = msg;
    this.actual = actual;
  }

  public void to_equal (Gee.List<PlayerField> expected) {
    if (is_plist_fields_equal (actual, expected)) {
      return;
    }

    Test.fail_printf (
      "%s: expected = %s, actual = %s",
      msg, format_plist_fields (expected), format_plist_fields (actual)
    );
  }
}

ExpectPlayerFields expect_player_fields (string msg, Gee.List<PlayerField> actual) {
  return new ExpectPlayerFields (msg, actual);
}
