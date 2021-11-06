namespace Gsw {

  public void print_duktape_stack (Duktape.Duktape vm) {
    var len = vm.get_top ();
    print ("Duktape stack:\n");

    for (int idx = len - 1; idx >= 0; idx--) {
      vm.dup (idx);
      print ("| %d: %s = %s\n", idx, vm.get_type (-1).to_string (), vm.safe_to_string (-1));
      vm.pop ();
    }
  }

  public string get_runtime_duktape_version () {
    var vm = new Duktape.Duktape.default ();

    vm.get_global_string ("Duktape");
    vm.get_prop_string (-1, "version");
    var version = vm.to_uint32 (-1);

    return format_duktape_version (version);
  }

  public string get_buildtime_duktape_version () {
    return format_duktape_version (Duktape.VERSION);
  }

  private string format_duktape_version (uint32 version) {
    var major = version / 10000;
    version -= major * 10000;
    var minor = version / 100;
    version -= minor * 100;
    var patch = version;

    return "%u.%02u.%02u".printf (major, minor, patch);
  }

  public string get_runtime_gtk_version () {
    return "%u.%02u.%02u".printf (Gtk.get_major_version (), Gtk.get_minor_version (), Gtk.get_minor_version ());
  }

  public string get_buildtime_gtk_version () {
    return "%u.%02u.%02u".printf (Gtk.MAJOR_VERSION, Gtk.MINOR_VERSION, Gtk.MICRO_VERSION);
  }

  public string format_time_duration (int64 microseconds) {
    var hours = microseconds / TimeSpan.HOUR;
    microseconds -= hours * TimeSpan.HOUR;
    var minutes = microseconds / TimeSpan.MINUTE;
    microseconds -= minutes * TimeSpan.MINUTE;
    var seconds = microseconds / TimeSpan.SECOND;

    if (hours > 0) {
      return "%lld:%02lld:%02lld".printf (hours, minutes, seconds);
    }

    return "%lld:%02lld".printf (minutes, seconds);
  }

  public Gee.List<File> get_data_dirs () {
    var list = new Gee.ArrayList<string> ();
    list.add (Environment.get_user_data_dir ());
    list.add_all_array (Environment.get_system_data_dirs ());

    var iter = list
      .map<File> ((dir) => File.new_build_filename (dir, Environment.get_prgname ()))
      .filter ((dir) => dir.query_exists () && dir.query_file_type (NONE) == DIRECTORY);

    var dirs = new Gee.ArrayList<File> ();
    dirs.add_all_iterator (iter);
    return dirs;
  }

  public string? find_file_in_data_dirs (string path) {
    var dirs = new Gee.ArrayList<string> ();
    dirs.add (Environment.get_user_data_dir ());

    foreach (var dir in Environment.get_system_data_dirs ())
      dirs.add (dir);

    foreach (var data_dir in dirs) {
      var filename = Path.build_filename (data_dir, Environment.get_prgname (), path);

      if (FileUtils.test (filename, FileTest.EXISTS))
        return filename;
    }

    return null;
  }
}
