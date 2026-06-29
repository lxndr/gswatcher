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

namespace Gsw {

public class ConsoleProtocolTestRunner {
  private enum DataDirection {
    SEND,
    RESPONSE,
    OUTPUT,
  }

  private struct DataPacket {
    public DataDirection direction;
    public Bytes data;
    public string text;
  }

  private string script_path;
  private string test_command = "";
  private Gee.Map<string, string> parameters = new Gee.HashMap<string, string> ();
  private Gee.Queue<DataPacket?> expected_flow = new Gee.ArrayQueue<DataPacket?> ();
  private Error? expected_error = null;

  public ConsoleProtocolTestRunner (string script_path) {
    this.script_path = script_path;
  }

  public ConsoleProtocolTestRunner with_command (string command) {
    this.test_command = command;
    return this;
  }

  public ConsoleProtocolTestRunner param (string key, string value) {
    parameters.set (key, value);
    return this;
  }

  public ConsoleProtocolTestRunner expect_send (uint8[] data) {
    expected_flow.add (DataPacket () {
      direction = SEND,
      data = new Bytes (data),
    });

    return this;
  }

  public ConsoleProtocolTestRunner response (uint8[] data) {
    expected_flow.add (DataPacket () {
      direction = RESPONSE,
      data = new Bytes (data),
    });

    return this;
  }

  public ConsoleProtocolTestRunner expect_response (string text) {
    expected_flow.add (DataPacket () {
      direction = OUTPUT,
      text = text,
    });

    return this;
  }

  public ConsoleProtocolTestRunner expect_error (Error error) {
    expected_error = error;
    return this;
  }

  public void run () {
    try {
      ConsoleProtocol proto = new ConsoleLuaProtocol (script_path);
      var actual_outputs = new Gee.ArrayList<string> ();

      proto.data_send.connect ((data) => {
        assert_false (expected_flow.is_empty);

        var expected_packet = expected_flow.poll ();

        assert_true (expected_packet.direction == SEND);
        expect_memory (data).to_equal (expected_packet.data.get_data ());

        while (!expected_flow.is_empty && expected_flow.peek ().direction == RESPONSE) {
          try {
            var packet = expected_flow.poll ();
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

      proto.response.connect ((line) => {
        assert_false (expected_flow.is_empty);

        var expected_packet = expected_flow.poll ();

        assert_true (expected_packet.direction == OUTPUT);
        assert_cmpstr (line, EQ, expected_packet.text);
        actual_outputs.add (line);
      });

      proto.send_command (test_command, parameters);
      run_main_context ();

      assert_true (expected_flow.is_empty);
    } catch (Error err) {
      if (expected_error == null) {
        assert_no_error (err);
      } else {
        assert_error (err, expected_error.domain, expected_error.code);
      }
    }
  }
}

}
