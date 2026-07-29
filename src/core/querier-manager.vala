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

using Gee;

namespace Gsw {

public class QuerierManager : Object {
  public uint update_interval { get; set; default = 5000; }
  public bool paused { get; set; }
  public GameDb game_db { get; construct; }
  public ProtocolRegistry protocol_registry { get; construct; }
  public TransportRegistry transport_registry { get; construct; }

  private uint timer_source;
  private Collection<unowned Querier> queriers;

  public QuerierManager (ProtocolRegistry protocol_registry, TransportRegistry transport_registry, GameDb game_db) {
    Object (
      protocol_registry : protocol_registry,
      transport_registry : transport_registry,
      game_db : game_db
    );
  }

  construct {
    queriers = new ConcurrentList<unowned Querier> ();

    notify["update_interval"].connect (update_timer);
    notify["paused"].connect (update_timer);
    update_timer ();
  }

  public Querier create_querier (Server server, string protocol_id) throws Error {
    var protocol = protocol_registry.create (protocol_id);
    var transport = transport_registry.create_net_transport (protocol.info.transport, server.host, server.qport);

    var querier = new Querier (server, (QueryProtocol) protocol, transport, game_db);
    querier.weak_ref ((WeakNotify) remove_querier);
    queriers.add (querier);

    update_timer ();
    return querier;
  }

  private void remove_querier (Querier querier) {
    queriers.remove (querier);
    update_timer ();
  }

  private void update_timer () {
    if (timer_source != 0) {
      Source.remove (timer_source);
      timer_source = 0;
      log (Config.LOG_DOMAIN, LEVEL_DEBUG, "update timer stopped");
    }

    if (!paused && queriers.size > 0) {
      timer_source = Timeout.add (update_interval, () => {
        update_tick ();
        return Source.CONTINUE;
      });

      log (Config.LOG_DOMAIN, LEVEL_DEBUG, "update timer started with interval %u ms", update_interval);
    }
  }

  private void update_tick () {
    foreach (var querier in queriers) {
      log (Config.LOG_DOMAIN, LEVEL_DEBUG, "querying %s:%d", querier.server.host, querier.server.qport);
      querier.query ();
    }
  }
}

}
