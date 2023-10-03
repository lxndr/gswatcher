install_data('gswatcher.png', install_dir: data_dir / 'icons/hicolor/128x128/apps', rename: 'ru.lxndr.gswatcher.png')
install_subdir('flags', install_dir: gswatcher_dir / 'icons')

free_icons = [
  'nexuiz'
]

if get_option('proprietary-icons')
  install_subdir('games', install_dir: gswatcher_dir / 'icons', exclude_directories: 'original')
else
  # TODO
endif
