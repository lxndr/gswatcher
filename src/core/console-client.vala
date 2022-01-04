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

  private uint timeout_source;

  public signal void response_received (string response);
  public signal void error_occured (Error error);

  construct {
    protocol.data_send.connect (send_data);
    protocol.response.connect (on_response);

    transport = TransportRegistry.get_instance ().create_net_transport (protocol.info.transport, host, port);
    transport.data_received.connect (on_data_received);
    transport.error.connect (on_transport_error);
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
    transport.data_received.disconnect (on_data_received);
    transport.error.disconnect (on_transport_error);
  }

  public void exec_command (string command) {
    try {
      var options = new Gee.HashMap<string, string> ();
      protocol.send_command (command, options);
      start_timeout_timer ();
    } catch (Error err) {
      on_error (new ConsoleError.PROCESSING ("failed to send command '%s': %s", command, err.message));
    }
  }

  private void send_data (uint8[] data) {
    transport.send (data);
    log (Config.LOG_DOMAIN, LEVEL_DEBUG, "sent data to %s:%u: length = %ld", host, port, data.length);
  }

  private void on_data_received (uint8[] data) {
    try {
      protocol.process_response (data);
    } catch (Error err) {
      on_error (new ConsoleError.PROCESSING ("failed to process data from %s:%u: %s", host, port, err.message));
    }
  }

  private void on_transport_error (Error err) {
    on_error (err);
  }

  private void on_response (string response) {
    stop_timeout_timer ();
    response_received (response);
  }

  private void on_error (Error err) {
    error_occured (err);
  }

  private void start_timeout_timer () {
    stop_timeout_timer ();

    timeout_source = Timeout.add (TIMEOUT_MS, () => {
      on_error (new ConsoleError.TIMEOUT ("failed to query %s:%d: %s", host, port, "failed to receive a response in reasonable amount of time"));
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
