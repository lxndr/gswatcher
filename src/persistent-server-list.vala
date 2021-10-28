namespace Gsw {

  class PersistentServerList : ServerList {
    public string config_file { get; private construct set; }
    public string root_path { get; private construct set; default = "/org/lxndr/gswatcher/servers/"; }

    private SettingsBackend settings_backend;

    construct {
      config_file = Path.build_filename (Environment.get_user_config_dir (), "gswatcher", "servers.ini");
      settings_backend = SettingsBackend.keyfile_settings_backend_new (config_file, root_path, null);
  
      items_changed.connect (on_items_changed);
      reload ();
    }

    public void on_items_changed (uint position, uint removed, uint added) {
      if (added > 0) {
        for (var i = position; i < position + added; i++) {
          var server = (Server) get_item (i);
          var path = root_path + server.address + "/";
          var server_settings = new Settings.with_backend_and_path ("org.lxndr.gswatcher.Server", settings_backend, path);
          server_settings.bind ("favorite", server, "favorite", SettingsBindFlags.DEFAULT);
        }
      }
  
      if (removed > 0) {
        try {
          for (var i = position; i < position + removed; i++) {
            var server = (Server) get_item (i);
            remove_name (server.address);
          }
        } catch (Error err) {
          log (Config.LOG_DOMAIN, LEVEL_WARNING, "failed to save server list to '%s': %s", config_file, err.message);
        }
      }
    }

    private void reload () {
      try {
        var names = get_names ();
  
        foreach (var name in names) {
          add (name);
        }
      } catch (Error err) {
        log (Config.LOG_DOMAIN, LEVEL_WARNING, "failed to load server list from '%s': %s", config_file, err.message);
      }
    }

    private string[] get_names () throws Error {
      try {
        var kf = new KeyFile ();
        kf.load_from_file (config_file, KeyFileFlags.NONE);
        return kf.get_groups ();
      } catch (FileError err) {
        if (err.code == FileError.NOENT) {
          return {};
        }

        throw err;
      }
    }
  
    private void remove_name (string name) throws Error {
      try {
        var kf = new KeyFile ();
        kf.load_from_file (config_file, KeyFileFlags.NONE);
        kf.remove_group (name);
      } catch (FileError err) {
        if (err.code != FileError.NOENT) {
          throw err;
        }
      }
    }
  }

}
