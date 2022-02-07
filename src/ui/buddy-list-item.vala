using Gtk;

namespace Gsw.Ui {

class BuddyListItem : Object {
  public Buddy? buddy { get; construct; }
  public string name { get ; set; }
  public virtual bool is_new { get; default = false; }

  public BuddyListItem (Buddy buddy) {
    Object (buddy : buddy, name : buddy.name);
  }

  public virtual void handle_start_editing () {
  }

  public virtual void handle_stop_editing (string text) {
    if (text != buddy.name) {
      name = text;
      // TODO: change name
    }
  }
}

class NewBuddyListItem : BuddyListItem {
  public override bool is_new { get; default = true; }

  public signal void added (string name);

  public NewBuddyListItem () {
    Object (name : _("Click here to add a new buddy"));
  }

  public override void handle_start_editing () {
    name = "";
  }

  public override void handle_stop_editing (string text) {
    var default_text = _("Click here to add a new buddy");

    if (text != "" && text != default_text)
      added (text);

    name = default_text;
  }
}

}
