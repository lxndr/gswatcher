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

[GtkTemplate (ui = "/io/github/lxndr/gswatcher/ui/server-list.ui")]
class ServerList : Widget {
  public ListModel client_list { get; set; }
  public Client? selected { get; protected set; }

  [GtkChild]
  private unowned SingleSelection selection;

  [GtkChild]
  private unowned ColumnView view;

  [GtkChild]
  private unowned ColumnViewColumn name_column;

  signal void remove (Client client);

  class construct {
    typeof (Gsw.Querier).ensure ();
    typeof (Gsw.ServerInfo).ensure ();

    set_layout_manager_type (typeof (BinLayout));
    set_css_name ("server-list");

    add_binding (
      Gdk.Key.Delete, 0,
      (widget, args) => {
        var server_list = (ServerList) widget;
        var client = (Client) server_list.selection.selected_item;
        var server = client?.server;

        if (server != null)
          server_list.remove (client);

        return true;
      },
      null
    );
  }

  construct {
    view.sort_by_column (name_column, SortType.ASCENDING);

    selection.items_changed.connect ((position, removed, added) => {
      if (view != null) {
        if (added > 0) {
#if GSW_GTK_4_12_SUPPORTED
          view.scroll_to (position, null, SELECT | FOCUS, null);
#endif
          view.grab_focus ();
        }

        if (removed > 0)
          view.grab_focus ();
      }
    });
  }

  public override void dispose () {
    get_first_child ().unparent ();
    base.dispose ();
  }

  [GtkCallback]
  private string format_server_name (string? server_name, string address) {
    return (server_name == null || server_name.length == 0) ? address : server_name;
  }

  [GtkCallback]
  private string? format_country_icon (string? ip_address) {
    if (ip_address == null)
      return null;

    var geoip_resolver = GeoIPResolver.get_instance ();
    var country_code = geoip_resolver.code_by_addr (ip_address).ascii_down ();
    return find_file_in_data_dirs (@"$(Config.ISO_FLAGS_DIR_NAME)/$(country_code).png");
  }

  [GtkCallback]
  private string? format_game_icon (string? game_id) {
    if (game_id == null)
      return null;

    return find_file_in_app_data_dirs (@"$(game_id).$(Config.GAME_ICON_EXT)", "icons/games", "GSW_GAME_ICONS_DIR");
  }

  [GtkCallback]
  private string? format_game_name (string? game_name, string? game_mode) {
    var name = "";

    if (game_name != null)
      name += game_name;

    if (game_mode != null && game_mode.length > 0)
      name += @" ($(game_mode))";

    return name;
  }

  [GtkCallback]
  private string? format_players (int num, int max) {
    return format_num_players (num, max);
  }

  [GtkCallback]
  [CCode (array_length = false)]
  public string[]? format_players_css_classes (int num, int max) {
    if (num != 0 && max > 0) {
      if (num >= max)
        return { "no-slots" };
      else
        return { "not-empty" };
    }

    return null;
  }

  [GtkCallback]
  private string? format_ping (int ping, Error? error) {
    if (error != null) {
      if (error is QuerierError.TIMEOUT) {
        return _("Timed out");
      } else {
        return _("Error");
      }
    }

    if (ping < 0)
      return "";

    return ping.to_string ();
  }

  [GtkCallback]
  [CCode (array_length = false)]
  public string[]? format_ping_css_classes (Error? error) {
    if (error == null)
      return null;
    return { "error" };
  }
}

}
