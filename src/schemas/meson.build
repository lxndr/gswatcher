schemas_dir = get_option('prefix') / get_option('datadir') / 'glib-2.0' / 'schemas'

settings_schemas = [
  'org.lxndr.gswatcher.Buddy.gschema.xml',
  'org.lxndr.gswatcher.Preferences.gschema.xml',
  'org.lxndr.gswatcher.Server.gschema.xml',
  'org.lxndr.gswatcher.Window.gschema.xml',
]

install_data(settings_schemas, install_dir: schemas_dir)

gnome.compile_schemas(depend_files: files(settings_schemas))
meson.add_install_script('glib-compile-schemas', schemas_dir)
