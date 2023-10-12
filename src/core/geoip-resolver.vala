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

[SingleInstance]
public class GeoIPResolver : Object {
  private GeoIP geoip;

  public static GeoIPResolver get_instance () {
    return new GeoIPResolver ();
  }

  construct {
    geoip = new GeoIP.open_type (CITY_REV1, STANDARD);

    if (geoip == null) {
      log (Config.LOG_DOMAIN, LEVEL_WARNING, "Failed to load GeoIPCity database");

      geoip = new GeoIP.open_type (COUNTRY, STANDARD);

      if (geoip == null) {
        log (Config.LOG_DOMAIN, LEVEL_ERROR, "Failed to load GeoIP database");
      }
    }
  }

  public string code_by_addr (string addr) {
    if (geoip == null)
      return "";

    switch (geoip.database_edition ()) {
      case GeoIP.Edition.CITY_REV1: {
        var rec = geoip.record_by_addr (addr);
        return rec == null ? "" : rec.country_code;
      }
      case GeoIP.Edition.COUNTRY: {
        var country_id = geoip.id_by_addr (addr);
        return GeoIP.code_by_id (country_id);
      }
      default:
        return "";
    }
  }

  public string city_by_addr (string addr) {
    if (geoip == null)
      return "";

    switch (geoip.database_edition ()) {
      case GeoIP.Edition.CITY_REV1: {
        var record = geoip.record_by_addr (addr);

        if (record == null)
          return "";

        return record.city == null
          ? record.country_name
          : "%s, %s".printf (record.country_name, record.city);
      }
      case GeoIP.Edition.COUNTRY: {
        var country_id = geoip.id_by_addr (addr);
        return GeoIP.name_by_id (country_id);
      }
      default:
        return "";
    }
  }
}

}
