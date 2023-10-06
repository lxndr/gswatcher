Game Server Watcher (GSWatcher) is a simple application that helps you to watch
the state of your favorite game servers and control remotely the servers of
your own. It also has a buddy list and can notify you if a buddy joins any of
servers.

### Features:

* Server list
* Buddy list
* Notifications when your buddy is on a server
* Multiple query protocols
* Remote console (Source only)

### Development

Compile gsettings schemas

`glib-compile-schemas --targetdir=tmp/schemas src/schemas`

Compile POT file

In build directory `meson compile gswatcher-pot`

Update PO file

In build directory `meson compile gswatcher-update-po`
