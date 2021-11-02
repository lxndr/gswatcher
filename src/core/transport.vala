namespace Gsw {

  abstract class Transport : Object {
    public string host { get; construct set; }
    public uint16 port { get; construct set; }

    public signal void receive (uint8[] data);

    public abstract void send (uint8[] data) throws Error;
  }

}
