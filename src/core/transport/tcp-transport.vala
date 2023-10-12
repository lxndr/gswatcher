/*
  Game Server Watcher
  Copyright (C) 2023  Alexander Burobin

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

class TcpTransport : NetTransport {
  private Socket? socket;
  private SocketSource? socket_source;
  private ByteArray output_buffer = new ByteArray ();

  public TcpTransport (string host, uint16 port) {
    Object (host : host, port : port);
  }

  ~TcpTransport () {
    close_socket ();
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

  private void open_socket () {
    try {
      socket = new Socket (IPV4, STREAM, DEFAULT);
      socket.blocking = false;
      setup_socket_source (OUT);

      try {
        socket.connect (saddr, cancellable);
      } catch (IOError err) {
        if (err.code != IOError.PENDING)
          throw err;
      }
    } catch (Error err) {
      on_error (err);
    }
  }

  private void close_socket () {
    try {
      if (socket_source != null)
        socket_source.destroy ();

      if (cancellable != null && !cancellable.is_cancelled ())
        cancellable.cancel ();

      if (socket != null)
        socket.close ();
    } catch (Error err) {
      log (Config.LOG_DOMAIN, LEVEL_ERROR, err.message);
    }
  }

  private void setup_socket_source (IOCondition condition) {
    if (socket_source != null)
      socket_source.destroy ();

    socket_source = socket.create_source (condition, cancellable);
    socket_source.set_callback (on_socket_event);
    socket_source.attach ();
  }

  private void send_pending_data () {
    try {
      socket.send (output_buffer.data, cancellable);
      output_buffer.set_size (0);
    } catch (Error err) {
      on_error (err);
    }
  }

  public override void send (uint8[] data) {
    output_buffer.append (data);
    flush ();
  }

  private void flush () {
    if (output_buffer.len > 0) {
      if (!is_resolved) {
        resolve_address ();
      } else if (socket == null) {
        open_socket ();
      } else {
        send_pending_data ();
      }
    }
  }

  private bool on_socket_event (Socket socket, IOCondition cond) {
    try {
      if (HUP in cond)
        throw new IOError.FAILED ("socket has hung up");

      if (ERR in cond)
        throw new IOError.FAILED ("socket error has occured");

      if (OUT in cond) {
        if (socket.check_connect_result ()) {
          connected ();
          setup_socket_source (IN);
          flush ();
          return Source.REMOVE;
        }
      }

      if (IN in cond) {
        var size = socket.get_available_bytes ();

        if (size > 0) {
          var data = new uint8[size];
          socket.receive (data, cancellable);
          data_received (data);
        } else {
          close_socket ();
          disconnected ();
        }
      }

      return Source.CONTINUE;
    } catch (Error err) {
      error (err);
      return Source.REMOVE;
    }
  }

  private void on_error (Error err) {
    error (err);
    close_socket ();
  }
}

}
