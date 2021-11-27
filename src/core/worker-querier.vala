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
    transport = querier_manager.create_transport (protocol.info.transport, server.host, server.qport);

    transport.receive.connect ((data) => {
      try {
        log (Config.LOG_DOMAIN, LEVEL_DEBUG, "received data from %s:%d: length = %ld", server.host, server.qport, data.length);
        protocol.process_response (data);
        error = null;
      } catch (Error err) {
        if (err is ProtocolError && err.code == ProtocolError.INVALID_RESPONSE) {
          return;
        }

        var msg = "failed to process data from %s:%d: %s".printf (server.host, server.qport, err.message);
        log (Config.LOG_DOMAIN, LEVEL_WARNING, msg);
        error = new QuerierError.PROCESSING (msg);
      }
    });

    protocol.data_send.connect ((data) => {
      try {
        transport.send (data);
        log (Config.LOG_DOMAIN, LEVEL_DEBUG, "sent data to %s:%d: length = %ld", server.host, server.qport, data.length);
      } catch (Error err) {
        var msg = "failed to send data to %s:%d: %s".printf (server.host, server.qport, err.message);
        log (Config.LOG_DOMAIN, LEVEL_ERROR, msg);
        error = new QuerierError.SENDING (msg);
      }
    });

    protocol.sinfo_update.connect ((sinfo) => {
      ping = (get_monotonic_time () - query_time) / 1000;
      remove_timeout_source ();

      var game_resolver = GameResolver.get_instance ();
      game_resolver.resolve (protocol.info.id, sinfo);
      this.sinfo = sinfo;

      if (plist_fields.get_n_items () == 0)
        foreach (var field in game_resolver.get_player_fields (this.sinfo.game_id))
          plist_fields.append (field);
    });

    protocol.plist_update.connect ((plist) => {
      this.plist.apply (plist);
    });
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

  private void remove_timeout_source () {
    if (timeout_source > 0) {
      Source.remove (timeout_source);
      timeout_source = 0;
    }
  }
}

}
