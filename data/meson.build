if host_machine.system() not in ['windows', 'darwin']
  # desktop file
  desktop_file_in = configure_file(
            input: 'gswatcher.desktop.in.in',
           output: '@0@.desktop.in'.format(conf_data.get('app_id')),
    configuration: conf_data,
  )

  desktop_file = i18n.merge_file(
          input: desktop_file_in,
         output: '@0@.desktop'.format(conf_data.get('app_id')),
           type: 'desktop',
         po_dir: '../po',
        install: true,
    install_dir: join_paths(get_option('datadir'), 'applications')
  )

  desktop_utils = find_program('desktop-file-validate', required: false)

  if desktop_utils.found()
    test('Validate desktop file', desktop_utils,
      args: [desktop_file]
    )
  endif

  # appstream file
  appstream_file_in = configure_file(
            input: 'gswatcher.appdata.xml.in.in',
           output: '@0@.appdata.xml.in'.format(conf_data.get('app_id')),
    configuration: conf_data,
  )
  appstream_file = i18n.merge_file(
          input: appstream_file_in,
         output: '@0@.appdata.xml'.format(conf_data.get('app_id')),
         po_dir: '../po',
        install: true,
    install_dir: join_paths(get_option('datadir'), 'metainfo')
  )

  appstream_util = find_program('appstream-util', required: false)

  if appstream_util.found()
    test('Validate appstream file', appstream_util,
      args: ['validate', '--nonet', appstream_file]
    )
  endif
endif

install_subdir('games', install_dir: gsw_datadir)

subdir('gschemas')
subdir('icons')
subdir('protocols')
