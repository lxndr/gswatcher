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

public static int main (string[] args) {
  Test.init (ref args);

  Test.add_func ("/core/GameDef/load/valid/full_definition", () => {
    try {
      var game = GameDef.load_from_data ("""
[Game]
id = cs2
protocol = source
port = 27015
qport-diff = 1

[Features]
console = source-console

[Match]
inf.appid = 730
inf.game = "Counter-Strike 2" or "CS2"

[Info]
game-name = "Counter-Strike 2"
game-mode = mapKeyword(inf["keywords"], ",", "Modes", "")

[Player]
Player = player,string,main
Score = score,number

[Modes]
competitive = Competitive

[Extra]
release-year = 2023
""");

      assert_cmpstr (game.id, EQ, "cs2");
      assert_cmpstr (game.protocol, EQ, "source");
      assert_cmpint (game.port, EQ, 27015);
      assert_cmpint (game.qport_diff, EQ, 1);
      assert_cmpstr (game.features[ProtocolFeature.CONSOLE], EQ, "source-console");

      assert_cmpint (game.inf_matches.size, EQ, 2);
      assert_true (game.inf_matches["appid"] is GameDef.EqualsExpression);
      assert_true (game.inf_matches["game"] is GameDef.OrExpression);

      assert_cmpint (game.inf.size, EQ, 2);

      assert_cmpint (game.pfields.size, EQ, 2);
      assert_cmpstr (game.pfields[0].title, EQ, "Player");
      assert_cmpstr (game.pfields[0].field, EQ, "player");
      assert_true (game.pfields[0].main);
      assert_false (game.pfields[1].main);

      assert_true (game.maps.has_key ("Modes"));
      assert_cmpstr (game.maps["Modes"]["competitive"], EQ, "Competitive");
    } catch (Error err) {
      assert_no_error (err);
    }
  });

  Test.add_func ("/core/GameDef/load/error/missing_id", () => {
    try {
      GameDef.load_from_data ("""
[Game]
protocol = source

[Match]
inf.appid = 1
""");
      assert_not_reached ();
    } catch (Error err) {
    }
  });

  Test.add_func ("/core/GameDef/load/error/invalid_port", () => {
    try {
      GameDef.load_from_data ("""
[Game]
id = x
protocol = source
port = 70000

[Match]
inf.appid = 1
""");
      assert_not_reached ();
    } catch (Error err) {
    }
  });

  Test.add_func ("/core/GameDef/load/error/match_key_without_inf_prefix", () => {
    try {
      GameDef.load_from_data ("""
[Game]
id = x
protocol = source

[Match]
appid = 1
""");
      assert_not_reached ();
    } catch (Error err) {
    }
  });

  Test.add_func ("/core/GameDef/load/error/malformed_match_expression", () => {
    try {
      GameDef.load_from_data ("""
[Game]
id = x
protocol = source

[Match]
inf.appid = "unterminated
""");
      assert_not_reached ();
    } catch (Error err) {
    }
  });

  Test.add_func ("/core/GameDef/load/generic/no_match_section", () => {
    try {
      var game = GameDef.load_from_data ("""
[Game]
id = generic-source
protocol = source
""");
      assert_true (game.is_generic);
    } catch (Error err) {
      assert_no_error (err);
    }
  });

  Test.add_func ("/core/GameDef/load/error/empty_match_section", () => {
    try {
      GameDef.load_from_data ("""
[Game]
id = x
protocol = source

[Match]
""");
      assert_not_reached ();
    } catch (Error err) {
    }
  });

  Test.add_func ("/core/GameDef/load/generic/specific_is_not_generic", () => {
    try {
      var game = GameDef.load_from_data ("""
[Game]
id = x
protocol = source

[Match]
inf.appid = 1
""");
      assert_false (game.is_generic);
    } catch (Error err) {
      assert_no_error (err);
    }
  });

  Test.add_func ("/core/GameDef/load/specificity/classification", () => {
    try {
      var game = GameDef.load_from_data ("""
[Game]
id = x
protocol = source

[Match]
inf.appid = 730
inf.game = "a" or "b"
inf.dir = regex("^cs", inf["dir"])
""");

      assert_cmpint (game.specificity.exact, EQ, 1);
      assert_cmpint (game.specificity.alternatives, EQ, 1);
      assert_cmpint (game.specificity.patterns, EQ, 1);
    } catch (Error err) {
      assert_no_error (err);
    }
  });

  Test.add_func ("/core/GameDef/load/specificity/ordering", () => {
    var two_exact = Specificity () { exact = 2 };
    var one_exact_many_patterns = Specificity () { exact = 1, patterns = 5 };
    assert_true (two_exact.compare (one_exact_many_patterns) > 0);

    var one_alternative = Specificity () { alternatives = 1 };
    var many_patterns = Specificity () { patterns = 3 };
    assert_true (one_alternative.compare (many_patterns) > 0);

    var equal_a = Specificity () { exact = 1 };
    var equal_b = Specificity () { exact = 1 };
    assert_cmpint (equal_a.compare (equal_b), EQ, 0);
  });

  Test.run ();
  return 0;
}
