using Gtk;

namespace Gsw.Ui {

  private string format_debug_info (Gee.Collection<TransportDesc> transports, Gee.Collection<ProtocolDesc> protocols) {
    var sysinfo = "GTK version:\n";
    sysinfo += "\tBuild time: %s\n".printf (get_buildtime_gtk_version ());
    sysinfo += "\tRun time: %s\n".printf (get_runtime_gtk_version ());

    sysinfo += "\nLua version:\n";
    sysinfo += "\tBuild time: %s\n".printf (LuaEx.get_buildtime_version ());

    try {
      var vm = new LuaEx ();
      sysinfo += "\tRun time: %s\n".printf (vm.get_runtime_version ());
    } catch (Error err) {
      sysinfo += "\tRun time: error getting version: %s\n".printf (err.message);
    }

    sysinfo += "\nRegistered transports:\n";

    foreach (var transport in transports) {
      sysinfo += "\t%s\n".printf (transport.id);
      sysinfo += "\t\tClass: %s\n".printf (transport.class_type.name ());
    }

    sysinfo += "\nRegistered protocols:\n";

    foreach (var protocol in protocols) {
      sysinfo += "\t%s\n".printf (protocol.id);
      sysinfo += "\t\tName: %s\n".printf (protocol.name);
      sysinfo += "\t\tTransport: %s\n".printf (protocol.transport);
      sysinfo += "\t\tClass: %s\n".printf (protocol.class_type.name ());

      for (var i = 0; i < protocol.class_params.length; i++) {
        var str_value = Value (typeof (string));
        protocol.class_values[i].transform (ref str_value);
        sysinfo += "\t\t\t%s = %s\n".printf (protocol.class_params[i], str_value.get_string ());
      }
    }

    return sysinfo;
  }

  void show_about (Window win, Gee.Collection<TransportDesc> transports, Gee.Collection<ProtocolDesc> protocols) {
    string[] authors = {
      "Alexander Burobin <lxndr87i@gmail.com>",
      null,
    };

    var dlg = new Adw.AboutWindow ();
    dlg.application_name = _("Game Server Watcher");
    dlg.application_icon = Environment.get_prgname ();
    dlg.version = Config.VERSION;
    dlg.website = Config.HOMEPAGE;
    dlg.developers = authors;
    dlg.license_type = License.GPL_3_0;
    dlg.issue_url = Config.ISSUES;
    dlg.debug_info = format_debug_info (transports, protocols);
    dlg.transient_for = win;
    dlg.present ();
  }

}
