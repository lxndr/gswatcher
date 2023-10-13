/*
  Game Server Watcher
  Copyright (C) 2023  Alexander Burobin

  This program is free software: you can redistribute it and/or modify
  it under the terms of the GNU Affero General Public License as
  published by the Free Software Foundation, either version 3 of the
  License, or (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU Affero General Public License for more details.

  You should have received a copy of the GNU Affero General Public License
  along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

namespace Gsw {

public errordomain LuaError {
  SCRIPT,
  MISSING_GLOBAL,
  RUNTIME_ERROR,
}

public delegate void Callback ();

public class LuaEx : Lua {
  public LuaEx () {
    base ();
  }

  public void load_script (string filepath) throws GLib.Error {
    if (l_do_file (filepath) != Status.OK)
      throw new LuaError.SCRIPT ("failed to load script '%s': %s", filepath, to_string (-1));
  }

  public void print_stack () {
    var len = get_top ();
    print ("Lua stack (size = %d):\n", len);
  
    for (int idx = len; idx > 0; idx--) {
      print ("| %d: %s = %s\n", idx, l_type_name (idx), l_to_lstring (idx, null));
      pop (1);
    }
  }

  public void print_globals () {
    print ("Lua globals:\n");
    push_globaltable ();
    push_nil ();

    while (next (-2) != 0) {
      print ("  %s = %s\n", to_string (-2), l_to_lstring (-1, null));
      pop (2);
    }

    pop (1);
  }

  public string get_runtime_version () throws GLib.Error {
    return version ().to_string ();
  }

  public static string get_buildtime_version () {
    return VERSION;
  }

  public void extract_array (int idx, Callback cb) {
    var table_idx = absindex (idx);
    push_nil ();

    while (next (table_idx) != 0) {
      cb ();
      pop (1);
    }
  }

  public void extract_object_to_map (int idx, Gee.Map<string, string> map) {
    var table_idx = absindex (idx);
    push_nil ();

    while (next (table_idx) != 0) {
      if (!is_none_or_nil (-1)) {
        var key = to_string (-2).make_valid ();

        if (type (-1) == Type.TABLE) {
          print ("TODO: handle table value\n");
        } else {
          var val = to_string (-1).make_valid ();
          map.set (key, val);
        }
      }

      pop (1);
    }
  }

  public void extract_object_to_gobject (int idx, Object object) {
    unowned ObjectClass object_class = object.get_class ();
    var table_idx = absindex (idx);

    push_nil ();

    while (next (table_idx) != 0) {
      var type = type (-1);
      var key = to_string (-2);

      if (type == Type.STRING) {
        var str = to_string (-1).make_valid ();
        var pspec = object_class.find_property (key);

        if (pspec.value_type.is_enum ()) {
          var enumc = (EnumClass) pspec.value_type.class_ref ();
          var val = enumc.get_value_by_nick (str);
          object.set (key, val.value);
        } else {
          object.set (key, str);
        }
      } else if (type == Type.NUMBER) {
        object.set (key, (int) to_integer (-1));
      } else if (type == Type.BOOLEAN) {
        object.set (key, to_boolean (-1));
      }

      pop (1);
    }
  }

  public void push_error_handler () {
    push_cfunction (lua_error);
  }

  private static int lua_error (Lua vm) {
    var msg = vm.to_string (1);
    vm.l_traceback (vm, msg, 0);
    return 1;
  }
}

[Compact (opaque = true)]
public class GlobalRoutine {
  unowned LuaEx vm;
  string routine;
  int initial_top;
  int nargs;

  public GlobalRoutine (LuaEx vm, string routine) throws Error {
    this.vm = vm;
    this.routine = routine;
    initial_top = this.vm.get_top ();
    vm.push_error_handler ();
    push_routine ();
  }

  ~GlobalRoutine () {
    vm.set_top (this.initial_top);
  }

  public void exec () throws Error {
    call_routine ();
  }

  private void push_routine () throws Error {
    var type = vm.get_global (routine);

    if (type != FUNCTION)
      throw new LuaError.MISSING_GLOBAL ("routine '%s' does not exist", routine);
  }

  private void call_routine () throws Error {
    if (vm.pcall (nargs, 0, initial_top + 1) != OK)
    throw new LuaError.RUNTIME_ERROR ("failed to call function: %s", vm.to_string (-1));
  }

  public unowned GlobalRoutine push_string (string str) {
    vm.push_string (str);
    nargs++;
    return this;
  }

  public unowned GlobalRoutine push_buffer (uint8[] data) {
    vm.push_buffer (data);
    nargs++;
    return this;
  }

  public unowned GlobalRoutine push_object (Gee.Map<string, string> options) {
    vm.new_table ();

    foreach (var entry in options) {
      vm.push_string (entry.value);
      vm.set_field (-2, entry.key);
    }

    nargs++;
    return this;
  }
}

[Compact (opaque = true)]
public class GlobalObject {
  unowned LuaEx vm;
  string path;
  string[] path_parts;

  public GlobalObject(LuaEx vm, string path) throws Error {
    this.vm = vm;
    this.path = path;

    path_parts = path.split (".");
    vm.push_globaltable ();

    foreach (var path_part in path_parts) {
      var type = vm.get_field (-1, path_part);

      if (type != Lua.Type.TABLE)
        throw new LuaError.MISSING_GLOBAL ("global '%s' is not object", path);
    }
  }

  ~GlobalObject() {
    vm.pop (path_parts.length + 1);
  }

  private void require_prop (string name) throws Error {
    var type = vm.get_field (-1, name);

    if (type == Lua.Type.NIL || type == Lua.Type.NONE) {
      vm.pop (1);
      throw new LuaError.MISSING_GLOBAL ("could not find global '%s.%s'", path, name);
    }
  }

  public string require_string (string name) throws Error {
    require_prop (name);
    var str = vm.l_check_string (-1);
    vm.pop (1);
    return str;
  }

  public uint32 require_uint32 (string name) throws Error {
    require_prop (name);
    var num = (uint32) vm.l_check_integer (-1);
    vm.pop (1);
    return num;
  }
}

}
