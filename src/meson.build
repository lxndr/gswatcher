# Ubuntu 22.04 LTS, Debian 12
gtk4_dep = dependency('gtk4', version: '>=4.6.9')
gtk4_dep_version = gtk4_dep.version()

libadwaita_dep = dependency('libadwaita-1', version: '>=1.1.7')
libadwaita_dep_version = libadwaita_dep.version()

dependencies = [
  gtk4_dep,
  libadwaita_dep,
  dependency('glib-2.0', version : '>=2.72'),
  dependency('pango', version : '>=1.50.6'),
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

if gtk4_dep_version.version_compare('>=4.8')
  add_project_arguments(
    '-D', 'GSW_GTK_4_8_SUPPORTED',
    language: 'vala',
  )
endif

if gtk4_dep_version.version_compare('>=4.12')
  add_project_arguments(
    '-D', 'GSW_GTK_4_12_SUPPORTED',
    language: 'vala',
  )
endif

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

if libadwaita_dep_version.version_compare('>=1.5.0')
  add_project_arguments(
    '-D', 'GSW_ADWAITA_1_5_SUPPORTED',
    language: 'vala',
  )
endif

if libadwaita_dep_version.version_compare('>=1.6.0')
  add_project_arguments(
    '-D', 'GSW_ADWAITA_1_6_SUPPORTED',
    language: 'vala',
  )
endif

subdir('core')

core_sources += configure_file(
  input: 'config.vala.in',
  output: 'config.vala',
  configuration: conf_data,
)

ui_sources = files(
  'ui/about.vala',
  'ui/buddy-list.vala',
  'ui/main-window.vala',
  'ui/player-list.vala',
  'ui/preferences.vala',
  'ui/server-console.vala',
  'ui/server-details.vala',
  'ui/server-info.vala',
  'ui/server-settings.vala',
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
  'server-watcher.vala',
  'utils.vala',
)

sources += gnome.compile_resources(
  'resources',
  'gswatcher.gresource.xml',
)

if host_machine.system() == 'windows'
  windows = import('windows')

  windows_rc = configure_file(input: 'windows.rc.in',
    output: 'windows.rc',
    configuration: conf_data,
  )

  windows_resources = windows.compile_resources(
    windows_rc,
    depends: windows_ico,
    include_directories: windows_rc_inc,
  )

  sources += windows_resources
endif

executable(
  meson.project_name(),
  sources,
  'main.vala',
  dependencies: dependencies,
  win_subsystem: 'windows',
  install: true,
)
