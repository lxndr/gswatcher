namespace Gsw {

public class Querier : Object {
  public Server server { get; construct; }
  public Protocol protocol { get; construct; }
  public Transport transport { get; construct; }

  public ServerInfo sinfo { get; private set; }
  public PlayerList plist { get; private set; }
  public ListStore plist_fields { get; private set; }
  public int64 ping { get; private set; default = -1; }

  private int64 query_time;

  construct {
    try {
      plist = new PlayerList ();
      plist_fields = new ListStore (typeof (PlayerField));

      transport.receive.connect ((data) => {
        try {
          log (Config.LOG_DOMAIN, LEVEL_DEBUG, "received data from %s:%d: length = %ld", server.host, server.qport, data.length);
          protocol.process_response (data);
        } catch (Error err) {
          log (Config.LOG_DOMAIN, LEVEL_ERROR, "failed to process data from %s:%d: %s", server.host, server.qport, err.message);
        }
      });

      protocol.data_send.connect ((data) => {
        try {
          transport.send (data);
          log (Config.LOG_DOMAIN, LEVEL_DEBUG, "sent data to %s:%d: length = %ld", server.host, server.qport, data.length);
        } catch (Error err) {
          log (Config.LOG_DOMAIN, LEVEL_ERROR, "failed to send data to %s:%d: %s", server.host, server.qport, err.message);
        }
      });

      protocol.sinfo_update.connect ((sinfo) => {
        ping = (get_monotonic_time () - query_time) / 1000;

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
    } catch (Error err) {
      error (err.message);
    }
  }

  public Querier (Server server, Protocol protocol, Transport transport) {
    Object (server : server, protocol : protocol, transport : transport);
  }

  public void query () {
    try {
      query_time = get_monotonic_time ();
      protocol.query ();
    } catch (Error err) {
      log (Config.LOG_DOMAIN, LEVEL_ERROR, "failed to query %s:%d: %s", server.host, server.qport, err.message);
    }
  }
}

}
