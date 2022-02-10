namespace Gsw {

class Game {
  public string id;
  public string protocol;
  public Gee.List<string> features = new Gee.ArrayList<string> ();
  public uint16 port;
  public int16 qport_diff;
  public Gee.Map<string, Expression> inf_matches = new Gee.HashMap<string, Expression> ();
  public Gee.Map<string, Expression> inf = new Gee.HashMap<string, Expression> ();
  public Gee.List<PlayerField> pfields = new Gee.ArrayList<PlayerField> ();
  public Gee.Map<string, Gee.Map<string, string>> maps = new Gee.HashMap<string, Gee.Map<string, string>> ();

  public Game (string id, string protocol) {
    this.id = id;
    this.protocol = protocol;
  }

  public bool matches (string protocol_id, Gee.Map<string, string> details) {
    if (protocol != protocol_id)
      return false;

    var ctx = new Gee.HashMap<string, Gee.Map<string, string>> ();
    ctx.set ("inf", details);

    foreach (var entry in inf_matches.entries)
      if (details.get (entry.key) != entry.value.eval (ctx))
        return false;

    return true;
  }
}

}
