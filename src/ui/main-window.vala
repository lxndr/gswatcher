using Gtk;

namespace Gsw.Ui {

[GtkTemplate (ui = "/org/lxndr/gswatcher/ui/main-window.ui")]
class MainWindow : Adw.ApplicationWindow {
  public Gsw.ServerList server_list { get; construct; }
  public Gsw.BuddyList buddy_list { get; construct; }
  public MapListModel client_list { get; private set; }

  class construct {
    typeof (Ui.ServerList).ensure ();
    typeof (Ui.ServerInfo).ensure ();
    typeof (Ui.ServerConsole).ensure ();
    typeof (Ui.ServerDetails).ensure ();
    typeof (Ui.BuddyList).ensure ();
    typeof (Ui.PlayerList).ensure ();
    typeof (Ui.Preferences).ensure ();
  }

  construct {
    client_list = new MapListModel (
      server_list,
      (obj) => new Client ((Gsw.Server) obj)
    );
  }

  public MainWindow (Gtk.Application application, Gsw.ServerList server_list, Gsw.BuddyList buddy_list) {
    Object (
      application : application,
      server_list : server_list,
      buddy_list : buddy_list
    );
  }

  [GtkCallback]
  private void add_server (string address) {
    server_list.add (address);
  }

  [GtkCallback]
  private void remove_server (Server server) {
    var dialog = new MessageDialog (this, DialogFlags.DESTROY_WITH_PARENT | DialogFlags.MODAL, MessageType.QUESTION, ButtonsType.YES_NO,
      _("Are you sure you want to remove \"%s\" from the server list?"), server.address);

    dialog.response.connect ((response_id) => {
      if (response_id == ResponseType.YES) {
        server_list.remove (server);
      }

      dialog.destroy ();
    });

    dialog.show ();
  }

  [GtkCallback]
  private void add_buddy (string name) {
    if (buddy_list.exists (name)) {
      var dialog = new MessageDialog (this, DialogFlags.DESTROY_WITH_PARENT | DialogFlags.MODAL, MessageType.ERROR, ButtonsType.OK,
        _("\"%s\" is already on your buddy list."), name);
      dialog.response.connect (() => dialog.destroy ());
      dialog.show ();
      return;
    }

    buddy_list.add (name);
  }

  [GtkCallback]
  private void remove_buddy (Buddy buddy) {
    var dialog = new MessageDialog (this, DialogFlags.DESTROY_WITH_PARENT | DialogFlags.MODAL, MessageType.QUESTION, ButtonsType.YES_NO,
      _("Are you sure you want to remove \"%s\" from the buddy list?"), buddy.name);

    dialog.response.connect ((response_id) => {
      if (response_id == ResponseType.YES) {
        buddy_list.remove (buddy);
      }

      dialog.destroy ();
    });

    dialog.show ();
  }
}

}
