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

[SingleInstance]
class UdpTransportManager : Object {
  private uint16 _local_port = 27100;
  private Socket socket;
  private SocketSource socket_source;
  private Gee.List<UdpTransport> transport_list = new Gee.ArrayList<UdpTransport> ();

  public signal void error (Error error);

  construct {
    open_socket ();
  }

  ~UdpTransportManager () {
    try {
      socket_source.destroy ();
      socket.close ();
    } catch (Error err) {
      error (err);
    }
  }

  public static UdpTransportManager get_instance () {
    return new UdpTransportManager ();
  }

  public uint16 local_port {
    get {
      return _local_port;
    }

    set {
      if (_local_port != value) {
        _local_port = value;
        open_socket ();
      }
    }
  }

  private void open_socket () {
    try {
      close_socket ();

      var bind_iaddr = new InetAddress.any (SocketFamily.IPV4);
      var bind_saddr = new InetSocketAddress (bind_iaddr, local_port);
      socket = new Socket (SocketFamily.IPV4, SocketType.DATAGRAM, SocketProtocol.UDP);
      socket.bind (bind_saddr, false);
      socket_source = socket.create_source (IOCondition.IN);
      socket_source.set_callback (packet_received);
      socket_source.attach ();

      log (Config.LOG_DOMAIN, LEVEL_INFO, "created listener udp socket: %d", local_port);
    } catch (Error err) {
      error (err);
    }
  }

  private void close_socket () {
    try {
      if (socket_source != null) {
        socket_source.destroy ();
      }

      if (socket != null) {
        socket.close ();
      }
    } catch (Error err) {
      error (err);
    } finally {
      socket_source = null;
      socket = null;
    }
  }

  public void send (InetSocketAddress saddr, uint8[] data) throws Error {
    socket.send_to (saddr, data);
  }

  private bool packet_received (Socket socket, IOCondition condition) {
    try {
      SocketAddress saddr;
      var data = new uint8[socket.get_available_bytes ()];
      socket.receive_from (out saddr, data);

      var addr = (InetSocketAddress) saddr;
      log (Config.LOG_DOMAIN, LEVEL_DEBUG, "received data from %s: length = %ld", addr.to_string (), data.length);

      foreach (var transport in transport_list) {
        if (
          transport.saddr != null &&
          addr.port == transport.saddr.port &&
          addr.address.equal (transport.saddr.address)
        ) {
          transport.data_received (data);
        }
      }
    } catch (Error err) {
      error (err);
    }

    return true;
  }

  public void register (UdpTransport transport) {
    transport_list.add (transport);
  }

  public void unregister (UdpTransport transport) {
    transport_list.remove (transport);
  }
}

}
