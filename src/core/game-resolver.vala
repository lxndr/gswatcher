namespace Gsw {

class Game {
  public string id;
  public string name;
  public string protocol;
  public bool phony;
  public Gee.Map<string, string> inf;
  public Gee.List<PlayerField> pfields;

  public Game (string id) {
    this.id = id;
    inf = new Gee.HashMap<string, string> ();
    pfields = new Gee.ArrayList<PlayerField> ();
  }

  public void extend (Game game) {
    if (name == null)
      name = game.name;

    if (protocol == null)
      protocol = game.protocol;

    inf.set_all (game.inf);
    pfields.add_all (game.pfields);
  }
}

[SingleInstance]
public class GameResolver : Object {
  private Gee.Map<string, Game> games;

  public static GameResolver get_instance () {
    return new GameResolver ();
  }

  construct {
    try {
      games = new Gee.HashMap<string, Game> ();

      var kf_file = File.new_for_uri ("resource:///org/lxndr/gswatcher/games.ini");
      var kf = new KeyFile ();
      kf.load_from_bytes (kf_file.load_bytes (), NONE);

      foreach (var ini_group in kf.get_groups ()) {
        var ini_group_parts = ini_group.split (".");
        var id = ini_group_parts[0];
        var game = ensure_game (id);

        if (ini_group_parts.length == 1)
          load_game (kf, ref game);
        else if (ini_group_parts.length == 2 && ini_group_parts[1] == "player")
          load_player_fields (kf, ref game, ini_group);
      }
    } catch (Error err) {
      log (Config.LOG_DOMAIN, LEVEL_ERROR, "Failed to load game definitions: %s", err.message);
    }
  }

  private Game ensure_game (string id) {
    if (!games.has_key (id))
      games.set (id, new Game (id));
    return games.get (id);
  }

  private void load_game (KeyFile kf, ref Game game) throws Error {
    foreach (var key in kf.get_keys (game.id)) {
      var value = kf.get_string (game.id, key);
      var parts = key.split (".", 2);
      var key1 = parts[0];
      var key2 = parts[1];

      switch (key1) {
        case "extends":
          var base_game = games[value];

          if (base_game == null)
            log (Config.LOG_DOMAIN, LEVEL_WARNING, "Could not find '%s' to extend '%s'", value, game.id);
          else
            game.extend (base_game);

          break;
        case "name":
          game.name = value;
          break;
        case "protocol":
          game.protocol = value;
          break;
        case "phony":
          game.phony = kf.get_boolean (game.id, key);
          break;
        case "inf":
          game.inf.set (key2, value);
          break;
      }
    }
  }

  private void load_player_fields (KeyFile kf, ref Game game, string ini_group) throws Error {
    foreach (var key in kf.get_keys (ini_group)) {
      // field = title;type;main
      var options = kf.get_string_list (ini_group, key);

      var field = new PlayerField () {
        field = key,
        title = options[0]
      };

      if (options.length >= 2)
        field.kind = PlayerFieldType.parse_nick (options[1]);

      if (options.length >= 3 && options[2] == "main")
        field.main = true;

      game.pfields.add (field);
    }
  }

  public bool resolve (Protocol protocol, ServerInfo inf) {
    var game = games.values.first_match ((game) => {
      if (game.phony || game.protocol != protocol.info.id)
        return false;

      foreach (var entry in game.inf.entries)
        if (inf.get (entry.key) != entry.value)
          return false;

      return true;
    });

    if (game == null)
      return false;

    inf.set ("game_id", game.id);
    inf.set ("game_name", game.name);
    return true;
  }

  public Gee.List<PlayerField>? get_player_fields (string game_id) {
    return games.has_key (game_id)
      ? games[game_id].pfields
      : null;
  }
}

}
