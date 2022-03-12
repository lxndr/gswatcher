namespace Gsw {

public class ConsoleJsProtocol : JsProtocol, ConsoleProtocol {
  public ConsoleJsProtocol (string script_path) throws Error {
    Object (script_path : script_path);
    init ();
  }

  private static int js_response (LuaEx vm) {
    var proto = (ConsoleJsProtocol) JsProtocol.get_this_pointer (vm);
    var resp = vm.to_string (0);
    proto.enqueue_callback (() => proto.response (resp));
    return 0;
  }

  protected override void register_globals () {
    base.register_globals ();

    Lua.Reg[] funcs = {
      { "response", (Lua.CFunction) js_response },
      { null },
    };

    register_gsw_functions (funcs);
  }

  public void send_command (string command, Gee.Map<string, string> options) throws Error {
    new GlobalRoutine (vm, "send_command")
      .push_string (command)
      .push_object (options)
      .exec ();
  }

  public override void process_response (uint8[] data) throws Error {
    try {
      new GlobalRoutine (vm, "process")
        .push_buffer (data)
        .exec ();
    } catch (JsError err) {
      // NOTE: very naive to simply check if it contains InvalidResponseError
      if (err.code == JsError.RUNTIME_ERROR) {
        if (err.message.index_of ("InvalidResponseError") >= 0)
          throw new ProtocolError.INVALID_RESPONSE (err.message);
        if (err.message.index_of ("AuthError") >= 0)
          throw new ProtocolError.AUTH_FAILED ("authorization failed");
      }

      throw err;
    }
  }
}

}
