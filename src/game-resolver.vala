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

  public static GameResolver get_instance () {
    return new GameResolver ();
  }

  construct {
    try {
      games = new Gee.ArrayList<Game?> ();

      var kf_file = File.new_for_uri ("resource:///org/lxndr/gswatcher/games.ini");
      var kf = new KeyFile ();
      kf.load_from_bytes (kf_file.load_bytes (), NONE);

      foreach (var id in kf.get_groups ()) {
        var name = "";
        var protocol = "";
        var inf = new Gee.HashMap<string, string> ();

        foreach (var key in kf.get_keys (id)) {
          var value = kf.get_string (id, key);
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
          continue;
        }

        if (protocol == "") {
          continue;
        }

        games.add (Game () {
          id = id,
          name = name,
          protocol = protocol,
          inf = inf
        });
      }
    } catch (Error err) {
      log (Config.LOG_DOMAIN, LEVEL_ERROR, "Failed to load game definitions: %s", err.message);
    }
  }

  public void resolve (Protocol protocol, ServerInfo inf) {
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
  }
}

}
