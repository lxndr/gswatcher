namespace Gsw {

[SingleInstance]
public class QuerierManager : Object {
  public Gee.List<Querier> queriers = new Gee.ArrayList<Querier> ();
  public uint update_interval { get; set; default = 5000; }
  public bool paused { get; set; }

  private uint timer_source;
  private int querier_iter = -1;

  public static QuerierManager get_instance () {
    return new QuerierManager ();
  }

  construct {
    notify["update_interval"].connect (update_timer);
    notify["paused"].connect (update_timer);
    update_timer ();
  }

  ~QuerierManager () {
    notify["update_interval"].disconnect (update_timer);
    notify["paused"].disconnect (update_timer);
  }

  public void register (Querier querier) {
    queriers.add (querier);
    update_timer ();
  }

  public void unregister (Querier querier) {
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
    var querier = (Querier) queriers[querier_iter];

    log (Config.LOG_DOMAIN, LEVEL_DEBUG, "querying %s:%d", querier.server.host, querier.server.qport);

    querier.query ();
  }

  private void advance_querier_iter () {
    var last_iter = queriers.size - 1;

    if (querier_iter >= last_iter)
      querier_iter = -1;

    querier_iter++;
  }
}

}
