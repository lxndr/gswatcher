using Gtk;

namespace Gsw.Ui {

// TODO: should use GtkListView

class OptionList : Widget {
  private Gee.Map<string, string> _values = new Gee.HashMap<string, string> ();

  public Gee.List<ProtocolOption> options { get; construct; }

  public Gee.Map<string, string> values {
    get {
      return _values;
    }

    set {
      _values.clear ();
      _values.set_all (value);
    }
  }

  class construct {
    set_layout_manager_type (typeof (GridLayout));
  }

  construct {
    for (var i = 0; i < options.size; i++) {
      var option = options[i];

      var label = new Label (option.label);
      place_widget (label, 0, i);

      var value = _values.has_key (option.id) ? _values[option.id] : "";
      var editor = create_editor_widget (option, value);
      place_widget (editor, 1, i);
    }
  }

  public OptionList (Gee.List<ProtocolOption> options) {
    Object (options : options);
  }

  protected override void dispose () {
    get_first_child ().unparent ();
    base.dispose ();
  }

  private Widget create_editor_widget (ProtocolOption option, string value) {
    switch (option.type) {
      case PASSWORD:
        var entry = new Entry ();
        entry.text = value;
        entry.visibility = false;
        entry.changed.connect (() => _values[option.id] = entry.text);
        return entry;
      default:
        return new Label ("Unknown");
    }
  }

  private void place_widget (Widget widget, int column, int row) {
    widget.set_parent (this);
    var layout_manager = get_layout_manager ();
    var widget_layout = (GridLayoutChild) layout_manager.get_layout_child (widget);
    widget_layout.column = column;
    widget_layout.row = row;
  }
}

}
