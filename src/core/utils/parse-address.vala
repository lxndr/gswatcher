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

static Regex address_re;

Regex create_address_re () ensures (result != null) {
  if (address_re == null) {
    try {
      address_re = new Regex ("^([a-z0-9\\.\\-]+):(\\d{1,5})(?::(\\d{1,5}))?$", RegexCompileFlags.CASELESS);
    } catch (Error err) {
      log (Config.LOG_DOMAIN, LEVEL_CRITICAL, "failed to create Regex: %s", err.message);
    }
  }

  return address_re;
}

public bool parse_address (string address, out string host, out uint16 gport, out uint16 qport) {
  host = address;
  gport = 0;
  qport = 0;

  MatchInfo match_info;
  var address_re = create_address_re ();

  if (!address_re.match (address, 0, out match_info))
    return false;

  if (!match_info.matches ())
    return false;

  var p1 = match_info.fetch (1);
  var p2 = match_info.fetch (2);
  var p3 = match_info.fetch (3);

  host = p1;
  gport = qport = (uint16) uint.parse (p2, 10);

  if (p3 != null && p3.length > 0)
    qport = (uint16) uint.parse (p3, 10);

  return true;
}

}
