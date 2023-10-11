namespace Gsw {

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

  public string? find_file_in_data_dirs (string path) {
    var dirs = new Gee.ArrayList<string> ();
    dirs.add (Environment.get_user_data_dir ());

    foreach (var dir in Environment.get_system_data_dirs ())
      dirs.add (dir);

    foreach (var data_dir in dirs) {
      var filename = Path.build_filename (data_dir, path);

      if (FileUtils.test (filename, FileTest.EXISTS))
        return filename;
    }

    return null;
  }

  public string? find_file_in_app_data_dirs (string path) {
    return find_file_in_data_dirs (Path.build_filename(Environment.get_prgname (), path));
  }

  public bool settings_nullable_string_getter (Value value, Variant variant, void* user_data) {
    if (!variant.is_of_type (VariantType.MAYBE))
      return false;

    var maybe = variant.get_maybe ();

    if (maybe == null) {
      value.set_string (null);
      return true;
    }

    if (!maybe.is_of_type (VariantType.STRING))
      return false;

    var str = maybe.get_string ();
    value.set_string (str);
    return true;
  }

  public Variant settings_nullable_string_setter (Value value, VariantType expected_type, void* user_data) {
    var str = value.get_string ();
    var variant = str == null ? null : new Variant.string (str);
    return new Variant.maybe (VariantType.STRING, variant);
  }

  public bool settings_nullable_datetime_getter (Value value, Variant variant, void* user_data) {
    if (!variant.is_of_type (VariantType.MAYBE))
      return false;

    var maybe = variant.get_maybe ();

    if (maybe == null) {
      value.set_boxed (null);
      return true;
    }

    if (!maybe.is_of_type (VariantType.STRING))
      return false;

    var str = maybe.get_string ();

    if (str.length == 0) {
      value.set_boxed (null);
      return true;
    }

    var time = new DateTime.from_iso8601 (str, null);

    if (time == null)
      return false;

    value.set_boxed (time);
    return true;
  }

  public Variant settings_nullable_datetime_setter (Value value, VariantType expected_type, void* user_data) {
    var time = (DateTime?) value.get_boxed ();
    var variant = time == null ? null : new Variant.string (time.format_iso8601 ());
    return new Variant.maybe (VariantType.STRING, variant);
  }
}
