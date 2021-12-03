namespace Gsw {

const uint TIMEOUT_MS = 2000;

public errordomain QuerierError {
  PROCESSING,
  SENDING,
  QUERYING,
  TIMEOUT,
}

public class WorkerQuerier : Querier {
  public Protocol protocol { get; construct; }
  public Transport transport { get; construct; }

  private int64 query_time;
  private uint timeout_source;

  construct {
    protocol.data_send.connect (send_data);
    protocol.details_update.connect (on_details_updated);
    protocol.sinfo_update.connect (on_sinfo_updated);
    protocol.plist_update.connect (on_plist_updated);

    transport = querier_manager.create_transport (protocol.info.transport, server.host, server.qport);
    transport.receive.connect (on_data_received);
  }

  public WorkerQuerier (QuerierManager querier_manager, Server server, Protocol protocol) {
    Object (
      querier_manager : querier_manager,
      server : server,
      protocol : protocol
    );
  }

  ~WorkerQuerier () {
    remove_timeout_source ();
  }

  public override void query () {
    try {
      remove_timeout_source ();

      timeout_source = Timeout.add (TIMEOUT_MS, () => {
        error = new QuerierError.TIMEOUT ("failed to receive a response in reasonable amount of time");
        timeout_source = 0;
        return Source.REMOVE;
      });

      query_time = get_monotonic_time ();

      protocol.query ();
    } catch (Error err) {
      var msg = "failed to query %s:%d: %s".printf (server.host, server.qport, err.message);
      log (Config.LOG_DOMAIN, LEVEL_ERROR, msg);
      error = new QuerierError.QUERYING (msg);
    }
  }

  private void resolve_game (Gee.Map<string, string> new_details) {
    var game_resolver = GameResolver.get_instance ();
    game_resolver.resolve (protocol.info.id, sinfo, new_details);

    if (sinfo.game_id != null && plist_fields.get_n_items () == 0)
      foreach (var field in game_resolver.get_player_fields (sinfo.game_id))
        plist_fields.append (field);
  }

  private void remove_timeout_source () {
    if (timeout_source > 0) {
      Source.remove (timeout_source);
      timeout_source = 0;
    }
  }

  private void send_data (uint8[] data) {
    try {
      transport.send (data);
      log (Config.LOG_DOMAIN, LEVEL_DEBUG, "sent data to %s:%u: length = %ld", server.host, server.qport, data.length);
    } catch (Error err) {
      var msg = "failed to send data to %s:%u: %s".printf (server.host, server.qport, err.message);
      log (Config.LOG_DOMAIN, LEVEL_ERROR, msg);
      error = new QuerierError.SENDING (msg);
    }
  }

  private void on_data_received (uint8[] data) {
    try {
      log (Config.LOG_DOMAIN, LEVEL_DEBUG, "received data from %s:%u: length = %ld", server.host, server.qport, data.length);
      protocol.process_response (data);
      error = null;
    } catch (Error err) {
      if (err is ProtocolError && err.code == ProtocolError.INVALID_RESPONSE) {
        return;
      }

      var msg = "failed to process data from %s:%u: %s".printf (server.host, server.qport, err.message);
      log (Config.LOG_DOMAIN, LEVEL_WARNING, msg);
      error = new QuerierError.PROCESSING (msg);
    }
  }

  private void on_details_updated (Gee.Map<string, string> new_details) {
    ping = (get_monotonic_time () - query_time) / 1000;
    remove_timeout_source ();

    if (sinfo.game_id == null)
      resolve_game (new_details);

    details.apply (new_details);
  }

  private void on_sinfo_updated (ServerInfo new_sinfo) {
    // freeze_notify ();

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

    // thaw_notify ();
  }

  private void on_plist_updated (Gee.ArrayList<Player> new_plist) {
    plist.apply (new_plist);
  }
}

}
