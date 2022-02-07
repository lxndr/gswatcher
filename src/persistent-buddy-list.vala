namespace Gsw {

class PersistentBuddyList : BuddyList {
  public string config_file { get; private construct set; }
  public string root_path { get; private construct set; default = "/org/lxndr/gswatcher/buddies/"; }

  private SettingsBackend settings_backend;

  construct {
    config_file = Path.build_filename (Environment.get_user_config_dir (), "gswatcher", "buddies.ini");
    settings_backend = SettingsBackend.keyfile_settings_backend_new (config_file, root_path, null);
    reload ();
  }

  public override Buddy add (string name) {
    var buddy = base.add (name);

    var path = root_path + buddy.name + "/";
    var buddy_settings = new Settings.with_backend_and_path ("org.lxndr.gswatcher.Buddy", settings_backend, path);
    buddy_settings.bind ("notifications", buddy, "notifications", SettingsBindFlags.DEFAULT);

    return buddy;
  }

  public override void remove (Buddy buddy) {
    try {
      remove_name (buddy.name);
      base.remove (buddy);
    } catch (Error err) {
      log (Config.LOG_DOMAIN, LEVEL_WARNING, "failed to save buddy list to '%s': %s", config_file, err.message);
    }
  }

  private void reload () {
    try {
      var names = get_names ();

      foreach (var name in names)
        add (name);
    } catch (Error err) {
      log (Config.LOG_DOMAIN, LEVEL_WARNING, "failed to load buddy list from '%s': %s", config_file, err.message);
    }
  }

  private string[] get_names () throws Error {
    // TODO: is there a better way to get names instead of directly read key file?
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
    // TODO: is there a better way to remove name besides directly editing ini file?
    var kf = new KeyFile ();
    kf.load_from_file (config_file, KeyFileFlags.NONE);
    kf.remove_group (name);
    kf.save_to_file (config_file);
  }
}

}
