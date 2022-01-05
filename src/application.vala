namespace Gsw {

  class Application : Gtk.Application {
    private ProtocolRegistry protocol_registry = ProtocolRegistry.get_instance ();
    private TransportRegistry transport_registry = TransportRegistry.get_instance ();
    private UdpTransportManager udp_transport_manager = UdpTransportManager.get_instance ();
    private PersistentServerList server_list;
    private BuddyList buddy_list;
    private QuerierManager querier_manager;
    private Gtk.Window main_window;

    construct {
      add_main_option_entries ({
        { "version", 'v', OptionFlags.NONE, OptionArg.NONE, null, "Display version number", null },
        { null }
      });
    }

    public Application () {
      Object (application_id: Config.APPID);
    }

    private string? pixbuf_dir {
      owned get {
        var data_dirs = Environment.get_system_data_dirs ();

        return data_dirs.length > 0
          ? Path.build_filename (data_dirs[0], "share", "pixmaps")
          : null;
      }
    }

    private string? icon_dir {
      owned get {
        var data_dirs = Environment.get_system_data_dirs ();

        return data_dirs.length > 0
          ? Path.build_filename (data_dirs[0], "share", "gswatcher", "icons")
          : null;
      }
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
        main_window = new Ui.MainWindow (this, querier_manager, buddy_list);
        main_window.show ();
        add_window (main_window);
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

      // queriers
      server_list = new PersistentServerList ();
      querier_manager = new QuerierManager (server_list);

      register_protocols.begin ((obj, res) => {
        register_protocols.end (res);
        server_list.reload ();
      });

      // buddies
      buddy_list = new PersistentBuddyList ();

      // settings
      var settings_file = Path.build_filename (Environment.get_user_config_dir (), "gswatcher", "preferences.ini");
      var settings_backend = SettingsBackend.keyfile_settings_backend_new (settings_file, "/org/lxndr/gswatcher/", null);
      var preferences = new Settings.with_backend ("org.lxndr.gswatcher.Preferences", settings_backend);
      preferences.bind ("local-udp-port", udp_transport_manager, "local-port", SettingsBindFlags.DEFAULT);
      preferences.bind ("query-interval", querier_manager, "update-interval", SettingsBindFlags.DEFAULT);

      // app actions
      ActionEntry[] action_entries = {
        { "about", activate_about },
        { "pause", activate_pause, null, "false" },
        { "show-notification", show_notification }
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

    private void show_notification (SimpleAction action, Variant? parameter) {
      string title, body;
      parameter.get_child (0, "s", out title);
      parameter.get_child (1, "s", out body);
      var n = new Notification (title);
      n.set_body (body);
      send_notification (null, n);
    }

    private async void register_protocols () {
      var files = yield get_data_files ("protocols", "GSW_PROTOCOLS_DIR");

      foreach (var file in files) {
        var script = file.get_path ();

        if (script.substring (-3) != ".js")
          continue;

        try {
          var protocol = new DummyJsProtocol (script);

          var script_value = Value (typeof (string));
          script_value.set_string (script);

          var class_type = typeof (QueryJsProtocol);

          if (protocol.info.feature == CONSOLE)
            class_type = typeof (ConsoleJsProtocol);

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
          log (Config.LOG_DOMAIN, LEVEL_WARNING, "failed to load protocol script '%s': %s", script, err.message);
        }
      }
    }
  }

}
