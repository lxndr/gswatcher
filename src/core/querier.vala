namespace Gsw {

public abstract class Querier : Object {
  public QuerierManager querier_manager { get; construct; }
  public Server server { get; construct; }

  public ServerInfo sinfo { get; protected set; }
  public PlayerList plist { get; protected set; }
  public ListStore plist_fields { get; protected set; }
  public int64 ping { get; protected set; default = -1; }
  public Error? error { get; protected set; }

  construct {
    plist = new PlayerList ();
    plist_fields = new ListStore (typeof (PlayerField));
  }

  public abstract void query ();
}

}
