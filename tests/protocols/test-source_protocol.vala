/*
  Game Server Watcher
  Copyright (C) 2025  Alexander Burobin

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
  Environment.set_variable ("GSW_PROTOCOLS_DIR", "../data/protocols", true);

  Test.init (ref args);

  Test.add_data_func ("/protocols/source/without_challenge", () => {
    try {
      var sinfo = create_sinfo ();
      var plist_fields = create_plist_fields ();
      var plist = create_plist ();

      new ProtocolTestRunner ("../data/protocols/source.lua")
        // query server info
        .expect_data (sinfo_request)
        .response (sinfo_response)
        // query player list
        .expect_data (plist_request)
        .response (plist_response)
        .expect_sinfo(sinfo)
        .expect_plist(plist_fields, plist)
        .run ();
    } catch (Error err) {
      assert_no_error (err);
    }
  });

  Test.add_data_func ("/protocols/source/with_challenge", () => {
    try {
      var sinfo = create_sinfo ();
      var plist_fields = create_plist_fields ();
      var plist = create_plist ();

      new ProtocolTestRunner ("../data/protocols/source.lua")
        // query server info
        .expect_data (sinfo_request)
        .response (challenge_response)
        // query server info with challenge
        .expect_data (sinfo_request_with_challenge)
        .response (sinfo_response)
        // query player list
        .expect_data (plist_request)
        .response (challenge_response)
        // query player list with challenge
        .expect_data (plist_request_with_challenge)
        .response (plist_response)
        .expect_sinfo(sinfo)
        .expect_plist(plist_fields, plist)
        .run ();
    } catch (Error err) {
      assert_no_error (err);
    }
  });

  Test.add_data_func ("/protocols/source/the_ship", () => {
    try {
      var sinfo = create_the_ship_sinfo ();
      var plist_fields = create_the_ship_plist_fields ();
      var plist = create_the_ship_plist ();

      new ProtocolTestRunner ("../data/protocols/source.lua")
        // query server info
        .expect_data (sinfo_request)
        .response (the_ship_sinfo_response)
        // query player list
        .expect_data (plist_request)
        .response (the_ship_plist_response)
        .expect_sinfo(sinfo)
        .expect_plist(plist_fields, plist)
        .run ();
    } catch (Error err) {
      assert_no_error (err);
    }
  });

  Test.run ();
  return 0;
}

private const uint8[] challenge_response = {
  0xFF, 0xFF, 0xFF, 0xFF, 0x41, 0x0A, 0x08, 0x5E, 0xEA
};

private const uint8[] sinfo_request = {
  0xFF, 0xFF, 0xFF, 0xFF, 0x54, 0x53, 0x6F, 0x75, 0x72, 0x63, 0x65, 0x20, 0x45, 0x6E, 0x67, 0x69,
  0x6E, 0x65, 0x20, 0x51, 0x75, 0x65, 0x72, 0x79, 0x00, 0xFF, 0xFF, 0xFF, 0xFF
};

private const uint8[] sinfo_request_with_challenge = {
  0xFF, 0xFF, 0xFF, 0xFF, 0x54, 0x53, 0x6F, 0x75, 0x72, 0x63, 0x65, 0x20, 0x45, 0x6E, 0x67, 0x69,
  0x6E, 0x65, 0x20, 0x51, 0x75, 0x65, 0x72, 0x79, 0x00, 0x0A, 0x08, 0x5E, 0xEA
};

private const uint8[] sinfo_response = {
  0xFF, 0xFF, 0xFF, 0xFF, 0x49, 0x02, 0x67, 0x61, 0x6D, 0x65, 0x32, 0x78, 0x73, 0x2E, 0x63, 0x6F,
  0x6D, 0x20, 0x43, 0x6F, 0x75, 0x6E, 0x74, 0x65, 0x72, 0x2D, 0x53, 0x74, 0x72, 0x69, 0x6B, 0x65,
  0x20, 0x53, 0x6F, 0x75, 0x72, 0x63, 0x65, 0x20, 0x23, 0x31, 0x00, 0x64, 0x65, 0x5F, 0x64, 0x75,
  0x73, 0x74, 0x00, 0x63, 0x73, 0x74, 0x72, 0x69, 0x6B, 0x65, 0x00, 0x43, 0x6F, 0x75, 0x6E, 0x74,
  0x65, 0x72, 0x2D, 0x53, 0x74, 0x72, 0x69, 0x6B, 0x65, 0x3A, 0x20, 0x53, 0x6F, 0x75, 0x72, 0x63,
  0x65, 0x00, 0xF0, 0x00, 0x05, 0x10, 0x04, 0x64, 0x6C, 0x00, 0x00, 0x31, 0x2E, 0x30, 0x2E, 0x30,
  0x2E, 0x32, 0x32, 0x00
};

private const uint8[] plist_request = {
  0xFF, 0xFF, 0xFF, 0xFF, 0x55, 0xFF, 0xFF, 0xFF, 0xFF
};

private const uint8[] plist_request_with_challenge = {
  0xFF, 0xFF, 0xFF, 0xFF, 0x55, 0x0A, 0x08, 0x5E, 0xEA
};

private const uint8[] plist_response = {
  0xFF, 0xFF, 0xFF, 0xFF, 0x44, 0x02, 0x01, 0x5B, 0x44, 0x5D, 0x2D, 0x2D, 0x2D, 0x2D, 0x3E, 0x54,
  0x2E, 0x4E, 0x2E, 0x57, 0x3C, 0x2D, 0x2D, 0x2D, 0x2D, 0x00, 0x0E, 0x00, 0x00, 0x00, 0xB4, 0x97,
  0x00, 0x44, 0x02, 0x4B, 0x69, 0x6C, 0x6C, 0x65, 0x72, 0x20, 0x21, 0x21, 0x21, 0x00, 0x05, 0x00,
  0x00, 0x00, 0x69, 0x24, 0xD9, 0x43
};

private const uint8[] the_ship_sinfo_response = {
  0xFF, 0xFF, 0xFF, 0xFF, 0x49, 0x07, 0x53, 0x68, 0x69, 0x70, 0x20, 0x53, 0x65, 0x72, 0x76, 0x65,
  0x72, 0x00, 0x62, 0x61, 0x74, 0x61, 0x76, 0x69, 0x65, 0x72, 0x00 ,0x73, 0x68, 0x69, 0x70, 0x00,
  0x54, 0x68, 0x65, 0x20, 0x53, 0x68, 0x69, 0x70, 0x00, 0x60, 0x09, 0x01, 0x05, 0x00, 0x6C, 0x77,
  0x00, 0x00, 0x01, 0x03, 0x03, 0x31, 0x2E, 0x30, 0x2E, 0x30, 0x2E, 0x34, 0x00
};

private const uint8[] the_ship_plist_response = {
  0xFF, 0xFF, 0xFF, 0xFF, 0x44, 0x06, 0x00, 0x53, 0x68, 0x69, 0x70, 0x6D, 0x61, 0x74, 0x65, 0x31,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0xBF, 0x01, 0x53, 0x68, 0x69, 0x70, 0x6D, 0x61,
  0x74, 0x65, 0x32, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0xBF, 0x02, 0x53, 0x68, 0x69,
  0x70, 0x6D, 0x61, 0x74, 0x65, 0x33, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0xBF, 0x03,
  0x53, 0x68, 0x69, 0x70, 0x6D, 0x61, 0x74, 0x65, 0x34, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x80, 0xBF, 0x04, 0x53, 0x68, 0x69, 0x70, 0x6D, 0x61, 0x74, 0x65, 0x35, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x80, 0xBF, 0x07, 0x28, 0x31, 0x29, 0x4C, 0x61, 0x6E, 0x64, 0x4C, 0x75, 0x62,
  0x62, 0x65, 0x72, 0x00, 0x00, 0x00, 0x00, 0x00, 0xD3, 0x8E, 0x68, 0x45, 0x00, 0x00, 0x00, 0x00,
  0xC4, 0x09, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xC4, 0x09, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0xC4, 0x09, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xC4, 0x09, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0xC4, 0x09, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xC4, 0x09, 0x00, 0x00
};

private Gee.Map<string, string> create_sinfo () {
  var sinfo = new Gee.HashMap<string, string> ();

  sinfo["protocol_version"] = "2";
  sinfo["name"] = "game2xs.com Counter-Strike Source #1";
  sinfo["map"] = "de_dust";
  sinfo["folder"] = "cstrike";
  sinfo["game"] = "Counter-Strike: Source";
  sinfo["appid"] = "240";
  sinfo["num_players"] = "5";
  sinfo["max_players"] = "16";
  sinfo["num_bots"] = "4";
  sinfo["server_type"] = "d";
  sinfo["environment"] = "l";
  sinfo["visibility"] = "0";
  sinfo["vac"] = "0";
  sinfo["version"] = "1.0.0.22";

  return sinfo;
}

private Gee.List<PlayerField> create_plist_fields () {
  var plist_fields = new Gee.ArrayList<PlayerField> ();

  plist_fields.add (new PlayerField () { title = "Name", field = "name", kind = 0 });
  plist_fields.add (new PlayerField () { title = "Score", field = "score", kind = 1 });
  plist_fields.add (new PlayerField () { title = "Time", field = "duration", kind = 2 });

  return plist_fields;
}

private Gee.List<Player> create_plist () {
  var plist = new Gee.ArrayList<Player> ();

  var player = new Player ();
  player["index"] = "1";
  player["name"] = "[D]---->T.N.W<----";
  player["score"] = "14";
  player["duration"] = "514.370361328125";
  plist.add (player);

  player = new Player ();
  player["index"] = "2";
  player["name"] = "Killer !!!";
  player["score"] = "5";
  player["duration"] = "434.28445434570312";
  plist.add (player);

  return plist;
}

private Gee.Map<string, string> create_the_ship_sinfo () {
  var sinfo = new Gee.HashMap<string, string> ();

  sinfo["protocol_version"] = "7";
  sinfo["name"] = "Ship Server";
  sinfo["map"] = "batavier";
  sinfo["folder"] = "ship";
  sinfo["game"] = "The Ship";
  sinfo["appid"] = "2400";
  sinfo["num_players"] = "1";
  sinfo["max_players"] = "5";
  sinfo["num_bots"] = "0";
  sinfo["server_type"] = "l";
  sinfo["environment"] = "w";
  sinfo["visibility"] = "0";
  sinfo["vac"] = "0";
  sinfo["version"] = "1.0.0.4";
  sinfo["the_ship_mode"] = "1";
  sinfo["the_ship_witnesses"] = "3";
  sinfo["the_ship_duration"] = "3";

  return sinfo;
}

private Gee.List<PlayerField> create_the_ship_plist_fields () {
  var plist_fields = new Gee.ArrayList<PlayerField> ();

  plist_fields.add (new PlayerField () { title = "Name", field = "name", kind = 0 });
  plist_fields.add (new PlayerField () { title = "Score", field = "score", kind = 1 });
  plist_fields.add (new PlayerField () { title = "Time", field = "duration", kind = 2 });
  plist_fields.add (new PlayerField () { title = "Deaths", field = "deaths", kind = 1 });
  plist_fields.add (new PlayerField () { title = "Money", field = "money", kind = 1 });

  return plist_fields;
}

private Gee.List<Player> create_the_ship_plist () {
  var plist = new Gee.ArrayList<Player> ();

  var player = new Player ();
  player["index"] = "0";
  player["name"] = "Shipmate1";
  player["score"] = "0";
  player["duration"] = "-1";
  player["deaths"] = "0";
  player["money"] = "2500";
  plist.add (player);

  player = new Player ();
  player["index"] = "1";
  player["name"] = "Shipmate2";
  player["score"] = "0";
  player["duration"] = "-1";
  player["deaths"] = "0";
  player["money"] = "2500";
  plist.add (player);

  player = new Player ();
  player["index"] = "2";
  player["name"] = "Shipmate3";
  player["score"] = "0";
  player["duration"] = "-1";
  player["deaths"] = "0";
  player["money"] = "2500";
  plist.add (player);

  player = new Player ();
  player["index"] = "3";
  player["name"] = "Shipmate4";
  player["score"] = "0";
  player["duration"] = "-1";
  player["deaths"] = "0";
  player["money"] = "2500";
  plist.add (player);

  player = new Player ();
  player["index"] = "4";
  player["name"] = "Shipmate5";
  player["score"] = "0";
  player["duration"] = "-1";
  player["deaths"] = "0";
  player["money"] = "2500";
  plist.add (player);

  player = new Player ();
  player["index"] = "7";
  player["name"] = "(1)LandLubber";
  player["score"] = "0";
  player["duration"] = "3720.926513671875";
  player["deaths"] = "0";
  player["money"] = "2500";
  plist.add (player);

  return plist;
}
