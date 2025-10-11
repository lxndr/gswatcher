convert = find_program('convert', version: '>=6.0.0')

app_icon_source = 'gswatcher.png'
icon_sizes = [16, 24, 32, 48, 64, 128, 256, 512]

foreach size : icon_sizes
  output_filename = '@0@-@1@x@1@.png'.format(conf_data.get('app_exec'), size)
  install_dir = join_paths(get_option('datadir'), 'icons/hicolor', '@0@x@0@'.format(size), 'apps')
  install_filename = '@0@.png'.format(conf_data.get('app_exec'))

  app_icon = custom_target(output_filename,
    input: app_icon_source,
    output: output_filename,
    command: [
      convert,
      '@INPUT@',
      '-resize', '@0@x@0@'.format(size),
      '-define', 'png:compression-level=9',
      '-define', 'png:compression-strategy=1',
      '-define', 'png:exclude-chunk=all',
      '@OUTPUT@',
    ],
    build_by_default: true,
  )

  install_data(
    app_icon.full_path(),
    install_dir: install_dir,
    rename: install_filename,
  )
endforeach

subdir('games')

windows_rc_inc = include_directories('.')
