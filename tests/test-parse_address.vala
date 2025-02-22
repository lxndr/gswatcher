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

struct CorrectTableItem {
  unowned string name;
  unowned string addr;
  unowned string host;
  uint16 gport;
  uint16 qport;
}

struct IncorrectTableItem {
  unowned string name;
  unowned string addr;
}

public static int main (string[] args) {
  Test.init (ref args);

  CorrectTableItem[] CORRECT_TABLE = {
    { name: "correct", addr: "awsm.tk:27011", host: "awsm.tk", gport: 27011, qport: 27011 },
    { name: "correct_with_qport", addr: "awsm.tk:27011:30000", host: "awsm.tk", gport: 27011, qport: 30000 },
  };

  foreach (unowned var row in CORRECT_TABLE) {
    Test.add_data_func (@"/core/utils/parse_address/$(row.name)", () => {
      string host;
      uint16 gport;
      uint16 qport;

      assert_true (parse_address (row.addr, out host, out gport, out qport));
      assert_cmpstr (host, EQ, row.host);
      assert_cmpuint (gport, EQ, row.gport);
      assert_cmpuint (qport, EQ, row.qport);
    });
  }

  IncorrectTableItem[] INCORRECT_TABLE = {
    { name: "incorrect", addr: "awsm.tk" },
  };

  foreach (unowned var row in INCORRECT_TABLE) {
    Test.add_data_func (@"/core/utils/parse_address/$(row.name)", () => {
      string host;
      uint16 gport;
      uint16 qport;

      assert_false (parse_address (row.addr, out host, out gport, out qport));
    });
  }

  Test.run ();
  return 0;
}
