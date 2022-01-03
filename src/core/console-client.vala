namespace Gsw {

errordomain ConsoleError {
  SENDING,
  PROCESSING,
  TIMEOUT,
}

public class ConsoleClient : Object {
  public string host { get; construct; }
  public uint16 port { get; construct; }
  public ConsoleProtocol protocol { get; construct; }
  public NetTransport transport { get; construct; }

  private Gee.List<string> _pending_commands = new Gee.ArrayList<string> ();
  private bool request_pending;
  private uint timeout_source;

  public signal void authorized ();
  public signal void command_sent (string command);
  public signal void response_received (string response);
  public signal void error_occured (Error error);

  construct {
    protocol.data_send.connect (send_data);
    protocol.response.connect (on_response);

    transport = TransportRegistry.get_instance ().create_net_transport (protocol.info.transport, host, port);
    transport.receive.connect (on_data_received);
    transport.notify["ready"].connect (handle_pending_query);
  }

  public ConsoleClient (string host, uint16 port, ConsoleProtocol protocol) {
    Object (
      host : host,
      port : port,
      protocol : protocol
    );
  }

  ~ConsoleClient () {
    stop_timeout_timer ();

    protocol.data_send.disconnect (send_data);
    protocol.response.disconnect (on_response);
    transport.receive.disconnect (on_data_received);
    transport.notify["ready"].disconnect (handle_pending_query);
  }

  public void exec_command (string command) {
    _pending_commands.add (command);
    flush ();
  }

  private void flush () {
    try {
      if (!_pending_commands.is_empty) {
        var command = _pending_commands.first ();
        _pending_commands.remove_at (0);
        protocol.send_command (command);
        start_timeout_timer ();
        command_sent (command);
      }
    } catch (Error err) {
      error_occured (new ConsoleError.SENDING ("failed to send command to %s:%u: %s", host, port, err.message));
    }
  }

  private void send_data (uint8[] data) {
    try {
      transport.send (data);
      log (Config.LOG_DOMAIN, LEVEL_DEBUG, "sent data to %s:%u: length = %ld", host, port, data.length);
    } catch (Error err) {
      error_occured (new ConsoleError.SENDING ("failed to send data to %s:%u: %s", host, port, err.message));
    }
  }

  private void handle_pending_query () {
    if (request_pending && transport.ready)
      flush ();
  }

  private void on_data_received (uint8[] data) {
    try {
      protocol.process_response (data);
    } catch (Error err) {
      error_occured (new ConsoleError.PROCESSING ("failed to process data from %s:%u: %s", host, port, err.message));
    }
  }

  private void on_response (string response) {
    stop_timeout_timer ();
    response_received (response);
  }

  private void start_timeout_timer () {
    stop_timeout_timer ();

    timeout_source = Timeout.add (TIMEOUT_MS, () => {
      error_occured (new ConsoleError.TIMEOUT ("failed to query %s:%d: %s", host, port, "failed to receive a response in reasonable amount of time"));
      timeout_source = 0;
      return Source.REMOVE;
    });
  }

  private void stop_timeout_timer () {
    if (timeout_source > 0) {
      Source.remove (timeout_source);
      timeout_source = 0;
    }
  }
}

}
