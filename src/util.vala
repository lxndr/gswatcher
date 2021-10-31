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

  bool parse_address (string address, out string host, out uint16 gport, out uint16 qport) {
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
    gport = qport = (uint16) uint.parse (p2);

    if (p3 != null && p3.length > 0)
      qport = (uint16) uint.parse (p3);

    return true;
  }

  public void print_duktape_stack (Duktape.Duktape vm) {
    var len = vm.get_top ();
    print ("Duktape stack:\n");

    for (int idx = len - 1; idx >= 0; idx--) {
      vm.dup (idx);
      print ("| %d: %s = %s\n", idx, vm.get_type (-1).to_string (), vm.safe_to_string (-1));
      vm.pop ();
    }
  }

  string format_time_duration (int64 microseconds) {
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
}
