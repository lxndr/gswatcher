namespace Gsw {

public class ServerInfo : Gee.HashMap<string, string> {
  public string name {
    owned get {
      return get ("name");
    }
  }

  public string game_id {
    owned get {
      return get ("game_id");
    }
  }

  public string game_name {
    owned get {
      return get ("game_name");
    }
  }

  public string? game_mode {
    owned get {
      return get ("game_mode");
    }
  }

  public string map {
    owned get {
      return get ("map");
    }
  }

  public int num_players {
    get {
      return get_player_count ("num_players");
    }
  }

  public int max_players {
    get {
      return get_player_count ("max_players");
    }
  }

  public bool has_password {
    get {
      return get_boolean_value ("has_password", false);
    }
  }

  public bool secure {
    get {
      return get_boolean_value ("secure", false);
    }
  }

  private int get_player_count (string key) {
    return has_key (key)
      ? int.parse (@get (key))
      : -1;
  }

  private bool get_boolean_value (string key, bool def) {
    if (!has_key (key)) {
      return def;
    }

    var value = @get (key);
    return string_to_bool (value, def);
  }
}

}
