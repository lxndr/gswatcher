namespace Gsw {

public class Server : Object {
  public string host;
  public uint16 gport;
  public uint16 qport;
  public bool favorite { get; set; }
  public string protocol { get; set; }
  public string server_name { get; set; }
  public string game_id { get; set; }

  public Server (string address) {
    parse_address (address, out host, out gport, out qport);
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
