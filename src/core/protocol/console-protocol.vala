namespace Gsw {

public interface ConsoleProtocol : Protocol {
  public signal void response (string message);

  public abstract void send_command (string command, Gee.Map<string, string> options) throws Error;
}

}
