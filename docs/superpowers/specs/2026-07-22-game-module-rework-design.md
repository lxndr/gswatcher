# Game Module Rework — Design

Date: 2026-07-22
Scope: `src/core/game`, plus app-wide removal of `[SingleInstance]` services.

## Goals

- Make game definition parsing and resolution unit-testable in isolation.
- Remove singleton coupling: all services constructed and wired explicitly by a
  composition root.
- Replace the fragile string-length "weight" conflict resolution with a
  specificity model.
- Add per-protocol generic game definitions that act as a base layer for
  specific games and as identification for otherwise unmatched servers.
- Non-blocking definition loading that works with slow storage and large
  definition sets.

## Non-Goals (out of scope)

- Fixture tests over `data/games/*.ini` or recorded server responses
  (future work; unit tests only for now).
- Changing the expression language syntax (no `and`, comparisons, etc.).
- A binary cache for parsed definitions.

## Module Layout (`src/core/game/`)

| Unit | Responsibility |
|---|---|
| `Game` | Parsed definition data; evaluates its match rules against `ServerDetails`; applies its `[Info]` expressions to a `ServerInfo`. |
| `GameDefinitionLoader` | Pure parser: `KeyFile` in, `Game` out. No I/O. All INI validation lives here. |
| `GameDb` | Owns loaded games. File discovery + async loading, `ensure_loaded ()`, resolution (specificity ranking + generic layering), TTL cache, lookup API (`get_plist_fields`, `get_feature_protocol`). Replaces `GameResolver`. |
| `expression.vala` | Expression AST. Gains `MatchExpression` interface and `EqualsExpression`. |
| `expression-parser.vala` | Recursive-descent parser. Function creation moves to a registry. |

`GameResolver` is deleted.

## Game Definitions

### Specific definitions (unchanged format)

As today: `[Game]` (id, protocol, optional port/qport-diff), optional
`[Features]`, mandatory `[Match]`, optional `[Info]`, `[Player]`, `[Extra]`,
and custom map sections.

### Generic definitions (new)

A definition **without a `[Match]` section** is the *generic* definition for
its protocol. Absence of `[Match]` is the semantic marker; shipped files use
the naming convention `generic-<protocol>.ini`. Rules:

- At most one generic per protocol. A second generic for the same protocol is
  a load error (logged, file skipped).
- A generic may define everything a specific definition can except `[Match]`:
  default `port`, `[Features]`, `[Info]`, `[Player]`, custom maps.
- Specific definitions must still have a non-empty `[Match]` section
  (load error otherwise, as today).

### Resolution and layering

Resolution input: protocol id + `ServerDetails`. Output: a resolved pair
`(generic Game?, specific Game?)`, at least one non-null, or no result.

1. All specific games for the protocol whose match rules all pass are
   candidates. The winner is the candidate with the highest specificity
   (below). The protocol's generic game (if any) is always part of the pair.
2. If no specific game matches and the protocol has no generic, resolution
   fails (server stays unidentified, as today).
3. Applying the pair to a `ServerInfo`:
   - `[Info]`: generic's expressions evaluate first, then the specific's —
     specific overwrites per key.
   - `game-id`: the specific's id if present, else the generic's id.
   - Player fields: the specific's `[Player]` table if it defines one,
     otherwise the generic's. Whole-table, never row-merged.
   - Features: looked up in the specific first, then the generic.
   - `port` / `qport-diff`: specific value wins when set; the generic provides
     the protocol default.
4. Expressions evaluate against their own file's custom map sections only.
   No cross-file map inheritance.

Behavior change (intended): with shipped generics, nearly every responding
server resolves to *some* game id. UI states keyed on "unidentified server"
become rare.

### Specificity (replaces `weight`)

Each `[Match]` rule is classified at load time; a game's specificity is the
tuple, compared lexicographically:

```
(count of exact-literal rules, count of or-of-literal rules, count of regex/function rules)
```

- Exact equality is more specific than a disjunction, which is more specific
  than a pattern. Rule verbosity has no effect.
- Ties between two matching specific games: log a warning naming both game
  ids, then resolve by game id (lexicographic) for determinism.
- The old `weight` field, its `+= value.length` accounting, and the sort in
  `resolve ()` are removed. No compatibility shims.

## Loading Model

- `GameDb.load_async ()` is started by the composition root at startup;
  startup never blocks on definition I/O (definitions may live on slow mounts,
  and definition count is unbounded).
