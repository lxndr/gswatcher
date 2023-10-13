Game Server Watcher (GSWatcher) is a simple application that helps you to watch
the state of your favorite game servers and control remotely the servers of
your own. It also has a buddy list and can notify you if a buddy joins any of
servers.

![Game Server Watcher](https://raw.githubusercontent.com/lxndr/gswatcher/v1.6.7/docs/screenshots/01.png)

## Features

* Server list
* Buddy list
* Notifications when your buddy is on a server
* Multiple query protocols
* Remote console (Source only)

## For users

This project isn't actively developed. If you have interest adding or updating support
for specific game, please, write a ticket. Also, feel free to fork and create a MR.

## Installation

### ArchLinux

https://aur.archlinux.org/packages/gswatcher

```
yay -S gswatcher
```

### FlatHub

https://flathub.org/apps/io.github.lxndr.gswatcher

```
flatpak install flathub io.github.lxndr.gswatcher
```

## Development

```
meson setup builddir
meson compile -C builddir
```

### Compile POT file

In build directory

```
meson compile gswatcher-pot
```

### Update PO file

In build directory

```
meson compile gswatcher-update-po
```
