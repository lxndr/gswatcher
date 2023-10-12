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

public errordomain ProtocolError {
  INVALID_RESPONSE,
  AUTH_FAILED,
  NOT_IMPLEMENTED,
}

public enum ProtocolFeature {
  QUERY,
  CONSOLE;

  public static ProtocolFeature parse_nick (string nick) {
    var enumc = (EnumClass) typeof (ProtocolFeature).class_ref ();
    var eval = enumc.get_value_by_nick (nick);

    return eval == null
      ? ProtocolFeature.QUERY
      : (ProtocolFeature) eval.value;
  }
}

public class ProtocolInfo {
  public ProtocolFeature feature;
  public string id;
  public string name;
  public string transport;
}

public interface Protocol : Object, Initable {
  public abstract ProtocolInfo info { get; protected set; }

  public signal void data_send (uint8[] data);

  public abstract void process_response (uint8[] data) throws Error;
}

}
