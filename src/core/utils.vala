namespace Gsw {

static Regex address_re;

Regex create_address_re () ensures (result != null) {
  if (address_re == null) {
    try {
      address_re = new Regex ("^([a-z0-9\\.\\-]+):(\\d{1,5})(?::(\\d{1,5}))?$", RegexCompileFlags.CASELESS);
    } catch (Error err) {
      log (Config.LOG_DOMAIN, LEVEL_CRITICAL, "failed to create Regex: %s", err.message);
    }
  }

  return address_re;
}

public bool parse_address (string address, out string host, out uint16 gport, out uint16 qport) {
  host = address;
  gport = 0;
  qport = 0;

  MatchInfo match_info;
  var address_re = create_address_re ();

  if (!address_re.match (address, 0, out match_info))
    return false;

  if (!match_info.matches ())
    return false;

  var p1 = match_info.fetch (1);
  var p2 = match_info.fetch (2);
  var p3 = match_info.fetch (3);

  host = p1;
  gport = qport = (uint16) uint.parse (p2);

  if (p3 != null && p3.length > 0)
    qport = (uint16) uint.parse (p3);

  return true;
}

public bool string_to_bool (string value, bool def) {
  bool as_bool;
  int as_int;

  if (bool.try_parse (value, out as_bool))
    return as_bool;

  if (int.try_parse (value, out as_int))
    return as_int != 0;

  return def;
}

public Gee.List<File> get_data_dirs (string subdir, string? data_dir_env = null) {
  var dirs = new Gee.ArrayList<File> ();
  var prgname = Environment.get_prgname ();

  if (data_dir_env != null) {
    var env_data_dir = Environment.get_variable (data_dir_env);

    if (env_data_dir != null) {
      dirs.add (File.new_for_path (env_data_dir));
    }
  }

  var user_data_dir = Environment.get_user_data_dir ();
  dirs.add (File.new_build_filename (user_data_dir, prgname, subdir));

  var system_data_dirs = Environment.get_system_data_dirs ();
  foreach (var dir in system_data_dirs)
    dirs.add (File.new_build_filename (dir, prgname, subdir));

  return dirs;
}

public async Gee.List<File> get_data_files (string subdir, string? data_dir_env = null, Cancellable? cancellable = null) {
  var files = new Gee.ArrayList<File> ();
  var data_dirs = get_data_dirs (subdir, data_dir_env);

  foreach (var data_dir in data_dirs) {
    try {
      var enumerator = yield data_dir.enumerate_children_async ("standard::*", FileQueryInfoFlags.NOFOLLOW_SYMLINKS);

      FileInfo info;
      while ((info = enumerator.next_file (null)) != null) {
        var file = data_dir.get_child(info.get_name ());
        files.add (file);
      }
    } catch (Error err) {
      if (err.code != IOError.NOT_FOUND)
        log (Config.LOG_DOMAIN, LEVEL_WARNING, "error reading directory %s: %s", data_dir.get_path (), err.message);
    }
  }

  return files;
}

public class DataFileContents {
  public File file;
  public uint8[] data;
}

public async Gee.List<DataFileContents> get_data_files_contents (string dirname, string? data_dir_env = null, Cancellable? cancellable = null) throws Error {
  var contents = new Gee.ArrayList<DataFileContents> ();
  var files = yield get_data_files (dirname, data_dir_env, cancellable);

  foreach (var file in files) {
    try {
      uint8[] data;
      yield file.load_contents_async (cancellable, out data, null);

      contents.add(new DataFileContents () {
        file = file,
        data = data,
      });
    } catch (IOError err) {
      warning ("error reading file: %s", err.message);
    }
  }

  return contents;
}

}
