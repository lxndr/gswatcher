namespace Gsw {

public class ServerList : Object, ListModel {
  private Gee.List<Server> list;

  construct {
    list = new Gee.ArrayList<Server> ();
  }

  public Object? get_item (uint position) {
    return list[(int) position];
  }

  public Type get_item_type () {
    return typeof (Server);
  }

  public uint get_n_items () {
    return list.size;
  }

  public virtual Server add (string address) {
    var server = new Server (address);
    list.add (server);
    items_changed (list.size - 1, 0, 1);
    return server;
  }

  public virtual void remove (Server server) {
    var idx = list.index_of (server);

    if (idx > -1) {
      list.remove_at (idx);
      items_changed (idx, 1, 0);
    }
  }
}

}
