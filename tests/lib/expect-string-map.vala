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

[Compact (opaque = true)]
class ExpectStringMap {
    private string msg;
    private Gee.Map<string, string> actual;

    public ExpectStringMap (string msg, Gee.Map<string, string> actual) {
        this.msg = msg;
        this.actual = actual;
    }

    public void to_equal (Gee.Map<string, string> expected) {
        if (is_string_map_equal (actual, expected)) {
            return;
        }

        Test.fail_printf (
            "%s: expected = %s, actual = %s",
            msg, format_string_map (expected), format_string_map (actual)
        );
    }
}

ExpectStringMap expect_string_map (string msg, Gee.Map<string, string> actual) {
    return new ExpectStringMap (msg, actual);
}
