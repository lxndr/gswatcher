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
