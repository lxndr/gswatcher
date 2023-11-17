icons = [
  'aa.png',
  'aa2.png',
  'aa3.png',
  'aapg.png',
  'aarena.png',
  'ahl.png',
  'aq2.png',
  'cs.png',
  'cs2.png',
  'csgo.png',
  'css.png',
  'czero.webp',
  'iosoccer.png',
  'mc.webp',
  'mta.png',
  'syn.png',
  'warsow.svg',
  'zp.png',
]

convert = find_program('convert')
icon_size = get_option('game-icon-size')
icon_ext = get_option('game-icon-size')

foreach icon : icons
  custom_target(icon,
    input: icon,
    output: '@BASENAME@.@0@'.format(icon_ext),
    command: [
      convert,
      '@INPUT@',
      '-resize', '@0@x@0@'.format(icon_size),
      '-define', 'png:compression-level=9',
      '-define', 'png:compression-strategy=1',
      '-define', 'png:exclude-chunk=all',
      '@OUTPUT@',
    ],
    build_by_default: true,
    install : true,
    install_dir : join_paths(gsw_datadir, 'icons'),
  )
endforeach
