settings_schemas = files([
  'io.github.lxndr.gswatcher.Buddy.gschema.xml',
  'io.github.lxndr.gswatcher.Preferences.gschema.xml',
  'io.github.lxndr.gswatcher.Server.gschema.xml',
])

gnome.compile_schemas(depend_files: settings_schemas, build_by_default: true)

install_data(
  settings_schemas,
  install_dir: join_paths(get_option('datadir'), 'glib-2.0/schemas'),
)
