project('gswatcher', 'c', 'vala',
        version: '1.7.0-beta1',
  meson_version: '>= 0.61.2',
)

gnome = import('gnome')
i18n = import('i18n')
vala_compiler = meson.get_compiler('vala')

conf_data = configuration_data({
  'app_id': 'io.github.lxndr.gswatcher',
  'app_name': 'Game Server Watcher',
  'app_summary': 'A simple game server browser and an administrative tool.',
  'app_exec': meson.project_name(),
  'app_version': meson.project_version(),
  'app_homepage': 'https://github.com/lxndr/gswatcher',
  'app_bugtracker': 'https://github.com/lxndr/gswatcher/issues',
  'app_log_domain': 'Gsw',
  'app_vala_version': vala_compiler.version(),
})

gsw_datadir = join_paths(get_option('datadir'), meson.project_name())

subdir('data')
subdir('src')
subdir('po')

if get_option('tests')
  subdir('tests')
endif

gnome.post_install(
  gtk_update_icon_cache: true,
  glib_compile_schemas: true,
  update_desktop_database: host_machine.system() not in ['darwin', 'windows'],
)
