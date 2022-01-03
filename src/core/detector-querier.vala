namespace Gsw {

public class DetectorQuerier : Querier {
  private Gee.List<Querier> queriers = new Gee.ArrayList<Querier> ();
  public Querier? detected_querier { get; private set; }
  public ConsoleClient? console_client { get; private set; }

  private Gtk.ExpressionWatch details_watch;
  private Gtk.ExpressionWatch sinfo_watch;
  private Gtk.ExpressionWatch plist_watch;
  private Gtk.ExpressionWatch plist_fields_watch;
  private Gtk.ExpressionWatch ping_watch;
  private Gtk.ExpressionWatch error_watch;

  public DetectorQuerier (Server server) {
    Object (server : server);
  }

  construct {
    var detector_expr = new Gtk.ObjectExpression (this);
    var detected_expr = new Gtk.PropertyExpression (typeof (DetectorQuerier), detector_expr, "detected_querier");

    var details_expr = new Gtk.PropertyExpression (typeof (WorkerQuerier), detected_expr, "details");
    details_watch = details_expr.bind (this, "details", null);

    var sinfo_expr = new Gtk.PropertyExpression (typeof (WorkerQuerier), detected_expr, "sinfo");
    sinfo_watch = sinfo_expr.bind (this, "sinfo", null);

    var plist_expr = new Gtk.PropertyExpression (typeof (WorkerQuerier), detected_expr, "plist");
    plist_watch = plist_expr.bind (this, "plist", null);

    var plist_fields_expr = new Gtk.PropertyExpression (typeof (WorkerQuerier), detected_expr, "plist_fields");
    plist_fields_watch = plist_fields_expr.bind (this, "plist_fields", null);

    var ping_expr = new Gtk.PropertyExpression (typeof (WorkerQuerier), detected_expr, "ping");
    ping_watch = ping_expr.bind (this, "ping", null);

    var error_expr = new Gtk.PropertyExpression (typeof (WorkerQuerier), detected_expr, "error");
    error_watch = error_expr.bind (this, "error", null);

    reset ();
  }

  ~DetectorQuerier () {
    // INFO: check later. probably we do not need to unwatch
    details_watch.unwatch ();
    sinfo_watch.unwatch ();
    plist_watch.unwatch ();
    plist_fields_watch.unwatch ();
    ping_watch.unwatch ();
    error_watch.unwatch ();
  }

  private void reset () {
    detected_querier = null;
    queriers.clear ();

    foreach (var protocol_desc in ProtocolRegistry.get_instance ().list_by_feature (QUERY)) {
      try {
        var protocol = ProtocolRegistry.get_instance ().create (protocol_desc.id);
        var querier = new WorkerQuerier (server, (QueryProtocol) protocol);
        querier.update.connect (on_protocol_detected);
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

  private void on_protocol_detected (Querier querier) {
    detected_querier = querier;

    var supports_console = GameResolver.get_instance ().supports_feature (querier.sinfo.game_id, "console");

    if (supports_console) {
      try {
        var proto = (ConsoleProtocol) ProtocolRegistry.get_instance ().create ("source-console");
        console_client = new ConsoleClient (querier.server.host, querier.server.gport, proto);
      } catch (Error err) {
        log (Config.LOG_DOMAIN, LEVEL_ERROR, "failed to create protocol '%s': %s", "source-console", err.message);
      }
    }

    foreach (var it in queriers) {
      it.update.disconnect (on_protocol_detected);

      if (it != querier)
        it.dispose();
    }

    queriers.clear ();
  }
}

}
