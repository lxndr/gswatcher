namespace Gsw {

public enum PlayerFieldType {
  STRING,
  NUMBER,
  DURATION;

  public static PlayerFieldType parse_nick (string nick) {
    var enumc = (EnumClass) typeof (PlayerFieldType).class_ref ();
    var eval = enumc.get_value_by_nick (nick);

    return eval == null
      ? PlayerFieldType.STRING
      : (PlayerFieldType) eval.value;
  }
}

public class PlayerField : Object {
  public string title { get; set; }
  public string field { get; set; }
  public PlayerFieldType kind { get; set; default = STRING; }
  public bool main { get; set; }
}

public abstract interface QueryProtocol : Protocol {
  public signal void sinfo_update (Gee.Map<string, string> details, ServerInfo sinfo);
  public signal void plist_update (Gee.List<PlayerField> plist_fields, Gee.ArrayList<Player> plist);

  public abstract void query () throws Error;
}

}
