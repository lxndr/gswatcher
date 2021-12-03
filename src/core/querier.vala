namespace Gsw {

public abstract class Querier : Object {
  public QuerierManager querier_manager { get; construct; }
  public Server server { get; construct; }

  public ServerDetailsList details { get; protected set; default = new ServerDetailsList (); }
  public ServerInfo sinfo { get; protected set; default = new ServerInfo (); }
  public PlayerList plist { get; protected set; default = new PlayerList (); }
  public ListStore? plist_fields { get; protected set; default = new ListStore (typeof (PlayerField)); }
  public int64 ping { get; protected set; default = -1; }
  public Error? error { get; protected set; }

  public abstract void query ();
}

}
