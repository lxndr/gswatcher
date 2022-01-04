using Gtk;

namespace Gsw.Ui {

[GtkTemplate (ui = "/org/lxndr/gswatcher/ui/server-console.ui")]
class ServerConsole : Widget {
  private ConsoleClient? _client;

  [GtkChild]
  private unowned TextBuffer log_buffer;

  class construct {
    set_layout_manager_type (typeof (BinLayout));
  }

  protected override void dispose () {
    get_first_child ().unparent ();
    base.dispose ();
  }

  public ConsoleClient client {
    get {
      return _client;
    }

    set {
      if (_client != null) {
        _client.response_received.disconnect (on_response_received);
        _client.error_occured.disconnect (on_error_occured);
      }

      _client = value;

      if (_client != null) {
        _client.response_received.connect (on_response_received);
        _client.error_occured.connect (on_error_occured);
      }
    }
  }

  [GtkCallback]
  private void entry_activated (Entry entry) {
    send_command (entry.text);
    entry.text = "";
  }

  [GtkCallback]
  private void entry_icon_released (Entry entry, EntryIconPosition icon_pos) {
    if (icon_pos == EntryIconPosition.SECONDARY) {
      send_command (entry.text);
      entry.text = "";
    }
  }

  private void send_command (string cmd) {
    log (cmd);

    client.exec_command (cmd);
  }

  private void log (string msg) {
    TextIter iter;
    log_buffer.get_end_iter (out iter);
    log_buffer.insert (ref iter, msg, -1);
    log_buffer.insert (ref iter, "\n", -1);
  }

  private void on_response_received (string response) {
    log (response);
  }

  private void on_error_occured (Error err) {
    print ("Error: %s\n", err.message);
    log (err.message);
  }

}

}
