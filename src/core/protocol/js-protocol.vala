namespace Gsw {

public abstract class JsProtocol : Object, Initable, Protocol {
  public ProtocolInfo info { get; protected set; }
  public string script_path { get; construct; }

  protected LuaEx vm = new LuaEx ();
  private Gee.List<Source> callback_sources = new Gee.LinkedList<Source>();

  private static string THIS_PROTOCOL_POINTER_KEY = "__thisProtocolPointer";

  protected abstract void process_response (uint8[] data) throws Error;

  ~JsProtocol () {
    foreach (var source in callback_sources)
      source.destroy ();
  }

  public static unowned JsProtocol get_this_pointer (LuaEx vm) {
    vm.get_field (Lua.PseudoIndex.REGISTRY, THIS_PROTOCOL_POINTER_KEY);
    unowned JsProtocol self = (JsProtocol) vm.to_userdata (-1);
    vm.pop (1);
    return self;
  }

  private static int js_send (LuaEx vm) {
    var proto = get_this_pointer (vm);
    var data = vm.l_check_buffer (1);
    proto.enqueue_callback (() => proto.data_send (data));
    return 0;
  }

  public bool init (Cancellable? cancellable = null) throws Error {
    // base library
    vm.l_requiref ("base", Lua.open_base, false);
    vm.pop (1);

    // pacakge library
    // TODO: disable c libraries
    var patterns = new Gee.ArrayList<string> ();
    var dirs = get_data_dirs ("protocols", "GSW_PROTOCOLS_DIR");
    foreach (var dir in dirs) patterns.add (Path.build_filename (dir.get_path (), "?.lua"));
    var path = string.joinv (";", patterns.to_array ());

    vm.l_requiref ("package", Lua.open_package, false);
    vm.push_string (path);
    vm.set_field (-2, "path");
    vm.pop (1);

    // string library
    vm.l_requiref ("string", Lua.open_string, true);
    vm.pop (1);

    // math library
    vm.l_requiref ("math", Lua.open_math, true);
    vm.pop (1);

    // table library
    vm.l_requiref ("table", Lua.open_table, true);
    vm.pop (1);

    // debug library
    vm.l_requiref ("debug", Lua.open_debug, true);
    vm.pop (1);

    //
    register_globals ();
    vm.load_script (script_path);
    info = fetch_info ();
    return true;
  }

  protected virtual void register_globals () {
    vm.new_table ();
    vm.set_global ("gsw");

    vm.push_lightuserdata (this);
    vm.set_field (Lua.PseudoIndex.REGISTRY, THIS_PROTOCOL_POINTER_KEY);

    Lua.Reg[] funcs = {
      { "send",    (Lua.CFunction) js_send },
      { null },
    };

    register_gsw_functions (funcs);
  }

  protected void register_gsw_functions (Lua.Reg[] funcs) {
    vm.get_global ("gsw");
    vm.l_set_funcs (funcs, 0);
    vm.pop (1);
  }

  private ProtocolInfo fetch_info () throws Error {
    var obj = new GlobalObject (vm, "protocol");

    var info = new ProtocolInfo ();
    info.id = obj.require_string ("id");
    info.name = obj.require_string ("name");
    info.transport = obj.require_string ("transport");
    info.feature = ProtocolFeature.parse_nick (obj.require_string ("feature"));

    return info;
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
