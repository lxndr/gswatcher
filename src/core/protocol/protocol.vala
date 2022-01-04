namespace Gsw {

public errordomain ProtocolError {
  INVALID_RESPONSE,
  AUTH_FAILED,
  NOT_IMPLEMENTED,
}

public enum ProtocolFeature {
  QUERY,
  CONSOLE;

  public static ProtocolFeature parse_nick (string nick) {
    var enumc = (EnumClass) typeof (ProtocolFeature).class_ref ();
    var eval = enumc.get_value_by_nick (nick);

    return eval == null
      ? ProtocolFeature.QUERY
      : (ProtocolFeature) eval.value;
  }
}

public enum ProtocolOptionType {
  PASSWORD
}

public class ProtocolOption {
  public string id;
  public string label;
  public ProtocolOptionType type;
}

public class ProtocolInfo {
  public ProtocolFeature feature;
  public string id;
  public string name;
  public string transport;
  public Gee.List<ProtocolOption> options;
}

public interface Protocol : Object {
  public abstract bool initialized { get; protected set; }
  public abstract ProtocolInfo info { get; protected set; }

  public signal void data_send (uint8[] data);

  public abstract void initialize () throws Error;
  public abstract void process_response (uint8[] data) throws Error;
}

}
