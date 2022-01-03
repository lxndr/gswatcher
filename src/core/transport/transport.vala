namespace Gsw {

public abstract class Transport : Object {
  public bool ready { get; protected set; }

  public signal void receive (uint8[] data);
  public signal void error (Error error);

  public abstract void send (uint8[] data) throws Error;
}

}
