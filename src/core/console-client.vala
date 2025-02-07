/*
  Game Server Watcher
  Copyright (C) 2023  Alexander Burobin

  This program is free software: you can redistribute it and/or modify
  it under the terms of the GNU Affero General Public License as
  published by the Free Software Foundation, either version 3 of the
  License, or (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU Affero General Public License for more details.

  You should have received a copy of the GNU Affero General Public License
  along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

namespace Gsw {

errordomain ConsoleError {
  SENDING,
  PROCESSING,
  AUTH_FAILED,
  TIMEOUT,
}

public class ConsoleClient : Object {
  public string host { get; construct; }
  public uint16 port { get; construct; }
  public string password { get; construct; }
  public ConsoleProtocol protocol { get; construct; }
  public NetTransport transport { get; construct; }

  private uint timeout_id;

  public signal void connected ();
  public signal void disconnected ();
  public signal void authenticated ();
  public signal void response_received (string response);
  public signal void error_occurred (Error error);

  construct {
    protocol.data_send.connect (send_data);
    protocol.response.connect (on_response);

    transport = TransportRegistry.get_instance ().create_net_transport (protocol.info.transport, host, port);
    transport.connected.connect (() => connected ());
    transport.authenticated.connect (() => authenticated ());
    transport.disconnected.connect (() => disconnected ());
    transport.data_received.connect (on_data_received);
    transport.error.connect (on_transport_error);
  }

  public ConsoleClient (string host, uint16 port, string password, ConsoleProtocol protocol) {
    Object (
      host : host,
      port : port,
      password : password,
      protocol : protocol
    );
  }

  ~ConsoleClient () {
    stop_timeout_timer ();
  }

  public void exec_command (string command) {
    try {
      var options = new Gee.HashMap<string, string> ();
      options.set ("password", password);
      protocol.send_command (command, options);
      start_timeout_timer ();
    } catch (Error err) {
      on_error (new ConsoleError.PROCESSING ("failed to send command '%s': %s", command, err.message));
    }
  }

  private void send_data (uint8[] data) {
    transport.send (data);
    log (Config.LOG_DOMAIN, LEVEL_DEBUG, "sent data to %s:%u: length = %ld", transport.host, transport.port, data.length);
  }

  private void on_data_received (uint8[] data) {
    try {
      protocol.process_response (data);
    } catch (Error err) {
      if (err is ProtocolError.AUTH_FAILED)
        on_error (new ConsoleError.AUTH_FAILED ("authentication failed"));
      else
        on_error (new ConsoleError.PROCESSING ("failed to process data from %s:%u: %s", transport.host, transport.port, err.message));
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
    stop_timeout_timer ();
    error_occurred (err);
  }

  private void start_timeout_timer () {
    stop_timeout_timer ();

    timeout_id = Timeout.add (TIMEOUT_MS, () => {
      on_error (new ConsoleError.TIMEOUT ("failed to query %s:%d: %s", transport.host, transport.port, "failed to receive a response in reasonable amount of time"));
      timeout_id = 0;
      return Source.REMOVE;
    });
  }

  private void stop_timeout_timer () {
    if (timeout_id > 0) {
      Source.remove (timeout_id);
      timeout_id = 0;
    }
  }
}

}
