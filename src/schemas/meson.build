settings_schemas = [
  'ru.lxndr.gswatcher.Buddy.gschema.xml',
  'ru.lxndr.gswatcher.Preferences.gschema.xml',
  'ru.lxndr.gswatcher.Server.gschema.xml',
]

install_data(
  settings_schemas,
  install_dir: join_paths(get_option('datadir'), 'glib-2.0/schemas'),
)

gnome.compile_schemas(build_by_default: true)
