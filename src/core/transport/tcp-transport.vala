namespace Gsw {

class TcpTransport : NetTransport {
  private Socket socket;
  private SocketSource socket_source;
  private Cancellable cancellable;

  public TcpTransport (string host, uint16 port) {
    Object (host : host, port : port);
  }

  ~TcpTransport () {
    close_socket ();
  }

  private void open_socket () {
    try {
      close_socket ();

      cancellable = new Cancellable ();

      socket = new Socket (SocketFamily.IPV4, SocketType.STREAM, SocketProtocol.TCP);
      socket_source = socket.create_source (IOCondition.IN, cancellable);
      socket_source.set_callback (data_received);
      socket_source.attach ();

      socket.connect (saddr, cancellable);
    } catch (Error err) {
      error (err);
    }
  }

  private void close_socket () {
    try {
      if (socket_source != null) {
        socket_source.destroy ();
      }

      if (cancellable != null && !cancellable.is_cancelled ()) {
        cancellable.cancel ();
      }

      if (socket != null) {
        socket.close ();
      }
    } catch (Error err) {
      error (err);
    } finally {
      socket_source = null;
      cancellable = null;
      socket = null;
    }
  }

  public override void send (uint8[] data) throws Error {
    if (socket == null)
      open_socket ();

    socket.send (data, cancellable);
  }

  private bool data_received (Socket socket, IOCondition condition) {
    try {
      if (condition == HUP)
        throw new IOError.FAILED ("the connection has been broken");

      var data = new uint8[socket.get_available_bytes ()];
      socket.receive (data, cancellable);
      receive (data);
    } catch (Error err) {
      error (err);
    }

    return true;
  }
}

}
