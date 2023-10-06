project('gswatcher', 'c', 'vala',
        version: '1.6.4',
  meson_version: '>= 1.0.1',
)

gnome = import('gnome')
i18n = import('i18n')

conf_data = configuration_data({
  'app_id': 'com.github.lxndr.gswatcher',
  'app_name': 'Game Server Watcher',
  'app_summary': 'A simple game server browser and an administrative tool',
  'app_exec': meson.project_name(),
  'app_version': meson.project_version(),
  'app_homepage': 'https://github.com/lxndr/gswatcher',
  'app_bugtracker': 'https://github.com/lxndr/gswatcher/issues',
  'app_log_domain': 'Gsw',
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
