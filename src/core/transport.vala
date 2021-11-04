namespace Gsw {

public abstract class Transport : Object {
  public string host { get; construct; }
  public uint16 port { get; construct; }
  public InetSocketAddress saddr { get; private set; }

  public signal void receive (uint8[] data);

  construct {
    try {
      var resolver = Resolver.get_default ();
      var addresses = resolver.lookup_by_name (host);
      var address = addresses.nth_data (0);
      saddr = new InetSocketAddress (address, port);

      log (Config.LOG_DOMAIN, LEVEL_DEBUG, "host %s resolved to %s", host, address.to_string ());
    } catch (Error err) {
      error (err.message);
    }
  }

  public abstract void send (uint8[] data) throws Error;
}

}
