namespace Gsw {

public class DetectorQuerier : Querier {
  private Gee.List<Querier> queriers;
  private Querier? detected_querier;

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

        querier.notify["sinfo"].connect(() => {
          on_protocol_detected (querier);
          sinfo = querier.sinfo;
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
    var worker = new WorkerQuerier (querier_manager, querier.server, querier.protocol);

    sinfo = worker.sinfo;
    plist = worker.plist;
    plist_fields = worker.plist_fields;
    ping = worker.ping;

    querier.notify["sinfo"].connect(() => sinfo = worker.sinfo);
    querier.notify["ping"].connect(() => ping = worker.ping);
    querier.notify["error"].connect(() => error = worker.error);

    detected_querier = worker;

    foreach (var it in queriers)
      it.dispose();
    queriers.clear ();
  }
}

}
