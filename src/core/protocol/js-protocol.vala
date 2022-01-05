namespace Gsw {

public abstract class JsProtocol : Object, Initable, Protocol {
  public ProtocolInfo info { get; protected set; }
  public string script_path { get; construct; }

  protected DuktapeEx vm = new DuktapeEx ();
  private Gee.List<Source> callback_sources = new Gee.LinkedList<Source>();

  private static string THIS_PROTOCOL_POINTER_KEY = Duktape.HIDDEN_SYMBOL ("__thisProtocolPointer");

  protected abstract void process_response (uint8[] data) throws Error;

  ~JsProtocol () {
    foreach (var source in callback_sources)
      source.destroy ();
  }

  public static JsProtocol get_this_pointer (DuktapeEx vm) {
    vm.get_global_string (THIS_PROTOCOL_POINTER_KEY);
    return (JsProtocol) vm.require_pointer (-1);
  }

  private static Duktape.Return js_send (DuktapeEx vm) {
    var proto = get_this_pointer (vm);
    var data = vm.require_buffer_data (0);
    proto.enqueue_callback (() => proto.data_send (data));
    return 0;
  }

  private static Duktape.Return js_print (DuktapeEx vm) {
    var str = vm.require_string (0);
    print ("%s\n", str);
    return 0;
  }

  public bool init (Cancellable? cancellable = null) throws Error {
    register_globals ();
    vm.load_script (script_path);
    info = fetch_info ();
    return true;
  }

  protected virtual void register_globals () {
    vm.push_object ();
    vm.put_global_string ("gsw");

    vm.push_pointer (this);
    vm.put_global_string (THIS_PROTOCOL_POINTER_KEY);

    Duktape.FunctionListEntry[] funcs = {
      { "send",    (Duktape.CFunction) js_send,    1 },
      { "print",   (Duktape.CFunction) js_print,   1 },
      { null },
    };

    register_gsw_functions (funcs);
  }

  protected void register_gsw_functions (Duktape.FunctionListEntry[] funcs) {
    vm.get_global_string ("gsw");
    vm.put_function_list (-1, funcs);
    vm.pop ();
  }

  private ProtocolInfo fetch_info () throws Error {
    var obj = new GlobalObject (vm, "module.info");

    var info = new ProtocolInfo ();
    info.id = obj.require_string ("id");
    info.name = obj.require_string ("name");
    info.transport = obj.require_string ("transport");
    info.feature = ProtocolFeature.parse_nick (obj.require_string ("feature"));

    // TODO: read from protocol info
    if (info.feature == CONSOLE) {
      info.options.add (new ProtocolOption () {
        id = "password",
        label = "Password",
        type = PASSWORD,
      });
    }

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
