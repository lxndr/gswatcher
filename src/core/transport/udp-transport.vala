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

class UdpTransport : NetTransport {
  private UdpTransportManager manager = UdpTransportManager.get_instance ();
  private uint8[]? output_message;

  construct {
    manager.register (this);
  }

  public UdpTransport (string host, uint16 port) {
    Object (host : host, port : port);
  }

  ~UdpTransport () {
    manager.unregister (this);
  }

  private void resolve_address () {
    resolve.begin ((obj, res) => {
      try {
        resolve.end (res);
        flush ();
      } catch (Error err) {
        on_error (err);
      }
    });
  }

  private void send_pending_data () {
    try {
      manager.send (saddr, output_message);
      output_message = null;
    } catch (Error err) {
      on_error (err);
    }
  }

  public override void send (uint8[] data) {
    output_message = data;
    flush ();
  }

  private void flush () {
    if (output_message != null) {
      if (!is_resolved) {
        resolve_address ();
      } else {
        send_pending_data ();
      }
    }
  }

  private void on_error (Error err) {
    error (err);
  }
}

}
