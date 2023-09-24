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
    var dialog = new Adw.MessageDialog (this, _("Remove server?"), null);
    dialog.format_body (_("Are you sure you want to remove \"%s\" from the server list?"), client.server.address);
    dialog.add_responses (
      "cancel", _("Cancel"),
      "remove", _("Remove")
    );
    dialog.set_response_appearance ("remove", DESTRUCTIVE);
    dialog.default_response = "cancel";
    dialog.close_response = "cancel";

    dialog.response.connect ((response_id) => {
      if (response_id == "remove")
        client_list.remove (client);
      dialog.destroy ();
    });

    dialog.present ();
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
      if (response_id == ResponseType.YES)
        buddy_list.remove (buddy);
      dialog.destroy ();
    });

    dialog.show ();
  }
}

}
