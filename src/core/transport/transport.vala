namespace Gsw {

public abstract class Transport : Object {
  public signal void connected ();
  public signal void disconnected ();
  public signal void authenticated ();
  public signal void data_received (uint8[] data);
  public signal void error (Error error);

  public abstract void send (uint8[] data);
}

}
