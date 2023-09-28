using Gtk;

namespace Gsw.Ui {

[GtkTemplate (ui = "/ru/lxndr/gswatcher/ui/server-details.ui")]
class ServerDetails : Widget {
  public Client? client { get; set; }

  class construct {
    typeof (ServerDetailsEntry).ensure ();

    set_layout_manager_type (typeof (BinLayout));
  }

  protected override void dispose () {
    get_first_child ().unparent ();
    base.dispose ();
  }
}

}
