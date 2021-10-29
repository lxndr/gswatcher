using Gtk;

namespace Gsw.Ui {

  [GtkTemplate (ui = "/org/lxndr/gswatcher/ui/main-window.ui")]
  class MainWindow : ApplicationWindow {
    public QuerierManager querier_manager { get; construct set; }
    public Gsw.BuddyList buddy_list { get; construct set; }

    class construct {
      typeof (Ui.ServerList).ensure ();
      typeof (Ui.BuddyList).ensure ();
      typeof (Ui.Preferences).ensure ();
    }

    public MainWindow (Application application, QuerierManager querier_manager, Gsw.BuddyList buddy_list) {
      Object (
        application : application,
        querier_manager : querier_manager,
        buddy_list : buddy_list
      );
    }

    [GtkCallback]
    private void remove_buddy (Buddy buddy) {
      var dialog = new MessageDialog (this, DialogFlags.DESTROY_WITH_PARENT | DialogFlags.MODAL, MessageType.QUESTION, ButtonsType.YES_NO,
        _("Are you sure you want to remove \"%s\" the list?"), buddy.name);

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
