/*
  Game Server Watcher
  Copyright (C) 2023  Alexander Burobin

  This program is free software: you can redistribute it and/or modify
  it under the terms of the GNU Affero General Public License as
  published by the Free Software Foundation, either version 3 of the
  License, or (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU Affero General Public License for more details.

  You should have received a copy of the GNU Affero General Public License
  along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

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
#if GSW_ADWAITA_1_2_SUPPORTED
  var dlg = new Adw.MessageDialog (parent, heading, body);
  dlg.add_response("cancel", _("Cancel"));
  dlg.add_response("remove", _("Remove"));
  dlg.set_response_appearance ("remove", DESTRUCTIVE);
  dlg.default_response = "cancel";
  dlg.close_response = "cancel";

  #if GSW_ADWAITA_1_3_SUPPORTED
    return (yield dlg.choose (null)) == "remove"
      ? ShowRemoveDialogResponse.REMOVE
      : ShowRemoveDialogResponse.CANCEL;
  #else
    var promise = new Gee.Promise<string> ();

    dlg.response.connect ((res) => {
      promise.set_value (res);
    });

    dlg.present ();

    try {
      return (yield promise.future.wait_async () == "remove")
        ? ShowRemoveDialogResponse.REMOVE
        : ShowRemoveDialogResponse.CANCEL;
    } catch (Gee.FutureError err) {
      return ShowRemoveDialogResponse.REMOVE;
    }
  #endif
#else
  var promise = new Gee.Promise<ShowRemoveDialogResponse> ();
  var dlg = new Gtk.MessageDialog (parent, MODAL, QUESTION, YES_NO, body);

  dlg.response.connect ((res) => {
    promise.set_value (res == Gtk.ResponseType.YES ? ShowRemoveDialogResponse.REMOVE : ShowRemoveDialogResponse.CANCEL);
  });

  dlg.present ();

  try {
    return yield promise.future.wait_async ();
  } catch (Gee.FutureError err) {
    return ShowRemoveDialogResponse.CANCEL;
  }
#endif
}

void show_error_dialog (Gtk.Window parent, string body) {
#if GSW_ADWAITA_1_2_SUPPORTED
  var dlg = new Adw.MessageDialog (parent, null, body);
  dlg.add_response("ok", _("OK"));
  dlg.set_response_appearance ("ok", SUGGESTED);
  dlg.default_response = "ok";
  dlg.close_response = "ok";
  dlg.present ();
#else
  var dlg = new Gtk.MessageDialog (parent, MODAL, QUESTION, OK, body);
  dlg.present ();
#endif
}

}
