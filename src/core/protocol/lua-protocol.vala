/*
  Game Server Watcher
  Copyright (C) 2010-2026 Alexander Burobin

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

errordomain LuaProtocolError {
  INVALID_PROTOCOL_INFO,
}

public abstract class LuaProtocol : Object, Initable, Protocol {
  public ProtocolInfo info { get; protected set; }
  public string script_path { get; construct; }

  protected LuaEx vm = new LuaEx ();
  private Gee.List<Source> callback_sources = new Gee.LinkedList<Source>();
  private Gee.List<File> module_dirs = new Gee.ArrayList<File> ();

  private static string THIS_PROTOCOL_POINTER_KEY = "__thisProtocolPointer";
  private static string REQUIRE_CACHE_KEY = "__gswRequireCache";

  protected abstract void process_response (uint8[] data) throws Error;

  ~LuaProtocol () {
    foreach (var source in callback_sources)
      source.destroy ();
  }

  public static unowned LuaProtocol get_this_pointer (LuaEx vm) {
    vm.get_field (Lua.PseudoIndex.REGISTRY, THIS_PROTOCOL_POINTER_KEY);
    unowned LuaProtocol self = (LuaProtocol) vm.to_userdata (-1);
    vm.pop (1);
    return self;
  }

  private static int lua_send (LuaEx vm) {
    var proto = get_this_pointer (vm);
    var data = vm.l_check_buffer (1);
    proto.enqueue_callback (() => proto.data_send (data));
    return 0;
  }

  private static int lua_require (LuaEx vm) {
    var proto = get_this_pointer (vm);
    var module_name = vm.l_check_string (1);
    return proto.require_module (module_name);
  }

  public bool init (Cancellable? cancellable = null) throws Error {
    configure_sandbox ();
    register_globals ();
    vm.load_script (script_path);
    info = fetch_info ();
    return true;
  }

  private void configure_sandbox () {
    vm.l_requiref ("base", Lua.open_base, false);
    vm.pop (1);

    vm.unset_global ("dofile");
    vm.unset_global ("loadfile");
    vm.unset_global ("load");
    vm.unset_global ("collectgarbage");

    vm.l_requiref ("string", Lua.open_string, true);
    vm.pop (1);
    vm.unset_field_global_table ("string", "dump");

    vm.l_requiref ("math", Lua.open_math, true);
    vm.pop (1);

    vm.l_requiref ("table", Lua.open_table, true);
    vm.pop (1);

    module_dirs = get_app_data_dirs ("protocols", "GSW_PROTOCOLS_DIR");
    vm.new_table ();
    vm.set_field (Lua.PseudoIndex.REGISTRY, REQUIRE_CACHE_KEY);

    vm.push_cfunction ((Lua.CFunction) lua_require);
    vm.set_global ("require");
  }

  protected virtual void register_globals () {
    vm.new_table ();
    vm.set_global ("gsw");

    vm.push_lightuserdata (this);
    vm.set_field (Lua.PseudoIndex.REGISTRY, THIS_PROTOCOL_POINTER_KEY);

    Lua.Reg[] funcs = {
      { "send",    (Lua.CFunction) lua_send },
      { null },
    };

    register_gsw_functions (funcs);
  }

  private int require_module (string module_name) {
    var initial_top = vm.get_top ();
    var relative_path = module_name_to_path (module_name);

    if (relative_path == null) {
      vm.set_top (initial_top);
      vm.push_string ("invalid module name '%s'".printf (module_name));
      return vm.error ();
    }

    vm.get_field (Lua.PseudoIndex.REGISTRY, REQUIRE_CACHE_KEY);
    var cache_idx = vm.absindex (-1);

    var cached_type = vm.get_field (cache_idx, module_name);

    if (cached_type != Lua.Type.NIL && cached_type != Lua.Type.NONE) {
      vm.remove (-2);
      return 1;
    }

    vm.pop (1);

    var module_path = find_file_in_dirs (relative_path, module_dirs);

    if (module_path == null) {
      vm.set_top (initial_top);
      vm.push_string ("module '%s' not found".printf (module_name));
      return vm.error ();
    }

    vm.push_boolean (true);
    vm.set_field (cache_idx, module_name);

    var status = vm.l_load_file (module_path);

    if (status != Lua.Status.OK) {
      var message = vm.to_string (-1).make_valid ();
      clear_module_cache (cache_idx, module_name);
      vm.set_top (initial_top);
      vm.push_string ("failed to load module '%s': %s".printf (module_name, message));
      return vm.error ();
    }

    status = vm.pcall (0, 1, 0);

    if (status != Lua.Status.OK) {
      var message = vm.to_string (-1).make_valid ();
      clear_module_cache (cache_idx, module_name);
      vm.set_top (initial_top);
      vm.push_string ("failed to load module '%s': %s".printf (module_name, message));
      return vm.error ();
    }

    if (vm.is_nil (-1)) {
      vm.pop (1);
      vm.push_boolean (true);
    }

    vm.push_value (-1);
    vm.set_field (cache_idx, module_name);
    vm.remove (-2);

    return 1;
  }

  private void clear_module_cache (int cache_idx, string module_name) {
    vm.push_nil ();
    vm.set_field (cache_idx, module_name);
  }

  private string? module_name_to_path (string module_name) {
    if (module_name == "" || Path.is_absolute (module_name) || module_name.index_of_char ('\\') >= 0)
      return null;

    var parts = module_name.replace (".", "/").split ("/");
    string? relative_path = null;

    foreach (var part in parts) {
      if (!is_safe_module_path_part (part))
        return null;

      relative_path = (relative_path == null)
        ? part
        : Path.build_filename (relative_path, part);
    }

    if (relative_path == null)
      return null;

    return @"$relative_path.lua";
  }

  private bool is_safe_module_path_part (string part) {
    if (part == "" || part == "." || part == "..")
      return false;

    for (var i = 0; i < part.length; i++) {
      var c = part[i];

      if (!((c >= 'A' && c <= 'Z') ||
            (c >= 'a' && c <= 'z') ||
            (c >= '0' && c <= '9') ||
            c == '_' || c == '-')) {
        return false;
      }
    }

    return true;
  }

  protected void register_gsw_functions (Lua.Reg[] funcs) {
    vm.get_global ("gsw");
    vm.l_set_funcs (funcs, 0);
    vm.pop (1);
  }

  private ProtocolInfo fetch_info () throws Error {
    try {
      var obj = new GlobalObject (vm, "protocol");

      var info = new ProtocolInfo ();
      info.id = obj.require_string ("id");
      info.name = obj.require_string ("name");
      info.transport = obj.require_string ("transport");
      info.feature = ProtocolFeature.parse_nick (obj.require_string ("feature"));

      return info;
    } catch (Error err) {
      throw new LuaProtocolError.INVALID_PROTOCOL_INFO ("invalid protocol info: %s", err.message);
    }
  }

  protected void enqueue_callback (owned Callback cb) {
    var source = new IdleSource ();

    source.set_callback(() => {
      cb ();
      source.destroy ();
      callback_sources.remove (source);
      return Source.REMOVE;
    });

    callback_sources.add (source);
    source.attach ();
  }
}

}
