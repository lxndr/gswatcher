namespace Gsw {

public class QueryLuaProtocol : LuaProtocol, QueryProtocol {
  public QueryLuaProtocol (string script_path) throws Error {
    Object (script_path : script_path);
    init ();
  }

  private static int lua_sinfo (LuaEx vm) {
    var proto = (QueryLuaProtocol) LuaProtocol.get_this_pointer (vm);
    vm.check_type (1, Lua.Type.TABLE);
    vm.check_type (2, Lua.Type.TABLE);

    var details = new Gee.HashMap<string, string> ();
    vm.extract_object_to_map (1, details);

    var sinfo = new ServerInfo ();
    vm.extract_object_to_gobject (2, sinfo);

    proto.enqueue_callback (() => proto.sinfo_update (details, sinfo));
    return 0;
  }

  private static int lua_plist (LuaEx vm) {
    var proto = (QueryLuaProtocol) LuaProtocol.get_this_pointer (vm);
    vm.check_type (1, Lua.Type.TABLE);
    vm.check_type (2, Lua.Type.TABLE);

    var pfields = new Gee.ArrayList<PlayerField> ();
    var players = new Gee.ArrayList<Player> ();

    vm.extract_array (1, () => {
      var field = new PlayerField ();
      vm.extract_object_to_gobject (-1, field);
      pfields.add (field);
    });

    vm.extract_array (2, () => {
      var player = new Player ();
      vm.extract_object_to_map (-1, player);
      players.add (player);
    });

    proto.enqueue_callback (() => proto.plist_update (pfields, players));
    return 0;
  }

  protected override void register_globals () {
    base.register_globals ();

    Lua.Reg[] funcs = {
      { "sinfo", (Lua.CFunction) lua_sinfo },
      { "plist", (Lua.CFunction) lua_plist },
      { null },
    };

    register_gsw_functions (funcs);
  }

  public void query () throws Error {
    new GlobalRoutine (vm, "query")
      .exec ();
  }

  public override void process_response (uint8[] data) throws Error {
    try {
      new GlobalRoutine (vm, "process")
        .push_buffer (data)
        .exec ();
    } catch (LuaError err) {
      if (err.code == LuaError.RUNTIME_ERROR && err.message.index_of ("invalid response:") >= 0) {
        throw new ProtocolError.INVALID_RESPONSE (err.message);
      }

      throw err;
    }
  }
}

}
