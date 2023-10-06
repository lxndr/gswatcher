settings_schemas = [
  'com.github.lxndr.gswatcher.Buddy.gschema.xml',
  'com.github.lxndr.gswatcher.Preferences.gschema.xml',
  'com.github.lxndr.gswatcher.Server.gschema.xml',
]

gnome.compile_schemas(build_by_default: true)

install_data(
  settings_schemas,
  install_dir: join_paths(get_option('datadir'), 'glib-2.0/schemas'),
)
