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
  dependency('gtk4'),
  dependency('gee-0.8'),
  dependency('duktape'),
]

core_sources = files(
  'protocols/js-protocol.vala',
  'transports/udp-transport-manager.vala',
  'transports/udp-transport.vala',
  'buddy-list.vala',
  'buddy.vala',
  'player.vala',
  'protocol.vala',
  'querier-manager.vala',
  'querier.vala',
  'server-info.vala',
  'server-list.vala',
  'server.vala',
  'transport.vala',
  'util.vala',
)

ui_sources = files(
  'ui/about.vala',
  'ui/buddy-list.vala',
  'ui/main-window.vala',
  'ui/preferences.vala',
  'ui/server-list-item.vala',
  'ui/server-list.vala',
)

sources = core_sources
sources += ui_sources

sources += files(
  'application.vala',
  'persistent-buddy-list.vala',
  'persistent-server-list.vala',
)

conf_data = configuration_data()
conf_data.set_quoted('GSW_APPID', 'org.lxndr.gswatcher')
conf_data.set_quoted('GSW_APPNAME', 'Game Server Watcher')
conf_data.set_quoted('GSW_VERSION', meson.project_version())
conf_data.set_quoted('GSW_HOMEPAGE', 'http://gswatcher.sourceforge.net')
conf_data.set_quoted('GSW_LOG_DOMAIN', 'Gsw')
conf_data.set_quoted('GETTEXT_PACKAGE', meson.project_name())

sources += configure_file(
  input: 'config.vala.in',
  output: 'config.vala',
  configuration: conf_data
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
