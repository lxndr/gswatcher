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

namespace Gsw.GameDef {

const string GAME_GROUP = "Game";
const string FEATURES_GROUP = "Features";
const string MATCH_GROUP = "Match";
const string PLAYER_GROUP = "Player";
const string INFO_GROUP = "Info";
const string EXTRA_GROUP = "Extra";

/**
 * Parses game definition INI content into a Game. Pure: performs no I/O.
 */
public Game load_from_data (string data) throws Error {
  return load_from_key_file (create_key_file_from_data (data));
}

/**
 * Parses game definition INI bytes into a Game. Pure: performs no I/O.
 */
public Game load_from_bytes (Bytes bytes) throws Error {
  return load_from_key_file (create_key_file_from_bytes (bytes));
}

/**
 * Parses a KeyFile game definition into a Game. Pure: performs no I/O.
 */
public Game load_from_key_file (KeyFile kf) throws Error {
  var game = new Game (
    kf.get_value (GAME_GROUP, "id"),
    kf.get_value (GAME_GROUP, "protocol")
  );

  foreach (var group in kf.get_groups ()) {
    switch (group) {
      case GAME_GROUP:
        load_game_group (kf, game);
        break;
      case FEATURES_GROUP:
        load_features_group (kf, game);
        break;
      case MATCH_GROUP:
        load_match_group (kf, game);
        break;
      case INFO_GROUP:
        load_info_group (kf, game);
        break;
      case PLAYER_GROUP:
        load_player_group (kf, game);
        break;
      case EXTRA_GROUP:
        break;
      default:
        load_map_group (kf, game, group);
        break;
    }
  }

  if (!kf.has_group (MATCH_GROUP)) {
    game.is_generic = true;
  } else if (game.inf_matches.size == 0) {
    throw new IOError.INVALID_DATA ("game '%s' has no match rules", game.id);
  }

  return game;
}

private KeyFile create_key_file_from_data (string data) throws Error {
  var kf = new KeyFile ();
  kf.set_list_separator (',');
  kf.load_from_data (data, -1, NONE);
  return kf;
}

private KeyFile create_key_file_from_bytes (Bytes bytes) throws Error {
  var kf = new KeyFile ();
  kf.set_list_separator (',');
  kf.load_from_bytes (bytes, NONE);
  return kf;
}

private void load_game_group (KeyFile kf, Game game) throws Error {
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
}

private void load_features_group (KeyFile kf, Game game) throws Error {
  if (kf.has_key (FEATURES_GROUP, "console"))
    game.features.set (CONSOLE, kf.get_string (FEATURES_GROUP, "console"));
}

private void load_match_group (KeyFile kf, Game game) throws Error {
  foreach (var key in kf.get_keys (MATCH_GROUP)) {
    var field = parse_match_field (key);
    var value = kf.get_string (MATCH_GROUP, key);
    var expr = parse_expression (key, MATCH_GROUP, value);

    if (expr is MatchExpression) {
      game.inf_matches[field] = (MatchExpression) expr;
    } else if (expr is EvaluatableExpression) {
      game.inf_matches[field] = new EqualsExpression ((EvaluatableExpression) expr);
    } else {
      throw new IOError.INVALID_DATA (
        "expression at '%s' (group '%s') must be an evaluatable or match expression",
        key, MATCH_GROUP
      );
    }

    classify (game, game.inf_matches[field]);
  }
}

private void classify (Game game, MatchExpression expr) {
  var eq = expr as EqualsExpression;

  if (eq != null && eq.is_literal) {
    game.specificity.exact++;
    return;
  }

  var or_expr = expr as OrExpression;

  if (or_expr != null && or_expr.all_literal) {
    game.specificity.alternatives++;
    return;
  }

  game.specificity.patterns++;
}

private void load_info_group (KeyFile kf, Game game) throws Error {
  foreach (var key in kf.get_keys (INFO_GROUP)) {
    var expr = parse_expression (key, INFO_GROUP, kf.get_string (INFO_GROUP, key));

    if (!(expr is EvaluatableExpression)) {
      throw new IOError.INVALID_DATA (
        "expression at '%s' (group '%s') has to be evaluatable expression",
        key, INFO_GROUP
      );
    }

    game.inf[key] = (EvaluatableExpression) expr;
  }
}

private void load_player_group (KeyFile kf, Game game) throws Error {
  foreach (var key in kf.get_keys (PLAYER_GROUP)) {
    /// title = field;type;main
    var options = kf.get_string_list (PLAYER_GROUP, key);

    var field = new PlayerField () {
      field = options[0],
      title = key
    };

    if (options.length >= 2)
      field.kind = PlayerFieldType.parse_nick (options[1]);

    if (options.length >= 3)
      field.main = options[2] == "main";

    game.pfields.add (field);
  }
}

private void load_map_group (KeyFile kf, Game game, string group) throws Error {
  var map = new Gee.HashMap<string, string> ();

  foreach (var key in kf.get_keys (group))
    map.set (key, kf.get_string (group, key));

  game.maps[group] = map;
}

private string parse_match_field (string key) throws Error {
  var parts = key.split (".", 2);

  if (parts.length < 2 || parts[0] != "inf")
    throw new IOError.INVALID_DATA ("invalid match key '%s' (group '%s'): expected 'inf.<field>'", key, MATCH_GROUP);

  return parts[1];
}

private Expression parse_expression (string key, string group, string value) throws Error {
  try {
    return parse (value);
  } catch (ExpressionParserError parser_err) {
    throw new IOError.INVALID_DATA (
      "failed to parse expression at '%s' (group '%s'): %s",
      key, group, parser_err.message
    );
  }
}

}
