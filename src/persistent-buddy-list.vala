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
    try {
      add_name (name);
    } catch (Error err) {
      log (Config.LOG_DOMAIN, LEVEL_ERROR, "failed to save buddy list to '%s': %s", config_file, err.message);
    }

    var buddy = base.add (name);

    var path = root_path + buddy.name + "/";
    var buddy_settings = new Settings.with_backend_and_path ("org.lxndr.gswatcher.Buddy", settings_backend, path);
    buddy_settings.bind ("notifications", buddy, "notifications", SettingsBindFlags.DEFAULT);
    buddy_settings.bind ("lastaddr", buddy, "lastaddr", SettingsBindFlags.DEFAULT);

    buddy_settings.bind_with_mapping (
      "lastseen",
      buddy,
      "lastseen",
      SettingsBindFlags.DEFAULT,
      (value, variant, user_data) => {
        var str = variant.get_string ();
        var time = new DateTime.from_iso8601 (str, null);
        value.set_boxed (time);
        return true;
      },
      (value, expected_type, user_data) => {
        var time = (DateTime) value.get_boxed ();
        return new Variant.string (time.format_iso8601 ());
      },
      null,
      null
    );

    return buddy;
  }

  public override void remove (Buddy buddy) {
    try {
      remove_name (buddy.name);
      base.remove (buddy);
    } catch (Error err) {
      log (Config.LOG_DOMAIN, LEVEL_ERROR, "failed to save buddy list to '%s': %s", config_file, err.message);
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

  private void add_name (string name) throws Error {
    // TODO: is there a better way to remove name besides directly editing ini file?
    var kf = new KeyFile ();

    if (FileUtils.test (config_file, FileTest.EXISTS))
      kf.load_from_file (config_file, KeyFileFlags.NONE);

    if (!kf.has_group (name))
      kf.set_boolean (name, "notifications", true);

    kf.save_to_file (config_file);
  }

  private void remove_name (string name) throws Error {
    // TODO: is there a better way to remove name besides directly editing ini file?
    var kf = new KeyFile ();
    kf.load_from_file (config_file, KeyFileFlags.NONE);

    if (kf.has_group (name))
      kf.remove_group (name);

    kf.save_to_file (config_file);
  }
}

}
