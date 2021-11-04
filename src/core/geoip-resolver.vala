namespace Gsw {

[SingleInstance]
public class GeoIPResolver : Object {
  private GeoIP.GeoIP geoip;

  public static GeoIPResolver get_instance () {
    return new GeoIPResolver ();
  }

  construct {
    geoip = new GeoIP.GeoIP (STANDARD);
  }

  public int id_by_addr (string addr) {
    return geoip.id_by_addr (addr);
  }
}

}
