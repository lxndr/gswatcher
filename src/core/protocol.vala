namespace Gsw {

public class ProtocolInfo {
  public string id;
  public string name;
  public string transport;
}

public abstract class Protocol : Object {
  public bool initialized { get; protected set; }
  public ProtocolInfo info { get; protected set; }

  public signal void data_send (uint8[] data);
  public signal void sinfo_update (ServerInfo sinfo);
  public signal void plist_update (Gee.ArrayList<Player> plist);

  public abstract void initialize () throws Error;
  public abstract void query () throws Error;
  public abstract void process_response (uint8[] data) throws Error;
}

}
