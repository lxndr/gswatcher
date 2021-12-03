namespace Gsw {

public class ServerInfo : Object {
  public string? server_name { get; set; }
  public string? server_type { get; set; }
  public string? server_os { get; set; }
  public string? game_id { get; set; }
  public string? game_name { get; set; }
  public string? game_mode { get; set; }
  public string? game_version { get; set; }
  public string? map { get; set; }
  public int num_players { get; set; default = -1; }
  public int max_players { get; set; default = -1; }
  public bool @private { get; set; }
  public bool secure { get; set; }
}

}
