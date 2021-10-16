project('gswatcher', 'c', version : '1.5.1')

subdir('po')

gnome = import('gnome')

cc = meson.get_compiler('c')
libm = cc.find_library('m')

add_project_arguments('-DGETTEXT_PACKAGE="@0@"'.format(meson.project_name()), language : 'c')
add_project_arguments('-DGS_VERSION="1.5.1"', language : 'c')
add_project_arguments('-DGS_HOMEPAGE="http://gswatcher.sourceforge.net"', language : 'c')

dependencies = [
  dependency('glib-2.0', version : '>=2.70.0'),
  dependency('gobject-2.0', version : '>=2.70.0'),
  dependency('gio-2.0', version : '>=2.70.0'),
  dependency('gmodule-2.0', version : '>=2.70.0'),
  dependency('gtk+-3.0', version : '>=3.24.30'),
  dependency('geoip', version : '>=1.6.12'),
  dependency('json-glib-1.0', version : '>=1.6.6'),
  libm,
]

sources = [
  'src/query/console-source.c',
  'src/query/console-telnet.c',
  'src/query/console.c',
  'src/query/proto-gamespy.c',
  'src/query/proto-gamespy2.c',
  'src/query/proto-gamespy3.c',
  'src/query/proto-quake.c',
  'src/query/proto-source.c',
  'src/query/querier.c',
  'src/query/updater.c',
  'src/query/utils.c',
  'src/query/watcher.c',
  'src/client.c',
  'src/gswatcher.c',
  'src/gtkpopupbutton.c',
  'src/gui-buddy-list.c',
  'src/gui-chat.c',
  'src/gui-console.c',
  'src/gui-info.c',
  'src/gui-log.c',
  'src/gui-notification.c',
  'src/gui-player-list.c',
  'src/gui-player-list.c',
  'src/gui-preferences.c',
  'src/gui-server-list.c',
  'src/gui-window.c',
  'src/platform.c',
]

sources += gnome.compile_resources(
  'resources',
  'src/gswatcher.gresource.xml',
  source_dir: 'src',
)

executable(
  meson.project_name(),
  sources,
  dependencies : dependencies,
  install: true,
)
