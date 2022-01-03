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
  dependency('gtk4', version : '>=4.5.0'),
  dependency('gee-0.8', version : '>=0.20.4'),
  dependency('duktape', version : '>=2.5.0'),
  dependency('geoip', version : '>=1.6.12'),
]

core_sources = files(
  'core/game/game-resolver.vala',
  'core/protocol/console-js-protocol.vala',
  'core/protocol/console-protocol.vala',
  'core/protocol/dummy-js-protocol.vala',
  'core/protocol/js-protocol.vala',
  'core/protocol/js-utils.vala',
  'core/protocol/protocol-registry.vala',
  'core/protocol/protocol.vala',
  'core/protocol/query-js-protocol.vala',
  'core/protocol/query-protocol.vala',
  'core/transport/net-transport.vala',
  'core/transport/tcp-transport.vala',
  'core/transport/transport-registry.vala',
  'core/transport/transport.vala',
  'core/transport/udp-transport-manager.vala',
  'core/transport/udp-transport.vala',
  'core/buddy-list.vala',
  'core/buddy.vala',
  'core/console-client.vala',
  'core/detector-querier.vala',
  'core/geoip-resolver.vala',
  'core/player-list.vala',
  'core/querier-manager.vala',
  'core/querier.vala',
  'core/server-details.vala',
  'core/server-info.vala',
  'core/server-list.vala',
  'core/server.vala',
  'core/utils.vala',
  'core/worker-querier.vala',
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
  'ui/option-list.vala',
  'ui/player-list.vala',
  'ui/preferences.vala',
  'ui/server-console.vala',
  'ui/server-details.vala',
  'ui/server-info.vala',
  'ui/server-list.vala',
  'ui/utils.vala',
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
