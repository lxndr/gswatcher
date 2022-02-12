namespace Gsw {

const uint TIMEOUT_MS = 2000;

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

  public ServerDetailsList details { get; protected set; default = new ServerDetailsList (); }
  public ServerInfo sinfo { get; protected set; default = new ServerInfo (); }
  public PlayerList plist { get; protected set; default = new PlayerList (); }
  public ListStore? plist_fields { get; protected set; default = new ListStore (typeof (PlayerField)); }
  public int64 ping { get; protected set; default = -1; }
  public Error? error { get; protected set; }

  private GameResolver game_resolver = GameResolver.get_instance ();
  private bool query_pending;
  private int64 query_time;
  private uint timeout_source;

  public signal void update ();

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

    server.protocol = protocol.info.id;
    server.game_id = sinfo.game_id;
    server.server_name = sinfo.server_name;

    this.sinfo.game_id = sinfo.game_id;
    this.sinfo.game_name = sinfo.game_name;
    this.sinfo.game_mode = sinfo.game_mode;
    this.sinfo.server_name = sinfo.server_name;
    this.sinfo.server_type = sinfo.server_type;
    this.sinfo.server_os = sinfo.server_os;
    this.sinfo.game_version = sinfo.game_version;
    this.sinfo.map = sinfo.map;
    this.sinfo.num_players = sinfo.num_players;
    this.sinfo.max_players = sinfo.max_players;
    this.sinfo.private = sinfo.private;
    this.sinfo.secure = sinfo.secure;

    this.details.apply (details);
    update ();
  }

  private void on_plist_updated (Gee.List<PlayerField> default_pfields, Gee.ArrayList<Player> plist) {
    if (this.plist_fields.get_n_items () == 0) {
      var game_pfields = game_resolver.get_plist_fields (sinfo.game_id);
      var new_pfields = (game_pfields != null && game_pfields.size > 0) ? game_pfields : default_pfields;

      foreach (var field in new_pfields)
        this.plist_fields.append (field);
    }

    this.plist.apply (plist);
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
