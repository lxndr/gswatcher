using Gtk;

namespace Gsw.Ui {

[GtkTemplate (ui = "/com/github/lxndr/gswatcher/ui/server-console.ui")]
class ServerConsole : Widget {
  public Client? _client;

  [GtkChild]
  private unowned Gtk.TextView text_view;

  public Client? client {
    get {
      return _client;
    }

    set {
      if (_client?.console_log_buffer != null) {
        _client.console_log_buffer.changed.disconnect(scroll_to_bottom);
      }

      _client = value;

      if (_client?.console_log_buffer != null) {
        text_view.buffer = _client.console_log_buffer;
        _client.console_log_buffer.changed.connect(scroll_to_bottom);
        scroll_to_bottom ();
      }
    }
  }

  class construct {
    typeof (ConsoleOptionList).ensure ();

    set_layout_manager_type (typeof (BinLayout));
  }

  protected override void dispose () {
    get_first_child ().unparent ();
    base.dispose ();
  }

  [GtkCallback]
  private void entry_activated (Entry entry) {
    client.send_console_command (entry.text);
    entry.text = "";
  }

  [GtkCallback]
  private void entry_icon_released (Entry entry, EntryIconPosition icon_pos) {
    if (icon_pos == EntryIconPosition.SECONDARY) {
      client.send_console_command (entry.text);
      entry.text = "";
    }
  }

  private void scroll_to_bottom () {
    TextIter iter;
    text_view.buffer.get_end_iter (out iter);
    var mark = text_view.buffer.create_mark (null, iter, true);
    text_view.scroll_to_mark (mark, 0, true, 0, 0);
    text_view.buffer.delete_mark (mark);
  }
}

}
