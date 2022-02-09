using Gtk;

namespace Gsw.Ui {

  [GtkTemplate (ui = "/org/lxndr/gswatcher/ui/main-window.ui")]
  class MainWindow : Adw.ApplicationWindow {
    public QuerierManager querier_manager { get; construct; }
    public Gsw.BuddyList buddy_list { get; construct; }

    class construct {
      typeof (Ui.ServerList).ensure ();
      typeof (Ui.ServerInfo).ensure ();
      typeof (Ui.ServerConsole).ensure ();
      typeof (Ui.ServerDetails).ensure ();
      typeof (Ui.BuddyList).ensure ();
      typeof (Ui.PlayerList).ensure ();
      typeof (Ui.Preferences).ensure ();
    }

    public MainWindow (Gtk.Application application, QuerierManager querier_manager, Gsw.BuddyList buddy_list) {
      Object (
        application : application,
        querier_manager : querier_manager,
        buddy_list : buddy_list
      );
    }

    [GtkCallback]
    private void add_server (string address) {
      querier_manager.server_list.add (address);
    }

    [GtkCallback]
    private void remove_server (Server server) {
      var dialog = new MessageDialog (this, DialogFlags.DESTROY_WITH_PARENT | DialogFlags.MODAL, MessageType.QUESTION, ButtonsType.YES_NO,
        _("Are you sure you want to remove \"%s\" from the server list?"), server.address);

      dialog.response.connect ((response_id) => {
        if (response_id == ResponseType.YES) {
          querier_manager.server_list.remove (server);
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

    [GtkCallback]
    private bool can_show_sinfo (Querier? querier) {
      return querier != null && !(querier is NewQuerier);
    }
  }

}
