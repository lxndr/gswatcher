using Gtk;

namespace Gsw.Ui {

[GtkTemplate (ui = "/com/github/lxndr/gswatcher/ui/console-option-list.ui")]
class ConsoleOptionList : Widget {
  private Server? _server;

  [GtkChild]
  private unowned Adjustment port_entry;

  [GtkChild]
  private unowned Entry password_entry;

  class construct {
    set_layout_manager_type (typeof (BinLayout));
  }

  protected override void dispose () {
    get_first_child ().unparent ();
    base.dispose ();
  }

  public Server server {
    get {
      return _server;
    }

    set {
      port_entry.value_changed.disconnect (port_changed);
      password_entry.changed.disconnect (password_changed);

      _server = value;

      if (_server != null) {
        port_entry.value = _server.console_port;
        port_entry.value_changed.connect (port_changed);

        password_entry.text = _server.console_password;
        password_entry.changed.connect (password_changed);
      }
    }
  }

  private void port_changed (Adjustment entry) {
    server.console_port = (uint16) entry.value;
  }

  private void password_changed (Editable entry) {
    server.console_password = entry.text;
  }
}

}
