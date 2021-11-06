using Gtk;

namespace Gsw.Ui {

  void show_about (Window win, Gee.Collection<TransportDesc> transports, Gee.Collection<ProtocolDesc> protocols) {
    string[] authors = {
      "Alexander Burobin <lxndr87i@gmail.com>",
      null
    };

    var sysinfo = "GTK version:\n";
    sysinfo += "\tBuild time: %s\n".printf (get_buildtime_gtk_version ());
    sysinfo += "\tRun time: %s\n".printf (get_runtime_gtk_version ());

    sysinfo += "\nDuktape version:\n";
    sysinfo += "\tBuild time: %s\n".printf (get_buildtime_duktape_version ());
    sysinfo += "\tRun time: %s\n".printf (get_runtime_duktape_version ());

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

    show_about_dialog (
      win,
      "authors", authors,
      "license-type", License.GPL_3_0,
      "logo-icon-name", Environment.get_prgname (),
      "version", Config.VERSION,
      "website", Config.HOMEPAGE,
      "system-information", sysinfo
    );
  }

}
