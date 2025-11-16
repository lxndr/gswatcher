[![Organic - Made by Humans](https://img.shields.io/static/v1?label=Organic&message=Made+by+Humans&color=brightgreen)](https://github.com/lxndr/gswatcher)

Game Server Watcher is a simple application that helps you to monitor
the state of your favorite game servers and control remotely the servers of
your own. It also has a buddy list and can notify you if a buddy joins any of
servers.

![Game Server Watcher](https://raw.githubusercontent.com/lxndr/gswatcher/v1.7.5/docs/screenshots/01.png)

## Features

* Server list
* Buddy list
* Notifications when your buddy is on a server
* Multiple query protocols
* Remote console (Source Engine only)

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

### Snap

https://snapcraft.io/gswatcher

```
sudo snap install gswatcher
```

### Windows

There are Windows portable builds available in Release section. Be warned that some features don't work like notifications.

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

## Acknowledgements

This application uses IP to location data provided by MaxMind. Many thanks to MaxMind for providing the GeoIP database that enables location resolution functionality.

For more information about MaxMind and their products, visit: https://www.maxmind.com/
