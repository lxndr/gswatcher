/*
  Game Server Watcher
  Copyright (C) 2025  Alexander Burobin

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

[Compact]
[CCode (cprefix = "lua_", cheader_filename = "lauxlib.h", free_function = "lua_close", cname = "lua_State")]
public class Lua {
  public const string VERSION;
  public const int MULTRET;

  [SimpleType]
  [CCode (cname = "lua_Number", has_type_id = false)]
  public struct Number : double {}

  [SimpleType]
  [CCode (cname = "lua_Integer", has_type_id = false)]
  public struct Integer : int64 {}

  public enum PseudoIndex {
    [CCode (cname = "LUA_REGISTRYINDEX")]
    REGISTRY,
  }

  [CCode (cprefix = "LUA_T", cname = "int", has_type_id = false)]
  public enum Type {
    NONE,
    NIL,
    BOOLEAN,
    LIGHTUSERDATA,
    NUMBER,
    STRING,
    TABLE,
    FUNCTION,
    USERDATA,
    THREAD
  }

  [CCode (cprefix = "LUA_", cname = "int", has_type_id = false)]
  public enum Status {
    OK,
    ERRRUN,
    ERRMEM,
    ERRERR,
    ERRSYNTAX,
    YIELD,
    ERRFILE,
  }

  [SimpleType]
  [CCode (cname = "luaL_Reg", has_type_id = false)]
  public struct Reg {
    public unowned string name;
    public CFunction func;
  }

  [CCode (cname = "lua_CFunction", has_target = false)]
  public delegate int CFunction (Lua vm);

  [CCode (cname = "luaL_newstate")]
  public Lua ();

  public Type absindex (int index);
  [CCode (cname = "lua_getfield")]
  public Type get_field (int index, string k);
  [CCode (cname = "lua_tonumber")]
  public Number to_number (int index);
  [CCode (cname = "lua_tointeger")]
  public Integer to_integer (int index);
  [CCode (cname = "lua_tostring")]
  public unowned string to_string (int index);

  /**
   * Converts the value at the given index to a generic C pointer (void*).
   * The value can be a userdata, a table, a thread, a string, or a function; otherwise, `lua_topointer` returns NULL.
   * Different objects will give different pointers. There is no way to convert the pointer back to its original value.
   *
   * Typically this function is used only for hashing and debug information.
   */
  [CCode (cname = "lua_topointer")]
  public void* to_pointer (int index);

  [CCode (cname = "lua_toboolean")]
  public bool to_boolean (int index);

  /**
   * Returns the index of the top element in the stack. Because indices start at 1, this result is equal to the number of elements in the stack; in particular, 0 means an empty stack.
   *
   * Notice that a negative index -x is equivalent to the positive index gettop - x + 1.
   */
  [CCode (cname = "lua_gettop")]
  public int get_top ();
  [CCode (cname = "lua_settop")]
  public void set_top (int index);

  /**
   * Pops n elements from the stack.
   */
  public void pop (int n);
  public Lua.Type type (int index);
  public Status pcall (int nargs, int nresults, int errfunc);
  public Number version ();
  [CCode (cname = "lua_pushnil")]
  public void push_nil ();
  [CCode (cname = "lua_pushcfunction")]
  public void push_cfunction (CFunction fn);
  [CCode (cname = "lua_pushstring")]
  public void push_string (string s);
  [CCode (cname = "lua_pushinteger")]
  public void push_integer (Integer i);

  [CCode (cname = "lua_pushlstring")]
  public void push_lstring (string s, size_t size);
  [CCode (cname = "lua_pushlstring", array_length = false)]
  public void push_buffer ([CCode (array_length_type = "size_t")] uint8[] data);

  [CCode (cname = "lua_pushlightuserdata")]
  public void push_lightuserdata (void* p);
  [CCode (cname = "lua_isnil")]
  public bool is_nil (int index);
  [CCode (cname = "luaL_checktype")]
  public void check_type (int arg, Type t);
  public int error ();
  public int next (int index);
  [CCode (cname = "lua_setglobal")]
  public void set_global (string name);
  [CCode (cname = "lua_getglobal")]
  public Type get_global (string name);

  /**
   * Pushes the global environment onto the stack.
   */
  [CCode (cname = "lua_pushglobaltable")]
  public void push_globaltable ();
  [CCode (cname = "lua_setfield")]
  public void set_field (int index, string key);
  [CCode (cname = "lua_newtable")]
  public void new_table ();
  [CCode (cname = "lua_isnoneornil")]
  public bool is_none_or_nil (int index);
  [CCode (cname = "lua_touserdata")]
  public void* to_userdata (int index);

  [CCode (cname = "luaL_dofile")]
  public Status l_do_file (string filename);

  [CCode (cname = "luaL_dostring")]
  public Status l_do_string (string str);

  [CCode (cname = "lua_isinteger")]
  public bool is_integer (int arg);
  [CCode (cname = "luaL_checkinteger")]
  public Integer l_check_integer (int arg);

  [CCode (cname = "luaL_checklstring")]
  public unowned string l_check_lstring (int arg, out size_t length);
  [CCode (cname = "luaL_checklstring", array_length_type = "size_t")]
  public unowned uint8[] l_check_buffer (int arg);

  [CCode (cname = "luaL_checknumber")]
  public Number l_check_number (int arg);

  [CCode (cname = "luaL_checkstring")]
  public unowned string l_check_string (int arg);

  [CCode (cname = "luaL_checkudata")]
  public void* l_check_userdata (int arg, string type_name);

  [CCode (cname = "luaL_getsubtable")]
  public bool l_get_subtable (int index, string field_name);

  [CCode (cname = "luaL_len")]
  public Integer l_len (int index);

  [CCode (cname = "luaL_loadfile")]
  public Status l_load_file (string filename);

  [CCode (cname = "luaL_loadstring")]
  public Status l_load_string (string str);

  [CCode (cname = "luaL_requiref")]
  public void l_requiref (string mod_name, CFunction open_fn, bool global);

  [CCode (cname = "luaL_setfuncs")]
  public void l_set_funcs ([CCode (array_length = false, array_null_terminated = true)] Reg[] funcs, int nup);

  [CCode (cname = "luaL_tolstring")]
  public unowned string l_to_lstring (int arg, out size_t size);

  [CCode (cname = "luaL_traceback")]
  public void l_traceback (Lua vm, string msg, int level);

  [CCode (cname = "luaL_typeerror")]
  public unowned string l_type_error (int arg, string type_name);

  [CCode (cname = "luaL_typename")]
  public unowned string l_type_name (int index);

  // standard library
  [CCode (cheader_filename = "lualib.h", cname = "luaopen_base")]
  public static int open_base (Lua vm);
  [CCode (cheader_filename = "lualib.h", cname = "luaopen_math")]
  public static int open_math (Lua vm);
  [CCode (cheader_filename = "lualib.h", cname = "luaopen_package")]
  public static int open_package (Lua vm);
  [CCode (cheader_filename = "lualib.h", cname = "luaopen_string")]
  public static int open_string (Lua vm);
  [CCode (cheader_filename = "lualib.h", cname = "luaopen_table")]
  public static int open_table (Lua vm);
  [CCode (cheader_filename = "lualib.h", cname = "luaopen_debug")]
  public static int open_debug (Lua vm);
}
