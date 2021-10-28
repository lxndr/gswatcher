namespace Gsw {

  class BuddyList : Object, ListModel {
    private Gee.List<Buddy> list;

    construct {
      list = new Gee.ArrayList<Buddy> ();
    }

    public Object? get_item (uint position) {
      return list[(int) position];
    }

    public Type get_item_type () {
      return typeof (Buddy);
    }

    public uint get_n_items () {
      return list.size;
    }

    public Buddy add (string name) {
      var buddy = new Buddy (name);
      list.add (buddy);
      items_changed (list.size - 1, 0, 1);
      return buddy;
    }

    public void remove (Buddy buddy) {
      var idx = list.index_of (buddy);

      if (idx > -1) {
        list.remove_at (idx);
        items_changed (idx, 1, 0);
      }
    }

    public void remove_by_name (string name) {
      var buddy = list.first_match ((buddy) => {
        return buddy.name == name;
      });

      if (buddy != null) {
        remove (buddy);
      }
    }
  }

}
