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

public delegate NetTransport NetTransportFactory (string host, uint16 port);

public class TransportDesc {
  public string id;
  public NetTransportFactory factory;

  public TransportDesc (string id, owned NetTransportFactory factory) {
    this.id = id;
    this.factory = (owned) factory;
  }
}

public class TransportRegistry : Object {
  private Gee.Map<string, TransportDesc> transports = new Gee.HashMap<string, TransportDesc> ();

  public Gee.Collection<TransportDesc> list () {
    return transports.values.read_only_view;
  }

  public void register (TransportDesc desc) {
    transports.set (desc.id, desc);
  }

  public NetTransport? create_net_transport (string id, string host, uint16 port) {
    if (!transports.has_key (id)) {
      warning ("unknown transport '%s'", id);
      return null;
    }

    return transports[id].factory (host, port);
  }
}

}
