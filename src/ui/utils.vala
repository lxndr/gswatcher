/*
  Game Server Watcher
  Copyright (C) 2010-2026 Alexander Burobin

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
  #if GSW_ADWAITA_1_6_SUPPORTED
    var dlg = new Adw.AlertDialog (heading, body);
  #else
    var dlg = new Adw.MessageDialog (parent, heading, body);
  #endif

  dlg.add_response("cancel", _("Cancel"));
  dlg.add_response("remove", _("Remove"));
  dlg.set_response_appearance ("remove", DESTRUCTIVE);
  dlg.default_response = "cancel";
  dlg.close_response = "cancel";

  #if GSW_ADWAITA_1_3_SUPPORTED
    #if GSW_ADWAITA_1_6_SUPPORTED
      return (yield dlg.choose (parent, null)) == "remove"
        ? ShowRemoveDialogResponse.REMOVE
        : ShowRemoveDialogResponse.CANCEL;
    #else
      return (yield dlg.choose (null)) == "remove"
        ? ShowRemoveDialogResponse.REMOVE
        : ShowRemoveDialogResponse.CANCEL;
    #endif
  #else
    var promise = new Gee.Promise<string> ();

    dlg.response.connect ((res) => {
      dlg.destroy ();
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
    dlg.destroy ();
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

void show_error_dialog (Gtk.Window? parent, string body) {
#if GSW_ADWAITA_1_6_SUPPORTED
  var dlg = new Adw.AlertDialog (_("Error"), body);
  dlg.add_response("ok", _("OK"));
  dlg.set_response_appearance ("ok", SUGGESTED);
  dlg.default_response = "ok";
  dlg.close_response = "ok";
  dlg.present (parent);
#elif GSW_ADWAITA_1_2_SUPPORTED
  var dlg = new Adw.MessageDialog (parent, null, body);
  dlg.add_response("ok", _("OK"));
  dlg.set_response_appearance ("ok", SUGGESTED);
  dlg.default_response = "ok";
  dlg.close_response = "ok";
  dlg.present ();
#else
  var dlg = new Gtk.MessageDialog (parent, MODAL, ERROR, OK, body);
  dlg.response.connect ((res) => {
    dlg.destroy ();
  });
  dlg.present ();
#endif
}

struct GetColumnViewItemResult {
  Gtk.Widget row_widget;
  Object? list_item;
  uint pos;
}

// https://discourse.gnome.org/t/gtk4-finding-a-row-data-on-gtkcolumnview/8465/2
GetColumnViewItemResult? get_column_view_item_for_y (Gtk.ColumnView view, double y) {
  var child = view.get_first_child ();
  var header_alloc = Gtk.Allocation ();

  while (child != null) {
    if (child.get_type ().name () == "GtkColumnViewRowWidget") {
      child.get_allocation (out header_alloc);
    }

    if (child.get_type ().name () == "GtkColumnListView") {
      var result = check_list_widgets (child, (int) y, header_alloc.y + header_alloc.height);

      if (result != null) {
        result.list_item = view.model.get_item (result.pos);
        return result;
      }
    }

    child = child.get_next_sibling ();
  }

  return null;
}

GetColumnViewItemResult? check_list_widgets (Gtk.Widget w, int y, int header_height) {
	var child = w.get_first_child ();
	var line_no = -1;
	var curr_y = header_height;

	while (child != null) {
		if (child.get_type ().name () == "GtkColumnViewRowWidget") {
			line_no++;

			var alloc = Gtk.Allocation ();
      child.get_allocation (out alloc);

			if (y > curr_y && y <= header_height + alloc.height + alloc.y) {
				return GetColumnViewItemResult () {
          row_widget = child,
          pos = line_no
        };
			}

			curr_y = header_height + alloc.height + alloc.y;
		}

		child = child.get_next_sibling ();
	}

	return null;
}

void remove_all_children (Gtk.Widget parent) {
  var child = parent.get_first_child ();

  while (child != null) {
      var next = child.get_next_sibling ();
      child.unparent ();
      child = next;
  }
}

}
