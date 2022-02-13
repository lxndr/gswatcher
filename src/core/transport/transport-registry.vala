namespace Gsw {

public class TransportDesc {
  public string id;
  public Type class_type;
}

[SingleInstance]
class TransportRegistry : Object {
  private Gee.Map<string, TransportDesc> transports = new Gee.HashMap<string, TransportDesc> ();

  public static TransportRegistry get_instance () {
    return new TransportRegistry ();
  }

  public Gee.Collection<TransportDesc> list () {
    return transports.values.read_only_view;
  }

  public void register (TransportDesc desc) {
    var id = desc.id;
    var type = desc.class_type;

    if (!type.is_a (typeof (Transport))) {
      log (Config.LOG_DOMAIN, LEVEL_ERROR, "failed to register transport '%s'", type.name ());
      return;
    }

    transports.set (id, desc);
  }

  public NetTransport? create_net_transport (string id, string host, uint16 port) {
    if (!transports.has_key (id)) {
      log (Config.LOG_DOMAIN, LEVEL_ERROR, "unknown transport '%s'", id);
      return null;
    }

    var desc = transports[id];
    return (NetTransport) Object.new (desc.class_type, "host", host, "port", port);
  }
}

}
