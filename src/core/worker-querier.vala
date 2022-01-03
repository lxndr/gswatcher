namespace Gsw {

const uint TIMEOUT_MS = 2000;

public errordomain QuerierError {
  PROCESSING,
  SENDING,
  QUERYING,
  TIMEOUT,
}

public class WorkerQuerier : Querier {
  public QueryProtocol protocol { get; construct; }
  public NetTransport transport { get; construct; }

  private GameResolver game_resolver = GameResolver.get_instance ();
  private bool query_pending;
  private int64 query_time;
  private uint timeout_source;

  construct {
    notify["error"].connect(on_error);

    protocol.data_send.connect (send_data);
    protocol.details_update.connect (on_details_updated);
    protocol.sinfo_update.connect (on_sinfo_updated);
    protocol.plist_update.connect (on_plist_updated);

    transport = TransportRegistry.get_instance ().create_net_transport (protocol.info.transport, server.host, server.qport);
    transport.receive.connect (on_data_received);
    transport.notify["ready"].connect (handle_pending_query);

    game_resolver.notify["ready"].connect (handle_pending_query);
  }

  public WorkerQuerier (Server server, QueryProtocol protocol) {
    Object (
      server : server,
      protocol : protocol
    );
  }

  ~WorkerQuerier () {
    stop_timeout_timer ();

    notify["error"].disconnect(on_error);
    protocol.data_send.disconnect (send_data);
    protocol.details_update.disconnect (on_details_updated);
    protocol.sinfo_update.disconnect (on_sinfo_updated);
    protocol.plist_update.disconnect (on_plist_updated);
    transport.receive.disconnect (on_data_received);
    transport.notify["ready"].disconnect (handle_pending_query);
    game_resolver.notify["ready"].disconnect (handle_pending_query);
  }

  public override void query () {
    try {
      if (!(transport.ready && game_resolver.ready)) {
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

  private void resolve_game (Gee.Map<string, string> new_details) {
    var game_resolver = GameResolver.get_instance ();
    game_resolver.resolve (protocol.info.id, sinfo, new_details);

    server.protocol = protocol.info.id;
    server.game_id = sinfo.game_id;

    if (sinfo.game_id != null && plist_fields.get_n_items () == 0)
      foreach (var field in game_resolver.get_player_fields (sinfo.game_id))
        plist_fields.append (field);
  }

  private void send_data (uint8[] data) {
    try {
      transport.send (data);
      log (Config.LOG_DOMAIN, LEVEL_DEBUG, "sent data to %s:%u: length = %ld", server.host, server.qport, data.length);
    } catch (Error err) {
      error = new QuerierError.SENDING ("failed to send data to %s:%u: %s", server.host, server.qport, err.message);
    }
  }

  private void handle_pending_query () {
    if (query_pending && transport.ready && game_resolver.ready)
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

  private void on_details_updated (Gee.Map<string, string> new_details) {
    stop_timeout_timer ();

    if (sinfo.game_id == null)
      resolve_game (new_details);

    details.apply (new_details);
    update ();
  }

  private void on_sinfo_updated (ServerInfo new_sinfo) {
    server.server_name = new_sinfo.server_name;

    if (new_sinfo.game_id != null)
      sinfo.game_id = new_sinfo.game_id;

    if (new_sinfo.game_name != null)
      sinfo.game_name = new_sinfo.game_name;

    sinfo.server_name = new_sinfo.server_name;
    sinfo.server_type = new_sinfo.server_type;
    sinfo.server_os = new_sinfo.server_os;
    sinfo.game_mode = new_sinfo.game_mode;
    sinfo.game_version = new_sinfo.game_version;
    sinfo.map = new_sinfo.map;
    sinfo.num_players = new_sinfo.num_players;
    sinfo.max_players = new_sinfo.max_players;
    sinfo.private = new_sinfo.private;
    sinfo.secure = new_sinfo.secure;

    update ();
  }

  private void on_plist_updated (Gee.ArrayList<Player> new_plist) {
    plist.apply (new_plist);
    update ();
  }

  private void on_error () {
    if (error != null) {
      log (Config.LOG_DOMAIN, LEVEL_WARNING, error.message);
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

    timeout_source = Timeout.add (TIMEOUT_MS, () => {
      error = new QuerierError.TIMEOUT("failed to query %s:%d: %s", server.host, server.qport, "failed to receive a response in reasonable amount of time");
      timeout_source = 0;
      return Source.REMOVE;
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
