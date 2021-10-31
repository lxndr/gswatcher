using Gtk;

namespace Gsw.Ui {

[GtkTemplate (ui = "/org/lxndr/gswatcher/ui/server-info.ui")]
class ServerInfo : Widget {
  public Querier? querier { get; set; }

  class construct {
    set_layout_manager_type (typeof (BinLayout));
  }

  construct {
    var style_provider = new CssProvider ();
    style_provider.load_from_resource ("/org/lxndr/gswatcher/ui/server-info.css");
    get_style_context ().add_provider (style_provider, uint.MAX);
  }

  public override void dispose () {
    get_first_child ().unparent ();
    base.dispose ();
  }
}

}
