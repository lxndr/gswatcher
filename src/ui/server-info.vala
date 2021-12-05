using Gtk;

namespace Gsw.Ui {

[GtkTemplate (ui = "/org/lxndr/gswatcher/ui/server-info.ui")]
class ServerInfo : Widget {
  private Querier? _querier;

  [GtkChild]
  private unowned Label name_ctl;

  [GtkChild]
  private unowned Label address_ctl;

  [GtkChild]
  private unowned Label location_ctl;

  [GtkChild]
  private unowned Label game_ctl;

  [GtkChild]
  private unowned Label game_mode_ctl;

  [GtkChild]
  private unowned Label map_ctl;

  [GtkChild]
  private unowned Label players_ctl;

  [GtkChild]
  private unowned Label private_ctl;

  [GtkChild]
  private unowned Label secure_ctl;

  [GtkChild]
  private unowned Label version_ctl;

  [GtkChild]
  private unowned Label error_ctl;

  class construct {
    set_layout_manager_type (typeof (BinLayout));
    set_css_name ("server-info");
  }

  construct {
    update_sinfo ();
    update_error ();
  }

  public override void dispose () {
    get_first_child ().unparent ();
    base.dispose ();
  }

  public Querier? querier {
    get {
      return _querier;
    }

    set {
      if (_querier != null) {
        _querier.sinfo.notify.disconnect (update_sinfo);
        _querier.notify["sinfo"].disconnect (update_sinfo);
        _querier.notify["error"].disconnect (update_error);
      }

      _querier = value;

      if (_querier != null) {
        _querier.notify["sinfo"].connect (() => {
          update_sinfo ();
          _querier.sinfo.notify.connect (update_sinfo);
        });
  
        _querier.sinfo.notify.connect (update_sinfo);
        _querier.notify["error"].connect (update_error);
      }

      update_sinfo ();
      update_error ();
    }
  }

  private void update_sinfo () {
    name_ctl.label = (_querier?.sinfo?.server_name != null)
      ? _querier.sinfo.server_name
      : _("N/A");

    address_ctl.label = _querier != null
      ? querier.server.address
      : _("N/A");

    location_ctl.label = _querier != null
      ? format_location ()
      : _("N/A");

    game_ctl.label = (_querier?.sinfo?.game_name != null)
      ? _querier.sinfo.game_name
      : _("N/A");

    game_mode_ctl.label = (_querier?.sinfo?.game_mode != null)
      ? _querier.sinfo.game_mode
      : _("N/A");

    map_ctl.label = (_querier?.sinfo?.map != null)
      ? _querier.sinfo.map
      : _("N/A");

    players_ctl.label = (_querier?.sinfo != null)
      ? format_players (_querier.sinfo)
      : _("N/A");

    private_ctl.label = (_querier?.sinfo?.private != null)
      ? format_boolean (_querier.sinfo.private)
      : _("N/A");

    secure_ctl.label = (_querier?.sinfo?.secure != null)
      ? format_boolean (_querier.sinfo.secure)
      : _("N/A");

    version_ctl.label = (_querier?.sinfo?.game_version != null)
      ? _querier.sinfo.game_version
      : _("N/A");
  }

  private void update_error () {
    error_ctl.visible = _querier?.error != null;
    error_ctl.label = _querier?.error?.message;
  }

  private string format_location () {
    var addr = get_querier_ip_address (querier);

    if (addr == null)
      return _("N/A");

    var geoip_resolver = GeoIPResolver.get_instance ();
    var location = geoip_resolver.city_by_addr (addr);
    return location;
  }

  private string format_players (Gsw.ServerInfo sinfo) {
    var num = sinfo.num_players;
    var max = sinfo.max_players;
    var num_s = num < 0 ? "-" : num.to_string ();
    var max_s = max < 0 ? "-" : max.to_string ();
    return @"$(num_s) / $(max_s)";
  }

  private string format_boolean (bool val) {
    return val ? _("Yes") : _("No");
  }
}

}
