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

[SingleInstance]
public class GeoIPResolver : Object {
  private GeoIP? geoip;

  public Error? init_error;

  public static GeoIPResolver get_instance () {
    return new GeoIPResolver ();
  }

  construct {
    lock (geoip) {
      var db_fname = find_file_in_data_dirs ("GeoIP/GeoIPCity.dat");

      if (db_fname == null) {
        db_fname = find_file_in_data_dirs ("GeoIP/GeoIP.dat");
      }

      if (db_fname == null) {
        init_error = new IOError.FAILED (_("failed to find GeoIP database"));
        log (Config.LOG_DOMAIN, LEVEL_WARNING, init_error.message);
        return;
      }

      geoip = new GeoIP.open (db_fname, STANDARD | SILENCE);

      if (geoip == null) {
        init_error = new IOError.FAILED (_("failed to load GeoIP database"));
        log (Config.LOG_DOMAIN, LEVEL_WARNING, init_error.message);
      }
    }
  }

  public string code_by_addr (string addr) {
    lock (geoip) {
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
  }

  public string city_by_addr (string addr) {
    lock (geoip) {
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

}
