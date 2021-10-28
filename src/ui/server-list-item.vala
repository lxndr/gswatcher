namespace Gsw.Ui {

  const string[] noslots_classes = { "no-slots" };
  const string[] notempty_classes = { "not-empty" };
  const string[] newentry_classes = { "new-entry" };

  abstract class BaseServerListItem : Object {
    public string name { get; protected set; default = ""; }
    public bool name_editable { get; protected set; default = false; }
    public string[]? name_css_classes { get; protected set; default = null; }
    public string game { get; protected set; default = ""; }
    public string? game_icon { get; protected set; default = null; }
    public string map { get; protected set; default = ""; }
    public int num_players { get; protected set; default = -1; }
    public string players { get; protected set; default = ""; }
    public string[]? players_css_classes { get; protected set; default = null; }
    public string ping { get; protected set; default = ""; }
  }

  class NewServerListItem : BaseServerListItem {
    public NewServerListItem () {
      Object (
        name : "Click here to add new server",
        name_editable : true,
        name_css_classes: newentry_classes
      );
    }
  }

  class ServerListItem : BaseServerListItem {
    public Querier querier { get; construct set; }

    construct {
      name = querier.server.address;

      querier.notify["sinfo"].connect (() => {
        var sinfo = querier.sinfo;
        var server = querier.server;

        name = (sinfo.name != null && sinfo.name.length > 0)
          ? sinfo.name : server.address;
        game = sinfo.game_name;
        game_icon = Path.build_filename ("./icons/games", sinfo.game_id + ".png");
        map = sinfo.map;
        num_players = sinfo.num_players;
        players = format_players (sinfo.num_players, sinfo.max_players);
        players_css_classes = format_players_css_classes (sinfo.num_players, sinfo.max_players);
      });

      querier.notify["ping"].connect(() => {
        ping = querier.ping.to_string ();
      });
    }

    public ServerListItem (Querier querier) {
      Object (querier : querier);
    }

    private string format_players (int num, int max) {
      var num_s = num < 0 ? "-" : num.to_string ();
      var max_s = max < 0 ? "-" : max.to_string ();
      return @"$(num_s) / $(max_s)";
    }

    public string[]? format_players_css_classes (int num, int max) {  
      if (num != 0 && max > 0)
        return num >= max ? noslots_classes : notempty_classes;
      return null;
    }
  }

}
