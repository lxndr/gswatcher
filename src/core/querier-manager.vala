namespace Gsw {

public class ProtocolDesc : ProtocolInfo {
  public Type class_type;
  public string[] class_params;
  public Value[] class_values;
}

public class TransportDesc {
  public string id;
  public Type class_type;
}

public class QuerierManager : Object {
  public ServerList server_list { get; construct set; }
  public Gtk.MapListModel querier_list { get; private construct set; }
  public uint update_interval { get; set; default = 5000; }
  public bool paused { get; set; }

  private Gee.Map<string, ProtocolDesc> protocols;
  private Gee.Map<string, TransportDesc> transports;

  private uint timer_source;
  private int querier_iter = -1;

  construct {
    protocols = new Gee.HashMap<string, ProtocolDesc> ();
    transports = new Gee.HashMap<string, TransportDesc> ();

    querier_list = new Gtk.MapListModel(server_list, (item) => {
      var server = (Server) item;
      var querier = new DetectorQuerier (this, server);
      querier.query ();
      return querier;
    });

    querier_list.items_changed.connect (update_timer);

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

  public void register_protocol (ProtocolDesc desc)
    ensures (desc.id != null && desc.id != "")
    ensures (desc.name != null && desc.name != "")
    ensures (desc.transport != null && desc.transport != "")
    ensures (desc.class_type.is_a (typeof (Protocol)))
  {
    protocols.set (desc.id, desc);
  }

  public Protocol? create_protocol (string id) throws Error {
    if (!protocols.has_key (id)) {
      log (Config.LOG_DOMAIN, LEVEL_ERROR, "unknown protocol '%s'", id);
      return null;
    }

    var desc = protocols[id];
    var protocol = (Protocol) Object.new_with_properties (desc.class_type, desc.class_params, desc.class_values);
    protocol.initialize ();
    return protocol;
  }

  public Gee.Collection<TransportDesc> get_transports () {
    return transports.values.read_only_view;
  }

  public Gee.Collection<ProtocolDesc> get_protocols () {
    return protocols.values.read_only_view;
  }

  public void register_transport (TransportDesc desc) {
    var id = desc.id;
    var type = desc.class_type;

    if (!type.is_a (typeof (Transport))) {
      log (Config.LOG_DOMAIN, LEVEL_ERROR, "failed to register transport '%s'", type.name ());
      return;
    }

    transports.set (id, desc);
  }

  public NetTransport? create_transport (string id, string host, uint16 port) {
    if (!transports.has_key (id)) {
      log (Config.LOG_DOMAIN, LEVEL_ERROR, "unknown transport '%s'", id);
      return null;
    }

    var desc = transports[id];
    return (NetTransport) Object.new (desc.class_type, "host", host, "port", port);
  }
}

}
