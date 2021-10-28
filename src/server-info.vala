namespace Gsw {

  struct ServerInfo {
    public string name;
    public string game_id;
    public string game_name;
    public string game_mode;
    public string map;
    public int num_players;
    public int max_players;
    public string server_type;
    public string os_type;
    public bool private;
    public bool secure;
    public string version;

    public ServerInfo() {
      num_players = -1;
      max_players = -1;
    }
  }

}
