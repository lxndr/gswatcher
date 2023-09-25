using Gtk;

namespace Gsw.Ui {

[GtkTemplate (ui = "/org/lxndr/gswatcher/ui/main-window.ui")]
class MainWindow : Adw.ApplicationWindow {
  public Gsw.BuddyList buddy_list { get; construct; }
  public Gsw.ServerList client_list { get; construct; }

  class construct {
    typeof (Ui.ServerList).ensure ();
    typeof (Ui.ServerInfo).ensure ();
    typeof (Ui.ServerConsole).ensure ();
    typeof (Ui.ServerDetails).ensure ();
    typeof (Ui.BuddyList).ensure ();
    typeof (Ui.PlayerList).ensure ();
    typeof (Ui.Preferences).ensure ();
  }

  public MainWindow (Gtk.Application application, Gsw.ServerList client_list, Gsw.BuddyList buddy_list) {
    Object (
      application : application,
      client_list : client_list,
      buddy_list : buddy_list
    );
  }

  [GtkCallback]
  private void add_server (string address) {
    client_list.add (address);
  }

  [GtkCallback]
  private void remove_server (Client client) {
    var msg_heading = _("Remove server?");
    var msg_body = _("Are you sure you want to remove \"%s\" from the server list?").printf (client.server.address);

    show_remove_dialog.begin (this, msg_heading, msg_body, (obj, res) => {
      if (show_remove_dialog.end (res) == ShowRemoveDialogResponse.REMOVE) {
        client_list.remove (client);
      }
    });
  }

  [GtkCallback]
  private void add_buddy (string name) {
    if (buddy_list.exists (name)) {
      var msg = _("\"%s\" is already on your buddy list.").printf (name);
      show_error_dialog (this, msg);
      return;
    }

    buddy_list.add (name);
  }

  [GtkCallback]
  private void remove_buddy (Buddy buddy) {
    var msg_heading = _("Remove buddy?");
    var msg_body = _("Are you sure you want to remove \"%s\" from the buddy list?").printf (buddy.name);

    show_remove_dialog.begin (this, msg_heading, msg_body, (obj, res) => {
      if (show_remove_dialog.end (res) == ShowRemoveDialogResponse.REMOVE) {
        buddy_list.remove (buddy);
      }
    });
  }
}

}
