namespace Gsw {

public class Buddy : Object {
  public string name { get; set; }
  public DateTime lastseen { get; set; }
  public string lastaddr { get; set; }
  public bool notifications { get; set; }

  public Buddy (string name) {
    Object (name : name);
  }
}

}
