namespace Gsw {

  class Server : Object {
    public string host;
    public uint16 gport;
    public uint16 qport;
    public bool favorite { get; set; }

    public Server (string address) {
      parse_address (address, out host, out gport, out qport);
    }

    public string address {
      owned get {
        var builder = new StringBuilder (host);
        builder.append_printf (":%d", gport);

        if (gport != qport) {
          builder.append_printf (":%d", qport);
        }

        return builder.str;
      }
    }
  }

}
