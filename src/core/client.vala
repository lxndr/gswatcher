namespace Gsw {

public class Client : Object {
  private Gee.List<Querier> tmp_queriers = new Gee.ArrayList<Querier> ();
  public Server server { get; construct; }
  public Querier? querier { get; private set; }
  public ConsoleClient? console_client { get; private set; }

  public Client (Server server) {
    Object (server : server);
  }

  construct {
    reset ();
  }

  private void reset () {
    querier = null;
    tmp_queriers.clear ();

    foreach (var protocol_desc in ProtocolRegistry.get_instance ().list_by_feature (QUERY)) {
      try {
        var protocol = ProtocolRegistry.get_instance ().create (protocol_desc.id);
        var querier = new Querier (server, (QueryProtocol) protocol);
        querier.update.connect (on_protocol_detected);
        tmp_queriers.add (querier);
      } catch (Error err) {
        log (Config.LOG_DOMAIN, LEVEL_ERROR, "failed to create protocol '%s': %s", protocol_desc.id, err.message);
      }
    }
  }

  private void on_protocol_detected (Querier querier) {
    this.querier = querier;

    var console_protocol = GameResolver.get_instance ().get_feature_protocol (querier.sinfo.game_id, CONSOLE);

    if (console_protocol != null) {
      try {
        var proto = (ConsoleProtocol) ProtocolRegistry.get_instance ().create (console_protocol);
        console_client = new ConsoleClient (querier.server.host, querier.server.gport, proto);
      } catch (Error err) {
        log (Config.LOG_DOMAIN, LEVEL_ERROR, "failed to create protocol '%s': %s", "source-console", err.message);
      }
    }

    foreach (var it in tmp_queriers) {
      it.update.disconnect (on_protocol_detected);

      if (it != querier)
        it.dispose();
    }

    tmp_queriers.clear ();
  }
}

}
