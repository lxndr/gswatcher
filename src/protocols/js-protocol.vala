namespace Gsw {

const string THIS_PROTOCOL_POINTER_KEY = "__thisProtocolPointer"; // FIXME: use DUK_HIDDEN_SYMBOL

delegate void Callback ();

public errordomain JsProtocolError {
  SCRIPT_LOADING,
  FUNC_EXECUION,
  INVALID_MODULE,
}

static Duktape.Return js_send(Duktape.Duktape vm) {
  vm.get_global_string (THIS_PROTOCOL_POINTER_KEY);
  var proto = (JsProtocol) vm.require_pointer (-1);
  var data = vm.require_buffer_data (0);

  proto.enqueue_callback (() => {
    proto.data_send (data);
  });

  return 0;
}

static Duktape.Return js_sinfo(Duktape.Duktape vm) {
  var sinfo = new ServerInfo ();

  vm.get_global_string (THIS_PROTOCOL_POINTER_KEY);
  var proto = (JsProtocol) vm.require_pointer (-1);

  vm.require_object (0);
  vm.enum (0, OWN_PROPERTIES_ONLY);

  while (vm.next (-1, true)) {
    var key = vm.get_string (-2);
    var val = vm.safe_to_string (-1);
    sinfo.set (key, val);
    vm.pop_2 ();
  }

  proto.enqueue_callback (() => proto.sinfo_update (sinfo));
  return 0;
}

static Duktape.Return js_plist(Duktape.Duktape vm) {
  vm.get_global_string (THIS_PROTOCOL_POINTER_KEY);
  var proto = (JsProtocol) vm.require_pointer (-1);

  vm.require_object (0);

  if (!vm.is_array (0)) {
    vm.push_error_object (TYPE_ERROR, "gsw.plist() argument must be an array");
    return vm.throw ();
  }

  var len = vm.get_length (0);
  var players = new Gee.ArrayList<Player> ();

  for (var i = 0; i < len; i++) {
    vm.get_prop_index (0, i);
    var player = new Player ();

    vm.enum (-1, OWN_PROPERTIES_ONLY);

    while (vm.next (-1, true)) {
      var key = vm.get_string (-2);
      var val = vm.safe_to_string (-1);
      player.set (key, val);
      vm.pop_2 ();
    }

    players.add (player);
    vm.pop_2 ();
  }

  proto.enqueue_callback (() => proto.plist_update (players));
  return 0;
}

static Duktape.Return js_print(Duktape.Duktape vm) {
  var str = vm.require_string (0);
  print("%s\n", str);
  return 0;
}

class JsProtocol : Protocol {
  public string script_path { get; construct; }

  private Gee.List<Source> callback_sources = new Gee.LinkedList<Source>();

  private Duktape.Duktape vm = new Duktape.Duktape.default ();

  public JsProtocol (string script_path) throws Error {
    Object (script_path : script_path);
    initialize ();
  }

  ~JsProtocol() {
    foreach (var source in callback_sources) {
      source.destroy ();
    }
  }

  public override void initialize () throws Error {
    if (!initialized) {
      load_script ();
      register_globals ();
      exec_script ();
      info = fetch_info ();
      initialized = true;
    }
  }

  private void load_script () throws Error {
    string script;
    FileUtils.get_contents (script_path, out script);

    vm.push_string (script_path);

    if (vm.pcompile_string_filename (0, script) != Duktape.Exec.SUCCESS) {
      throw new JsProtocolError.SCRIPT_LOADING ("failed to compile '%s': %s", script_path, vm.safe_to_stacktrace (-1));
    }
  }

  private void exec_script () throws Error {
    if (vm.pcall(0) != Duktape.Exec.SUCCESS) {
      throw new JsProtocolError.FUNC_EXECUION ("failed to execute '%s': %s", script_path, vm.safe_to_stacktrace (-1));
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
    vm.put_global_string (THIS_PROTOCOL_POINTER_KEY);
  }

  private ProtocolInfo fetch_info () throws Error {
    var info = new ProtocolInfo ();

    if (!vm.get_global_string ("module"))
      throw new JsProtocolError.INVALID_MODULE ("could not find global '%s'", "module");

    if (!vm.get_prop_string (-1, "info"))
      throw new JsProtocolError.INVALID_MODULE ("could not find global '%s'", "module.info");

    if (!vm.is_object (-1))
      throw new JsProtocolError.INVALID_MODULE ("global 'module.info' is not object");

    if (!vm.get_prop_string (-1, "id"))
      throw new JsProtocolError.INVALID_MODULE ("could not find global '%s'", "module.info.id");
    info.id = vm.safe_to_string (-1);
    vm.pop ();

    if (!vm.get_prop_string (-1, "name"))
      throw new JsProtocolError.INVALID_MODULE ("could not find global '%s'", "module.info.name");
    info.name = vm.safe_to_string (-1);
    vm.pop ();

    if (!vm.get_prop_string (-1, "transport"))
      throw new JsProtocolError.INVALID_MODULE ("could not find global '%s'", "module.info.transport");
    info.transport = vm.get_string (-1);
    vm.pop ();

    return info;
  }

  public override void query () throws Error {
    if (!vm.get_global_string ("module")) {
      throw new JsProtocolError.INVALID_MODULE ("could not file global 'module'");
    }

    if (!vm.get_prop_string (-1, "query")) {
      throw new JsProtocolError.INVALID_MODULE ("function 'module.query' does not exist");
    }

    if (vm.pcall (0) != Duktape.Exec.SUCCESS) {
      throw new JsProtocolError.INVALID_MODULE ("failed to call 'module.query':\n%s", vm.safe_to_stacktrace (-1));
    }

    vm.pop_2 ();
  }

  public override void process_response (uint8[] data) throws Error {
    if (!vm.get_global_string ("module")) {
      throw new JsProtocolError.INVALID_MODULE ("could not file global 'module'");
    }

    if (!vm.get_prop_string (-1, "processResponse")) {
      throw new JsProtocolError.INVALID_MODULE ("function 'module.processResponse' does not exist");
    }

    vm.push_external_buffer ();
    vm.config_buffer (-1, data);
    vm.push_buffer_object (-1, 0, data.length, NODEJS_BUFFER);
    vm.remove(-2);

    if (vm.pcall(1) != Duktape.Exec.SUCCESS) {
      var msg = vm.safe_to_stacktrace (-1);

      if (msg.index_of ("InvalidResponseError") == 0) {
        throw new ProtocolError.INVALID_RESPONSE (msg);
      }

      throw new JsProtocolError.FUNC_EXECUION ("failed to call 'module.processResponse': %s", msg);
    }

    vm.pop_2();
  }

  public void enqueue_callback (owned Callback cb) {
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
