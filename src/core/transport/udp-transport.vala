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
