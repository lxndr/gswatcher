namespace Gsw {

[SingleInstance]
class GameResolver : Object {
  public bool ready { get; private set; }

  private Gee.Map<string, Game> games = new Gee.HashMap<string, Game> ();

  construct {
    load_games.begin ((obj, res) => {
      try {
        load_games.end (res);
        ready = true;
      } catch (Error err) {
        log (Config.LOG_DOMAIN, LEVEL_ERROR, "Failed to load game definitions: %s", err.message);
      }
    });
  }

  public static GameResolver get_instance () {
    return new GameResolver ();
  }

  private async void load_games () throws Error {
    var ini_files = yield get_data_files_contents ("games", "GSW_GAMES_DIR", null);

    foreach (var ini_file in ini_files) {
      try {
        var kf = new KeyFile ();
        kf.set_list_separator (',');
        kf.load_from_bytes (new Bytes.static (ini_file.data), NONE);
        var game = load_game (kf);
        games[game.id] = game;
      } catch (Error err) {
        log (Config.LOG_DOMAIN, LEVEL_ERROR, "Failed to load game definition from '%s': %s", ini_file.file.get_path (), err.message);
      }
    }
  }

  private Game load_game (KeyFile kf) throws Error {
    var game_id = kf.get_value ("Game", "id");
    var protocol = kf.get_value ("Game", "protocol");
    var game = new Game (game_id, protocol);

    foreach (var group in kf.get_groups ()) {
      switch (group) {
        case "Games":
          if (kf.has_key ("Game", "port")) {
            var port = kf.get_integer ("Game", "port");
            if (!(port > 0 && port <= uint16.MAX))
              throw new KeyFileError.INVALID_VALUE ("invalid port value");
            game.port = (uint16) port;
          }

          if (kf.has_key ("Game", "qport-diff")) {
            var qport_diff = kf.get_integer ("Game", "qport-diff");
            if (!(qport_diff >= int16.MIN && qport_diff <= int16.MAX))
              throw new KeyFileError.INVALID_VALUE ("invalid qport-diff value");
            game.qport_diff = (int16) qport_diff;
          }

        break;
      case "Features":
        if (kf.has_key ("Features", "console"))
          game.features.set (CONSOLE, kf.get_string ("Features", "console"));
        break;
      case "Match":
        foreach (var key in kf.get_keys ("Match")) {
          var value = kf.get_string ("Match", key);
          var parts = key.split (".", 2);
          var key1 = parts[0];
          var key2 = parts[1];

          var parser = new ExpressionParser (value);

          switch (key1) {
            case "inf":
              game.inf_matches[key2] = parser.parse ();
              break;
          }
        }

        break;
      case "Info":
        if (kf.has_group ("Info")) {
          foreach (var key in kf.get_keys ("Info")) {
            var val = kf.get_string ("Info", key);
            var parser = new ExpressionParser (val);
            game.inf[key] = parser.parse ();
          }
        }

        break;
      case "Player":
        if (kf.has_group ("Player"))
          load_player_fields (kf, ref game);
        break;
      default:
        var map = new Gee.HashMap<string, string> ();

        foreach (var key in kf.get_keys (group))
          map.set (key, kf.get_string (group, key));

        game.maps[group] = map;
        break;
      }
    }

    return game;
  }

  private void load_player_fields (KeyFile kf, ref Game game) throws Error {
    foreach (var key in kf.get_keys ("Player")) {
      // title = field;type;main
      var options = kf.get_string_list ("Player", key);

      var field = new PlayerField () {
        field = options[0],
        title = key
      };

      if (options.length >= 2)
        field.kind = PlayerFieldType.parse_nick (options[1]);

      if (options.length >= 3 && options[2] == "main")
        field.main = true;

      game.pfields.add (field);
    }
  }

  public bool resolve (string protocol_id, ServerInfo sinfo, Gee.Map<string, string> details) {
    var game = games.values.first_match (game => game.matches (protocol_id, details));

    if (game == null)
      return false;

    game.enhance_sinfo (sinfo, details);
    return true;
  }

  public Gee.List<PlayerField>? get_plist_fields (string? game_id) {
    if (!(game_id != null && games.has_key (game_id)))
      return null;

    return games[game_id].pfields;
  }

  public string? get_feature_protocol (string? game_id, ProtocolFeature feature) {
    if (!(game_id != null && games.has_key (game_id)))
      return null;

    var game = games[game_id];
    return game.features[feature];
  }
}

}
