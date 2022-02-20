namespace Gsw {

public class Server : Object {
  public string host;
  public uint16 gport;
  public uint16 qport;
  public string protocol { get; set; }
  public string server_name { get; set; }
  public string game_id { get; set; }
  public uint16 console_port { get; set; }
  public string console_password { get; set; }

  public Server (string address) {
    if (parse_address (address, out host, out gport, out qport))
      if (console_port == 0)
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
