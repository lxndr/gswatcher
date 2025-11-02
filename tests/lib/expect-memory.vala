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
class ExpectMemory {
    private uint8[] actual;

    public ExpectMemory(uint8[] actual) {
        this.actual = actual;
    }

    public void to_equal(uint8[] expected) {
        if (actual.length != expected.length) {
            var s = new StringBuilder ("expected:");

            foreach (var b in expected) {
                s.append_printf (" %02X", b);
            }

            s.append (", actual:");

            foreach (var b in actual) {
                s.append_printf (" %02X", b);
            }

            Test.fail_printf (s.str);
        }
    }
}

ExpectMemory expect_memory(uint8[] actual) {
    return new ExpectMemory(actual);
}
