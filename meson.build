project('gswatcher', 'c', 'vala', version : '1.6.0-alpha.4')

data_dir = get_option('prefix') / get_option('datadir')
gswatcher_dir = data_dir / meson.project_name()

subdir('src')
subdir('src/schemas')
subdir('src/scripts/dist')
subdir('icons')

install_data('gswatcher.desktop', install_dir: data_dir / 'applications')
install_subdir('games', install_dir: gswatcher_dir)

if get_option('tests')
  subdir('tests')
endif
