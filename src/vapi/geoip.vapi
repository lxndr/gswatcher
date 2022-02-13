[Compact]
[CCode (free_function = "GeoIP_delete", cname = "GeoIP", cprefix = "GeoIP_", cheader_filename = "GeoIP.h")]
public class GeoIP {
  [Flags]
  [CCode (cname = "GeoIPOptions", has_type_id = false, cprefix = "GEOIP_")]
  public enum Options {
    STANDARD,
    MEMORY_CACHE,
    CHECK_CACHE,
    INDEX_CACHE,
    MMAP_CACHE,
    SILENCE,
  }

  [CCode (cname = "int", has_type_id = false)]
  public enum Edition {
    [CCode (cname = "GEOIP_COUNTRY_EDITION")]
    COUNTRY,
    [CCode (cname = "GEOIP_COUNTRY_EDITION_V6")]
    COUNTRY_V6,
    [CCode (cname = "GEOIP_CITY_EDITION_REV1")]
    CITY_REV1,
    [CCode (cname = "GEOIP_REGION_EDITION_REV1")]
    REGION_REV1,
  }

  [CCode (cname = "GeoIPRecord", has_type_id = false, free_function = "GeoIPRecord_delete", cheader_filename = "GeoIPCity.h")]
  public struct Record {
    public string country_code;
    public string country_code3;
    public string country_name;
    public string region;
    public string city;
    public string postal_code;
    public float latitude;
    public float longitude;
    public int metro_code;
    public int dma_code;
    public int area_code;
    public int charset;
    public string continent_code;
    public int netmask;
  }

  [CCode (cname = "GeoIP_new")]
  public GeoIP (Options options);
  [CCode (cname = "GeoIP_open_type")]
  public GeoIP.open_type (Edition edition, Options options);

  public uchar database_edition ();
  public int id_by_addr (string addr);
  public int id_by_addr_v6 (string addr);

  [CCode (cheader_filename = "GeoIPCity.h")]
  public Record? record_by_addr(string addr);

  public static unowned string code_by_id (int id);
  public static unowned string name_by_id (int id);
}
