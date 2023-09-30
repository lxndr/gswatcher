namespace Gsw {

string? format_num_players (int num, int max) {
  if (num < 0 && max < 0)
    return null;

  var num_s = num < 0 ? "-" : num.to_string ();
  var max_s = max < 0 ? "-" : max.to_string ();
  return @"$(num_s) / $(max_s)";
}

enum ShowRemoveDialogResponse {
  CANCEL,
  REMOVE,
}

async ShowRemoveDialogResponse show_remove_dialog (Gtk.Window parent, string heading, string body) {
  var dlg = new Adw.MessageDialog (parent, heading, body);
  dlg.add_response("cancel", _("Cancel"));
  dlg.add_response("remove", _("Remove"));
  dlg.set_response_appearance ("remove", DESTRUCTIVE);
  dlg.default_response = "cancel";
  dlg.close_response = "cancel";

  var res = yield show_message_dialog (dlg);

  return res == "remove"
    ? ShowRemoveDialogResponse.REMOVE
    : ShowRemoveDialogResponse.CANCEL;
}

void show_error_dialog (Gtk.Window parent, string body) {
  var dlg = new Adw.MessageDialog (parent, null, body);
  dlg.add_response("ok", _("OK"));
  dlg.set_response_appearance ("ok", SUGGESTED);
  dlg.default_response = "ok";
  dlg.close_response = "ok";
  dlg.present ();
}

async string show_message_dialog (Adw.MessageDialog dlg) {
  #if GSW_ADWAITA_1_3_SUPPORTED
    return yield dlg.choose (null);
  #else
    var promise = new Gee.Promise<string> ();

    dlg.response.connect ((res) => {
      promise.set_value (res);
    });

    dlg.present ();

    try {
      return yield promise.future.wait_async ();
    } catch (Gee.FutureError err) {
      return "cancel";
    }
  #endif
}

}
