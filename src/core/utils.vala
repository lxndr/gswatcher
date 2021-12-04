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

public Gee.Iterator<File> get_data_dirs_2 (string? data_dir_env = null) {
  var dirs = new Gee.ArrayList<string> ();

  if (data_dir_env != null) {
    var env_data_dir = Environ.get_variable (null, data_dir_env);

    if (env_data_dir != null) {
      dirs.add (env_data_dir);
    }
  }

  dirs.add (Environment.get_user_data_dir ());
  dirs.add_all_array (Environment.get_system_data_dirs ());

  return dirs.map<File> ((dir) => File.new_build_filename (dir, Environment.get_prgname ()));
}

public async Gee.List<File> get_data_files (string subdir, string? data_dir_env = null, Cancellable? cancellable = null) {
  var files = new Gee.ArrayList<File> ();
  var data_dirs = get_data_dirs_2 ();

  while (data_dirs.next ()) {
    var data_dir = data_dirs.get ();
    var dir_file = data_dir.get_child (subdir);

    try {
      var enumerator = yield dir_file.enumerate_children_async ("standard::*", FileQueryInfoFlags.NOFOLLOW_SYMLINKS, Priority.DEFAULT, null);

      FileInfo info;
      while ((info = enumerator.next_file (null)) != null) {
        var file = dir_file.get_child(info.get_name ());
        files.add (file);
      }
    } catch (Error err) {
      if (err.code != IOError.NOT_FOUND)
        log (Config.LOG_DOMAIN, LEVEL_WARNING, "error reading directory %s: %s", dir_file.get_path (), err.message);
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
