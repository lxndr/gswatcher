public static int main (string[] args) {
  Environment.set_application_name (Gsw.Config.APPNAME);
  var data_dirs = Environment.get_system_data_dirs ();

  if (data_dirs.length > 0) {
    var locale_dir = Path.build_path (data_dirs[0], "locale");

    Intl.bindtextdomain (Gsw.Config.GETTEXT_PACKAGE, locale_dir);
    Intl.bind_textdomain_codeset (Gsw.Config.GETTEXT_PACKAGE, "UTF-8");
    Intl.textdomain (Gsw.Config.GETTEXT_PACKAGE);
  }

   var app = new Gsw.Application ();
  return app.run (args);
}
