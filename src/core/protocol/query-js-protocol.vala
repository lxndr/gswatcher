namespace Gsw {

public class QueryJsProtocol : JsProtocol, QueryProtocol {
  public QueryJsProtocol (string script_path) throws Error {
    Object (script_path : script_path);
    init ();
  }

  private static Duktape.Return js_sinfo (DuktapeEx vm) {
    var proto = (QueryJsProtocol) JsProtocol.get_this_pointer (vm);

    var details = new Gee.HashMap<string, string> ();
    vm.extract_object_to_map (0, details);

    var sinfo = new ServerInfo ();
    vm.extract_object_to_gobject (1, sinfo);

    proto.enqueue_callback (() => proto.sinfo_update (details, sinfo));
    return 0;
  }

  private static Duktape.Return js_plist (DuktapeEx vm) {
    var pfields = new Gee.ArrayList<PlayerField> ();
    var players = new Gee.ArrayList<Player> ();

    var proto = (QueryJsProtocol) JsProtocol.get_this_pointer (vm);

    var ret = vm.extract_array (0, () => {
      var field = new PlayerField ();
      vm.extract_object_to_gobject (-1, field);
      pfields.add (field);
    });

    if (ret < 0)
      return ret;

    ret = vm.extract_array (1, () => {
      var player = new Player ();
      vm.extract_object_to_map (-1, player);
      players.add (player);
    });

    if (ret < 0)
      return ret;

    proto.enqueue_callback (() => proto.plist_update (pfields, players));
    return 0;
  }

  protected override void register_globals () {
    base.register_globals ();

    Duktape.FunctionListEntry[] funcs = {
      { "sinfo",   (Duktape.CFunction) js_sinfo,   2 },
      { "plist",   (Duktape.CFunction) js_plist,   2 },
      { null },
    };

    register_gsw_functions (funcs);
  }

  public void query () throws Error {
    new GlobalRoutine (vm, "module.query")
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
