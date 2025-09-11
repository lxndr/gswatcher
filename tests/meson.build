test_core_sources = files(
  'lib/expect-memory.vala',
  'lib/expect-player-fields.vala',
  'lib/expect-player-list.vala',
  'lib/expect-string-map.vala',
  'lib/format-player-list.vala',
  'lib/format-plist-field.vala',
  'lib/format-plist-fields.vala',
  'lib/format-string-map.vala',
  'lib/is-plist-equal.vala',
  'lib/is-plist-fields-equal.vala',
  'lib/is-string-map-equal.vala',
  'lib/protocol-test-runner.vala',
)

subdir('protocols')

test('core/utils/game_def_expression', executable(
  'test-game_def_expression',
  core_sources,
  test_core_sources,
  'test-game_def_expression.vala',
  dependencies : dependencies,
))

test('core/utils/parse_address', executable(
  'test-parse_address',
  core_sources,
  test_core_sources,
  'test-parse_address.vala',
  dependencies : dependencies,
))
