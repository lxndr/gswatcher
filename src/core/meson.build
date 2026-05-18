core_sources = files(
  'game/expression-parser.vala',
  'game/expression.vala',
  'game/game-resolver.vala',
  'game/game.vala',
  'geo-location-resolver/maxmind-location-resolver.vala',
  'geo-location-resolver/geo-location-resolver.vala',
  'protocol/console-lua-protocol.vala',
  'protocol/console-protocol.vala',
  'protocol/dummy-lua-protocol.vala',
  'protocol/lua-protocol.vala',
  'protocol/lua-utils.vala',
  'protocol/protocol-registry.vala',
  'protocol/protocol.vala',
  'protocol/query-lua-protocol.vala',
  'protocol/query-protocol.vala',
  'transport/net-transport.vala',
  'transport/tcp-transport.vala',
  'transport/transport-registry.vala',
  'transport/transport.vala',
  'transport/udp-transport-manager.vala',
  'transport/udp-transport.vala',
  'utils/data-files.vala',
  'utils/get-filename-extension.vala',
  'utils/hash-string-map.vala',
  'utils/hashable-string-map.vala',
  'utils/parse-quake-color-codes.vala',
  'utils/parse-address.vala',
  'utils/strip-pango-markup.vala',
  'utils/ttl-cache.vala',
  'buddy-list.vala',
  'buddy.vala',
  'client.vala',
  'console-client.vala',
  'player-list.vala',
  'player.vala',
  'querier-manager.vala',
  'querier.vala',
  'server-details-list.vala',
  'server-details.vala',
  'server-info.vala',
  'server-list.vala',
  'server.vala',
)

core_sources += configure_file(
  input: 'config.vala.in',
  output: 'core_config.vala',
  configuration: conf_data,
)

test('core/utils/hashable-string-map', executable(
  'hashable-string-map_test',
  core_sources,
  'utils/hashable-string-map_test.vala',
  dependencies : dependencies,
))

test('core/utils/parse_address', executable(
  'parse-address_test',
  core_sources,
  'utils/parse-address_test.vala',
  dependencies : dependencies,
))

test('core/utils/ttl-cache', executable(
  'ttl-cache_test',
  core_sources,
  'utils/ttl-cache_test.vala',
  dependencies : dependencies,
))

test('core/server-list', executable(
  'server-list_test',
  core_sources,
  'server-list_test.vala',
  dependencies : dependencies,
))

test('core/buddy-list', executable(
  'buddy-list_test',
  core_sources,
  'buddy-list_test.vala',
  dependencies : dependencies,
))
