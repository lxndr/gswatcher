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

using Gtk;

namespace Gsw.Ui {

[GtkTemplate (ui = "/io/github/lxndr/gswatcher/ui/main-window.ui")]
class MainWindow : Adw.ApplicationWindow {
  public Gsw.BuddyList buddy_list { get; construct; }
  public Gsw.ServerList client_list { get; construct; }

  [GtkChild]
  private unowned Adw.ViewStack stack;

  [GtkChild]
  private unowned MenuButton add_server_menu_button;
  [GtkChild]
  private unowned Popover add_server_popover;
  [GtkChild]
  private unowned Entry add_server_address_entry;

  [GtkChild]
  private unowned MenuButton add_buddy_menu_button;
  [GtkChild]
  private unowned Popover add_buddy_popover;
  [GtkChild]
  private unowned Entry add_buddy_name_entry;

  class construct {
    typeof (Ui.ServerList).ensure ();
    typeof (Ui.ServerInfo).ensure ();
    typeof (Ui.ServerConsole).ensure ();
    typeof (Ui.ServerDetails).ensure ();
    typeof (Ui.BuddyList).ensure ();
    typeof (Ui.PlayerList).ensure ();
    typeof (Ui.Preferences).ensure ();
  }

  public MainWindow (Gtk.Application application, Gsw.ServerList client_list, Gsw.BuddyList buddy_list) {
    Object (
      application : application,
      client_list : client_list,
      buddy_list : buddy_list
    );
  }

  [GtkCallback]
  private void on_stack_changed () {
    add_server_menu_button.visible = stack.visible_child_name == "server_list";
    add_buddy_menu_button.visible = stack.visible_child_name == "buddy_list";
  }

  [GtkCallback]
  private void on_add_server_clicked () {
    add_server_popover.popdown ();
    var address = add_server_address_entry.text;
    add_server_address_entry.text = "";

    if (address.length == 0) {
      return;
    }

    if (client_list.exists (address)) {
      var msg = _("\"%s\" is already on your server list.").printf (address);
      show_error_dialog (this, msg);
      return;
    }

    client_list.add (address);
  }

  [GtkCallback]
  private void remove_server (Client client) {
    var msg_heading = _("Remove server?");
    var msg_body = _("Are you sure you want to remove \"%s\" from the server list?").printf (client.server.address);

    show_remove_dialog.begin (this, msg_heading, msg_body, (obj, res) => {
      if (show_remove_dialog.end (res) == ShowRemoveDialogResponse.REMOVE) {
        client_list.remove (client);
      }
    });
  }

  [GtkCallback]
  private void on_add_buddy_clicked () {
    add_buddy_popover.popdown ();
    var name = add_buddy_name_entry.text;
    add_buddy_name_entry.text = "";

    if (name.length == 0) {
      return;
    }

    if (buddy_list.exists (name)) {
      var msg = _("\"%s\" is already on your buddy list.").printf (name);
      show_error_dialog (this, msg);
      return;
    }

    buddy_list.add (name);
  }

  [GtkCallback]
  private void remove_buddy (Buddy buddy) {
    var msg_heading = _("Remove buddy?");
    var msg_body = _("Are you sure you want to remove \"%s\" from the buddy list?").printf (buddy.name);

    show_remove_dialog.begin (this, msg_heading, msg_body, (obj, res) => {
      if (show_remove_dialog.end (res) == ShowRemoveDialogResponse.REMOVE) {
        buddy_list.remove (buddy);
      }
    });
  }
}

}
