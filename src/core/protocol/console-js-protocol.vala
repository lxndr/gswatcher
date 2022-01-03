namespace Gsw {

public class ConsoleJsProtocol : JsProtocol, ConsoleProtocol {
  public ConsoleJsProtocol (string script_path) throws Error {
    Object (script_path : script_path);
    initialize ();
  }

  private static Duktape.Return js_response (DuktapeEx vm) {
    var proto = (ConsoleJsProtocol) JsProtocol.get_this_pointer (vm);
    var resp = vm.require_string (0);
    proto.enqueue_callback (() => proto.response (resp));
    return 0;
  }

  protected override void register_globals () {
    base.register_globals ();

    Duktape.FunctionListEntry[] funcs = {
      { "response", (Duktape.CFunction) js_response, 1 },
      { null },
    };

    register_gsw_functions (funcs);
  }

  public void send_command (string command) throws Error {
    new GlobalRoutine (vm, "module.sendCommand")
      .push_string (command)
      .exec ();
  }

  public override void process_response (uint8[] data) throws Error {
    try {
      new GlobalRoutine (vm, "module.processResponse")
        .push_buffer (data)
        .exec ();
    } catch (JsError err) {
      // NOTE: very naive to simply check if it contains InvalidResponseError
      if (err.code == JsError.RUNTIME_ERROR && err.message.index_of ("InvalidResponseError") >= 0) {
        throw new ProtocolError.INVALID_RESPONSE (err.message);
      }

      throw err;
    }
  }
}

}
