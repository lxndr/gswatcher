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

Game make_game (string id, string protocol, bool generic = false) {
  var game = new Game (id, protocol);
  game.is_generic = generic;
  return game;
}

Game load_game (string ini) {
  try {
    return GameDef.load_from_data (ini);
  } catch (Error err) {
    assert_no_error (err);
    assert_not_reached ();
  }
}

public static int main (string[] args) {
  Test.init (ref args);

  Test.add_func ("/core/GameDb/ensure_loaded/waits_for_finish_loading", () => {
    var db = new GameDb ();
    var loop = new MainLoop ();
    var resumed = false;

    db.ensure_loaded.begin ((obj, res) => {
      db.ensure_loaded.end (res);
      resumed = true;
      loop.quit ();
    });

    assert_false (resumed);
    db.finish_loading ();
    loop.run ();
    assert_true (resumed);
  });

  Test.add_func ("/core/GameDb/ensure_loaded/immediate_when_loaded", () => {
    var db = new GameDb ();
    db.finish_loading ();

    var loop = new MainLoop ();
    var resumed = false;

    db.ensure_loaded.begin ((obj, res) => {
      db.ensure_loaded.end (res);
      resumed = true;
      loop.quit ();
    });

    loop.run ();
    assert_true (resumed);
  });

  Test.add_func ("/core/GameDb/ensure_loaded/multiple_waiters", () => {
    var db = new GameDb ();
    var loop = new MainLoop ();
    var resumed = 0;

    for (var i = 0; i < 3; i++) {
      db.ensure_loaded.begin ((obj, res) => {
        db.ensure_loaded.end (res);
        resumed++;

        if (resumed == 3)
          loop.quit ();
      });
    }

    db.finish_loading ();
    loop.run ();
    assert_cmpint (resumed, EQ, 3);
  });

  Test.add_func ("/core/GameDb/add_game/duplicate_generic_rejected", () => {
    var db = new GameDb ();
    db.add_game (make_game ("generic-source", "source", true));

    Test.expect_message (Config.LOG_DOMAIN, LEVEL_WARNING, "*duplicate generic*");
    db.add_game (make_game ("another-generic", "source", true));
  });

  Test.add_func ("/core/GameDb/resolve/layering", () => {
    var db = new GameDb ();

    db.add_game (load_game ("""
[Game]
id = generic-source
protocol = source

[Info]
game-name = inf["game"]
game-mode = inf["mode"]
"""));

    db.add_game (load_game ("""
[Game]
id = cs2
protocol = source

[Match]
inf.appid = 730

[Info]
game-name = "Counter-Strike 2"
"""));

    db.finish_loading ();

    var details = new ServerDetails ();
    details["appid"] = "730";
    details["game"] = "from-generic";
    details["mode"] = "casual";

    var sinfo = new ServerInfo ();
    assert_true (db.resolve ("source", sinfo, details));
    assert_cmpstr (sinfo.game_id, EQ, "cs2");
    assert_cmpstr (sinfo.game_name, EQ, "Counter-Strike 2");
    assert_cmpstr (sinfo.game_mode, EQ, "casual");

    // Second resolve hits the cache; must not use-after-free.
    var sinfo2 = new ServerInfo ();
    assert_true (db.resolve ("source", sinfo2, details));
    assert_cmpstr (sinfo2.game_id, EQ, "cs2");
    assert_cmpstr (sinfo2.game_name, EQ, "Counter-Strike 2");
    assert_cmpstr (sinfo2.game_mode, EQ, "casual");
  });

  Test.add_func ("/core/GameDb/resolve/generic_only", () => {
    var db = new GameDb ();

    db.add_game (load_game ("""
[Game]
id = generic-source
protocol = source

[Info]
game-name = inf["game"]
"""));

    db.finish_loading ();

    var details = new ServerDetails ();
    details["game"] = "Some Mod";

    var sinfo = new ServerInfo ();
    assert_true (db.resolve ("source", sinfo, details));
    assert_cmpstr (sinfo.game_id, EQ, "generic-source");
    assert_cmpstr (sinfo.game_name, EQ, "Some Mod");
  });

  Test.add_func ("/core/GameDb/resolve/no_match_no_generic", () => {
    var db = new GameDb ();

    db.add_game (load_game ("""
[Game]
id = cs2
protocol = source

[Match]
inf.appid = 730
"""));
    db.finish_loading ();

    var details = new ServerDetails ();
    details["appid"] = "1";

    var sinfo = new ServerInfo ();
    assert_false (db.resolve ("source", sinfo, details));
  });

  Test.add_func ("/core/GameDb/resolve/most_specific_wins", () => {
    var db = new GameDb ();

    db.add_game (load_game ("""
[Game]
id = by-pattern
protocol = source

[Match]
inf.appid = regex("(730)", inf["appid"])
"""));
    db.add_game (load_game ("""
[Game]
id = by-literal
protocol = source

[Match]
inf.appid = 730
"""));
    db.finish_loading ();

    var details = new ServerDetails ();
    details["appid"] = "730";

    var sinfo = new ServerInfo ();
    assert_true (db.resolve ("source", sinfo, details));
    assert_cmpstr (sinfo.game_id, EQ, "by-literal");
  });

  Test.add_func ("/core/GameDb/resolve/ambiguity_warns_and_resolves_by_id", () => {
    var db = new GameDb ();

    db.add_game (load_game ("""
[Game]
id = zebra
protocol = source

[Match]
inf.appid = 730
"""));
    db.add_game (load_game ("""
[Game]
id = aardvark
protocol = source

[Match]
inf.appid = 730
"""));
    db.finish_loading ();

    var details = new ServerDetails ();
    details["appid"] = "730";

    Test.expect_message (null, LEVEL_WARNING, "*ambiguous game match*");

    var sinfo = new ServerInfo ();
    assert_true (db.resolve ("source", sinfo, details));
    assert_cmpstr (sinfo.game_id, EQ, "aardvark");
  });

  Test.add_func ("/core/GameDb/get_plist_fields/generic_fallback", () => {
    var db = new GameDb ();

    db.add_game (load_game ("""
[Game]
id = generic-source
protocol = source

[Player]
Player = player,string,main
"""));

    db.add_game (load_game ("""
[Game]
id = cs2
protocol = source

[Match]
inf.appid = 730
"""));
    db.finish_loading ();

    var fields = db.get_plist_fields ("cs2");
    assert_nonnull (fields);
    assert_cmpint (fields.size, EQ, 1);
    assert_cmpstr (fields[0].title, EQ, "Player");

    assert_null (db.get_plist_fields ("unknown"));
    assert_null (db.get_plist_fields (null));
  });

  Test.add_func ("/core/GameDb/get_feature_protocol/generic_fallback", () => {
    var db = new GameDb ();

    db.add_game (load_game ("""
[Game]
id = generic-source
protocol = source

[Features]
console = source-console
"""));

    db.add_game (load_game ("""
[Game]
id = cs2
protocol = source

[Match]
inf.appid = 730
"""));
    db.finish_loading ();

    assert_cmpstr (db.get_feature_protocol ("cs2", ProtocolFeature.CONSOLE), EQ, "source-console");
    assert_null (db.get_feature_protocol ("unknown", ProtocolFeature.CONSOLE));
    assert_null (db.get_feature_protocol (null, ProtocolFeature.CONSOLE));
  });

  Test.run ();
  return 0;
}
