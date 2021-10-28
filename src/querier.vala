namespace Gsw {
  class Querier : Object {
    public Server server { get; construct set; }
    public Protocol protocol { get; construct set; }
    public UdpTransport transport { get; construct set; }

    public ServerInfo sinfo { get; private set; }
    public Gee.List<Player> plist { get; private set; }
    public int64 ping { get; private set; default = -1; }

    private int64 query_time;

    construct {
      try {
        transport = new UdpTransport (server.host, server.qport);
        protocol = new JsProtocol("../src/scripts/dist/source.js");

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
          this.sinfo = sinfo;
        });

        protocol.plist_update.connect ((plist) => {
          this.plist = plist;
        });
      } catch (Error err) {
        error (err.message);
      }
    }

    public Querier (Server server) {
      Object (server : server);
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
