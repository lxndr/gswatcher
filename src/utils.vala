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
