namespace Gsw {

public abstract class NetTransport : Transport {
  public string host { get; construct; }
  public uint16 port { get; construct; }
  public InetSocketAddress saddr { get; private set; }

  private Cancellable cancellable;

  construct {
    resolve ();
  }

  private void resolve () {
    if (cancellable != null && !cancellable.is_cancelled ())
      cancellable.cancel ();
    cancellable = new Cancellable ();

    var resolver = Resolver.get_default ();
    resolver.lookup_by_name_with_flags_async.begin (host, IPV4_ONLY, cancellable, (obj, res) => {
      try {
        var addresses = resolver.lookup_by_name_with_flags_async.end (res);
  
        if (addresses.length () > 0) {
          var address = addresses.nth_data (0);
          log (Config.LOG_DOMAIN, LEVEL_DEBUG, "host %s resolved to %s", host, address.to_string ());
          saddr = new InetSocketAddress (address, port);
          ready = true;
        }
      } catch (Error err) {
        error (err);
      }  
    });
  }
}

}
