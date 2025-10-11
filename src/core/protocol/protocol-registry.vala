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

public class ProtocolDesc : ProtocolInfo {
  public Type class_type;
  public string[] class_params;
  public Value[] class_values;
}

[SingleInstance]
public class ProtocolRegistry : Object {
  private Gee.Map<string, ProtocolDesc> protocols = new Gee.HashMap<string, ProtocolDesc> ();

  public static ProtocolRegistry get_instance () {
    return new ProtocolRegistry ();
  }

  public Gee.Collection<ProtocolDesc> list () {
    return protocols.values.read_only_view;
  }

  public Gee.Collection<ProtocolDesc> list_by_feature (ProtocolFeature feature) {
    var iter = protocols.values.filter ((protocol) => protocol.feature == feature);
    var arr = new Gee.ArrayList<ProtocolDesc> ();
    arr.add_all_iterator (iter);
    return arr.read_only_view;
  }

  public void register (ProtocolDesc desc)
    requires (desc.id != null && desc.id != "")
    requires (desc.name != null && desc.name != "")
    requires (desc.transport != null && desc.transport != "")
    requires (desc.class_type.is_a (typeof (Protocol)))
  {
    protocols.set (desc.id, desc);
  }

  public Protocol? create (string id) throws Error {
    if (!protocols.has_key (id)) {
      log (Config.LOG_DOMAIN, LEVEL_ERROR, "unknown protocol '%s'", id);
      return null;
    }

    var desc = protocols[id];
    var protocol = (Protocol) Object.new_with_properties (desc.class_type, desc.class_params, desc.class_values);
    protocol.init ();
    return protocol;
  }
}

}
