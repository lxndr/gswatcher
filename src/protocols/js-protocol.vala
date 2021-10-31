namespace Gsw {
  const string THIS_PROTOCOL_POINTER_KEY = "__thisProtocolPointer"; // FIXME: use DUK_HIDDEN_SYMBOL

  static Duktape.Return js_send(Duktape.Duktape vm) {
    vm.get_global_string (THIS_PROTOCOL_POINTER_KEY);
    var proto = (Protocol) vm.require_pointer (-1);
    var data = vm.require_buffer_data (0);

    Idle.add (() => {
      proto.data_send (data);
      return Source.REMOVE;
    });

    return 0;
  }

  static Duktape.Return js_sinfo(Duktape.Duktape vm) {
    var sinfo = ServerInfo ();

    vm.get_global_string (THIS_PROTOCOL_POINTER_KEY);
    var proto = (Protocol) vm.require_pointer (-1);

    vm.require_object (0);

    vm.get_prop_string (0, "gameId");
    sinfo.game_id = vm.get_string (-1);

    vm.get_prop_string (0, "gameName");
    sinfo.game_name = vm.get_string (-1);

    vm.get_prop_string (0, "gameMode");
    sinfo.game_mode = vm.get_string (-1);

    vm.get_prop_string (0, "name");
    sinfo.name = vm.get_string (-1);

    vm.get_prop_string (0, "map");
    sinfo.map = vm.get_string (-1);

    vm.get_prop_string (0, "version");
    sinfo.version = vm.get_string (-1);

    vm.get_prop_string (0, "serverType");
    sinfo.server_type = vm.get_string (-1);

    vm.get_prop_string (0, "osType");
    sinfo.os_type = vm.get_string (-1);

    vm.get_prop_string (0, "numPlayers");
    sinfo.num_players = vm.to_int (-1);

    vm.get_prop_string (0, "maxPlayers");
    sinfo.max_players = vm.to_int (-1);

    vm.get_prop_string (0, "private");
    sinfo.private = vm.get_boolean_default (-1, false);

    vm.get_prop_string (0, "secure");
    sinfo.secure = vm.get_boolean_default (-1, false);

    Idle.add (() => {
      proto.sinfo_update (sinfo);
      return Source.REMOVE;
    });

    return 0;
  }

  static Duktape.Return js_plist(Duktape.Duktape vm) {
    vm.get_global_string (THIS_PROTOCOL_POINTER_KEY);
    var proto = (Protocol) vm.require_pointer (-1);

    vm.require_object (0);

    if (!vm.is_array (0)) {
      vm.push_error_object (Duktape.Error.TYPE_ERROR, "gsw.plist() argument must be an array");
      return vm.throw ();
    }

    var len = vm.get_length (0);
    var players = new Gee.ArrayList<Player> ();

    for (var i = 0; i < len; i++) {
      vm.get_prop_index (0, i);
      var player = new Player ();

      vm.enum (-1, Duktape.Enum.OWN_PROPERTIES_ONLY);

      while (vm.next (-1, true)) {
        var key = vm.get_string (-2);
        var val = vm.safe_to_string (-1);
        player.set (key, val);
        vm.pop_2 ();
      }

      players.add (player);
      vm.pop_2 ();
    }

    Idle.add (() => {
      proto.plist_update (players);
      return Source.REMOVE;
    });

    return 0;
  }

  static Duktape.Return js_print(Duktape.Duktape vm) {
    var str = vm.require_string (0);
    print("%s\n", str);
    return 0;
  }

  class JsProtocol : Protocol {
    private Duktape.Duktape vm;

    public JsProtocol (string filename) throws Error {
      vm = new Duktape.Duktape.default ();
      load_script (filename);
      register_globals ();
      exec_script (filename);
      fetch_info ();
    }

    private void load_script (string filename) throws Error {
      string script;
      FileUtils.get_contents (filename, out script);

      vm.push_string (filename);

      if (vm.pcompile_string_filename (0, script) != Duktape.Exec.SUCCESS) {
        throw new ProtocolError.UNKNOWN ("Failed to compile '%s': %s", filename, vm.safe_to_string (-1));
      }
    }

    private void exec_script (string filename) throws Error {
      if (vm.pcall(0) != Duktape.Exec.SUCCESS) {
        throw new ProtocolError.UNKNOWN ("Failed to execute '%s': %s", filename, vm.safe_to_string (-1));
      }

      vm.pop();
    }

    private void register_globals () {
      Duktape.FunctionListEntry[] funcs = {
        { "send",  js_send,  1 },
        { "sinfo", js_sinfo, 1 },
        { "plist", js_plist, 1 },
        { "print", js_print, 1 },
        { null }
      };

      vm.push_object ();
      vm.put_function_list (-1, funcs);
      vm.put_global_string ("gsw");

      vm.push_pointer (this);
      vm.put_global_string ("__thisProtocolPointer");
    }

    private void fetch_info () throws Error {
      if (!vm.get_global_string ("module")) {
        throw new ProtocolError.UNKNOWN ("Could not file global 'module'");
      }

      vm.get_prop_string (-1, "info");

      vm.get_prop_string (-1, "name");
      info.name = vm.require_string (-1);
      vm.pop ();

      vm.get_prop_string (-1, "version");
      info.version = vm.require_string (-1);
      vm.pop ();

      vm.get_prop_string (-1, "transport");
      info.transport = vm.require_string (-1);
      vm.pop ();
    }

    public override void query () throws ProtocolError {
      if (!vm.get_global_string ("module")) {
        throw new ProtocolError.UNKNOWN ("Could not file global 'module'");
      }

      if (!vm.get_prop_string (-1, "query")) {
        throw new ProtocolError.UNKNOWN ("Function 'query' does not exist");
      }

      if (vm.pcall(0) != Duktape.Exec.SUCCESS) {
        throw new ProtocolError.UNKNOWN ("Failed to call 'query()':\n%s", vm.safe_to_stacktrace (-1));
      }

      vm.pop_2();
    }

    public override void process_response (uint8[] data) throws ProtocolError {
      if (!vm.get_global_string ("module")) {
        throw new ProtocolError.UNKNOWN ("Could not file global 'module'");
      }

      if (!vm.get_prop_string (-1, "processResponse")) {
        throw new ProtocolError.UNKNOWN ("Function 'query' does not exist");
      }

      vm.push_external_buffer ();
      vm.config_buffer (-1, data);

      if (vm.pcall(1) != Duktape.Exec.SUCCESS) {
        throw new ProtocolError.UNKNOWN ("Failed to call 'processResponse()': %s", vm.safe_to_stacktrace (-1));
      }

      vm.pop_2();
    }
  }

}
