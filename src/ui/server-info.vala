using Gtk;

namespace Gsw.Ui {

[GtkTemplate (ui = "/org/lxndr/gswatcher/ui/server-info.ui")]
class ServerInfo : Widget {
  public Client? client { get; set; }

  class construct {
    typeof (Gsw.ServerInfo).ensure ();
    set_layout_manager_type (typeof (BinLayout));
    set_css_name ("server-info");
  }

  public override void dispose () {
    get_first_child ().unparent ();
    base.dispose ();
  }

  [GtkCallback]
  private string na_if_null (string? str) {
    return str == null ? _("n/a") : str;
  }

  [GtkCallback]
  private string format_boolean (bool val) {
    return val ? _("yes") : _("no");
  }

  [GtkCallback]
  private string? format_location (string? ip_address) {
    if (ip_address == null)
      return null;

    var geoip_resolver = GeoIPResolver.get_instance ();
    var location = geoip_resolver.city_by_addr (ip_address);
    return location;
  }

  [GtkCallback]
  private string? format_players (int num, int max) {
    return format_num_players (num, max);
  }

  [GtkCallback]
  private string? format_error (Error? error) {
    if (error == null)
      return null;

    if (error is QuerierError.TIMEOUT)
      return _("Timed out");

    return  error.message;
  }
}

}
