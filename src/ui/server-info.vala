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

using Gtk;

namespace Gsw.Ui {

[GtkTemplate (ui = "/io/github/lxndr/gswatcher/ui/server-info.ui")]
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

    var geoip_resolver = MaxMindLocationResolver.get_instance ();
    return geoip_resolver.location_by_ip (ip_address);
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
