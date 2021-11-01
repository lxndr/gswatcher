[CCode (lower_case_cprefix = "GeoIP_", cheader_filename = "GeoIP.h")]
namespace GeoIP {

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
  }

	[Compact]
	[CCode (free_function = "GeoIP_delete", cname = "GeoIP", cprefix = "GeoIP_")]
	public class GeoIP {
    public GeoIP (Options options);

    public uchar database_edition ();
    public int id_by_addr (string addr);
    public int id_by_addr_v6 (string addr);

    public static unowned string code_by_id (int id);
    public static unowned string name_by_id (int id);
  }

}
