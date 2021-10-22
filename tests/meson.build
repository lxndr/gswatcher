dependencies = [
  dependency('glib-2.0', version : '>=2.70.0'),
]

testUtils = executable(
  'test-utils',
  [
    'test-utils.vala',
    '../src/query/utils.vala',
  ],
  dependencies : dependencies,
)

test('utils', testUtils)
