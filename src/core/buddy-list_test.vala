/*
  Game Server Watcher
  Copyright (C) 2010-2026 Alexander Burobin

  This program is free software: you can redistribute it and/or modify
  it under the terms of the GNU Affero General Public License as
  published by the Free Software Foundation, either version 3 of the
  License, or (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU Affero General Public License for more details.

  You should have received a copy of the GNU Affero General Public License
  along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

using Gsw;
using Gee;

public static int main (string[] args) {
  Test.init (ref args);

  Test.add_func ("/core/BuddyList/get()", () => {
    var list = new BuddyList ();
    var buddy1 = list.add ("player1");
    var buddy2 = list.add ("player2");

    assert (list.get (0) == buddy1);
    assert (list.get (1) == buddy2);
  });

  Test.add_func ("/core/BuddyList/iterator()", () => {
    var list = new BuddyList ();
    var buddy1 = list.add ("player1");
    var buddy2 = list.add ("player2");

    var iterator = list.iterator ();
    assert_true (iterator.next ());
    assert (iterator.get () == buddy1);
    assert_true (iterator.next ());
    assert (iterator.get () == buddy2);
    assert_false (iterator.next ());
  });

  Test.add_func ("/core/BuddyList/add()", () => {
    var list = new BuddyList ();

    list.items_changed.connect ((pos, removed, added) => {
      assert_cmpuint (list.get_n_items (), EQ, 1);
      assert_cmpuint (pos, EQ, 0);
      assert_cmpuint (removed, EQ, 0);
      assert_cmpuint (added, EQ, 1);
    });

    Buddy? added_signal_buddy = null;
    list.added.connect ((buddy) => {
      assert_nonnull (buddy);
      added_signal_buddy = buddy;
    });

    list.removed.connect ((buddy) => {
      assert_not_reached ();
    });

    var buddy = list.add ("player1");

    assert_cmpuint (list.get_n_items (), EQ, 1);
    assert_nonnull (buddy);
    assert (list[0] == buddy);
    assert (added_signal_buddy == buddy);
  });

  Test.add_func ("/core/BuddyList/add()/duplicate", () => {
    var list = new BuddyList ();
    var buddy1 = list.add ("player1");

    list.items_changed.connect ((pos, removed, added) => {
      assert_not_reached ();
    });

    list.added.connect ((buddy) => {
      assert_not_reached ();
    });

    list.removed.connect ((buddy) => {
      assert_not_reached ();
    });

    var buddy2 = list.add ("player1");

    assert_cmpuint (list.get_n_items (), EQ, 1);
    assert_nonnull (buddy1);
    assert (buddy1 == buddy2);
  });

  Test.add_func ("/core/BuddyList/remove()", () => {
    var list = new BuddyList ();
    var buddy = list.add ("player1");

    list.items_changed.connect ((pos, removed, added) => {
      assert_cmpuint (list.get_n_items (), EQ, 0);
      assert_cmpuint (pos, EQ, 0);
      assert_cmpuint (removed, EQ, 1);
      assert_cmpuint (added, EQ, 0);
    });

    list.added.connect ((buddy) => {
      assert_not_reached ();
    });

    Buddy? removed_signal_buddy = null;
    list.removed.connect ((buddy) => {
      assert_nonnull (buddy);
      removed_signal_buddy = buddy;
    });

    list.remove (buddy);

    assert_cmpuint (list.get_n_items (), EQ, 0);
    assert_nonnull (removed_signal_buddy);
    assert (removed_signal_buddy == buddy);
  });

  Test.add_func ("/core/BuddyList/remove()/not_found", () => {
    var list = new BuddyList ();
    var buddy = new Buddy ("player1");

    list.items_changed.connect ((pos, removed, added) => {
      assert_not_reached ();
    });

    list.added.connect ((buddy) => {
      assert_not_reached ();
    });

    list.removed.connect ((buddy) => {
      assert_not_reached ();
    });

    list.remove (buddy);

    assert_cmpuint (list.get_n_items (), EQ, 0);
  });

  Test.add_func ("/core/BuddyList/find_by_name()", () => {
    var list = new BuddyList ();
    var buddy = list.add ("player1");

    var found = list.find_by_name ("player1");
    assert_nonnull (found);
    assert (found == buddy);
  });

  Test.add_func ("/core/BuddyList/find_by_name()/not_found", () => {
    var list = new BuddyList ();
    list.add ("player1");

    var found = list.find_by_name ("player2");
    assert_null (found);
  });

  Test.add_func ("/core/BuddyList/exists()", () => {
    var list = new BuddyList ();
    list.add ("player1");

    assert_true (list.exists ("player1"));
  });

  Test.add_func ("/core/BuddyList/exists()/not_found", () => {
    var list = new BuddyList ();
    list.add ("player1");

    assert_false (list.exists ("player2"));
  });

  /* ListModel implementation tests */

  Test.add_func ("/core/BuddyList/get_item_type()", () => {
    var list = new BuddyList ();
    assert (list.get_item_type () == typeof (Buddy));
  });

  Test.add_func ("/core/BuddyList/get_n_items()", () => {
    var list = new BuddyList ();
    assert_cmpuint (list.get_n_items (), EQ, 0);

    list.add ("player1");
    assert_cmpuint (list.get_n_items (), EQ, 1);

    list.add ("player2");
    assert_cmpuint (list.get_n_items (), EQ, 2);
  });

  Test.add_func ("/core/BuddyList/get_item()", () => {
    var list = new BuddyList ();
    var buddy1 = list.add ("player1");
    var buddy2 = list.add ("player2");

    assert (list.get_item (0) == buddy1);
    assert (list.get_item (1) == buddy2);
    assert_null (list.get_item (2));
  });

  Test.add_func ("/core/BuddyList/items_changed/addition", () => {
    var list = new BuddyList ();

    list.items_changed.connect ((pos, removed, added) => {
      assert_cmpuint (list.get_n_items (), EQ, 1);
      assert_cmpuint (pos, EQ, 0);
      assert_cmpuint (removed, EQ, 0);
      assert_cmpuint (added, EQ, 1);
    });

    list.add ("localhost:27015");
  });

  Test.add_func ("/core/BuddyList/items_changed/removal", () => {
    var list = new BuddyList ();
    var buddy = list.add ("localhost:27015");

    list.items_changed.connect ((pos, removed, added) => {
      assert_cmpuint (list.get_n_items (), EQ, 0);
      assert_cmpuint (pos, EQ, 0);
      assert_cmpuint (removed, EQ, 1);
      assert_cmpuint (added, EQ, 0);
    });

    list.remove (buddy);
  });

  Test.run ();
  return 0;
}
