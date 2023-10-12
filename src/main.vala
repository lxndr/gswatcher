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

public static int main (string[] args) {
  Environment.set_application_name (Gsw.Config.APPNAME);
  var data_dirs = Environment.get_system_data_dirs ();

  if (data_dirs.length > 0) {
    var locale_dir = Path.build_path (data_dirs[0], "locale");

    Intl.bindtextdomain (Gsw.Config.GETTEXT_PACKAGE, locale_dir);
    Intl.bind_textdomain_codeset (Gsw.Config.GETTEXT_PACKAGE, "UTF-8");
    Intl.textdomain (Gsw.Config.GETTEXT_PACKAGE);
  }

  var app = new Gsw.Application ();
  return app.run (args);
}
