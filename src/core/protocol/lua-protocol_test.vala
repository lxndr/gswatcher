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

using Gsw;

private int script_count = 0;

public static int main (string[] args) {
  Environment.set_variable ("GSW_PROTOCOLS_DIR", "../data/protocols", true);

  Test.init (ref args);

  Test.add_func ("/core/protocol/lua_protocol/base_loaders_are_unavailable", () => {
    assert_loads_protocol (
      """
      assert(dofile == nil)
      assert(loadfile == nil)
      assert(load == nil)
      """
    );
  });

  Test.add_func ("/core/protocol/lua_protocol/debug_library_is_unavailable", () => {
    assert_loads_protocol (
      """
      assert(debug == nil)
      """
    );
  });

  Test.add_func ("/core/protocol/lua_protocol/package_loaders_are_unavailable", () => {
    assert_loads_protocol (
      """
      assert(package == nil)
      """
    );
  });

  Test.add_func ("/core/protocol/lua_protocol/unsafe_library_functions_are_unavailable", () => {
    assert_loads_protocol (
      """
      assert(collectgarbage == nil)
      assert(string.dump == nil)
      """
    );
  });

  Test.add_func ("/core/protocol/lua_protocol/require_loads_to_boolean", () => {
    assert_loads_protocol (
      """
      local to_boolean = require("lib/to_boolean")
      assert(to_boolean("true") == true)
      assert(to_boolean("no") == false)
      """
    );
  });

  Test.add_func ("/core/protocol/lua_protocol/require_caches_lua_modules", () => {
    assert_loads_protocol (
      """
      local to_boolean = require("lib/to_boolean")
      assert(require("lib/to_boolean") == to_boolean)
      """
    );
  });

  Test.add_func ("/core/protocol/lua_protocol/require_rejects_traversal", () => {
    assert_loads_protocol (
      """
      assert(not pcall(require, "../outside"))
      assert(not pcall(require, "lib/../to_boolean"))
      """
    );
  });

  Test.add_func ("/core/protocol/lua_protocol/require_rejects_absolute_paths", () => {
    assert_loads_protocol (
      """
      assert(not pcall(require, "/outside"))
      """
    );
  });

  Test.add_func ("/core/protocol/lua_protocol/require_rejects_backslashes", () => {
    assert_loads_protocol (
      """
      assert(not pcall(require, "lib\\to_boolean"))
      """
    );
  });

  Test.run ();
  return 0;
}

private void assert_loads_protocol (string script) {
  string? path = null;

  try {
    path = write_protocol_script (script);
    var protocol = new DummyLuaProtocol (path);

    assert_true (protocol.info.id == "lua-protocol-test");
    assert_true (protocol.info.name == "Lua Protocol Test");
    assert_true (protocol.info.transport == "udp");
    assert_true (protocol.info.feature == ProtocolFeature.QUERY);
  } catch (Error err) {
    assert_no_error (err);
  } finally {
    if (path != null)
      FileUtils.remove (path);
  }
}

private string write_protocol_script (string script) throws FileError {
  var path = "lua-protocol-test-%d.lua".printf (script_count++);
  FileUtils.set_contents (path, script +
    """
    protocol = {
      id = "lua-protocol-test",
      name = "Lua Protocol Test",
      transport = "udp",
      feature = "query",
    }
    """
  );

  return path;
}
