namespace Gsw {

public class ServerList : Object, ListModel {
  private Gee.List<Client> list = new Gee.ArrayList<Client> ();

  public Object? get_item (uint position) {
    return list[(int) position];
  }

  public Type get_item_type () {
    return typeof (Client);
  }

  public uint get_n_items () {
    return list.size;
  }

  public uint size {
    get {
      return get_n_items ();
    }
  }

  public new Client get (uint index) {
    return (Client) get_item (index);
  }

  public virtual Client add (string address) {
    var client = new Client (new Server (address));
    list.add (client);
    items_changed (list.size - 1, 0, 1);
    return client;
  }

  public virtual void remove (Client client) {
    var idx = list.index_of (client);

    if (idx > -1) {
      list.remove_at (idx);
      items_changed (idx, 1, 0);
    }
  }
}

}
