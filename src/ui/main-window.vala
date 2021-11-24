using Gtk;

namespace Gsw.Ui {

  [GtkTemplate (ui = "/org/lxndr/gswatcher/ui/main-window.ui")]
  class MainWindow : ApplicationWindow {
    public QuerierManager querier_manager { get; construct; }
    public Gsw.BuddyList buddy_list { get; construct; }

    protected Querier selected_querier { get; private set; }

    class construct {
      typeof (Ui.ServerList).ensure ();
      typeof (Ui.ServerInfo).ensure ();
      typeof (Ui.BuddyList).ensure ();
      typeof (Ui.PlayerList).ensure ();
      typeof (Ui.Preferences).ensure ();

      var display = Gdk.Display.get_default ();
      var style_provider = new CssProvider ();
      style_provider.load_from_resource ("/org/lxndr/gswatcher/ui/styles.css");
      StyleContext.add_provider_for_display (display, style_provider, uint.MAX);
    }

    public MainWindow (Application application, QuerierManager querier_manager, Gsw.BuddyList buddy_list) {
      Object (
        application : application,
        querier_manager : querier_manager,
        buddy_list : buddy_list
      );
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
    private void querier_selection_change (Querier querier) {
      selected_querier = querier;
    }
  }

}
