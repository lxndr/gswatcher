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

using Gsw;

void add_match_rule (Game game, string key, string pattern) {
  try {
    var parser = new GameDef.ExpressionParser (pattern);
    game.inf_matches[key] = parser.parse ();
  } catch (Error err) {
    assert_no_error (err);
  }
}

void add_inf_rule (Game game, string sinfo_key, string pattern) {
  try {
    var parser = new GameDef.ExpressionParser (pattern);
    var expr = parser.parse ();
    assert_true (expr is GameDef.EvaluatableExpression);
    game.inf[sinfo_key] = (GameDef.EvaluatableExpression) expr;
  } catch (Error err) {
    assert_no_error (err);
  }
}

public static int main (string[] args) {
  Test.init (ref args);

  Test.add_func ("/core/Game/matches()/protocol_mismatch", () => {
    var game = new Game ("cs2", "source");
    add_match_rule (game, "appid", "730");

    var details = new ServerDetails ();
    details["appid"] = "730";

    assert_false (game.matches ("quake3", details));
  });

  Test.add_func ("/core/Game/matches()/no_rules", () => {
    var game = new Game ("generic", "source");
    var details = new ServerDetails ();

    assert_true (game.matches ("source", details));
  });

  Test.add_func ("/core/Game/matches()/single_literal_match", () => {
    var game = new Game ("cs2", "source");
    add_match_rule (game, "appid", "730");

    var details = new ServerDetails ();
    details["appid"] = "730";

    assert_true (game.matches ("source", details));
  });

  Test.add_func ("/core/Game/matches()/single_literal_no_match", () => {
    var game = new Game ("cs2", "source");
    add_match_rule (game, "appid", "730");

    var details = new ServerDetails ();
    details["appid"] = "240";

    assert_false (game.matches ("source", details));
  });

  Test.add_func ("/core/Game/matches()/all_rules_must_match", () => {
    var game = new Game ("cs2", "source");
    add_match_rule (game, "appid", "730");
    add_match_rule (game, "game", "\"Counter-Strike 2\"");

    var matching = new ServerDetails ();
    matching["appid"] = "730";
    matching["game"] = "Counter-Strike 2";

    assert_true (game.matches ("source", matching));

    var wrong_appid = new ServerDetails ();
    wrong_appid["appid"] = "240";
    wrong_appid["game"] = "Counter-Strike 2";

    assert_false (game.matches ("source", wrong_appid));

    var wrong_game = new ServerDetails ();
    wrong_game["appid"] = "730";
    wrong_game["game"] = "Counter-Strike";

    assert_false (game.matches ("source", wrong_game));
  });

  Test.add_func ("/core/Game/matches()/logical_or_rule", () => {
    var game = new Game ("example", "source");
    add_match_rule (game, "mode", "\"competitive\" or \"casual\"");

    var competitive = new ServerDetails ();
    competitive["mode"] = "competitive";

    assert_true (game.matches ("source", competitive));

    var casual = new ServerDetails ();
    casual["mode"] = "casual";

    assert_true (game.matches ("source", casual));

    var other = new ServerDetails ();
    other["mode"] = "deathmatch";

    assert_false (game.matches ("source", other));
  });

  Test.add_func ("/core/Game/matches()/missing_detail_key", () => {
    var game = new Game ("cs2", "source");
    add_match_rule (game, "appid", "730");

    var details = new ServerDetails ();

    assert_false (game.matches ("source", details));
  });

  Test.add_func ("/core/Game/matches()/expression_error_returns_false", () => {
    var game = new Game ("cs2", "source");
    add_match_rule (game, "appid", "inf[\"missing\"]");

    var details = new ServerDetails ();
    details["appid"] = "730";

    Test.expect_message (null, LEVEL_WARNING, "*failed to evaluate value of 'appid'*");
    assert_false (game.matches ("source", details));
  });

  Test.add_func ("/core/Game/enhance_sinfo()/sets_game_id", () => {
    var game = new Game ("cs2", "source");
    var sinfo = new ServerInfo ();
    var details = new ServerDetails ();

    game.enhance_sinfo (sinfo, details);

    assert_cmpstr (sinfo.game_id, EQ, "cs2");
  });

  Test.add_func ("/core/Game/enhance_sinfo()/literal_property", () => {
    var game = new Game ("cod", "quake3");
    add_inf_rule (game, "game-name", "\"Call of Duty\"");

    var sinfo = new ServerInfo ();
    var details = new ServerDetails ();

    game.enhance_sinfo (sinfo, details);

    assert_cmpstr (sinfo.game_id, EQ, "cod");
    assert_cmpstr (sinfo.game_name, EQ, "Call of Duty");
  });

  Test.add_func ("/core/Game/enhance_sinfo()/reads_inf_details", () => {
    var game = new Game ("example", "source");
    add_inf_rule (game, "map", "inf[\"mapname\"]");

    var sinfo = new ServerInfo ();
    var details = new ServerDetails ();
    details["mapname"] = "de_dust2";

    game.enhance_sinfo (sinfo, details);

    assert_cmpstr (sinfo.map, EQ, "de_dust2");
  });

  Test.add_func ("/core/Game/enhance_sinfo()/failed_expression_leaves_property", () => {
    var game = new Game ("cs2", "source");
    add_inf_rule (game, "game-name", "map[\"missing\"]");
    add_inf_rule (game, "map", "\"de_dust2\"");

    var sinfo = new ServerInfo () {
      game_name = "unchanged",
    };
    var details = new ServerDetails ();

    Test.expect_message (null, LEVEL_WARNING, "*failed to evaluate value of 'game-name'*");
    game.enhance_sinfo (sinfo, details);

    assert_cmpstr (sinfo.game_name, EQ, "unchanged");
    assert_cmpstr (sinfo.map, EQ, "de_dust2");
    assert_cmpstr (sinfo.game_id, EQ, "cs2");
  });

  Test.run ();
  return 0;
}
