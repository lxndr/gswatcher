gnome = import('gnome')

add_project_arguments(
  ['--vapidir', join_paths(meson.current_source_dir(), 'vapi')],
  language: 'vala',
)

add_project_arguments(
    '-DGETTEXT_PACKAGE="@0@"'.format(meson.project_name()),
    language: 'c',
)

dependencies = [
  dependency('gtk4', version : '>=4.0.3'),
  dependency('gee-0.8', version : '>=0.20.4'),
  dependency('duktape', version : '>=2.5.0'),
  dependency('geoip', version : '>=1.6.12'),
]

core_sources = files(
  'core/buddy-list.vala',
  'core/buddy.vala',
  'core/detector-querier.vala',
  'core/game-resolver.vala',
  'core/geoip-resolver.vala',
  'core/net-transport.vala',
  'core/player-list.vala',
  'core/protocol.vala',
  'core/querier-manager.vala',
  'core/querier.vala',
  'core/server-details.vala',
  'core/server-info.vala',
  'core/server-list.vala',
  'core/server.vala',
  'core/transport.vala',
  'core/utils.vala',
  'core/worker-querier.vala',
  'protocols/js-protocol-utils.vala',
  'protocols/js-protocol.vala',
  'protocols/script-protocol.vala',
  'transports/udp-transport-manager.vala',
  'transports/udp-transport.vala',
)

conf_data = configuration_data()
conf_data.set_quoted('GSW_APPID', 'org.lxndr.gswatcher')
conf_data.set_quoted('GSW_APPNAME', 'Game Server Watcher')
conf_data.set_quoted('GSW_VERSION', meson.project_version())
conf_data.set_quoted('GSW_HOMEPAGE', 'http://gswatcher.sourceforge.net')
conf_data.set_quoted('GSW_LOG_DOMAIN', 'Gsw')
conf_data.set_quoted('GETTEXT_PACKAGE', meson.project_name())

core_sources += configure_file(
  input: 'config.vala.in',
  output: 'config.vala',
  configuration: conf_data
)

ui_sources = files(
  'ui/about.vala',
  'ui/buddy-list.vala',
  'ui/main-window.vala',
  'ui/player-list.vala',
  'ui/preferences.vala',
  'ui/server-details.vala',
  'ui/server-info.vala',
  'ui/server-list-item.vala',
  'ui/server-list.vala',
)

sources = core_sources
sources += ui_sources

sources += files(
  'application.vala',
  'persistent-buddy-list.vala',
  'persistent-server-list.vala',
  'utils.vala',
)

sources += gnome.compile_resources(
    'resources',
    'gswatcher.gresource.xml',
)

executable(
  meson.project_name(),
  sources,
  'main.vala',
  dependencies : dependencies,
  install: true,
)
