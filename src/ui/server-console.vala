using Gtk;

namespace Gsw.Ui {

[GtkTemplate (ui = "/ru/lxndr/gswatcher/ui/server-console.ui")]
class ServerConsole : Widget {
  public Client? _client;

  [GtkChild]
  private unowned Gtk.TextView text_view;

  public Client? client {
    get {
      return _client;
    }

    set {
      _client = value;

      if (_client?.console_log_buffer != null) {
        text_view.buffer = _client.console_log_buffer;

        TextIter iter;
        text_view.buffer.get_end_iter (out iter);
        var mark = text_view.buffer.create_mark (null, iter, true);
        text_view.scroll_mark_onscreen (mark);
        text_view.buffer.delete_mark (mark);
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
}

}
