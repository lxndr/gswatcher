/*
  Game Server Watcher
  Copyright (C) 2023  Alexander Burobin

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

  double ms2s (uint32 secs) {
    return (double) secs / 1000;
  }

  uint32 s2ms (double ms) {
    return (uint32) (ms * 1000);
  }

  [GtkTemplate (ui = "/io/github/lxndr/gswatcher/ui/preferences.ui")]
  class Preferences : Widget {
    [GtkChild]
    private unowned SpinButton query_interval;

    [GtkChild]
    private unowned Adjustment query_interval_adjustment;

    [GtkChild]
    private unowned SpinButton local_udp_port;

    [GtkChild]
    private unowned Adjustment local_udp_port_adjustment;

    [GtkChild]
    private unowned Switch enable_notifications;

    class construct {
      set_layout_manager_type (typeof (BinLayout));
      set_css_name ("preferences");
    }

    construct {
      var settings_file = Path.build_filename (Environment.get_user_config_dir (), "gswatcher", "preferences.ini");
      var settings_backend = SettingsBackend.keyfile_settings_backend_new (settings_file, "/io/github/lxndr/gswatcher/", null);
      var settings = new GLib.Settings.with_backend (@"$(Config.APPID).Preferences", settings_backend);
      var schema = settings.settings_schema;

      // query interval
      var range_tuple = schema.get_key ("query-interval").get_range ().get_child_value (1).get_variant ();
      query_interval_adjustment.lower = ms2s (range_tuple.get_child_value (0).get_uint32 ());
      query_interval_adjustment.upper = ms2s (range_tuple.get_child_value (1).get_uint32 ());

      settings.bind_with_mapping (
        "query-interval",
        query_interval,
        "value",
        DEFAULT,
        (value, variant, user_data) => {
          var secs = ms2s (variant.get_uint32 ());
          value.set_double (secs);
          return true;
        },
        (value, expected_type, user_data) => {
          var ms = s2ms (value.get_double ());
          return new Variant.uint32 (ms);
        },
        null,
        null
      );

      // local udp port
      range_tuple = schema.get_key ("local-udp-port").get_range ().get_child_value (1).get_variant ();
      local_udp_port_adjustment.lower = (double) range_tuple.get_child_value (0).get_uint16 ();
      local_udp_port_adjustment.upper = (double) range_tuple.get_child_value (1).get_uint16 ();
      settings.bind ("local-udp-port", local_udp_port, "value", DEFAULT);

      // enable notifications
      settings.bind ("enable-notifications", enable_notifications, "active", DEFAULT);
    }

    protected override void dispose () {
      get_first_child ().unparent ();
      base.dispose ();
    }
  }

}
