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

using Gtk;

namespace Gsw.Ui {

[GtkTemplate (ui = "/io/github/lxndr/gswatcher/ui/server-console.ui")]
class ServerConsole : Widget {
  public Client? _client;

  [GtkChild]
  private unowned TextView text_view;

  [GtkChild]
  private unowned Entry entry;

  [GtkChild]
  private unowned Popover options_popover;

  private Gee.BidirIterator<string>? history_iter;

  public Client? client {
    get {
      return _client;
    }

    set {
      if (_client != null) {
        _client.notify["console-command-history"].disconnect(handle_history_change);
        _client.notify["console-log-buffer"].disconnect(handle_log_buffer_change);

        if (_client.console_log_buffer != null)
          _client.console_log_buffer.changed.disconnect(scroll_to_bottom);
      }

      _client = value;

      if (_client != null) {
        _client.notify["console-command-history"].connect(handle_history_change);
        _client.notify["console-log-buffer"].connect(handle_log_buffer_change);
        handle_log_buffer_change ();

        if (_client.console_log_buffer != null) {
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

  private void handle_log_buffer_change () {
    if (_client.console_log_buffer != null)
      text_view.buffer = _client.console_log_buffer;
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

  [GtkCallback]
  private void on_options_values_changed (uint16 port, string password) {
    client.console_port = port;
    client.console_password = password;
    options_popover.popdown ();
    entry.grab_focus ();
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
