/*
  Game Server Watcher
  Copyright (C) 2010-2026 Alexander Burobin

  This program is free software: you can redistribute it and/or modify
  it under the terms of the GNU Affero General Public License as
  published by the Free Software Foundation, either version 3 of the
  License, or (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU Affero General Public License for more details.

  You should have received a copy of the GNU Affero General Public License
  along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

namespace Gsw {

const string GAME_GROUP = "Game";
const string FEATURES_GROUP = "Features";
const string MATCH_GROUP = "Match";
const string PLAYER_GROUP = "Player";
const string INFO_GROUP = "Info";
const string EXTRA_GROUP = "Extra";

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
        log (Config.LOG_DOMAIN, LEVEL_ERROR, "failed to load game definitions: %s", err.message);
      }
    });
  }

  public static GameResolver get_instance () {
    return new GameResolver ();
  }

  private async void load_games () throws Error {
    var ini_files = yield get_app_data_files_contents ("games", "GSW_GAMES_DIR", null);

    foreach (var ini_file in ini_files) {
      try {
        var ext = get_filename_extension (ini_file.file);

        if (ext != "ini")
          continue;

        var kf = new KeyFile ();
        kf.set_list_separator (',');
        kf.load_from_bytes (new Bytes.static (ini_file.data), NONE);
        var game = load_game (kf);
        games[game.id] = game;
      } catch (Error err) {
        log (Config.LOG_DOMAIN, LEVEL_ERROR, "failed to load game definition from '%s': %s", ini_file.file.get_path (), err.message);
      }
    }
  }

  private Game load_game (KeyFile kf) throws Error {
    var game_id = kf.get_value (GAME_GROUP, "id");
    var protocol = kf.get_value (GAME_GROUP, "protocol");
    var game = new Game (game_id, protocol);

    foreach (var group in kf.get_groups ()) {
      switch (group) {
        case GAME_GROUP:
          if (kf.has_key (GAME_GROUP, "port")) {
            var port = kf.get_integer (GAME_GROUP, "port");
            if (!(port > 0 && port <= uint16.MAX))
              throw new KeyFileError.INVALID_VALUE ("invalid port value");
            game.port = (uint16) port;
          }

          if (kf.has_key (GAME_GROUP, "qport-diff")) {
            var qport_diff = kf.get_integer (GAME_GROUP, "qport-diff");
            if (!(qport_diff >= int16.MIN && qport_diff <= int16.MAX))
              throw new KeyFileError.INVALID_VALUE ("invalid qport-diff value");
            game.qport_diff = (int16) qport_diff;
          }

        break;
      case FEATURES_GROUP:
        if (kf.has_key (FEATURES_GROUP, "console"))
          game.features.set (CONSOLE, kf.get_string (FEATURES_GROUP, "console"));
        break;
      case MATCH_GROUP:
        foreach (var key in kf.get_keys (MATCH_GROUP)) {
          var value = kf.get_string (MATCH_GROUP, key);
          var parts = key.split (".", 2);
          var key1 = parts[0];
          var key2 = parts[1];

          try {
            var parser = new GameDef.ExpressionParser (value);

            switch (key1) {
              case "inf":
                game.inf_matches[key2] = parser.parse ();
                break;
            }
          } catch (GameDef.ExpressionParserError parser_err) {
            throw new IOError.INVALID_DATA ("failed to parse expression at '%s' (group 'Match'): %s", key, parser_err.message);
          }
        }

        break;
      case INFO_GROUP:
        if (kf.has_group (INFO_GROUP)) {
          foreach (var key in kf.get_keys (INFO_GROUP)) {
            var val = kf.get_string (INFO_GROUP, key);

            try {
              var parser = new GameDef.ExpressionParser (val);
              game.inf[key] = parser.parse ();
            } catch (GameDef.ExpressionParserError parser_err) {
              throw new IOError.INVALID_DATA ("failed to parse expression at '%s' (group 'Info'): %s", key, parser_err.message);
            }
          }
        }

        break;
      case PLAYER_GROUP:
        if (kf.has_group (PLAYER_GROUP))
          load_player_fields (kf, ref game);
        break;
      case EXTRA_GROUP:
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
    foreach (var key in kf.get_keys (PLAYER_GROUP)) {
      // title = field;type;main
      var options = kf.get_string_list (PLAYER_GROUP, key);

      var field = new PlayerField () {
        field = options[0],
        title = key
      };

      if (options.length >= 2)
        field.kind = PlayerFieldType.parse_nick (options[1]);

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
    if (game_id == null || !games.has_key (game_id))
      return null;

    return games[game_id].pfields;
  }

  public string? get_feature_protocol (string? game_id, ProtocolFeature feature) {
    if (game_id == null || !games.has_key (game_id))
      return null;

    var game = games[game_id];
    return game.features[feature];
  }
}

}
