namespace Gsw {
  class UdpTransport : Transport {
    private UdpTransportManager manager;

    public InetSocketAddress saddr { get; private set; }

    construct {
      try {
        manager = UdpTransportManager.get_default ();
        manager.register (this);

        var resolver = Resolver.get_default ();
        var addresses = resolver.lookup_by_name (host);
        var address = addresses.nth_data (0);
        saddr = new InetSocketAddress (address, port);

        log (Config.LOG_DOMAIN, LEVEL_DEBUG, "host %s resolved to %s", host, address.to_string ());
      } catch (Error err) {
        error (err.message);
      }
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
