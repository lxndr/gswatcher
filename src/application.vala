namespace Gsw {

  class Application : Gtk.Application {
    private ServerList server_list;
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
      var udp_transport_manager = UdpTransportManager.get_default ();
      udp_transport_manager.error.connect((err) => {
        log (Config.LOG_DOMAIN, LEVEL_ERROR, err.message);
      });

      // queriers
      server_list = new PersistentServerList ();
      querier_manager = new QuerierManager (server_list);

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
      Ui.show_about (main_window);
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
  }

}
