namespace Gsw {

public abstract class NetTransport : Transport {
  public string host { get; construct; }
  public uint16 port { get; construct; }
  public InetSocketAddress? saddr { get; private set; }

  protected Cancellable cancellable = new Cancellable ();

  ~NetTransport () {
    if (cancellable != null && !cancellable.is_cancelled ())
      cancellable.cancel ();
  }

  public bool is_resolved {
    get {
      return saddr != null;
    }
  }

  protected async void resolve () throws Error {
    var resolver = Resolver.get_default ();
    var addresses = yield resolver.lookup_by_name_with_flags_async (host, IPV4_ONLY, cancellable);
  
    if (addresses.length () == 0)
      throw new IOError.NOT_FOUND ("no address found"); // TODO: handle

    var address = addresses.nth_data (0);
    saddr = new InetSocketAddress (address, port);
  }
}

}
