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

public enum PlayerFieldType {
  STRING,
  NUMBER,
  DURATION;

  public static PlayerFieldType parse_nick (string nick) {
    var enumc = (EnumClass) typeof (PlayerFieldType).class_ref ();
    var eval = enumc.get_value_by_nick (nick);

    return eval == null
      ? PlayerFieldType.STRING
      : (PlayerFieldType) eval.value;
  }
}

public class PlayerField : Object {
  public string title { get; set; }
  public string field { get; set; }
  public PlayerFieldType kind { get; set; default = STRING; }
  public bool main { get; set; default = false; }
}

public abstract interface QueryProtocol : Protocol {
  public signal void sinfo_update (ServerDetails details, ServerInfo sinfo);
  public signal void plist_update (Gee.List<PlayerField> plist_fields, Gee.ArrayList<Player> plist);

  public abstract void query () throws Error;
}

}
