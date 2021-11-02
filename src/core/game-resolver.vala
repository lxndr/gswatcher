namespace Gsw {

struct Game {
  public string id;
  public string name;
  public string protocol;
  public Gee.Map<string, string> inf;
}

[SingleInstance]
class GameResolver : Object {
  private Gee.List<Game?> games;
  private Gee.Map<string, Gee.List<PlayerField>> player_fields_map;

  public static GameResolver get_instance () {
    return new GameResolver ();
  }

  construct {
    try {
      games = new Gee.ArrayList<Game?> ();
      player_fields_map = new Gee.HashMap<string, Gee.List<PlayerField>> ();

      var kf_file = File.new_for_uri ("resource:///org/lxndr/gswatcher/games.ini");
      var kf = new KeyFile ();
      kf.load_from_bytes (kf_file.load_bytes (), NONE);

      foreach (var ini_group in kf.get_groups ()) {
        var ids = ini_group.split (".");

        if (ids.length == 1) {
          var game = load_game (kf, ini_group);

          if (game != null)
            games.add (game);
        } else if (ids.length == 2 && ids[1] == "player") {
          var id = ids[0];
          player_fields_map.set (id, load_player_fields (kf, ini_group, id));
        }
      }
    } catch (Error err) {
      log (Config.LOG_DOMAIN, LEVEL_ERROR, "Failed to load game definitions: %s", err.message);
    }
  }

  private Game? load_game (KeyFile kf, string ini_group) throws Error {
    var name = "";
    var protocol = "";
    var inf = new Gee.HashMap<string, string> ();

    foreach (var key in kf.get_keys (ini_group)) {
      var value = kf.get_string (ini_group, key);
      var parts = key.split (".");
      var key1 = parts[0];
      var key2 = parts[1];

      switch (key1) {
        case "name":
          name = value;
          break;
        case "protocol":
          protocol = value;
          break;
        case "inf":
          inf.set (key2, value);
          break;
      }
    }

    if (name == "") {
      return null;
    }

    if (protocol == "") {
      return null;
    }

    return Game () {
      id = ini_group,
      name = name,
      protocol = protocol,
      inf = inf
    };
  }

  private Gee.List<PlayerField> load_player_fields (KeyFile kf, string ini_group, string id) throws Error {
    var fields = new Gee.ArrayList<PlayerField> ();

    foreach (var key in kf.get_keys (ini_group)) {
      var options = kf.get_string (ini_group, key).split (",");
      var field = new PlayerField ();
      field.field = key;
      field.title = options[0];

      if (options.length >= 2) {
        field.kind = PlayerFieldType.parse_nick (options[1]);
      }

      if (options.length >= 3 && options[2] == "main") {
        field.main = true;
      }

      fields.add (field);
    }

    return fields;
  }

  public void resolve (Protocol protocol, ServerInfo inf, out Gee.List<PlayerField> player_fields) {
    var game = games.first_match ((game) => {
      if (game.protocol != protocol.info.id) {
        return false;
      }

      foreach (var entry in game.inf.entries) {
        if (inf.get (entry.key) != entry.value) {
          return false;
        }
      }

      return true;
    });

    if (game != null) {
      inf.set ("game_id", game.id);
      inf.set ("game_name", game.name);
    }

    player_fields = player_fields_map[game.id];

    if (player_fields == null) {
      player_fields = player_fields_map[protocol.info.id];

      if (player_fields == null) {
        player_fields = new Gee.ArrayList<PlayerField> ();
      }
    }
  }
}

}
