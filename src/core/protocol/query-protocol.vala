namespace Gsw {

public abstract interface QueryProtocol : Protocol {
  public signal void details_update (Gee.Map<string, string> details);
  public signal void sinfo_update (ServerInfo sinfo);
  public signal void plist_update (Gee.ArrayList<Player> plist);

  public abstract void query () throws Error;
}

}
