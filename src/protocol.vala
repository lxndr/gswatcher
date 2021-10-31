namespace Gsw {

  errordomain ProtocolError {
    UNKNOWN
  }

  struct ProtocolInfo {
    public string name;
    public string version;
    public string transport;
  }

  abstract class Protocol : Object {
    public ProtocolInfo info;

    public signal void data_send (uint8[] data);
    public signal void sinfo_update (ServerInfo sinfo);
    public signal void plist_update (Gee.ArrayList<Player> plist);

    public abstract void query () throws ProtocolError;
    public abstract void process_response (uint8[] data) throws ProtocolError;
  }

}