- `GameDb.ensure_loaded ()` (async): returns immediately when loading has
  completed; otherwise suspends the caller and resumes all waiters when it
  does. Implemented as a list of pending continuations.
- `Querier.query ()` begins with `yield game_db.ensure_loaded ()` — queriers
  wait for the complete definition set before resolving (partial sets could
  misidentify). The `ready` property, both `notify["ready"]` subscriptions,
  and `Querier.query_pending` are deleted. `Client`'s console-feature update
  runs from an `ensure_loaded` continuation instead of
  `on_game_resolver_ready_change`.
- Error handling: per-file parse errors are logged and the file skipped
  (as today). Total load failure yields an empty set — queries proceed
  without game identification.

## Expression Subsystem

- **Function registry**: map of function name → factory, the single source of
  truth for `regex`, `mapKeyword`, `toMarkup`. `parse_function ()` looks up
  the registry; the unknown-function error message lists registered names, so
  it cannot go stale again.
- **`MatchExpression` interface**: `bool matches (ExpressionContext ctx,
  string value)`. `OrExpression` implements it directly. Plain evaluatable
  expressions are wrapped by the loader in a new `EqualsExpression`
  (`value == expr.eval (ctx)`). `Game.inf_matches` becomes
  `Map<string, MatchExpression>`; the `is`-type dispatch and the unreachable
  warning branch in `Game.matches ()` are removed.
- **Regex precompilation**: `RegexExpression` compiles its pattern at
  construction when the pattern argument is a literal (every current use);
  dynamic patterns still compile per evaluation.
- `ExpressionParser` keeps its one-shot object API; a static convenience
  `GameDef.parse (string)` wraps construct-and-parse.

## Composition Root

All six `[SingleInstance]` classes lose the attribute and their static
`get_instance ()`. `Gsw.Application` constructs and owns, in order:

```
UdpTransportManager
TransportRegistry      (transport factories capture the manager)
ProtocolRegistry
GameDb                 (load_async started immediately)
QuerierManager         (protocol registry, transport registry, game db)
MaxMindLocationResolver
PersistentServerList   (receives what Client construction needs)
```

Injection details:

- `TransportRegistry` registers **factory closures**
  (`(host, port) => new UdpTransport (manager, host, port)`) instead of
  `Type` + reflective `Object.new` — the only clean way to hand transports
  their dependencies.
- `QuerierManager` takes `ProtocolRegistry`, `TransportRegistry`, `GameDb`
  and passes them to each `Querier` it creates.
- `Client` takes `QuerierManager`, `GameDb`, `ProtocolRegistry`,
  `TransportRegistry` (the last for `ConsoleClient`). `ServerList` /
  `PersistentServerList` receive these to construct `Client`s.
- The two UI widgets using `MaxMindLocationResolver` (`ui/server-list.vala`,
  `ui/server-info.vala`) receive it via a settable property, assigned where
  the widgets are created.
- `GameDb`'s methods used by consumers are `virtual` so tests can stub it.

## Testing (unit tests only, this iteration)

- `GameDefinitionLoader`: inline INI strings → assertions on parsed `Game`
  (ports, features, match rules, info expressions, player fields, custom
  maps; error cases: missing id, bad port, malformed expressions, specific
  without `[Match]`).
- Expression parser/eval: existing table-driven tests carry over; add
  registry error message and `EqualsExpression` cases.
- Specificity: tuple classification and ordering; tie warning + deterministic
  tie-break.
- Resolution/layering: in-memory `GameDb` with generic + specific games;
  info overlay order, game-id choice, player-field fallback, feature
  fallback, no-match/no-generic failure.
- `ensure_loaded ()`: waiter resumed on completion; immediate return when
  already loaded.
- Existing `game_test.vala` (matches/enhance) adapted to the new interfaces.

Future work (explicitly deferred): an audit test that loads every shipped
`data/games/*.ini` through the loader and asserts parseability and absence of
specificity ties; later, recorded server-response fixtures asserting
end-to-end resolution.

## Migration Notes

- `data/games` gains `generic-<protocol>.ini` files for shipped protocols
  (source, quake3, gamespy4, ...) carrying protocol-default port, console
  feature, and baseline `[Info]` mappings; specific definitions keep working
  unchanged and now inherit the generic base layer at resolution time.
- `docs/game-definition-files.md` is updated: generic definitions, layering
  semantics, specificity model (weight section replaced).
