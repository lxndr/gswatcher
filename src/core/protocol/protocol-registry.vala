namespace Gsw {

public class ProtocolDesc : ProtocolInfo {
  public Type class_type;
  public string[] class_params;
  public Value[] class_values;
}

[SingleInstance]
class ProtocolRegistry : Object {
  private Gee.Map<string, ProtocolDesc> protocols = new Gee.HashMap<string, ProtocolDesc> ();

  public static ProtocolRegistry get_instance () {
    return new ProtocolRegistry ();
  }

  public Gee.Collection<ProtocolDesc> list () {
    return protocols.values.read_only_view;
  }

  public Gee.Collection<ProtocolDesc> list_by_feature (ProtocolFeature feature) {
    var iter = protocols.values.filter ((protocol) => protocol.feature == feature);
    var arr = new Gee.ArrayList<ProtocolDesc> ();
    arr.add_all_iterator (iter);
    return arr.read_only_view;
  }

  public void register (ProtocolDesc desc)
    ensures (desc.id != null && desc.id != "")
    ensures (desc.name != null && desc.name != "")
    ensures (desc.transport != null && desc.transport != "")
    ensures (desc.class_type.is_a (typeof (Protocol)))
  {
    protocols.set (desc.id, desc);
  }

  public Protocol? create (string id) throws Error {
    if (!protocols.has_key (id)) {
      log (Config.LOG_DOMAIN, LEVEL_ERROR, "unknown protocol '%s'", id);
      return null;
    }

    var desc = protocols[id];
    var protocol = (Protocol) Object.new_with_properties (desc.class_type, desc.class_params, desc.class_values);
    protocol.initialize ();
    return protocol;
  }
}

}
