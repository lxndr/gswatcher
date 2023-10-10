libadwaita_dep = dependency('libadwaita-1', version : '>=1.1.7')
libadwaita_dep_version = libadwaita_dep.version()

dependencies = [
  dependency('gtk4', version : '>=4.6.9'),
  libadwaita_dep,
  dependency('gee-0.8', version : '>=0.20.4'),
  dependency('lua', version : '>=5.4'),
  dependency('geoip', version : '>=1.6.12'),
  meson.get_compiler('c').find_library('m', required: false),
]

add_project_arguments(
  ['--vapidir', join_paths(meson.current_source_dir(), 'vapi')],
  language: 'vala',
)

add_project_arguments(
    '-DGETTEXT_PACKAGE="@0@"'.format(meson.project_name()),
    language: 'c',
)

if libadwaita_dep_version.version_compare('>=1.2.0')
  add_project_arguments(
    '-D', 'GSW_ADWAITA_1_2_SUPPORTED',
    language: 'vala',
  )
endif

if libadwaita_dep_version.version_compare('>=1.3.0')
  add_project_arguments(
    '-D', 'GSW_ADWAITA_1_3_SUPPORTED',
    language: 'vala',
  )
endif

core_sources = files(
  'core/game/expression-parser.vala',
  'core/game/expression.vala',
  'core/game/game-resolver.vala',
  'core/game/game.vala',
  'core/protocol/console-lua-protocol.vala',
  'core/protocol/console-protocol.vala',
  'core/protocol/dummy-lua-protocol.vala',
  'core/protocol/lua-protocol.vala',
  'core/protocol/lua-utils.vala',
  'core/protocol/protocol-registry.vala',
  'core/protocol/protocol.vala',
  'core/protocol/query-lua-protocol.vala',
  'core/protocol/query-protocol.vala',
  'core/transport/net-transport.vala',
  'core/transport/tcp-transport.vala',
  'core/transport/transport-registry.vala',
  'core/transport/transport.vala',
  'core/transport/udp-transport-manager.vala',
  'core/transport/udp-transport.vala',
  'core/buddy-list.vala',
  'core/buddy.vala',
  'core/client.vala',
  'core/console-client.vala',
  'core/geoip-resolver.vala',
  'core/player-list.vala',
  'core/querier-manager.vala',
  'core/querier.vala',
  'core/server-details.vala',
  'core/server-info.vala',
  'core/server-list.vala',
  'core/server.vala',
  'core/utils.vala',
)

core_sources += configure_file(
  input: 'config.vala.in',
  output: 'config.vala',
  configuration: conf_data,
)

ui_sources = files(
  'ui/about.vala',
  'ui/buddy-list.vala',
  'ui/console-option-list.vala',
  'ui/main-window.vala',
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
  'buddy-watcher.vala',
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
  dependencies: dependencies,
  win_subsystem: 'windows',
  install: true,
)
