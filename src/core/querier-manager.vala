using Gee;

namespace Gsw {

[SingleInstance]
public class QuerierManager : Object {
  public uint update_interval { get; set; default = 5000; }
  public bool paused { get; set; }

  private uint timer_source;
  private Collection<unowned Querier> queriers;
  private Iterator<unowned Querier> querier_iter;

  public static QuerierManager get_instance () {
    return new QuerierManager ();
  }

  construct {
    queriers = new ConcurrentList<unowned Querier> ();
    querier_iter = queriers.iterator ();

    notify["update_interval"].connect (update_timer);
    notify["paused"].connect (update_timer);
    update_timer ();
  }

  public Querier create_querier (Server server, string protocol_id) throws Error {
    var protocol = ProtocolRegistry.get_instance ().create (protocol_id);

    var querier = new Querier (server, (QueryProtocol) protocol);
    querier.weak_ref ((WeakNotify) remove_querier);
    queriers.add (querier);

    update_timer ();
    return querier;
  }

  private void remove_querier (Querier querier) {
    queriers.remove (querier);
    update_timer ();
  }

  private void update_timer () {
    if (timer_source != 0) {
      Source.remove (timer_source);
      timer_source = 0;
      log (Config.LOG_DOMAIN, LEVEL_DEBUG, "update timer stopped");
    }

    if (!paused && queriers.size > 0) {
      var interval = update_interval / queriers.size;

      timer_source = Timeout.add (interval, () => {
        update_tick ();
        return Source.CONTINUE;
      });

      log (Config.LOG_DOMAIN, LEVEL_DEBUG, "update timer started with interval %u ms", interval);
    }
  }

  private void update_tick () {
    advance_querier_iter ();
    var querier = querier_iter.get ();

    log (Config.LOG_DOMAIN, LEVEL_DEBUG, "querying %s:%d", querier.server.host, querier.server.qport);

    querier.query ();
  }

  private void advance_querier_iter () {
    if (!querier_iter.has_next ())
      querier_iter = queriers.iterator ();
    querier_iter.next ();
  }
}

}
