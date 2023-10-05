namespace Gsw {

public class Server : Object {
  public string host;
  public uint16 gport;
  public uint16 qport;
  public uint16 console_port { get; set; default = 0; }
  public string console_password { get; set; default = ""; }

  public Server (string address) {
    if (parse_address (address, out host, out gport, out qport))
      console_port = gport;
  }

  public string address {
    owned get {
      var res = @"$(host):$(gport)";

      if (gport != qport)
        res += @":$(qport)";

      return res;
    }
  }
}

}
