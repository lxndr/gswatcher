namespace Gsw {

class UdpTransport : Transport {
  private UdpTransportManager manager;

  construct {
    manager = UdpTransportManager.get_default ();
    manager.register (this);
  }

  public UdpTransport (string host, uint16 port) {
    Object (host : host, port : port);
  }

  ~UdpTransport () {
    manager.unregister (this);
  }

  public override void send (uint8[] data) throws Error {
    manager.send (saddr, data);
  }
}

}
