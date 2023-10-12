/*
  Game Server Watcher
  Copyright (C) 2023  Alexander Burobin

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

public class Server : Object {
  public string host;
  public uint16 gport;
  public uint16 qport;
  public uint16 console_port { get; set; default = 0; }
  public string console_password { get; set; default = ""; }

  public Server (string address) {
    if (parse_address (address, out host, out gport, out qport))
      console_port = gport;
  }

  public string address {
    owned get {
      var res = @"$(host):$(gport)";

      if (gport != qport)
        res += @":$(qport)";

      return res;
    }
  }
}

}
