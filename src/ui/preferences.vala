using Gtk;

namespace Gsw.Ui {

  [GtkTemplate (ui = "/org/lxndr/gswatcher/ui/preferences.ui")]
  class Preferences : Widget {
    class construct {
      set_layout_manager_type (typeof (BinLayout));
    }

    protected override void dispose () {
      get_first_child ().unparent ();
      base.dispose ();
    }
  }

}
