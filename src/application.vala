namespace Gsw {

  class Application : Gtk.Application {
    private Settings preferences;
    private ServerList server_list;
    private BuddyList buddy_list;
    private QuerierManager querier_manager;

    private Gtk.Window? main_window;

    construct {
      var settings_backend = SettingsBackend.keyfile_settings_backend_new ("preferences.ini", "/org/lxndr/gswatcher/", null);
      preferences = new Settings.with_backend ("org.lxndr.gswatcher.Preferences", settings_backend);

      // udp transport manager
      var udp_transport_manager = new UdpTransportManager ();
      udp_transport_manager.error.connect((err) => {
        log (Config.LOG_DOMAIN, LEVEL_ERROR, err.message);
      });

      preferences.changed["local-udp-port"].connect(restore_local_udp_port_setting);
      restore_local_udp_port_setting ();

      // queriers
      server_list = new PersistentServerList ();
      querier_manager = new QuerierManager (server_list);

      preferences.changed["query-interval"].connect(restore_query_interval_setting);
      restore_query_interval_setting ();

      // buddies
      buddy_list = new PersistentBuddyList ();

      // command line options
      add_main_option_entries ({
        { "version", 'v', OptionFlags.NONE, OptionArg.NONE, null, "Display version number", null }
      });

      // app actions
      ActionEntry[] action_entries = {
        { "about", activate_about },
      };

      add_action_entries (action_entries, this);
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

    private string server_list_path {
      owned get {
        var config_dir = Environment.get_user_config_dir ();
        return Path.build_filename (config_dir, "serverlist.json");
      }
    }

    private string buddy_list_path {
      owned get {
        var config_dir = Environment.get_user_config_dir ();
        return Path.build_filename (config_dir, "buddylist.json");
      }
    }

    private string preferences_path {
      owned get {
        var config_dir = Environment.get_user_config_dir ();
        return Path.build_filename (config_dir, "preferences.json");
      }
    }

    public double interval { get; set; }
    public bool pause { get; set; }
    public bool enable_notifications { get; set; }

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

      var style_provider = new Gtk.CssProvider();
      style_provider.load_from_resource("/org/lxndr/gswatcher/css/server-list.css");
      Gtk.StyleContext.add_provider_for_display(Gdk.Display.get_default (), style_provider, Gtk.STYLE_PROVIDER_PRIORITY_APPLICATION);

      // Gtk.IconTheme.get_default ().append_search_path (pixmap_dir);

      // Gsq.set_default_local_port (default_port);
      // load_buddy_list ();
    }

    private void activate_about (SimpleAction action, Variant? parameter) {
      Ui.show_about (main_window);
    }

    private void restore_local_udp_port_setting () {
      uint16 local_udp_port;
      preferences.get("local-udp-port", "q", out local_udp_port);

      var udp_transport_manager = new UdpTransportManager ();
      udp_transport_manager.local_port = local_udp_port;
    }

    private void restore_query_interval_setting () {
      var query_interval = preferences.get_uint ("query-interval");
      querier_manager.update_interval = query_interval;
    }

    /*
    public Client add_server (string address) {

    }

    public void remove_server (Client client) {

    }

    public void remove_server_ask (Client client) {
      var dialog = new Gtk.MessageDialog (main_window, Gtk.DialogFlags.DESTROY_WITH_PARENT, Gtk.MessageType.QUESTION, Gtk.ButtonsType.YES_NO, _("Are you sure you want to remove \"%s\" ( %s) from the list?"), client->querier->name, client.address);

      if (dialog.run () == Gtk.ResponseType.YES) {
        gui_slist_remove (client);
        remove_server (client);
      }
    }

    public Client find_server (string address) {
      uint16 gport, qport;
      var host = parse_address (address, out gport, out qport);

      if (host == null)
        return null;

      foreach (var client in server_list) {
        if (host == client.querier.address) {
          var iaddr = client.querier.idrr;

          if (addr == null)
            continue;

          var ip = iaddr.to_string ();

          if (host == ip)
            continue;
        }

        if (gport > 0) {
          if (client.querier.gport == gport)
            return client;
        } else {
          if (client.querier.gport_auto)
            return client;
        }
      }

      return null;
    }

    public void save_server_list ()
    {

    }

    public Buddy add_buddy (string name, int64 lastseen, string? lastaddr, bool notify) {
      var buddy = add_buddy_real (name, lastseen, lastaddr, notify);
      save_buddy_list ();
      return buddy;
    }

    private Buddy add_buddy_real (string name, int64 lastseen, string? lastaddr, bool notify) {
      var buddy = new Buddy ();
      buddy.name = name;
      buddy.lastseen = new DateTime.from_unix_local (lastseen);
      buddy.lastaddr = lastaddr;
      buddy.notify = notify;

      update_buddy (buddy);
      buddy_list.insert (buddy.name, buddy);
      return buddy;
    }

    public void change_buddy (string name, bool notify)
    {
      var buddy = find_buddy (name);
      buddy.notify = notify;
      save_buddy_list ();
    }

    public void remove_buddy (string name) {
      buddy_list.remove (name);
      save_buddy_list ();
    }

    public Buddy find_buddy (string name) {
      return buddy_list.lookup (name);
    }

    private void update_buddy (Buddy buddy) {
      buddy.servers.clear ();

      foreach (var server in server_list) {
        if (server.querier.find_player (buddy.name)) {
          buddy.servers.add (client);
        }
      }
    }

    private void load_buddy_list () {
      try {
        var parser = new Json.Parser ();
        parser.load_from_file (buddy_list_path);

        var root = parser.get_root ();
        var list = root.get_object ();

        if (list) {
          list.for_each ((key, value) => {
            var obj = list.get_object (name);

            if (obj != null) {
              var buddy = add_buddy_real (
                name,
                obj.get_int_member ("lastseen"),
                obj.get_string_member ("lastseen"),
                obj.get_boolean_member ("lastseen")
              );

              gui_blist_add (buddy);
            }
          });
        }
      } catch (Error err) {
        log (null, LEVEL_WARNING, _("Couldn't load buddy list: %s"), err.message);
      }
    }

    private void save_buddy_list ()
    {
      var list = new Json.Object ();

      buddy_list.for_each ((key, value) => {
        var obj = new Json.Object ();
        obj.set_int_member ("lastseen", buddy.lastseen);
        obj.set_string_member ("lastaddr", buddy.lastseen);
        obj.set_boolean_member ("notify", buddy.lastseen);
        obj.set_object_member (buddy.name, obj);
      });

      var root = new Json.Node ();
      root.init_object (list);

      try {
        var gen = new Json.Generator ();
        gen.pretty = true;
        gen.root = root;
        gen.to_file (buddy_list_path);
      } catch (Error err) {
        log (null, LEVEL_WARNING, _("Couldn't save buddy list: %s"), err.message);
      }
    }

  */
  }

}
