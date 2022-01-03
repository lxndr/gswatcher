using Gtk;

namespace Gsw.Ui {

[GtkTemplate (ui = "/org/lxndr/gswatcher/ui/server-console.ui")]
class ServerConsole : Widget {
  public ConsoleClient client { get; set; }

  [GtkChild]
  private unowned TextBuffer log_buffer;

  class construct {
    set_layout_manager_type (typeof (BinLayout));
  }

  protected override void dispose () {
    get_first_child ().unparent ();
    base.dispose ();
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
  }

  private void log (string msg) {
    TextIter iter;
    log_buffer.get_end_iter (out iter);
    log_buffer.insert (ref iter, msg, -1);
    log_buffer.insert (ref iter, "\n", -1);
  }
}

}
