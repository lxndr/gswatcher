namespace Gsw {

public class QuerierManager : Object {
  public ServerList server_list { get; construct set; }
  public Gtk.MapListModel querier_list { get; private construct set; }
  public uint update_interval { get; set; default = 5000; }
  public bool paused { get; set; }

  private uint timer_source;
  private int querier_iter = -1;

  construct {
    querier_list = new Gtk.MapListModel(server_list, (server) => {
      return new Querier ((Server) server);
    });

    notify["update_interval"].connect (update_timer);
    notify["paused"].connect (update_timer);
    update_timer ();
  }

  public QuerierManager (ServerList server_list) {
    Object (server_list: server_list);
  }

  private void update_timer () {
    if (timer_source != 0) {
      Source.remove (timer_source);
      timer_source = 0;
      log (Config.LOG_DOMAIN, LEVEL_DEBUG, "update timer stopped");
    }

    if (!paused && querier_list.get_n_items () > 0) {
      var interval = update_interval / querier_list.get_n_items ();

      timer_source = Timeout.add (interval, () => {
        update_tick ();
        return Source.CONTINUE;
      });

      log (Config.LOG_DOMAIN, LEVEL_DEBUG, "update timer started with interval %u ms", interval);
    }
  }

  private void update_tick () {
    advance_querier_iter ();
    var querier = (Querier) querier_list.get_item (querier_iter);

    log (Config.LOG_DOMAIN, LEVEL_DEBUG, "querying %s:%d", querier.server.host, querier.server.qport);

    querier.query ();
  }

  private void advance_querier_iter () {
    var last_iter = querier_list.get_n_items () - 1;
    if (querier_iter >= last_iter)
      querier_iter = -1;
    querier_iter++;
  }
}

}
