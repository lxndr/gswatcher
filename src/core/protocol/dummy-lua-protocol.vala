namespace Gsw {

public class DummyLuaProtocol : LuaProtocol {
  public DummyLuaProtocol (string script_path) throws Error {
    Object (script_path : script_path);
    init ();
  }

  public override void process_response (uint8[] data) throws Error {
    throw new ProtocolError.NOT_IMPLEMENTED ("not implemented");
  }
}

}
