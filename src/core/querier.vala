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

const uint TIMEOUT_MS = 5000;

public errordomain QuerierError {
  PROCESSING,
  SENDING,
  QUERYING,
  TIMEOUT,
}

public class Querier : Object {
  public Server server { get; construct; }
  public QueryProtocol protocol { get; construct; }
  public NetTransport transport { get; construct; }
  public int64 ping { get; protected set; default = -1; }
  public Error? error { get; protected set; }

  private GameResolver game_resolver = GameResolver.get_instance ();
  private bool query_pending;
  private int64 query_time;
  private uint timeout_id;
  private string? game_id;
  private bool plist_fields_resolved;

  public signal void address_resolve (string address);
  public signal void details_update (Gee.Map<string, string> details);
  public signal void sinfo_update (ServerInfo sinfo);
  public signal void plist_fields_update (Gee.List<PlayerField> plist_fields);
  public signal void plist_update (Gee.ArrayList<Player> plist);

  construct {
    notify["error"].connect(on_error);

    protocol.data_send.connect (send_data);
    protocol.sinfo_update.connect (on_sinfo_updated);
    protocol.plist_update.connect (on_plist_updated);

    transport = TransportRegistry.get_instance ().create_net_transport (protocol.info.transport, server.host, server.qport);
    transport.resolved.connect (on_address_resolved);
    transport.data_received.connect (on_data_received);

    game_resolver.notify["ready"].connect (handle_pending_query);

    query ();
  }

  public Querier (Server server, QueryProtocol protocol) {
    Object (
      server : server,
      protocol : protocol
    );
  }

  ~Querier () {
    stop_timeout_timer ();

    notify["error"].disconnect(on_error);
    protocol.data_send.disconnect (send_data);
    protocol.sinfo_update.disconnect (on_sinfo_updated);
    protocol.plist_update.disconnect (on_plist_updated);
    transport.resolved.disconnect (on_address_resolved);
    transport.data_received.disconnect (on_data_received);
    game_resolver.notify["ready"].disconnect (handle_pending_query);
  }

  public void query () {
    try {
      if (!game_resolver.ready) {
        query_pending = true;
        return;
      }

      start_timeout_timer ();
      start_ping_timer ();
      protocol.query ();
      query_pending = false;
    } catch (Error err) {
      error = new QuerierError.QUERYING ("failed to query %s:%d: %s", server.host, server.qport, err.message);
    }
  }

  private void send_data (uint8[] data) {
    transport.send (data);
    log (Config.LOG_DOMAIN, LEVEL_DEBUG, "sent data to %s:%u: length = %ld", server.host, server.qport, data.length);
  }

  private void handle_pending_query () {
    if (query_pending && game_resolver.ready)
      query ();
  }

  private void on_address_resolved (string address) {
    address_resolve (address);
  }

  private void on_data_received (uint8[] data) {
    try {
      stop_ping_timer ();
      protocol.process_response (data);
      error = null;
    } catch (Error err) {
      if (err is ProtocolError && err.code == ProtocolError.INVALID_RESPONSE) {
        return;
      }

      error = new QuerierError.PROCESSING ("failed to process data from %s:%u: %s", server.host, server.qport, err.message);
    }
  }

  private void on_sinfo_updated (Gee.Map<string, string> details, ServerInfo sinfo) {
    stop_timeout_timer ();
    game_resolver.resolve (protocol.info.id, sinfo, details);

    if (game_id == null)
      game_id = sinfo.game_id;

    details_update (details);
    sinfo_update (sinfo);
  }

  private void on_plist_updated (Gee.List<PlayerField> default_pfields, Gee.ArrayList<Player> plist) {
    if (!plist_fields_resolved) {
      var game_pfields = game_resolver.get_plist_fields (game_id);
      var new_pfields = (game_pfields != null && game_pfields.size > 0) ? game_pfields : default_pfields;
      plist_fields_update (new_pfields);
      plist_fields_resolved = true;
    }

    plist_update (plist);
  }

  private void on_error () {
    if (error != null) {
      if (error.code != QuerierError.TIMEOUT)
        log (Config.LOG_DOMAIN, LEVEL_DEBUG, error.message);
      stop_timeout_timer ();
      stop_ping_timer ();
      ping = -1;
      query_pending = false;
    }
  }

  private void start_ping_timer () {
    query_time = get_monotonic_time ();
  }

  private void stop_ping_timer () {
    if (query_time > 0) {
      ping = (get_monotonic_time () - query_time) / 1000;
      query_time = 0;
    }
  }

  private void start_timeout_timer () {
    var self = this; // `this` does not get passed on Ubuntu 22.04
    stop_timeout_timer ();

    timeout_id = Timeout.add (TIMEOUT_MS, () => {
      self.error = new QuerierError.TIMEOUT(
        "failed to query %s:%d: %s",
        self.server.host,
        self.server.qport, "failed to receive a response in reasonable amount of time"
      );
      self.timeout_id = 0;
      return Source.REMOVE;
    });
  }

  private void stop_timeout_timer () {
    if (timeout_id > 0) {
      Source.remove (timeout_id);
      timeout_id = 0;
    }
  }
}

}
