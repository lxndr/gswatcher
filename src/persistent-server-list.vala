namespace Gsw {

class PersistentServerList : ServerList {
  public string config_file { get; private construct set; }
  public string root_path { get; private construct set; default = "/com/github/lxndr/gswatcher/servers/"; }

  private SettingsBackend settings_backend;

  construct {
    config_file = Path.build_filename (Environment.get_user_config_dir (), "gswatcher", "servers.ini");
    settings_backend = SettingsBackend.keyfile_settings_backend_new (config_file, root_path, null);
  }

  public override Client add (string name) {
    var client = base.add (name);

    var path = root_path + client.server.address + "/";
    var server_settings = new Settings.with_backend_and_path (@"$(Config.APPID).Server", settings_backend, path);
    server_settings.bind ("protocol-id", client, "protocol-id", DEFAULT | GET_NO_CHANGES);
    server_settings.bind ("game-id", client.sinfo, "game-id", DEFAULT | GET_NO_CHANGES);
    server_settings.bind ("server-name", client.sinfo, "server-name", DEFAULT | GET_NO_CHANGES);
    server_settings.bind ("console-port", client.server, "console-port", DEFAULT);
    server_settings.bind ("console-password", client.server, "console-password", DEFAULT);

    server_settings.bind_with_mapping (
      "console-command-history",
      client,
      "console-command-history",
      DEFAULT | GET_NO_CHANGES,
      (value, variant, user_data) => {
        var list = new Gee.ArrayList<string>.wrap (variant.get_strv (), (a, b) => a == b);
        value.set_object (list);
        return true;
      },
      (value, expected_type, user_data) => {
        var list = (Gee.ArrayList<string>) value.get_object ();
        return new Variant.strv (list.to_array ());
      },
      null,
      null
    );

    return client;
  }

  public override void remove (Client client) {
    try {
      remove_name (client.server.address);
      base.remove (client);
    } catch (Error err) {
      log (Config.LOG_DOMAIN, LEVEL_WARNING, "failed to save server list to '%s': %s", config_file, err.message);
    }
  }

  public void reload () {
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
    // TODO: is there a better way to get addresses instead of directly read key file?
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

  private void remove_name (string address) throws Error {
    // TODO: is there a better way to remove address besides directly editing ini file?
    var kf = new KeyFile ();
    kf.load_from_file (config_file, KeyFileFlags.NONE);
    if (kf.has_group (address))
      kf.remove_group (address);
    kf.save_to_file (config_file);
  }
}

}
