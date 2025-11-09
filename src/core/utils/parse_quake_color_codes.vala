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

namespace Gsw {

Regex color_re;

const string[] COLORS = {
  "black",
  "red",
  "green",
  "yellow",
  "blue",
  "cyan",
  "magenta",
  "white",
  "orange"
};

public string parse_quake_color_codes (string input) {
  init_color_regex ();

  var result = new StringBuilder ();
  var parts = color_re.split (input);
  unowned string? current_color = null;

  foreach (var part in parts) {
    if (part.length == 2 && part[0] == '^' && part[1] >= '0' && part[1] <= '8') {
      var code = (int) (part[1] - '0');
      current_color = COLORS[code];
    } else {
      var escaped_text = Markup.escape_text (part);

      if (current_color == null || part.length == 0) {
        result.append (escaped_text);
      } else {
        result.append_printf ("<span color=\"%s\">%s</span>", current_color, escaped_text);
      }
    }
  }

  return result.str;
}

private void init_color_regex () {
  if (color_re == null) {
    try {
      color_re = new Regex ("(\\^[0-8])", 0, 0);
    } catch (Error err) {
      critical ("failed to create regexp: %s", err.message);
    }
  }
}

}
