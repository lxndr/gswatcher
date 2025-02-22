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

namespace Gsw {

public class DummyLuaProtocol : LuaProtocol {
  public DummyLuaProtocol (string script_path) throws Error {
    Object (script_path : script_path);
    init ();
  }

  public override void process_response (uint8[] data) throws Error {
    throw new ProtocolError.NOT_IMPLEMENTED ("not implemented");
  }
}

}
