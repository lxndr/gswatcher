using Gtk;

namespace Gsw.Ui {

  void show_about (Window win) {
    string[] authors = {
      "Alexander Burobin <lxndr87i@gmail.com>",
      null
    };

    show_about_dialog (
      win,
      "authors", authors,
      "license-type", License.GPL_3_0,
      "logo-icon-name", "gswatcher",
      "version", Config.VERSION,
      "website", Config.HOMEPAGE
    );
  }

}
