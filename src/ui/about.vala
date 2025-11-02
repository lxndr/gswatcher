/*
  Game Server Watcher
  Copyright (C) 2010-2026 Alexander Burobin

  This program is free software: you can redistribute it and/or modify
  it under the terms of the GNU Affero General Public License as
  published by the Free Software Foundation, either version 3 of the
  License, or (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU Affero General Public License for more details.

  You should have received a copy of the GNU Affero General Public License
  along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

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

    sysinfo += "\nVala version:\n";
    sysinfo += "\tBuild time: %s\n".printf (Config.VALA_VERSION);

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
      "Alexander Burobin <alexandrburobin@gmail.com>",
      null,
    };

#if GSW_ADWAITA_1_5_SUPPORTED
    var dlg = new Adw.AboutDialog ();
    dlg.application_name = Config.APPNAME;
    dlg.application_icon = Config.APPID;
    dlg.version = Config.VERSION;
    dlg.website = Config.HOMEPAGE;
    dlg.developers = authors;
    dlg.license_type = License.AGPL_3_0;
    dlg.issue_url = Config.ISSUES;
    dlg.debug_info = format_debug_info (transports, protocols);
    dlg.present (win);
#elif GSW_ADWAITA_1_2_SUPPORTED
    var dlg = new Adw.AboutWindow ();
    dlg.application_name = Config.APPNAME;
    dlg.application_icon = Config.APPID;
    dlg.version = Config.VERSION;
    dlg.website = Config.HOMEPAGE;
    dlg.developers = authors;
    dlg.license_type = License.AGPL_3_0;
    dlg.issue_url = Config.ISSUES;
    dlg.debug_info = format_debug_info (transports, protocols);
    dlg.transient_for = win;
    dlg.present ();
#else
    var dlg = new AboutDialog ();
    dlg.program_name = Config.APPNAME;
    dlg.logo_icon_name = Config.APPID;
    dlg.version = Config.VERSION;
    dlg.website = Config.HOMEPAGE;
    dlg.authors = authors;
    dlg.license_type = License.AGPL_3_0;
    dlg.system_information = format_debug_info (transports, protocols);
    dlg.transient_for = win;
    dlg.modal = true;
    dlg.present ();
#endif
  }
}
