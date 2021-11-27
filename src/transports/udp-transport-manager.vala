namespace Gsw {

[SingleInstance]
class UdpTransportManager : Object {
  private uint16 _local_port = 27100;
  private Socket socket;
  private SocketSource socket_source;
  private Gee.List<UdpTransport> transport_list;

  public signal void error (Error error);

  construct {
    transport_list = new Gee.ArrayList<UdpTransport> ();
    open_socket ();
  }

  UdpTransportManager () {
    Object ();
  }

  ~UdpTransportManager () {
    try {
      socket_source.destroy ();
      socket.close ();
    } catch (Error err) {
      error (err);
    }
  }

  public static UdpTransportManager get_default () {
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

      foreach (var transport in transport_list) {
        if (
          addr.port == transport.saddr.port &&
          addr.address.equal (transport.saddr.address)
        ) {
          transport.receive (data);
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
