namespace Gsw {

public errordomain JsError {
  SCRIPT,
  MISSING_GLOBAL,
  RUNTIME_ERROR,
}

public delegate void Callback ();

public class DuktapeEx : Duktape {
  public DuktapeEx () {
    base.default ();
  }

  public void load_script (string filepath) throws GLib.Error {
    string script;
    FileUtils.get_contents (filepath, out script);

    push_string (filepath);

    if (pcompile_string_filename (0, script) != Duktape.Exec.SUCCESS) {
      throw new JsError.SCRIPT ("failed to load script '%s': %s", filepath, safe_to_stacktrace (-1));
    }

    if (pcall (0) != Duktape.Exec.SUCCESS) {
      throw new JsError.RUNTIME_ERROR ("failed to execute '%s': %s", filepath, safe_to_stacktrace (-1));
    }

    pop ();
  }

  public void print_duktape_stack () {
    var len = get_top ();
    print ("Duktape stack:\n");
  
    for (int idx = len - 1; idx >= 0; idx--) {
      dup (idx);
      print ("| %d: %s = %s\n", idx, get_type (-1).to_string (), safe_to_string (-1));
      pop ();
    }
  }

  public string get_runtime_version () throws GLib.Error {
    var obj = new GlobalObject (this, "Duktape");
    var version = obj.require_uint32 ("version");
    return format_duktape_version (version);
  }

  public static string get_buildtime_version () {
    return format_duktape_version (VERSION);
  }

  private static string format_duktape_version (Duktape.UInt32 version) {
    var major = version / 10000;
    version -= major * 10000;
    var minor = version / 100;
    version -= minor * 100;
    var patch = version;
  
    return "%u.%02u.%02u".printf (major, minor, patch);
  }  

  public Return extract_array (Index idx, Callback cb) {
    if (!is_array (idx))
      return type_error ("argument %d must be an array", idx);
  
    var len = get_length (idx);
  
    for (var i = 0; i < len; i++) {
      get_prop_index (idx, i);
      cb ();
      pop ();
    }
  
    return 0;
  }

  public void extract_object_to_map (Index idx, Gee.Map<string, string> map) {
    enum (idx, OWN_PROPERTIES_ONLY);
  
    while (next (-1, true)) {
      if (!is_null_or_undefined (-1)) {
        var key = get_string (-2);
        var val = to_string (-1);
        map.set (key, val);
      }

      pop_2 ();
    }
  
    pop ();
  }

  public void extract_object_to_gobject (Index idx, Object object) {
    enum (idx, OWN_PROPERTIES_ONLY);

    while (next (-1, true)) {
      var type = get_type (-1);
      var key = get_string (-2);

      if (type == Type.STRING)
        object.set (key, to_string (-1));
      else if (type == Type.NUMBER)
        object.set (key, to_int (-1));
      else if (type == Type.BOOLEAN)
        object.set (key, to_boolean (-1));

      pop_2 ();
    }
  
    pop ();
  }
}

[Compact (opaque = true)]
public class GlobalRoutine {
  unowned Duktape duk;
  string routine;
  Duktape.Index initial_top;
  Duktape.Index nargs;

  public GlobalRoutine (Duktape duk, string routine) throws Error {
    this.duk = duk;
    this.routine = routine;
    this.initial_top = this.duk.get_top ();
    push_routine ();
  }

  ~GlobalRoutine () {
    duk.pop_n (duk.get_top () - this.initial_top);
  }

  public void exec () throws Error {
    call_routine ();
  }

  private void push_routine () throws Error {
    var path = routine.split (".");
    duk.push_global_object ();

    foreach (var path_part in path) {
      if (!duk.get_prop_string (-1, path_part)) {
        throw new JsError.MISSING_GLOBAL ("routine '%s' does not exist", routine);
      }
    }
  }

  private void call_routine () throws Error {
    if (duk.pcall (nargs) != Duktape.Exec.SUCCESS) {
      var msg = duk.safe_to_stacktrace (-1);
      throw new JsError.RUNTIME_ERROR ("failed to call '%s': %s", routine, msg);
    }
  }

  public unowned GlobalRoutine push_string (string command) {
    duk.push_string (command);
    nargs++;
    return this;
  }

  public unowned GlobalRoutine push_buffer (uint8[] data) {
    duk.push_external_buffer ();
    duk.config_buffer (-1, data);
    duk.push_buffer_object (-1, 0, data.length, NODEJS_BUFFER);
    duk.remove (-2);
    nargs++;
    return this;
  }
}

[Compact (opaque = true)]
public class GlobalObject {
  unowned DuktapeEx duk;
  string path;
  string[] path_parts;

  public GlobalObject(DuktapeEx duk, string path) throws Error {
    this.duk = duk;
    this.path = path;

    path_parts = path.split (".");
    duk.push_global_object ();

    foreach (var path_part in path_parts) {
      if (!duk.get_prop_string (-1, path_part))
        throw new JsError.MISSING_GLOBAL ("global '%s' does not exist", path);

      if (!duk.is_object (-1))
        throw new JsError.MISSING_GLOBAL ("global '%s' is not object", path);
    }
  }

  ~GlobalObject() {
    duk.pop_n (path_parts.length + 1);
  }

  private void require_prop (string name) throws Error {
    if (!duk.get_prop_string (-1, name))
      throw new JsError.MISSING_GLOBAL ("could not find global '%s.%s'", path, name);

    if (duk.is_null_or_undefined (-1)) {
      duk.pop ();
      throw new JsError.MISSING_GLOBAL ("could not find global '%s.%s'", path, name);
    }
  }

  public string require_string (string name) throws Error {
    require_prop (name);
    var str = duk.safe_to_string (-1);
    duk.pop ();
    return str;
  }

  public uint32 require_uint32 (string name) throws Error {
    require_prop (name);
    var num = duk.to_uint32 (-1);
    duk.pop ();
    return num;
  }
}

}
