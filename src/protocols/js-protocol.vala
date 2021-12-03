namespace Gsw {

class JsProtocol : ScriptProtocol {
  public string script_path { get; construct; }

  private DuktapeEx vm = new DuktapeEx ();

  private static string THIS_PROTOCOL_POINTER_KEY = Duktape.HIDDEN_SYMBOL ("__thisProtocolPointer");

  public JsProtocol (string script_path) throws Error {
    Object (script_path : script_path);
    initialize ();
  }

  private static JsProtocol get_this_pointer (DuktapeEx vm) {
    vm.get_global_string (THIS_PROTOCOL_POINTER_KEY);
    return (JsProtocol) vm.require_pointer (-1);
  }

  private static Duktape.Return js_send (DuktapeEx vm) {
    var proto = get_this_pointer (vm);
    var data = vm.require_buffer_data (0);
    proto.enqueue_callback (() => proto.data_send (data));
    return 0;
  }

  private static Duktape.Return js_details (DuktapeEx vm) {
    var proto = get_this_pointer (vm);
    var map = new Gee.HashMap<string, string> ();
    vm.require_object (0);
    vm.extract_object_to_map (0, map);
    proto.enqueue_callback (() => proto.details_update (map));
    return 0;
  }

  private static Duktape.Return js_sinfo (DuktapeEx vm) {
    var proto = get_this_pointer (vm);
    var sinfo = new ServerInfo ();
    vm.require_object (0);
    vm.extract_object_to_gobject (0, sinfo);
    proto.enqueue_callback (() => proto.sinfo_update (sinfo));
    return 0;
  }

  private static Duktape.Return js_plist (DuktapeEx vm) {
    var players = new Gee.ArrayList<Player> ();

    var proto = get_this_pointer (vm);
    vm.require_object (0);

    var ret = vm.extract_array (0, () => {
      var player = new Player ();
      vm.extract_object_to_map (-1, player);
      players.add (player);
    });

    if (ret < 0)
      return ret;

    proto.enqueue_callback (() => proto.plist_update (players));
    return 0;
  }

  private static Duktape.Return js_print (DuktapeEx vm) {
    var str = vm.require_string (0);
    print ("%s\n", str);
    return 0;
  }
  
  public override void initialize () throws Error {
    if (!initialized) {
      register_globals ();
      vm.load_script (script_path);
      info = fetch_info ();
      initialized = true;
    }
  }

  private void register_globals () {
    Duktape.FunctionListEntry[] funcs = {
      { "send",    (Duktape.CFunction) js_send,    1 },
      { "details", (Duktape.CFunction) js_details, 1 },
      { "sinfo",   (Duktape.CFunction) js_sinfo,   1 },
      { "plist",   (Duktape.CFunction) js_plist,   1 },
      { "print",   (Duktape.CFunction) js_print,   1 },
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

    var obj = new GlobalObject (vm, "module.info");
    info.id = obj.require_string ("id");
    info.name = obj.require_string ("name");
    info.transport = obj.require_string ("transport");

    return info;
  }

  public override void query () throws Error {
    new GlobalRoutine (vm, "module.query").exec ();
  }

  public override void process_response (uint8[] data) throws Error {
    try {
      new GlobalRoutine (vm, "module.processResponse")
        .push_buffer (data)
        .exec ();
    } catch (JsError err) {
      // NOTE: very naive to simply check if it contains InvalidResponseError
      if (err.code == JsError.RUNTIME_ERROR && err.message == "InvalidResponseError") {
        throw new ProtocolError.INVALID_RESPONSE (err.message);
      }
    }
  }
}

}
