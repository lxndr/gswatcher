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

namespace Gsw {

  public string get_runtime_gtk_version () {
    return "%u.%02u.%02u".printf (Gtk.get_major_version (), Gtk.get_minor_version (), Gtk.get_minor_version ());
  }

  public string get_buildtime_gtk_version () {
    return "%u.%02u.%02u".printf (Gtk.MAJOR_VERSION, Gtk.MINOR_VERSION, Gtk.MICRO_VERSION);
  }

  public string format_time_duration (int64 microseconds) {
    var hours = microseconds / TimeSpan.HOUR;
    microseconds -= hours * TimeSpan.HOUR;
    var minutes = microseconds / TimeSpan.MINUTE;
    microseconds -= minutes * TimeSpan.MINUTE;
    var seconds = microseconds / TimeSpan.SECOND;

    if (hours > 0) {
      return "%lld:%02lld:%02lld".printf (hours, minutes, seconds);
    }

    return "%lld:%02lld".printf (minutes, seconds);
  }

  public bool settings_nullable_string_getter (Value value, Variant variant, void* user_data) {
    if (!variant.is_of_type (VariantType.MAYBE))
      return false;

    var maybe = variant.get_maybe ();

    if (maybe == null) {
      value.set_string (null);
      return true;
    }

    if (!maybe.is_of_type (VariantType.STRING))
      return false;

    var str = maybe.get_string ();
    value.set_string (str);
    return true;
  }

  public Variant settings_nullable_string_setter (Value value, VariantType expected_type, void* user_data) {
    var str = value.get_string ();
    var variant = str == null ? null : new Variant.string (str);
    return new Variant.maybe (VariantType.STRING, variant);
  }

  public bool settings_nullable_datetime_getter (Value value, Variant variant, void* user_data) {
    if (!variant.is_of_type (VariantType.MAYBE))
      return false;

    var maybe = variant.get_maybe ();

    if (maybe == null) {
      value.set_boxed (null);
      return true;
    }

    if (!maybe.is_of_type (VariantType.STRING))
      return false;

    var str = maybe.get_string ();

    if (str.length == 0) {
      value.set_boxed (null);
      return true;
    }

    var time = new DateTime.from_iso8601 (str, null);

    if (time == null)
      return false;

    value.set_boxed (time);
    return true;
  }

  public Variant settings_nullable_datetime_setter (Value value, VariantType expected_type, void* user_data) {
    var time = (DateTime?) value.get_boxed ();
    var variant = time == null ? null : new Variant.string (time.format_iso8601 ());
    return new Variant.maybe (VariantType.STRING, variant);
  }
}
