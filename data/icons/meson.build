install_data(
  'gswatcher.png',
  install_dir: join_paths(get_option('datadir'), 'icons/hicolor/128x128/apps'),
  rename: conf_data.get('app_id') + '.png',
)

install_subdir('flags', install_dir: join_paths(gsw_datadir, 'icons'))

free_icons = [
  'nexuiz'
]

if get_option('proprietary-icons')
  install_subdir('games', install_dir: join_paths(gsw_datadir, 'icons'), exclude_directories: 'original')
else
  # TODO
endif
