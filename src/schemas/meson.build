settings_schemas = [
  get_option('app-id') + '.Buddy.gschema.xml',
  get_option('app-id') + '.Preferences.gschema.xml',
  get_option('app-id') + '.Server.gschema.xml',
]

install_data(
  settings_schemas,
  install_dir: join_paths(get_option('datadir'), 'glib-2.0/schemas'),
)

gnome.compile_schemas(build_by_default: true)
