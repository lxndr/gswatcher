install_data(
  'gswatcher.png',
  install_dir: join_paths(get_option('datadir'), 'icons/hicolor/128x128/apps'),
  rename: conf_data.get('app_id') + '.png',
)

free_icons = files([
  'games/7d2d.png',
  'games/aa.png',
  'games/aa2.png',
  'games/aa3.png',
  'games/aapg.png',
  'games/aoc.png',
  'games/as.png',
  'games/asrd.png',
  'games/nexuiz.png',
])

if get_option('proprietary-icons')
  install_subdir('games', install_dir: join_paths(gsw_datadir, 'icons'), exclude_directories: 'original')
else
  install_data(free_icons, install_dir: join_paths(gsw_datadir, 'icons/games'))
endif
