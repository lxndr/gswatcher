namespace Gsw {

public delegate void Callback ();

public abstract class ScriptProtocol : Protocol {
  private Gee.List<Source> callback_sources = new Gee.LinkedList<Source>();

  ~ScriptProtocol() {
    foreach (var source in callback_sources)
      source.destroy ();
  }

  public void enqueue_callback (owned Callback cb) {
    var source = new IdleSource ();

    source.set_callback(() => {
      cb ();
      source.destroy ();
      callback_sources.remove (source);
      return Source.REMOVE;
    });

    callback_sources.add (source);
    source.attach ();
  }
}

}
