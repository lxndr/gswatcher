namespace Gsw {

class PersistentServerList : ServerList {
  public string config_file { get; private construct set; }
  public string root_path { get; private construct set; default = "/org/lxndr/gswatcher/servers/"; }

  private SettingsBackend settings_backend;

  construct {
    config_file = Path.build_filename (Environment.get_user_config_dir (), "gswatcher", "servers.ini");
    settings_backend = SettingsBackend.keyfile_settings_backend_new (config_file, root_path, null);
  }

  public override Server add (string name) {
    var server = base.add (name);

    var path = root_path + server.address + "/";
    var server_settings = new Settings.with_backend_and_path ("org.lxndr.gswatcher.Server", settings_backend, path);
    server_settings.bind ("favorite", server, "favorite", SettingsBindFlags.DEFAULT);
    server_settings.bind ("protocol", server, "protocol", SettingsBindFlags.DEFAULT);
    server_settings.bind ("game-id", server, "game-id", SettingsBindFlags.DEFAULT);
    server_settings.bind ("server-name", server, "server-name", SettingsBindFlags.DEFAULT);

    return server;
  }

  public override void remove (Server server) {
    try {
      remove_name (server.address);
      base.remove (server);
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

  private void remove_name (string name) throws Error {
    // TODO: is there a better way to remove address besides directly editing ini file?
    var kf = new KeyFile ();
    kf.load_from_file (config_file, KeyFileFlags.NONE);
    if (kf.has_group (name))
      kf.remove_group (name);
    kf.save_to_file (config_file);
  }
}

}
