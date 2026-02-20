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

namespace Gsw {

class Application : Adw.Application {
  private ProtocolRegistry protocol_registry = ProtocolRegistry.get_instance ();
  private TransportRegistry transport_registry = TransportRegistry.get_instance ();
  private UdpTransportManager udp_transport_manager = UdpTransportManager.get_instance ();
  private QuerierManager querier_manager = QuerierManager.get_instance ();
  private PersistentServerList server_list;
  private BuddyList buddy_list;
  private BuddyWatcher buddy_watcher;
  private Settings preferences;
  private Gtk.Window main_window;

  construct {
    add_main_option_entries ({
      { "version", 'v', OptionFlags.NONE, OptionArg.NONE, null, _("Display version number"), null },
      { null }
    });
  }

  public Application () {
    Object (application_id: Config.APPID);
  }

  public override int handle_local_options (VariantDict options) {
    var version = options.lookup_value ("version", VariantType.BOOLEAN);

    if (version != null && version.get_boolean ()) {
      print ("%s\n", Config.VERSION);
      return 0;
    }

    return -1;
  }

  public override void activate () {
    if (main_window == null) {
      main_window = new Ui.MainWindow (this, server_list, buddy_list);
      add_window (main_window);

      main_window.close_request.connect (() => {
        if (preferences != null && preferences.get_boolean ("work-in-background")) {
          main_window.hide ();
          return true;
        }

        return false;
      });

      main_window.show ();

      var location_resolver = new MaxMindLocationResolver ();

      if (location_resolver.init_error != null) {
        var msg = _("Failed to initialize GeoIP: %s").printf (location_resolver.init_error.message);
        show_error_dialog (main_window, msg);
      }
    } else {
      main_window.present ();
    }
  }

  public override void startup () {
    base.startup ();

    // udp transport manager
    udp_transport_manager.error.connect((err) => {
      log (Config.LOG_DOMAIN, LEVEL_ERROR, err.message);
    });

    // transports
    transport_registry.register (new TransportDesc () {
      id = "udp",
      class_type = typeof (UdpTransport)
    });
    transport_registry.register (new TransportDesc () {
      id = "tcp",
      class_type = typeof (TcpTransport)
    });

    // servers
    server_list = new PersistentServerList ();

    // protocols
    register_protocols.begin ((obj, res) => {
      register_protocols.end (res);
      server_list.reload ();
    });

    // buddies
    buddy_list = new PersistentBuddyList ();
    buddy_watcher = new BuddyWatcher (server_list, buddy_list);
    buddy_watcher.online.connect (on_buddy_online);

    // settings
    var settings_file = Path.build_filename (Environment.get_user_config_dir (), "gswatcher", "preferences.ini");
    var settings_backend = SettingsBackend.keyfile_settings_backend_new (settings_file, "/io/github/lxndr/gswatcher/", null);
    preferences = new Settings.with_backend (@"$(Config.APPID).Preferences", settings_backend);
    preferences.bind ("local-udp-port", udp_transport_manager, "local-port", DEFAULT);
    preferences.bind ("query-interval", querier_manager, "update-interval", DEFAULT);

    // app actions
    ActionEntry[] action_entries = {
      { "about", activate_about },
      { "pause", activate_pause, null, "false" }
    };

    add_action_entries (action_entries, this);
  }

  private void activate_about (SimpleAction action, Variant? parameter) {
    Ui.show_about (main_window, transport_registry.list (), protocol_registry.list ());
  }

  private void activate_pause (SimpleAction action, Variant? parameter) {
    var paused = !action.get_state ().get_boolean ();
    action.set_state (new Variant.boolean (paused));
    querier_manager.paused = paused;
  }

  private void on_buddy_online (Buddy buddy, Client client) {
    if (!(preferences.get_boolean ("enable-notifications") && buddy.notifications))
      return;

    var title = _("Player %s has connected").printf (buddy.name);

    var text = _("Address: %s\nServer: %s\nPlayers: %d / %d").printf (
      client.server.address,
      client.sinfo.server_name_clean,
      client.sinfo.num_players,
      client.sinfo.max_players
    );

    var n = new Notification (title);
    n.set_body (text);
    send_notification (null, n);
  }

  private async void register_protocols () {
    var files = yield get_app_data_files ("protocols", "GSW_PROTOCOLS_DIR");

    foreach (var file in files) {
      var script = file.get_path ();

      if (script.substring (-4) != ".lua")
        continue;

      try {
        var protocol = new DummyLuaProtocol (script);

        var script_value = Value (typeof (string));
        script_value.set_string (script);

        var class_type = typeof (QueryLuaProtocol);

        if (protocol.info.feature == CONSOLE)
          class_type = typeof (ConsoleLuaProtocol);

        protocol_registry.register (new ProtocolDesc () {
          id = protocol.info.id,
          name = protocol.info.name,
          feature = protocol.info.feature,
          transport = protocol.info.transport,
          class_type = class_type,
          class_params = { "script-path" },
          class_values = { script_value },
        });
      } catch (Error err) {
        if (!(err is LuaProtocolError && err.code == LuaProtocolError.INVALID_PROTOCOL_INFO))
          log (Config.LOG_DOMAIN, LEVEL_WARNING, "failed to load protocol script '%s': %s", script, err.message);
      }
    }
  }
}

}
