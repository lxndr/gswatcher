namespace Gsw.Ui {

  const string[] noslots_classes = { "no-slots" };
  const string[] notempty_classes = { "not-empty" };
  const string[] newentry_classes = { "new-entry" };
  const string[] error_classes = { "error" };

  abstract class BaseServerListItem : Object {
    public string name { get; protected set; default = ""; }
    public bool name_editable { get; protected set; }
    public string[]? name_css_classes { get; protected set; }
    public string? country_icon { get; protected set; }
    public string game { get; protected set; default = ""; }
    public string? game_icon { get; protected set; }
    public string map { get; protected set; default = ""; }
    public int num_players { get; protected set; default = -1; }
    public string players { get; protected set; default = ""; }
    public string[]? players_css_classes { get; protected set; }
    public string ping { get; protected set; default = ""; }
    public string[]? ping_css_classes { get; protected set; }
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

      update_sinfo ();
      querier.sinfo.notify.connect (update_sinfo);
      querier.notify["sinfo"].connect (() => {
        update_sinfo ();
        querier.sinfo.notify.connect (update_sinfo);
      });

      querier.notify["ping"].connect(update_ping);
      update_ping ();

      querier.notify["error"].connect(update_error);
      update_error ();
    }

    public ServerListItem (Querier querier) {
      Object (querier : querier);
    }

    private void update_sinfo () {
      var sinfo = querier.sinfo;
      var server = querier.server;

      name = sinfo.server_name != null ? sinfo.server_name : server.address;
      game = sinfo.game_name;

      if (sinfo.game_mode != null) {
        game += " (%s)".printf (sinfo.game_mode);
      }

      game_icon = find_file_in_data_dirs ("./icons/games/" + sinfo.game_id + ".png");
      map = sinfo.map;
      num_players = sinfo.num_players;
      players = format_players (sinfo.num_players, sinfo.max_players);
      players_css_classes = format_players_css_classes (sinfo.num_players, sinfo.max_players);

      var addr = get_querier_ip_address (querier);

      if (addr != null) {
        var geoip_resolver = GeoIPResolver.get_instance ();
        var country_code = geoip_resolver.code_by_addr (addr);
        country_icon = find_file_in_data_dirs ("./icons/flags/" + country_code + ".png");
      }
    }

    private void update_ping () {
      ping = querier.ping >= 0
        ? querier.ping.to_string ()
        : "";
    }

    private void update_error () {
      if (querier.error == null) {
        ping = querier.ping.to_string ();
        ping_css_classes = null;
      } else {
        ping = "Error";
        ping_css_classes = error_classes;
      }
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
