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

using Gtk;

namespace Gsw.Ui {

class PlayerListSorter : Sorter {
  private PlayerField field;

  public PlayerListSorter (PlayerField player_field) {
    this.field = player_field;
  }

  public override Ordering compare (Object? item1, Object? item2) {
    if (item1 == null || item2 == null) {
      return Ordering.EQUAL;
    }

    var player1 = (Player) item1;
    var player2 = (Player) item2;

    var val1 = player1.get (field.field);
    var val2 = player2.get (field.field);

    if (val1 == val2) {
      return Ordering.EQUAL;
    }

    if (val1 == null) {
      return Ordering.LARGER;
    }

    if (val2 == null) {
      return Ordering.SMALLER;
    }

    switch (field.kind) {
      case PlayerFieldType.NUMBER:
      case PlayerFieldType.DURATION:
        double num1 = 0, num2 = 0;
        double.try_parse (val1, out num1, null);
        double.try_parse (val2, out num2, null);
        
        if (num1 < num2) {
          return Ordering.SMALLER;
        } else if (num1 > num2) {
          return Ordering.LARGER;
        }
        break;

      default:
        // String comparison (case-insensitive)
        var cmp = val1.collate (val2);
        return cmp < 0 ? Ordering.SMALLER : (cmp > 0 ? Ordering.LARGER : Ordering.EQUAL);
    }

    return Ordering.EQUAL;
  }
}

}
