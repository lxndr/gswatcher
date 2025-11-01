convert = find_program('convert', version: '>=6.0.0')

app_exec = conf_data.get('app_exec')
app_id = conf_data.get('app_id')

app_icon_name = f'@app_exec@.png'
app_icon_source = files(app_icon_name)
app_icon_sizes = [16, 24, 32, 48, 64, 128, 256, 512]

foreach size : app_icon_sizes
  size_str = f'@size@x@size@'
  output_filename = f'@app_id@_@size@.png'
  install_dir = join_paths(get_option('datadir'), 'icons/hicolor', size_str, 'apps')

  custom_target(
    output_filename,
    input: app_icon_source,
    output: output_filename,
    command: [
      convert,
      '@INPUT@',
      '-resize', size_str,
      '-define', 'png:compression-level=9',
      '-define', 'png:compression-strategy=1',
      '-define', 'png:exclude-chunk=all',
      '@OUTPUT@',
    ],
    install: true,
    install_dir: install_dir,
    build_by_default: true,
  )
endforeach

# Rename installed icons to have the correct name for icon theme compliance
meson.add_install_script('rename-icons.sh', get_option('prefix'), get_option('datadir'), app_id)

subdir('games')

windows_rc_inc = include_directories('.')
