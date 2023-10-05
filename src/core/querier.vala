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
  private uint timeout_source;
  private bool plist_fields_resolved;

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
    details_update (details);
    sinfo_update (sinfo);
  }

  private void on_plist_updated (Gee.List<PlayerField> default_pfields, Gee.ArrayList<Player> plist) {
    if (!plist_fields_resolved) {
      var game_pfields = game_resolver.get_plist_fields (server.game_id);
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
    stop_timeout_timer ();

    timeout_source = Timeout.add_once (TIMEOUT_MS, () => {
      error = new QuerierError.TIMEOUT("failed to query %s:%d: %s", server.host, server.qport, "failed to receive a response in reasonable amount of time");
      timeout_source = 0;
    });
  }

  private void stop_timeout_timer () {
    if (timeout_source > 0) {
      Source.remove (timeout_source);
      timeout_source = 0;
    }
  }
}

}
