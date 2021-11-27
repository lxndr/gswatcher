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
  private unowned Label has_password_ctl;

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
        _querier.notify["sinfo"].disconnect (update_sinfo);
        _querier.notify["error"].disconnect (update_error);
      }

      _querier = value;

      if (_querier != null) {
        _querier.notify["sinfo"].connect (update_sinfo);
        _querier.notify["error"].connect (update_error);
      }

      update_sinfo ();
      update_error ();
    }
  }

  private void update_sinfo () {
    name_ctl.label = (_querier?.sinfo != null && _querier.sinfo.has_key ("name"))
      ? _querier.sinfo["name"]
      : _("N/A");

    address_ctl.label = _querier != null
      ? querier.server.address
      : _("N/A");

    location_ctl.label = _querier != null
      ? format_location ()
      : _("N/A");

    game_ctl.label = (_querier?.sinfo != null && _querier.sinfo.has_key ("game_name"))
      ? _querier.sinfo["game_name"]
      : _("N/A");

    game_mode_ctl.label = (_querier?.sinfo != null && _querier.sinfo.has_key ("game_mode"))
      ? _querier.sinfo["game_mode"]
      : _("N/A");

    map_ctl.label = (_querier?.sinfo != null && _querier.sinfo.has_key ("map"))
      ? _querier.sinfo["map"]
      : _("N/A");

    players_ctl.label = (_querier?.sinfo != null && _querier.sinfo.has_key ("num_players") && _querier.sinfo.has_key ("max_players"))
      ? format_players (_querier.sinfo)
      : _("N/A");

    has_password_ctl.label = (_querier?.sinfo != null && _querier.sinfo.has_key ("has_password"))
      ? format_boolean (_querier.sinfo["has_password"])
      : _("N/A");

    version_ctl.label = (_querier?.sinfo != null && _querier.sinfo.has_key ("version"))
      ? _querier.sinfo["version"]
      : _("N/A");
  }

  private void update_error () {
    error_ctl.visible = _querier?.error != null;
    error_ctl.label = _querier?.error?.message;
  }

  private string format_location () {
    var addr = "0.0.0.0";// querier.transport.saddr.address.to_string ();
    var geoip_resolver = GeoIPResolver.get_instance ();
    var location = geoip_resolver.city_by_addr (addr);
    return location;
  }

  private string format_players (Gsw.ServerInfo sinfo) {
    return "%s / %s".printf (sinfo["num_players"], sinfo["max_players"]);
  }

  private string format_boolean (string val) {
    return string_to_bool (val, false) ? _("Yes") : _("No");
  }
}

}