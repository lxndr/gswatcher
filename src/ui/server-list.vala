using Gtk;

namespace Gsw.Ui {

class NewClient : Client {
  public NewClient () {
    Object (server : new Server ("fake:0"));
  }

  public signal void add (string address);
}

[GtkTemplate (ui = "/io/github/lxndr/gswatcher/ui/server-list.ui")]
class ServerList : Widget {
  public ListModel client_list { get; set; }
  public Client? selected { get; protected set; }

  [GtkChild]
  private unowned GLib.ListStore combined_list_store;

  [GtkChild]
  private unowned SortListModel sort_model;

  [GtkChild]
  private unowned SingleSelection selection;

  [GtkChild]
  private unowned ColumnView view;

  [GtkChild]
  private unowned ColumnViewColumn name_column;

  signal void add (string address);
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
    var new_client = new NewClient ();
    new_client.add.connect (address => add (address));

    var new_item_list = new GLib.ListStore (typeof (Client));
    new_item_list.append (new_client);

    combined_list_store.append (sort_model);
    combined_list_store.append (new_item_list);

    view.sort_by_column (name_column, SortType.ASCENDING);
  }

  public override void dispose () {
    get_first_child ().unparent ();
    base.dispose ();
  }

  [GtkCallback]
  private bool is_editor_row (Client? client) {
    if (client == null)
      return false;

    return client is NewClient;
  }

  [GtkCallback]
  private bool invert_boolean (bool b) {
    return !b;
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
    var country_code = geoip_resolver.code_by_addr (ip_address);
    return find_file_in_data_dirs ("./icons/flags/" + country_code + ".png");
  }

  [GtkCallback]
  private string? format_game_icon (string? game_id) {
    if (game_id == null)
      return null;

    return find_file_in_data_dirs ("./icons/games/" + game_id + ".png");
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

  [GtkCallback]
  private void handle_editing_change (Object obj, ParamSpec pspec) {
    // NOTE: `this` is GtkListItem
    var list_item = (ListItem) this;
    var client = (NewClient) list_item.item;
    var label = (Gtk.EditableLabel) obj;

    if (label.editing) {
      label.text = "";
    } else {
      var default_text = _("Click here to add new server");
      var text = label.text;

      if (text != "" && text != default_text)
        client.add (text);

      label.text = default_text;
    }
  }
}

}
