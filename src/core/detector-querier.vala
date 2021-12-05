namespace Gsw {

public class DetectorQuerier : Querier {
  private Gee.List<Querier> queriers;
  public Querier? detected_querier { get; private set; }

  public DetectorQuerier (QuerierManager querier_manager, Server server) {
    Object (
      querier_manager : querier_manager,
      server : server
    );
  }

  construct {
    queriers = new Gee.ArrayList<Querier> ();

    foreach (var protocol_desc in querier_manager.get_protocols ()) {
      try {
        var protocol = querier_manager.create_protocol (protocol_desc.id);
        var querier = new WorkerQuerier (querier_manager, server, protocol);

        querier.sinfo.notify.connect(() => {
          if (detected_querier == null)
            on_protocol_detected (querier);
        });

        queriers.add (querier);
      } catch (Error err) {
        log (Config.LOG_DOMAIN, LEVEL_ERROR, "failed to create protocol '%s': %s", protocol_desc.id, err.message);
      }
    }
  }

  public override void query () {
    if (detected_querier == null) {
      foreach (var querier in queriers)
        querier.query ();
    } else {
      detected_querier.query ();
    }
  }

  private void on_protocol_detected (WorkerQuerier querier) {
    detected_querier = querier;

    details = querier.details;
    sinfo = querier.sinfo;
    plist = querier.plist;
    plist_fields = querier.plist_fields;
    ping = querier.ping;

    querier.notify["ping"].connect(() => ping = querier.ping);
    querier.notify["error"].connect(() => error = querier.error);

    foreach (var it in queriers)
      if (it != querier)
        it.dispose();
    queriers.clear ();
  }
}

}
