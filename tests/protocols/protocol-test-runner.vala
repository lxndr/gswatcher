/*
  Game Server Watcher
  Copyright (C) 2025  Alexander Burobin

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

namespace Gsw {

class ProtocolTestRunner {
  private enum DataDirection {
    REQUEST,
    RESPONSE,
  }

  private struct DataPacket {
    public DataDirection direction;
    public Bytes data;
  }

  private string script_path;
  private Gee.Map<string, string> parameters = new Gee.HashMap<string, string> ();
  private Gee.Queue<DataPacket?> expected_data_flow = new Gee.ArrayQueue<DataPacket?> ();
  private Gee.Map<string, string> expected_sinfo = new Gee.HashMap<string, string> ();
  private Gee.List<PlayerField> expected_plist_fields = new Gee.ArrayList<PlayerField> ();
  private Gee.List<Player> expected_plist = new Gee.ArrayList<Player> ();
  private Error? expected_error = null;

  public ProtocolTestRunner (string script_path) {
    this.script_path = script_path;
  }

  public ProtocolTestRunner param (string key, string value) {
    parameters.set (key, value);
    return this;
  }

  public ProtocolTestRunner expect_data (uint8[] data) {
    expected_data_flow.add (DataPacket () {
      direction = REQUEST,
      data = new Bytes (data),
    });

    return this;
  }

  public ProtocolTestRunner response (uint8[] data) {
    expected_data_flow.add (DataPacket () {
      direction = RESPONSE,
      data = new Bytes (data),
    });

    return this;
  }

  public ProtocolTestRunner expect_sinfo (Gee.Map<string, string> sinfo) {
    expected_sinfo = sinfo;
    return this;
  }

  public ProtocolTestRunner expect_plist (Gee.List<PlayerField> plist_fields, Gee.List<Player> plist) {
    expected_plist_fields = plist_fields;
    expected_plist = plist;
    return this;
  }

  public ProtocolTestRunner expect_error (Error error) {
    expected_error = error;
    return this;
  }

  public void run () {
    try {
      QueryProtocol proto = new QueryLuaProtocol (script_path);
      Gee.Map<string, string> actual_sinfo = new Gee.HashMap<string, string> ();
      Gee.List<PlayerField> actual_plist_fields = new Gee.ArrayList <PlayerField> ();
      Gee.List<Player> actual_plist = new Gee.ArrayList <Player> ();

      proto.data_send.connect ((data) => {
        assert_false (expected_data_flow.is_empty);

        var expected_packet = expected_data_flow.poll ();

        assert_true (expected_packet.direction == REQUEST);
        assert_cmpmem (data, expected_packet.data.get_data ());

        while (!expected_data_flow.is_empty && expected_data_flow.peek ().direction == RESPONSE) {
          try {
            var packet = expected_data_flow.poll ();
            proto.process_response (packet.data.get_data ());
          } catch (Error err) {
            if (expected_error == null) {
              assert_no_error (err);
            } else {
              assert_error (err, expected_error.domain, expected_error.code);
            }
          }
        }
      });

      proto.sinfo_update.connect ((sinfo) => {
        actual_sinfo = sinfo;
      });

      proto.plist_update.connect ((plist_fields, plist) => {
        actual_plist_fields = plist_fields;
        actual_plist = plist;
      });

      proto.query ();

      assert_sinfo (actual_sinfo);
      assert_plist_fields (actual_plist_fields);
      assert_plist (actual_plist);
    } catch (Error err) {
      if (expected_error == null) {
        assert_no_error (err);
      } else {
        assert_error (err, expected_error.domain, expected_error.code);
      }
    }
  }

  private void assert_sinfo (Gee.Map<string, string> actual_sinfo) {
    assert_hashmap ("sinfo", actual_sinfo, expected_sinfo);
    assert_cmpuint (actual_sinfo.size, EQ, expected_sinfo.size);

    foreach (var entry in actual_sinfo.entries) {
      assert_true (expected_sinfo.has (entry.key, entry.value));
    }
  }

  private void assert_plist_fields (Gee.List<PlayerField> actual_plist_fields) {
    assert_cmpuint (actual_plist_fields.size, EQ, expected_plist_fields.size);

    for (var i = 0; i < actual_plist_fields.size; i++) {
      assert_cmpstr (actual_plist_fields[i].title, EQ, expected_plist_fields[i].title);
      assert_cmpstr (actual_plist_fields[i].field, EQ, expected_plist_fields[i].field);
      assert_cmpint (actual_plist_fields[i].kind, EQ, expected_plist_fields[i].kind);
    }
  }

  private void assert_plist (Gee.List<Player> actual_plist) {
    assert_cmpuint (actual_plist.size, EQ, expected_plist.size);

    for (var i = 0; i < actual_plist.size; i++) {
      assert_hashmap (@"plist[$(i)]", actual_plist[i], expected_plist[i]);
    }
  }
}

private void assert_hashmap (string hash_map_name, Gee.Map<string, string> actual, Gee.Map<string, string> expected) {
  foreach (var entry in expected) {
    if (!(actual.has_key (entry.key) && actual[entry.key] == expected[entry.key])) {
      Test.message (
        "FAIL: expected '%s' to have key/value %s = '%s', got '%s'",
        hash_map_name, entry.key, entry.value, actual[entry.key]
      );
    }
  }

  foreach (var entry in actual) {
    if (!expected.has_key (entry.key)) {
      Test.message (
        "FAIL: unexpected actual '%s' to have key %s",
        hash_map_name, entry.key
      );
    }
  }

  foreach (var entry in expected) {
    assert_true (actual.has (entry.key, entry.value));
  }

  foreach (var entry in actual) {
    if (!expected.has_key (entry.key)) {
      assert_false (actual.has_key (entry.key));
    }
  }
}

}
