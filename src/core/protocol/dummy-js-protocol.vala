namespace Gsw {

public class DummyJsProtocol : JsProtocol {
  public DummyJsProtocol (string script_path) throws Error {
    Object (script_path : script_path);
    initialize ();
  }

  public override void process_response (uint8[] data) throws Error {
    throw new ProtocolError.NOT_IMPLEMENTED ("not implemented");
  }
}

}
