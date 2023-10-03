using Gtk;

namespace Gsw.Ui {

[GtkTemplate (ui = "/com/github/lxndr/gswatcher/ui/server-console.ui")]
class ServerConsole : Widget {
  public Client? _client;

  [GtkChild]
  private unowned Gtk.TextView text_view;

  private Gee.BidirIterator<string>? history_iter;

  public Client? client {
    get {
      return _client;
    }

    set {
      if (_client != null) {
        _client.notify["console-command-history"].disconnect(handle_history_change);

        if (_client.console_log_buffer != null)
          _client.console_log_buffer.changed.disconnect(scroll_to_bottom);
      }

      _client = value;

      if (_client != null) {
        _client.notify["console-command-history"].connect(handle_history_change);

        if (_client.console_log_buffer != null) {
          text_view.buffer = _client.console_log_buffer;
          _client.console_log_buffer.changed.connect(scroll_to_bottom);
          scroll_to_bottom ();
        }
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

  private void handle_history_change () {
    history_iter = null;
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

  [GtkCallback]
  private bool entry_key_pressed (EventControllerKey controller, uint keyval, uint keycode, Gdk.ModifierType state) {
    if (client == null)
      return false;

    var entry = (Entry) controller.widget;

    switch (keyval) {
      case Gdk.Key.Up:
        if (history_iter != null && history_iter.valid) {
          if (history_iter.has_previous ()) {
            history_iter.previous ();
          } else {
            error_bell ();
            return true;
          }
        } else {
          history_iter = client.console_command_history.bidir_list_iterator ();

          if (!history_iter.last ()) {
            history_iter = null;
            error_bell ();
            return true;
          }
        }

        break;
      case Gdk.Key.Down:
        if (history_iter != null && history_iter.valid) {
          if (history_iter.has_next ()) {
            history_iter.next ();
          } else {
            entry.text = "";
            history_iter = null;
            return true;
          }
        } else {
          error_bell ();
          return true;
        }

        break;
      default:
        return false;
    }

    entry.text = history_iter.get ();
    entry.set_position ((int) entry.text_length);
    return true;
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
