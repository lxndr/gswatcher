# Game Module Rework Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Replace the singleton `GameResolver` with a testable `GameDefinitionLoader` + `GameDb` (async load, specificity-based matching, generic-definition layering) and wire every service explicitly from a composition root in `Application`.

**Architecture:** Pure INI parsing is extracted into `GameDefinitionLoader`; `GameDb` owns games, non-blocking loading with `ensure_loaded ()`, and layered resolution (generic base + most-specific match). All six `[SingleInstance]` services become constructor-injected, assembled in `Application.startup`.

**Tech Stack:** Vala, GLib/GObject, Gee, Meson, GLib Test framework.

**Spec:** `docs/superpowers/specs/2026-07-22-game-module-rework-design.md`

## Global Constraints

- **The agent never commits.** Every task ends with a STOP step: the user reviews, tests, and commits.
- Every new `.vala` file starts with the standard AGPL header copied verbatim from any existing source file (e.g. `src/core/game/game.vala` lines 1–17).
- Indentation: 2 spaces; space before `(` in calls/declarations (match existing style).
- Build dir: `build`. If missing: `meson setup build`. Compile: `meson compile -C build`. Tests: `meson test -C build <name> --print-errorlogs` (or run all with `meson test -C build --print-errorlogs`).
- New core sources must be added to `core_sources` in `src/core/meson.build`; new test executables registered there too (follow the existing `test('core/game', ...)` pattern).
- Each task must leave the whole project compiling and all tests passing.
- **Test placement:** unit tests live next to the unit they test (`src/.../<unit>_test.vala`, registered in `src/core/meson.build`); the `tests/` folder is for integration tests only.
- Unit tests only. No tests that read `data/games` or other installed files (deferred per spec).
- `log (Config.LOG_DOMAIN, LEVEL_ERROR, ...)` is fatal in GLib. New recoverable-error logging must use `warning ()` or `LEVEL_WARNING` so it is testable with `Test.expect_message` and never aborts the app.

---



### Task 0: Move expression unit tests next to the unit

**Files:**

- Rename: `tests/test-game_def_expression.vala` → `src/core/game/expression_test.vala`
- Modify: `tests/meson.build` (remove the old test target)
- Modify: `src/core/meson.build` (add the new test target)

**Interfaces:**

- Produces: the expression unit test now lives at `src/core/game/expression_test.vala` under the meson test name `core/game/expression`. All later tasks reference this path and name. File contents are unchanged except the test path prefix.

- [x] **Step 1: Move the file**

```bash
git mv tests/test-game_def_expression.vala src/core/game/expression_test.vala
```

- [x] **Step 2: Update the test path prefix inside the file**

In `src/core/game/expression_test.vala`, change the `Test.add_data_func` path from
`@"/core/utils/game_def_expression/$(row.name)"` to `@"/core/game/expression/$(row.name)"`.

- [x] **Step 3: Update meson**

In `tests/meson.build`, delete the whole block:

```meson
test('core/utils/game_def_expression', executable(
  'test-game_def_expression',
  core_sources,
  test_core_sources,
  'test-game_def_expression.vala',
  dependencies : dependencies,
))
```

In `src/core/meson.build`, next to the existing `test('core/game', ...)` block, add:

```meson
test('core/game/expression', executable(
  'expression_test',
  core_sources,
  'game/expression_test.vala',
  dependencies : dependencies,
))
```

- [x] **Step 4: Run the moved test**

Run: `meson test -C build core/game/expression --print-errorlogs`
Expected: PASS (identical assertions, new location).

- [x] **Step 5: STOP — user reviews, tests, and commits**

Suggested message: `test: move expression unit tests next to the unit`

---



### Task 1: Expression function registry

**Files:**

- Modify: `src/core/game/expression.vala` (add `Functions` namespace at end, before closing `}` of namespace `Gsw.GameDef`)
- Modify: `src/core/game/expression-parser.vala:115-131` (`parse_function`)
- Test: `src/core/game/expression_test.vala` (the `fn_unknown` table row)

**Interfaces:**

- Produces: `Gsw.GameDef.Functions.NAMES` (`const string[]`), `Gsw.GameDef.Functions.create (string name, Gee.List<EvaluatableExpression> args) throws ExpressionParserError` returning `EvaluatableExpression?` (null for unknown name). Task 2 makes factories actually throw.

- [x] **Step 1: Update the expected error message in the test (failing test)**

In `src/core/game/expression_test.vala`, change the `fn_unknown` row's error to:

```vala
{ name: "fn_unknown",       pattern: "unknownFn()",                                             result: null,           error: new GameDef.ExpressionParserError.INVALID_TOKEN ("failed to parse function: unknown function 'unknownFn' at position 11, supported functions: regex, mapKeyword, toMarkup") },
```

- [x] **Step 2: Run test to verify it fails**

Run: `meson test -C build core/game/expression --print-errorlogs`
Expected: FAIL on `fn_unknown` (message mismatch against the old "only supported 'regex' or 'mapKeyword'" text).

- [x] **Step 3: Add the registry to** `expression.vala`

At the end of `src/core/game/expression.vala`, inside `namespace Gsw.GameDef`:

```vala
/**
 * The single source of truth for expression functions available in game
 * definitions.
 */
namespace Functions {

  public const string[] NAMES = { "regex", "mapKeyword", "toMarkup" };

  /**
   * Creates a function expression by name, or returns null if the name
   * is not a registered function.
   */
  public EvaluatableExpression? create (string name, Gee.List<EvaluatableExpression> args) throws ExpressionParserError {
    switch (name) {
      case "regex":
        return new RegexExpression (args);
      case "mapKeyword":
        return new MapKeywordExpression (args);
      case "toMarkup":
        return new ToMarkupExpression (args);
      default:
        return null;
    }
  }

}
```

- [x] **Step 4: Replace** `parse_function` **in** `expression-parser.vala`

```vala
  private EvaluatableExpression parse_function (string name) throws ExpressionParserError {
    var args = parse_function_args ();
    var expr = Functions.create (name, args);

    if (expr == null) {
      throw new ExpressionParserError.INVALID_TOKEN (
        "failed to parse function: unknown function '%s' at position %u, supported functions: %s",
        name, scanner.position, string.joinv (", ", Functions.NAMES)
      );
    }

    return expr;
  }
```

Note: inside namespace `Gsw.GameDef`, `Functions.create` and `Functions.NAMES` resolve directly; if the compiler complains about `NAMES`, qualify as `Functions.NAMES`.

Also add the one-shot parse convenience at the end of `expression-parser.vala`, inside namespace `Gsw.GameDef` (next to `create_scanner`):

```vala
/**
 * Parses an expression string in one call.
 */
public Expression parse (string input) throws ExpressionParserError {
  var parser = new ExpressionParser (input);
  return parser.parse ();
}
```

- [x] **Step 5: Run tests to verify they pass**

Run: `meson test -C build core/game/expression core/game --print-errorlogs`
Expected: PASS.

- [x] **Step 6: STOP — user reviews, tests, and commits**

Suggested message: `refactor: introduce expression function registry`

---



### Task 2: Precompile literal regex patterns at parse time

**Files:**

- Modify: `src/core/game/expression.vala` (`LiteralExpression`, `RegexExpression`)
- Test: `src/core/game/expression_test.vala`

**Interfaces:**

- Produces: `LiteralExpression.value` readable property (used by Task 5's classification too). Invalid literal regex patterns now fail at **parse** time with `ExpressionParserError.INVALID_TOKEN`.

- [x] **Step 1: Write the failing test**

Append in `src/core/game/expression_test.vala` before `Test.run ()`:

```vala
  Test.add_func ("/core/game/expression/regex_invalid_literal_pattern", () => {
    try {
      GameDef.parse ("regex(\"[\", \"input\")");
      assert_not_reached ();
    } catch (GameDef.ExpressionParserError error) {
      assert_cmpint (error.code, EQ, GameDef.ExpressionParserError.INVALID_TOKEN);
    } catch (Error error) {
      assert_no_error (error);
    }
  });
```

- [x] **Step 2: Run test to verify it fails**

Run: `meson test -C build core/game/expression --print-errorlogs`
Expected: FAIL — currently parsing succeeds (`assert_not_reached` triggers), because the bad pattern only fails at eval.

- [x] **Step 3: Expose the literal value**

In `LiteralExpression`, replace `private string value = "";` with:

```vala
  public string value { get; private set; }
```

(The constructor body `this.value = value;` stays as is.)

- [x] **Step 4: Rewrite** `RegexExpression`

```vala
class RegexExpression : FunctionExpression {
  private Regex? compiled_regex;

  public RegexExpression (Gee.List<EvaluatableExpression> args) throws ExpressionParserError {
    base ("regex", args);

    if (args.size > 0 && args[0] is LiteralExpression) {
      var pattern = ((LiteralExpression) args[0]).value;

      try {
        compiled_regex = new Regex (pattern);
      } catch (RegexError err) {
        throw new ExpressionParserError.INVALID_TOKEN ("invalid regex pattern /%s/: %s", pattern, err.message);
      }
    }
  }

  public override string eval (ExpressionContext ctx) throws ExpressionError {
    var input = eval_arg (ctx, 1);
    var regex = compiled_regex ?? create_regex (eval_arg (ctx, 0));
    return match_regex (regex, input);
  }

  private Regex create_regex (string pattern) throws ExpressionError {
    try {
      return new Regex (pattern);
    } catch (RegexError err) {
      throw new ExpressionError.INVALID_FUNCTION ("failed to create regex for /%s/: %s", pattern, err.message);
    }
  }

  private string match_regex (Regex regex, string input) {
    MatchInfo match_info;

    if (!regex.match (input, 0, out match_info))
      return "";

    return match_info.fetch (1);
  }
}
```

Note the doc comment above the class stays. The old two-argument `create_regex (pattern, input)` is gone.

- [x] **Step 5: Run tests to verify they pass**

Run: `meson test -C build core/game/expression --print-errorlogs`
Expected: PASS, including the existing `fn_missing_arg` row (its message is unchanged: the input argument is evaluated first and is still the one reported missing... verify!). If `fn_missing_arg` now reports a different argument index, adjust that row's message to the actual output — behavior (an `INVALID_FUNCTION` error) is what matters.

- [ ] **Step 6: STOP — user reviews, tests, and commits**

Suggested message: `perf: compile literal regex patterns at parse time`

---



### Task 3: MatchExpression interface replaces LogicalExpression

**Files:**

- Modify: `src/core/game/expression.vala`
- Modify: `src/core/game/game.vala`
- Modify: `src/core/game/game-resolver.vala:100-132` (`[Match]` block)
- Test: `src/core/game/game_test.vala`, `src/core/game/expression_test.vala`

**Interfaces:**

- Produces: `public interface Gsw.GameDef.MatchExpression : Expression` with `bool matches (ExpressionContext ctx, string value) throws ExpressionError`; `public class Gsw.GameDef.EqualsExpression : Expression, MatchExpression` with constructor `EqualsExpression (EvaluatableExpression expr)` and `bool is_literal { get; }`; `OrExpression.all_literal` (`bool`, true when every alternative is a literal). `Game.inf_matches` becomes `Gee.Map<string, GameDef.MatchExpression>`. `LogicalExpression` is deleted.

- [x] **Step 1: Update tests to the new API (they will fail to compile — that is the red state)**

In `src/core/game/game_test.vala` replace `add_match_rule`:

```vala
void add_match_rule (Game game, string key, string pattern) {
  try {
    var parser = new GameDef.ExpressionParser (pattern);
    var expr = parser.parse ();

    if (expr is GameDef.MatchExpression)
      game.inf_matches[key] = (GameDef.MatchExpression) expr;
    else
      game.inf_matches[key] = new GameDef.EqualsExpression ((GameDef.EvaluatableExpression) expr);
  } catch (Error err) {
    assert_no_error (err);
  }
}
```

In `src/core/game/expression_test.vala`, replace the `LogicalExpression` branch of the table loop:

```vala
        if (expression is GameDef.MatchExpression) {
          var result = ((GameDef.MatchExpression) expression).matches (ctx, row.result);
          assert_true (result);
        }
```

- [x] **Step 2: Rewrite the expression types in** `expression.vala`

Delete the `LogicalExpression` interface. In its place add:

```vala
/**
 * An expression that decides whether a server detail value matches.
 */
public interface MatchExpression : Expression {
  public abstract bool matches (ExpressionContext ctx, string value) throws ExpressionError;
}

/**
 * A match expression that succeeds when the value equals the result of
 * evaluating the wrapped expression.
 */
public class EqualsExpression : Expression, MatchExpression {
  private EvaluatableExpression expr;

  public bool is_literal {
    get { return expr is LiteralExpression; }
  }

  public EqualsExpression (EvaluatableExpression expr) {
    this.expr = expr;
  }

  public bool matches (ExpressionContext ctx, string value) throws ExpressionError {
    return value == expr.eval (ctx);
  }
}
```

Change `OrExpression` to implement `MatchExpression` (doc comment stays):

```vala
public class OrExpression : Expression, MatchExpression {
  private Gee.List<EvaluatableExpression> expressions;

  public bool all_literal {
    get {
      foreach (var expr in expressions) {
        if (!(expr is LiteralExpression))
          return false;
      }

      return true;
    }
  }

  public OrExpression (Gee.List<EvaluatableExpression> expressions) {
    this.expressions = expressions;
  }

  public bool matches (ExpressionContext ctx, string value) throws ExpressionError {
    foreach (var expr in expressions) {
      if (value == expr.eval (ctx))
        return true;
    }

    return false;
  }
}
```

- [x] **Step 3: Simplify** `Game`

In `src/core/game/game.vala`:

```vala
  public Gee.Map<string, GameDef.MatchExpression> inf_matches = new Gee.HashMap<string, GameDef.MatchExpression> ();
```

and `matches ()` becomes:

```vala
  public bool matches (string protocol_id, ServerDetails details) {
    if (protocol != protocol_id)
      return false;

    var ctx = new GameDef.ExpressionContext ();
    ctx.set ("inf", details);

    foreach (var entry in inf_matches.entries) {
      try {
        var value = details.get (entry.key);

        if (value == null)
          return false;

        if (!entry.value.matches (ctx, value))
          return false;
      } catch (GameDef.ExpressionError err) {
        warning ("failed to evaluate value of '%s' for '%s': %s", entry.key, id, err.message);
        return false;
      }
    }

    return true;
  }
```

- [x] **Step 4: Update the loader side in** `game-resolver.vala`

Inside the `MATCH_GROUP` case, replace the `expr` handling:

```vala
          try {
            var parser = new GameDef.ExpressionParser (value);
            var expr = parser.parse ();
            game.weight += value.length;

            if (expr is GameDef.MatchExpression) {
              game.inf_matches[key2] = (GameDef.MatchExpression) expr;
            } else if (expr is GameDef.EvaluatableExpression) {
              game.inf_matches[key2] = new GameDef.EqualsExpression ((GameDef.EvaluatableExpression) expr);
            } else {
              throw new IOError.INVALID_DATA (
                "expression at '%s' (group '%s') must be an evaluatable or match expression",
                key, MATCH_GROUP
              );
            }
          } catch (GameDef.ExpressionParserError parser_err) {
            throw new IOError.INVALID_DATA ("failed to parse expression at '%s' (group '%s'): %s", key, MATCH_GROUP, parser_err.message);
          }
```

- [x] **Step 5: Run tests**

Run: `meson test -C build core/game/expression core/game --print-errorlogs`
Expected: PASS (all existing `Game.matches` behavior tests unchanged and green).

- [x] **Step 6: STOP — user reviews, tests, and commits**

Suggested message: `refactor: unify match rules behind MatchExpression`

---



### Task 4: Extract GameDefinitionLoader

**Files:**

- Create: `src/core/game/game-definition-loader.vala`
- Create: `src/core/game/game-definition-loader_test.vala`
- Modify: `src/core/game/game-resolver.vala` (delete parsing code, delegate)
- Modify: `src/core/game/game.vala` (make `Game` public)
- Modify: `src/core/meson.build` (source + test target)

**Interfaces:**

- Produces: `public class Gsw.GameDefinitionLoader` with static methods `Game load_from_key_file (KeyFile kf) throws Error`, `Game load_from_data (string data) throws Error`, `Game load_from_bytes (Bytes bytes) throws Error`. `Game` becomes `public class Game`. The group-name constants move out of `game-resolver.vala`.

- [x] **Step 1: Write the failing test**

Create `src/core/game/game-definition-loader_test.vala` (with license header):

```vala
using Gsw;

const string VALID_INI = """
[Game]
id = cs2
protocol = source
port = 27015
qport-diff = 1

[Features]
console = source-console

[Match]
inf.appid = 730
inf.game = "Counter-Strike 2" or "CS2"

[Info]
game-name = "Counter-Strike 2"
game-mode = mapKeyword(inf["keywords"], ",", "Modes", "")

[Player]
Player = player,string,main
Score = score,number

[Modes]
competitive = Competitive

[Extra]
release-year = 2023
""";

public static int main (string[] args) {
  Test.init (ref args);

  Test.add_func ("/core/GameDefinitionLoader/valid/full_definition", () => {
    try {
      var game = GameDefinitionLoader.load_from_data (VALID_INI);

      assert_cmpstr (game.id, EQ, "cs2");
      assert_cmpstr (game.protocol, EQ, "source");
      assert_cmpint (game.port, EQ, 27015);
      assert_cmpint (game.qport_diff, EQ, 1);
      assert_cmpstr (game.features[ProtocolFeature.CONSOLE], EQ, "source-console");

      assert_cmpint (game.inf_matches.size, EQ, 2);
      assert_true (game.inf_matches["appid"] is GameDef.EqualsExpression);
      assert_true (game.inf_matches["game"] is GameDef.OrExpression);

      assert_cmpint (game.inf.size, EQ, 2);

      assert_cmpint (game.pfields.size, EQ, 2);
      assert_cmpstr (game.pfields[0].title, EQ, "Player");
      assert_cmpstr (game.pfields[0].field, EQ, "player");
      assert_true (game.pfields[0].main);
      assert_false (game.pfields[1].main);

      assert_true (game.maps.has_key ("Modes"));
      assert_cmpstr (game.maps["Modes"]["competitive"], EQ, "Competitive");
    } catch (Error err) {
      assert_no_error (err);
    }
  });

  Test.add_func ("/core/GameDefinitionLoader/error/missing_id", () => {
    try {
      GameDefinitionLoader.load_from_data ("[Game]\nprotocol = source\n\n[Match]\ninf.appid = 1\n");
      assert_not_reached ();
    } catch (Error err) {
      assert_nonnull (err);
    }
  });

  Test.add_func ("/core/GameDefinitionLoader/error/invalid_port", () => {
    try {
      GameDefinitionLoader.load_from_data ("[Game]\nid = x\nprotocol = source\nport = 70000\n\n[Match]\ninf.appid = 1\n");
      assert_not_reached ();
    } catch (Error err) {
      assert_nonnull (err);
    }
  });

  Test.add_func ("/core/GameDefinitionLoader/error/match_key_without_inf_prefix", () => {
    try {
      GameDefinitionLoader.load_from_data ("[Game]\nid = x\nprotocol = source\n\n[Match]\nappid = 1\n");
      assert_not_reached ();
    } catch (Error err) {
      assert_nonnull (err);
    }
  });

  Test.add_func ("/core/GameDefinitionLoader/error/malformed_match_expression", () => {
    try {
      GameDefinitionLoader.load_from_data ("[Game]\nid = x\nprotocol = source\n\n[Match]\ninf.appid = \"unterminated\n");
      assert_not_reached ();
    } catch (Error err) {
      assert_nonnull (err);
    }
  });

  Test.add_func ("/core/GameDefinitionLoader/error/no_match_rules", () => {
    try {
      GameDefinitionLoader.load_from_data ("[Game]\nid = x\nprotocol = source\n");
      assert_not_reached ();
    } catch (Error err) {
      assert_nonnull (err);
    }
  });

  Test.run ();
  return 0;
}
```

Register in `src/core/meson.build`: add `'game/game-definition-loader.vala',` to `core_sources` (alphabetically before `'game/game-resolver.vala'`), and next to the existing `test('core/game', ...)` block add:

```meson
test('core/game-definition-loader', executable(
  'game-definition-loader_test',
  core_sources,
  'game/game-definition-loader_test.vala',
  dependencies : dependencies,
))
```

- [x] **Step 2: Run to verify it fails**

Run: `meson compile -C build`
Expected: FAIL — `GameDefinitionLoader` does not exist yet.

- [x] **Step 3: Create the loader by moving code**

Make `Game` public in `src/core/game/game.vala`: `class Game {` → `public class Game {`.

Create `src/core/game/game-definition-loader.vala`. Move the six `const string *_GROUP` constants from `game-resolver.vala` here **unchanged**, then:

```vala
namespace Gsw {

const string GAME_GROUP = "Game";
const string FEATURES_GROUP = "Features";
const string MATCH_GROUP = "Match";
const string PLAYER_GROUP = "Player";
const string INFO_GROUP = "Info";
const string EXTRA_GROUP = "Extra";

/**
 * Parses game definition INI content into Game objects. Pure: performs no I/O.
 */
public class GameDefinitionLoader {

  public static Game load_from_data (string data) throws Error {
    var kf = new KeyFile ();
    kf.set_list_separator (',');
    kf.load_from_data (data, -1, NONE);
    return load_from_key_file (kf);
  }

  public static Game load_from_bytes (Bytes bytes) throws Error {
    var kf = new KeyFile ();
    kf.set_list_separator (',');
    kf.load_from_bytes (bytes, NONE);
    return load_from_key_file (kf);
  }

  public static Game load_from_key_file (KeyFile kf) throws Error {
    // ... body of GameResolver.load_game moved here verbatim
    // (including the Task 3 version of the MATCH_GROUP block) ...
  }

  private static void load_player_fields (KeyFile kf, Game game) throws Error {
    // ... body of GameResolver.load_player_fields moved here verbatim,
    // with the `ref` removed from the parameter and call site ...
  }
}

}
```

The two moved bodies are copied without modification except: `load_player_fields (kf, ref game)` → `load_player_fields (kf, game)`.

- [x] **Step 4: Slim down** `game-resolver.vala`

Delete `load_game`, `load_player_fields`, and the moved constants. In `load_games ()`, the per-file body becomes:

```vala
      try {
        var ext = get_filename_extension (ini_file.file);

        if (ext != "ini")
          continue;

        var game = GameDefinitionLoader.load_from_bytes (new Bytes.static (ini_file.data));
        games[game.id] = game;
      } catch (Error err) {
        log (Config.LOG_DOMAIN, LEVEL_ERROR, "failed to load game definition from '%s': %s", ini_file.file.get_path (), err.message);
      }
```

- [x] **Step 5: Run tests**

Run: `meson test -C build core/game-definition-loader core/game --print-errorlogs`
Expected: PASS.

- [x] **Step 6: STOP — user reviews, tests, and commits**

Suggested message: `refactor: extract GameDefinitionLoader from GameResolver`

---



### Task 5: Specificity tuple replaces weight

**Files:**

- Modify: `src/core/game/game.vala` (add `Specificity`, drop `weight`)
- Modify: `src/core/game/game-definition-loader.vala` (classification)
- Modify: `src/core/game/game-resolver.vala:195-221` (`resolve`)
- Modify: `docs/game-definition-files.md` (Weight System section)
- Test: `src/core/game/game-definition-loader_test.vala`

**Interfaces:**

- Produces: `public struct Gsw.Specificity { public int exact; public int alternatives; public int patterns; public int compare (Specificity other); public string to_string (); }` and `Game.specificity` field. `Game.weight` is deleted.

- [x] **Step 1: Write the failing tests**

Append to `src/core/game/game-definition-loader_test.vala`:

```vala
  Test.add_func ("/core/GameDefinitionLoader/specificity/classification", () => {
    try {
      var game = GameDefinitionLoader.load_from_data ("""
[Game]
id = x
protocol = source

[Match]
inf.appid = 730
inf.game = "a" or "b"
inf.dir = regex("^cs", inf["dir"])
""");

      assert_cmpint (game.specificity.exact, EQ, 1);
      assert_cmpint (game.specificity.alternatives, EQ, 1);
      assert_cmpint (game.specificity.patterns, EQ, 1);
    } catch (Error err) {
      assert_no_error (err);
    }
  });

  Test.add_func ("/core/GameDefinitionLoader/specificity/ordering", () => {
    var two_exact = Specificity () { exact = 2 };
    var one_exact_many_patterns = Specificity () { exact = 1, patterns = 5 };
    assert_true (two_exact.compare (one_exact_many_patterns) > 0);

    var one_alternative = Specificity () { alternatives = 1 };
    var many_patterns = Specificity () { patterns = 3 };
    assert_true (one_alternative.compare (many_patterns) > 0);

    var equal_a = Specificity () { exact = 1 };
    var equal_b = Specificity () { exact = 1 };
    assert_cmpint (equal_a.compare (equal_b), EQ, 0);
  });
```

- [x] **Step 2: Run to verify failure**

Run: `meson compile -C build`
Expected: FAIL — `Specificity` unknown.

- [x] **Step 3: Implement**

In `src/core/game/game.vala` (top of namespace, above `Game`):

```vala
/**
 * Match specificity of a game definition, counted by rule kind.
 * Compared lexicographically: exact literal rules always outrank
 * or-alternative rules, which always outrank pattern rules.
 */
public struct Specificity {
  public int exact;
  public int alternatives;
  public int patterns;

  public int compare (Specificity other) {
    if (exact != other.exact)
      return exact - other.exact;

    if (alternatives != other.alternatives)
      return alternatives - other.alternatives;

    return patterns - other.patterns;
  }

  public string to_string () {
    return "(%d,%d,%d)".printf (exact, alternatives, patterns);
  }
}
```

In `Game`, replace `public int weight;` with `public Specificity specificity;`.

In `game-definition-loader.vala`, delete `game.weight += value.length;` and, after the expression is stored in `game.inf_matches[key2]`, add `classify (game, game.inf_matches[key2]);` with:

```vala
  private static void classify (Game game, GameDef.MatchExpression expr) {
    var eq = expr as GameDef.EqualsExpression;

    if (eq != null && eq.is_literal) {
      game.specificity.exact++;
      return;
    }

    var or_expr = expr as GameDef.OrExpression;

    if (or_expr != null && or_expr.all_literal) {
      game.specificity.alternatives++;
      return;
    }

    game.specificity.patterns++;
  }
```

In `game-resolver.vala` `resolve ()`, replace the sort and selection:

```vala
      matching_games.sort ((a, b) => {
        var cmp = b.specificity.compare (a.specificity);
        return cmp != 0 ? cmp : strcmp (a.id, b.id);
      });

      game = matching_games[0];

      if (matching_games.size > 1 && game.specificity.compare (matching_games[1].specificity) == 0)
        warning ("ambiguous game match: '%s' and '%s' have equal specificity %s",
                 game.id, matching_games[1].id, game.specificity.to_string ());
```

Also delete the dead `if (game == null) return false;` after the block (unreachable).

- [x] **Step 4: Run tests**

Run: `meson test -C build core/game-definition-loader core/game --print-errorlogs`
Expected: PASS.

- [x] **Step 5: Update the docs**

In `docs/game-definition-files.md`, replace the "Weight Calculation"/"Weight System" wording in the Conflict Resolution section with:

```markdown
### Specificity System

Each game definition gets a specificity tuple `(exact, alternatives, patterns)` counting its `[Match]` rules by kind:

- **exact** — rules comparing a field to a literal value (`inf.appid = 730`)
- **alternatives** — rules using `or` between literal values
- **patterns** — rules using functions such as `regex()` or non-literal expressions

Tuples are compared left to right: a definition with more exact rules always outranks one with fewer, regardless of the other counts. If two matching definitions have identical specificity, a warning is logged and the definition with the lexicographically smaller id is selected.
```

(Also update step 3/4 of the "Matching Process" list to say "specificity" instead of "weight".)

- [x] **Step 6: STOP — user reviews, tests, and commits**

Suggested message: `feat: replace weight heuristic with specificity tuples`

---



### Task 6: Generic game definitions in the loader

**Files:**

- Modify: `src/core/game/game.vala` (`is_generic` field)
- Modify: `src/core/game/game-definition-loader.vala`
- Modify: `docs/game-definition-files.md`
- Test: `src/core/game/game-definition-loader_test.vala`

**Interfaces:**

- Produces: `Game.is_generic` (`public bool`). Loader semantics: no `[Match]` section → generic; `[Match]` present but empty → error (unchanged message).

- [x] **Step 1: Update/add tests (failing)**

In `game-definition-loader_test.vala`, **replace** the `error/no_match_rules` test with:

```vala
  Test.add_func ("/core/GameDefinitionLoader/generic/no_match_section", () => {
    try {
      var game = GameDefinitionLoader.load_from_data ("[Game]\nid = generic-source\nprotocol = source\n");
      assert_true (game.is_generic);
    } catch (Error err) {
      assert_no_error (err);
    }
  });

  Test.add_func ("/core/GameDefinitionLoader/error/empty_match_section", () => {
    try {
      GameDefinitionLoader.load_from_data ("[Game]\nid = x\nprotocol = source\n\n[Match]\n");
      assert_not_reached ();
    } catch (Error err) {
      assert_nonnull (err);
    }
  });

  Test.add_func ("/core/GameDefinitionLoader/generic/specific_is_not_generic", () => {
    try {
      var game = GameDefinitionLoader.load_from_data ("[Game]\nid = x\nprotocol = source\n\n[Match]\ninf.appid = 1\n");
      assert_false (game.is_generic);
    } catch (Error err) {
      assert_no_error (err);
    }
  });
```

Note on `empty_match_section`: GLib's `KeyFile` cannot round-trip a section with zero keys, so the test uses a key that fails the `inf.<field>` format check — the point is that a `[Match]` section that yields no rules is still an error path, never a generic.

- [x] **Step 2: Run to verify failure**

Run: `meson compile -C build`
Expected: FAIL — `is_generic` unknown.

- [x] **Step 3: Implement**

In `Game`, add below `protocol`:

```vala
  public bool is_generic;
```

In `load_from_key_file`, replace the final check:

```vala
    if (!kf.has_group (MATCH_GROUP)) {
      game.is_generic = true;
    } else if (game.inf_matches.size == 0) {
      throw new IOError.INVALID_DATA ("game '%s' has no match rules", game_id);
    }
```

- [x] **Step 4: Run tests**

Run: `meson test -C build core/game-definition-loader --print-errorlogs`
Expected: PASS.

- [x] **Step 5: Document**

Add to `docs/game-definition-files.md` after the `[Match]` section description:

```markdown
### Generic Game Definitions

A definition **without** a `[Match]` section is the *generic* definition for its protocol. At most one generic definition per protocol is allowed. Shipped generics follow the naming convention `generic-<protocol>.ini`.

When a server is resolved, the generic definition for its protocol always applies first as a base layer: its `[Info]` expressions, player fields, features, and default port. If a specific game also matches, its values override the generic's per `[Info]` key, its `[Player]` table (as a whole) replaces the generic's if present, and its id becomes the server's `game-id`. If no specific game matches, the server is identified as the generic game. Expressions always evaluate against their own file's custom map sections only.
```

- [x] **Step 6: STOP — user reviews, tests, and commits**

Suggested message: `feat: support generic game definitions in loader`

---



### Task 7: GameDb — storage, loading, ensure_loaded

**Files:**

- Create: `src/core/game/game-db.vala`
- Create: `src/core/game/game-db_test.vala`
- Modify: `src/core/meson.build`

**Interfaces:**

- Produces: `public class Gsw.GameDb : Object` with `void add_game (Game game)`, `async void load ()`, `void finish_loading ()`, `async void ensure_loaded ()`. Resolution methods come in Task 8. `GameResolver` keeps existing behavior until Task 10.

- [x] **Step 1: Write the failing tests**

Create `src/core/game/game-db_test.vala` (with license header):

```vala
using Gsw;

Game make_game (string id, string protocol, bool generic = false) {
  var game = new Game (id, protocol);
  game.is_generic = generic;
  return game;
}

public static int main (string[] args) {
  Test.init (ref args);

  Test.add_func ("/core/GameDb/ensure_loaded/waits_for_finish_loading", () => {
    var db = new GameDb ();
    var loop = new MainLoop ();
    var resumed = false;

    db.ensure_loaded.begin ((obj, res) => {
      db.ensure_loaded.end (res);
      resumed = true;
      loop.quit ();
    });

    assert_false (resumed);
    db.finish_loading ();
    loop.run ();
    assert_true (resumed);
  });

  Test.add_func ("/core/GameDb/ensure_loaded/immediate_when_loaded", () => {
    var db = new GameDb ();
    db.finish_loading ();

    var loop = new MainLoop ();
    var resumed = false;

    db.ensure_loaded.begin ((obj, res) => {
      db.ensure_loaded.end (res);
      resumed = true;
      loop.quit ();
    });

    loop.run ();
    assert_true (resumed);
  });

  Test.add_func ("/core/GameDb/ensure_loaded/multiple_waiters", () => {
    var db = new GameDb ();
    var loop = new MainLoop ();
    var resumed = 0;

    for (var i = 0; i < 3; i++) {
      db.ensure_loaded.begin ((obj, res) => {
        db.ensure_loaded.end (res);
        resumed++;

        if (resumed == 3)
          loop.quit ();
      });
    }

    db.finish_loading ();
    loop.run ();
    assert_cmpint (resumed, EQ, 3);
  });

  Test.add_func ("/core/GameDb/add_game/duplicate_generic_rejected", () => {
    var db = new GameDb ();
    db.add_game (make_game ("generic-source", "source", true));

    Test.expect_message (Config.LOG_DOMAIN, LEVEL_WARNING, "*duplicate generic*");
    db.add_game (make_game ("another-generic", "source", true));
  });

  Test.run ();
  return 0;
}
```

Register in `src/core/meson.build`: add `'game/game-db.vala',` to `core_sources` and:

```meson
test('core/game-db', executable(
  'game-db_test',
  core_sources,
  'game/game-db_test.vala',
  dependencies : dependencies,
))
```

- [x] **Step 2: Run to verify failure**

Run: `meson compile -C build`
Expected: FAIL — `GameDb` unknown.

- [x] **Step 3: Implement** `game-db.vala`

```vala
namespace Gsw {

/**
 * Holds all known game definitions and resolves server details to games.
 *
 * Loading is non-blocking: load() is started by the composition root, and
 * consumers await ensure_loaded() before resolving.
 */
public class GameDb : Object {
  private class Waiter {
    public SourceFunc fn;

    public Waiter (owned SourceFunc fn) {
      this.fn = (owned) fn;
    }
  }

  private Gee.Map<string, Game> games = new Gee.HashMap<string, Game> ();
  private Gee.Map<string, Game> generics = new Gee.HashMap<string, Game> ();
  private bool loaded;
  private Gee.List<Waiter> waiters = new Gee.ArrayList<Waiter> ();

  /**
   * Adds a game definition. A second generic definition for the same
   * protocol is rejected with a warning; the first one wins.
   */
  public void add_game (Game game) {
    if (game.is_generic) {
      if (generics.has_key (game.protocol)) {
        warning ("duplicate generic game definition '%s' for protocol '%s' ('%s' is already registered)",
                 game.id, game.protocol, generics[game.protocol].id);
        return;
      }

      generics[game.protocol] = game;
    }

    games[game.id] = game;
  }

  /**
   * Discovers and loads all game definition files, then releases all
   * ensure_loaded() waiters. Per-file errors are logged and skipped.
   */
  public async void load () {
    try {
      var ini_files = yield get_app_data_files_contents ("games", "GSW_GAMES_DIR", null);

      foreach (var ini_file in ini_files) {
        try {
          if (get_filename_extension (ini_file.file) != "ini")
            continue;

          add_game (GameDefinitionLoader.load_from_bytes (new Bytes.static (ini_file.data)));
        } catch (Error err) {
          warning ("failed to load game definition from '%s': %s", ini_file.file.get_path (), err.message);
        }
      }
    } catch (Error err) {
      warning ("failed to load game definitions: %s", err.message);
    }

    finish_loading ();
  }

  /**
   * Marks the database as loaded and resumes all ensure_loaded() waiters.
   * Called by load(); public so tests and stubs can control readiness.
   */
  public void finish_loading () {
    loaded = true;

    foreach (var waiter in waiters)
      Idle.add ((owned) waiter.fn);

    waiters.clear ();
  }

  /**
   * Returns immediately if definitions are loaded; otherwise suspends the
   * caller until loading completes.
   */
  public async void ensure_loaded () {
    if (loaded)
      return;

    waiters.add (new Waiter (ensure_loaded.callback));
    yield;
  }
}

}
```

- [x] **Step 4: Run tests**

Run: `meson test -C build core/game-db --print-errorlogs`
Expected: PASS.

- [x] **Step 5: STOP — user reviews, tests, and commits**

Suggested message: `feat: add GameDb with non-blocking load and ensure_loaded`

---



### Task 8: GameDb — layered resolution, cache, lookups

**Files:**

- Modify: `src/core/game/game-db.vala`
- Test: `src/core/game/game-db_test.vala`

**Interfaces:**

- Produces on `GameDb` (all `virtual` so tests can stub):
  - `virtual bool resolve (string protocol_id, ServerInfo sinfo, ServerDetails details)`
  - `virtual Gee.List<PlayerField>? get_plist_fields (string? game_id)`
  - `virtual string? get_feature_protocol (string? game_id, ProtocolFeature feature)`
- Consumes: `GameDefinitionLoader.load_from_data` (tests), `TtlCache`, `Game.matches/enhance_sinfo/specificity/is_generic`.

- [x] **Step 1: Write the failing tests**

Add to `game-db_test.vala` a helper and tests:

```vala
Game load_game (string ini) {
  try {
    return GameDefinitionLoader.load_from_data (ini);
  } catch (Error err) {
    assert_no_error (err);
    assert_not_reached ();
  }
}
```

```vala
  Test.add_func ("/core/GameDb/resolve/layering", () => {
    var db = new GameDb ();

    db.add_game (load_game ("""
[Game]
id = generic-source
protocol = source

[Info]
game-name = inf["game"]
game-mode = inf["mode"]
"""));

    db.add_game (load_game ("""
[Game]
id = cs2
protocol = source

[Match]
inf.appid = 730

[Info]
game-name = "Counter-Strike 2"
"""));

    db.finish_loading ();

    var details = new ServerDetails ();
    details["appid"] = "730";
    details["game"] = "from-generic";
    details["mode"] = "casual";

    var sinfo = new ServerInfo ();
    assert_true (db.resolve ("source", sinfo, details));
    assert_cmpstr (sinfo.game_id, EQ, "cs2");                 // specific id wins
    assert_cmpstr (sinfo.game_name, EQ, "Counter-Strike 2");  // specific overrides generic
    assert_cmpstr (sinfo.game_mode, EQ, "casual");            // generic-only key survives
  });

  Test.add_func ("/core/GameDb/resolve/generic_only", () => {
    var db = new GameDb ();

    db.add_game (load_game ("""
[Game]
id = generic-source
protocol = source

[Info]
game-name = inf["game"]
"""));

    db.finish_loading ();

    var details = new ServerDetails ();
    details["game"] = "Some Mod";

    var sinfo = new ServerInfo ();
    assert_true (db.resolve ("source", sinfo, details));
    assert_cmpstr (sinfo.game_id, EQ, "generic-source");
    assert_cmpstr (sinfo.game_name, EQ, "Some Mod");
  });

  Test.add_func ("/core/GameDb/resolve/no_match_no_generic", () => {
    var db = new GameDb ();

    db.add_game (load_game ("[Game]\nid = cs2\nprotocol = source\n\n[Match]\ninf.appid = 730\n"));
    db.finish_loading ();

    var details = new ServerDetails ();
    details["appid"] = "1";

    var sinfo = new ServerInfo ();
    assert_false (db.resolve ("source", sinfo, details));
  });

  Test.add_func ("/core/GameDb/resolve/most_specific_wins", () => {
    var db = new GameDb ();

    db.add_game (load_game ("[Game]\nid = by-pattern\nprotocol = source\n\n[Match]\ninf.appid = regex(\"(730)\", inf[\"appid\"])\n"));
    db.add_game (load_game ("[Game]\nid = by-literal\nprotocol = source\n\n[Match]\ninf.appid = 730\n"));
    db.finish_loading ();

    var details = new ServerDetails ();
    details["appid"] = "730";

    var sinfo = new ServerInfo ();
    assert_true (db.resolve ("source", sinfo, details));
    assert_cmpstr (sinfo.game_id, EQ, "by-literal");
  });

  Test.add_func ("/core/GameDb/resolve/ambiguity_warns_and_resolves_by_id", () => {
    var db = new GameDb ();

    db.add_game (load_game ("[Game]\nid = zebra\nprotocol = source\n\n[Match]\ninf.appid = 730\n"));
    db.add_game (load_game ("[Game]\nid = aardvark\nprotocol = source\n\n[Match]\ninf.appid = 730\n"));
    db.finish_loading ();

    var details = new ServerDetails ();
    details["appid"] = "730";

    Test.expect_message (null, LEVEL_WARNING, "*ambiguous game match*");

    var sinfo = new ServerInfo ();
    assert_true (db.resolve ("source", sinfo, details));
    assert_cmpstr (sinfo.game_id, EQ, "aardvark");
  });

  Test.add_func ("/core/GameDb/get_plist_fields/generic_fallback", () => {
    var db = new GameDb ();

    db.add_game (load_game ("""
[Game]
id = generic-source
protocol = source

[Player]
Player = player,string,main
"""));

    db.add_game (load_game ("[Game]\nid = cs2\nprotocol = source\n\n[Match]\ninf.appid = 730\n"));
    db.finish_loading ();

    var fields = db.get_plist_fields ("cs2");
    assert_nonnull (fields);
    assert_cmpint (fields.size, EQ, 1);
    assert_cmpstr (fields[0].title, EQ, "Player");

    assert_null (db.get_plist_fields ("unknown"));
    assert_null (db.get_plist_fields (null));
  });

  Test.add_func ("/core/GameDb/get_feature_protocol/generic_fallback", () => {
    var db = new GameDb ();

    db.add_game (load_game ("""
[Game]
id = generic-source
protocol = source

[Features]
console = source-console
"""));

    db.add_game (load_game ("[Game]\nid = cs2\nprotocol = source\n\n[Match]\ninf.appid = 730\n"));
    db.finish_loading ();

    assert_cmpstr (db.get_feature_protocol ("cs2", ProtocolFeature.CONSOLE), EQ, "source-console");
    assert_null (db.get_feature_protocol ("unknown", ProtocolFeature.CONSOLE));
    assert_null (db.get_feature_protocol (null, ProtocolFeature.CONSOLE));
  });
```

- [x] **Step 2: Run to verify failure**

Run: `meson compile -C build`
Expected: FAIL — `resolve` not defined on `GameDb`.

- [x] **Step 3: Implement in** `game-db.vala`

Above the class:

```vala
const int64 GAME_DB_CACHE_TTL = 30 * TimeSpan.MINUTE;

/**
 * The result of resolving server details: the protocol's generic
 * definition (if any) plus the best matching specific definition (if any).
 * At least one of the two is non-null.
 */
public class ResolvedGame {
  public Game? generic;
  public Game? specific;
}
```

Inside `GameDb`, add the cache field:

```vala
  private TtlCache<ServerDetails, ResolvedGame> cache = new TtlCache<ServerDetails, ResolvedGame> (GAME_DB_CACHE_TTL);
```

and the methods:

```vala
  /**
   * Identifies the game for the given server details and applies its
   * information to sinfo (generic layer first, then the specific game).
   * Returns false when nothing could be resolved.
   */
  public virtual bool resolve (string protocol_id, ServerInfo sinfo, ServerDetails details) {
    var resolved = cache.get (details);

    if (resolved == null) {
      var generic = generics[protocol_id];
      var specific = find_best_match (protocol_id, details);

      if (generic == null && specific == null)
        return false;

      resolved = new ResolvedGame () { generic = generic, specific = specific };
      cache.set (details, resolved);
    }

    if (resolved.generic != null)
      resolved.generic.enhance_sinfo (sinfo, details);

    if (resolved.specific != null)
      resolved.specific.enhance_sinfo (sinfo, details);

    return true;
  }

  private Game? find_best_match (string protocol_id, ServerDetails details) {
    var matching = new Gee.ArrayList<Game> ();

    foreach (var game in games.values) {
      if (!game.is_generic && game.matches (protocol_id, details))
        matching.add (game);
    }

    if (matching.is_empty)
      return null;

    matching.sort ((a, b) => {
      var cmp = b.specificity.compare (a.specificity);
      return cmp != 0 ? cmp : strcmp (a.id, b.id);
    });

    var best = matching[0];

    if (matching.size > 1 && best.specificity.compare (matching[1].specificity) == 0)
      warning ("ambiguous game match: '%s' and '%s' have equal specificity %s",
               best.id, matching[1].id, best.specificity.to_string ());

    return best;
  }

  public virtual Gee.List<PlayerField>? get_plist_fields (string? game_id) {
    var game = game_id != null ? games[game_id] : null;

    if (game == null)
      return null;

    if (game.pfields.size == 0 && !game.is_generic) {
      var generic = generics[game.protocol];

      if (generic != null)
        return generic.pfields;
    }

    return game.pfields;
  }

  public virtual string? get_feature_protocol (string? game_id, ProtocolFeature feature) {
    var game = game_id != null ? games[game_id] : null;

    if (game == null)
      return null;

    if (game.features.has_key (feature))
      return game.features[feature];

    if (!game.is_generic) {
      var generic = generics[game.protocol];

      if (generic != null)
        return generic.features[feature];
    }

    return null;
  }
```

- [x] **Step 4: Run tests**



Run: `meson test -C build core/game-db --print-errorlogs`
Expected: PASS.

- [x] **Step 5: STOP — user reviews, tests, and commits**

Suggested message: `feat: layered game resolution in GameDb`

---



### Task 9: Querier and QuerierManager use GameDb

**Files:**

- Modify: `src/core/querier.vala`
- Modify: `src/core/querier-manager.vala`
- Modify: `src/application.vala`

**Interfaces:**

- Produces: `Querier (Server server, QueryProtocol protocol, NetTransport transport, GameDb game_db)`; `QuerierManager` gains settable `game_db`, `protocol_registry`, `transport_registry` properties (turned into construct-only in Task 12). `Querier` no longer references `GameResolver`, `query_pending`, or the `ready` flag.
- Transitional: `Client` keeps using `GameResolver` until Task 10, so both loaders run side by side — expected, temporary.

- [x] **Step 1: Rewrite** `QuerierManager`

Replace the properties/`create_querier`:

```vala
  public GameDb game_db { get; set; }
  public ProtocolRegistry protocol_registry { get; set; }
  public TransportRegistry transport_registry { get; set; }
```

```vala
  public Querier create_querier (Server server, string protocol_id) throws Error {
    var protocol = protocol_registry.create (protocol_id);
    var transport = transport_registry.create_net_transport (protocol.info.transport, server.host, server.qport);

    var querier = new Querier (server, (QueryProtocol) protocol, transport, game_db);
    querier.weak_ref ((WeakNotify) remove_querier);
    queriers.add (querier);

    update_timer ();
    return querier;
  }
```

- [x] **Step 2: Rewrite** `Querier`

- Add property: `public GameDb game_db { get; construct; }`
- Delete: `private GameResolver game_resolver = ...`, `private bool query_pending;`, `handle_pending_query ()`, and both `game_resolver.notify["ready"]` connect/disconnect lines (construct + destructor).
- In `construct`, delete the `transport = TransportRegistry.get_instance ()...` line (transport now arrives as a construct property; the `transport.resolved.connect` lines stay).
- New constructor:

```vala
  public Querier (Server server, QueryProtocol protocol, NetTransport transport, GameDb game_db) {
    Object (
      server : server,
      protocol : protocol,
      transport : transport,
      game_db : game_db
    );
  }
```

- Replace `query ()`:

```vala
  private bool query_in_flight;

  public void query () {
    if (query_in_flight)
      return;

    query_in_flight = true;

    query_async.begin ((obj, res) => {
      query_async.end (res);
      query_in_flight = false;
    });
  }

  private async void query_async () {
    yield game_db.ensure_loaded ();

    try {
      start_timeout_timer ();
      start_ping_timer ();
      protocol.query ();
    } catch (Error err) {
      error = new QuerierError.QUERYING ("failed to query %s:%d: %s", server.host, server.qport, err.message);
    }
  }
```

- In `on_sinfo_updated`: `game_resolver.resolve (...)` → `game_db.resolve (protocol.info.id, sinfo, details);`
- In `on_plist_updated`: `game_resolver.get_plist_fields (game_id)` → `game_db.get_plist_fields (game_id)`
- In `on_error`: delete `query_pending = false;`

- [x] **Step 3: Wire in** `Application.startup`

Add field `private GameDb game_db;` and, in `startup ()` right after the transport registrations:

```vala
    // games
    game_db = new GameDb ();
    game_db.load.begin ();

    querier_manager.game_db = game_db;
    querier_manager.protocol_registry = protocol_registry;
    querier_manager.transport_registry = transport_registry;
```

(This must run before `server_list = new PersistentServerList ();`.)

- [x] **Step 4: Compile and run all tests**

Run: `meson compile -C build && meson test -C build --print-errorlogs`
Expected: PASS. Note: `GameResolver` still exists and still serves `Client` — that is intentional until Task 10.

- [x] **Step 5: STOP — user reviews, tests (run the app: servers should query and get game names), and commits**

Suggested message: `refactor: Querier awaits GameDb instead of ready flag`

---



### Task 10: Client, ServerList, PersistentServerList take dependencies; delete GameResolver

**Files:**

- Modify: `src/core/client.vala`
- Modify: `src/core/server-list.vala`
- Modify: `src/persistent-server-list.vala`
- Modify: `src/application.vala`
- Delete: `src/core/game/game-resolver.vala` (and its line in `src/core/meson.build`)
- Test: `src/core/server-list_test.vala`, `tests/test-server-watcher.vala`

**Interfaces:**

- Produces:
  - `Client (Server server, QuerierManager querier_manager, ProtocolRegistry protocol_registry, TransportRegistry transport_registry, GameDb game_db)` — all four services as `{ get; construct; }` properties (`transport_registry` is consumed in Task 11).
  - `ServerList (QuerierManager querier_manager, ProtocolRegistry protocol_registry, TransportRegistry transport_registry, GameDb game_db)` and identical `PersistentServerList` constructor.
- `GameResolver` and its `[SingleInstance]` are gone.

- [x] **Step 1: Update tests first (red = compile failure)**

In **both** `src/core/server-list_test.vala` and `tests/test-server-watcher.vala`, add at file scope (after `using` lines):

```vala
ServerList make_server_list () {
  var protocol_registry = new ProtocolRegistry ();
  var transport_registry = new TransportRegistry ();
  var game_db = new GameDb ();
  game_db.finish_loading ();

  var querier_manager = new QuerierManager ();
  querier_manager.game_db = game_db;
  querier_manager.protocol_registry = protocol_registry;
  querier_manager.transport_registry = transport_registry;

  return new ServerList (querier_manager, protocol_registry, transport_registry, game_db);
}
```

Replace **every** `new ServerList ()` in both files with `make_server_list ()`.

In `src/core/server-list_test.vala:127-128`, the direct Client construction becomes:

```vala
    var list = make_server_list ();
    var client = new Client (new Server ("localhost:27015"), list.querier_manager, list.protocol_registry, list.transport_registry, list.game_db);
```

(`QuerierManager` etc. are still `[SingleInstance]` until Task 12, so `new` returns the shared instance — fine for tests.)

- [x] **Step 2: Rewrite** `Client` **wiring**

Replace the field block and constructor:

```vala
  public QuerierManager querier_manager { get; construct; }
  public ProtocolRegistry protocol_registry { get; construct; }
  public TransportRegistry transport_registry { get; construct; }
  public GameDb game_db { get; construct; }
```

```vala
  public Client (Server server, QuerierManager querier_manager, ProtocolRegistry protocol_registry, TransportRegistry transport_registry, GameDb game_db) {
    Object (
      server : server,
      querier_manager : querier_manager,
      protocol_registry : protocol_registry,
      transport_registry : transport_registry,
      game_db : game_db
    );
  }
```

In `construct`, replace the trailing `GameResolver` block (`var game_resolver = ...` through `on_game_resolver_ready_change ();`) with:

```vala
    game_db.ensure_loaded.begin ((obj, res) => {
      game_db.ensure_loaded.end (res);
      activate ();
    });
```

In the destructor, delete the two `game_resolver` lines. Delete `on_game_resolver_ready_change ()` entirely.

In `reset ()`: `ProtocolRegistry.get_instance ().list_by_feature (QUERY)` → `protocol_registry.list_by_feature (QUERY)`.

`setup_console_feature ()` becomes:

```vala
  private void setup_console_feature () {
    var game_id = sinfo.game_id;

    var console_protocol = (game_id != null && game_id.length > 0)
      ? game_db.get_feature_protocol (game_id, CONSOLE)
      : null;
```

(rest of the method unchanged).

In `setup_console_client ()`:

```vala
    var console_protocol = game_db.get_feature_protocol (sinfo.game_id, CONSOLE);

    try {
      var proto = (ConsoleProtocol) protocol_registry.create (console_protocol);
      console_client = new ConsoleClient (server.host, console_port, console_password, proto);
    } catch (Error err) {
```

- [x] **Step 3: Rewrite** `ServerList` **/** `PersistentServerList`

`ServerList` gains:

```vala
  public QuerierManager querier_manager { get; construct; }
  public ProtocolRegistry protocol_registry { get; construct; }
  public TransportRegistry transport_registry { get; construct; }
  public GameDb game_db { get; construct; }

  public ServerList (QuerierManager querier_manager, ProtocolRegistry protocol_registry, TransportRegistry transport_registry, GameDb game_db) {
    Object (
      querier_manager : querier_manager,
      protocol_registry : protocol_registry,
      transport_registry : transport_registry,
      game_db : game_db
    );
  }
```

and in `add ()`:

```vala
    var client = new Client (new Server (address), querier_manager, protocol_registry, transport_registry, game_db);
```

`PersistentServerList` gains a matching pass-through constructor:

```vala
  public PersistentServerList (QuerierManager querier_manager, ProtocolRegistry protocol_registry, TransportRegistry transport_registry, GameDb game_db) {
    Object (
      querier_manager : querier_manager,
      protocol_registry : protocol_registry,
      transport_registry : transport_registry,
      game_db : game_db
    );
  }
```

In `Application.startup`:

```vala
    server_list = new PersistentServerList (querier_manager, protocol_registry, transport_registry, game_db);
```

- [x] **Step 4: Delete** `GameResolver`

Delete `src/core/game/game-resolver.vala` and remove `'game/game-resolver.vala',` from `src/core/meson.build`. Grep to confirm nothing references it: `rg -n "GameResolver" src tests` must return nothing.

- [x] **Step 5: Compile and run all tests**

Run: `meson compile -C build && meson test -C build --print-errorlogs`
Expected: PASS.

- [x] **Step 6: STOP — user reviews, tests (run the app: game identification, player list columns, console availability), and commits**

Suggested message: `refactor: inject services into Client/ServerList, drop GameResolver`

---



### Task 11: Transport factories and ConsoleClient injection

**Files:**

- Modify: `src/core/transport/transport-registry.vala`
- Modify: `src/core/transport/udp-transport.vala`
- Modify: `src/core/console-client.vala`
- Modify: `src/core/client.vala` (`setup_console_client`)
- Modify: `src/application.vala` (registration)
- Modify: `src/ui/about.vala` (drop `class_type` line)

**Interfaces:**

- Produces: `public delegate NetTransport Gsw.NetTransportFactory (string host, uint16 port);` and `TransportDesc (string id, owned NetTransportFactory factory)`; `UdpTransport (UdpTransportManager manager, string host, uint16 port)`; `ConsoleClient (string host, uint16 port, string password, ConsoleProtocol protocol, NetTransport transport)`.
- Check first: `TcpTransport` must have a `(string host, uint16 port)` constructor — if it only has construct properties, add one following the `UdpTransport` pattern.

- [x] **Step 1: Rewrite** `TransportRegistry`

```vala
namespace Gsw {

public delegate NetTransport NetTransportFactory (string host, uint16 port);

public class TransportDesc {
  public string id;
  public NetTransportFactory factory;

  public TransportDesc (string id, owned NetTransportFactory factory) {
    this.id = id;
    this.factory = (owned) factory;
  }
}

[SingleInstance]
public class TransportRegistry : Object {
  private Gee.Map<string, TransportDesc> transports = new Gee.HashMap<string, TransportDesc> ();

  public static TransportRegistry get_instance () {
    return new TransportRegistry ();
  }

  public Gee.Collection<TransportDesc> list () {
    return transports.values.read_only_view;
  }

  public void register (TransportDesc desc) {
    transports.set (desc.id, desc);
  }

  public NetTransport? create_net_transport (string id, string host, uint16 port) {
    if (!transports.has_key (id)) {
      warning ("unknown transport '%s'", id);
      return null;
    }

    return transports[id].factory (host, port);
  }
}

}
```

(`[SingleInstance]`/`get_instance` are removed in Task 12.)

- [x] **Step 2: Inject the manager into** `UdpTransport`

```vala
class UdpTransport : NetTransport {
  public UdpTransportManager manager { get; construct; }
  private uint8[]? output_message;

  construct {
    manager.register (this);
  }

  public UdpTransport (UdpTransportManager manager, string host, uint16 port) {
    Object (manager : manager, host : host, port : port);
  }
```

(destructor and the rest unchanged — `manager` references now hit the property).

- [x] **Step 3: Update registration in** `Application.startup`

```vala
    // transports
    transport_registry.register (new TransportDesc ("udp", (host, port) => new UdpTransport (udp_transport_manager, host, port)));
    transport_registry.register (new TransportDesc ("tcp", (host, port) => new TcpTransport (host, port)));
```

- [x] **Step 4: Pass the transport into** `ConsoleClient`

In `console-client.vala`, delete the `transport = TransportRegistry.get_instance ()...` line from `construct` (the `transport.connected.connect` etc. lines stay) and extend the constructor:

```vala
  public ConsoleClient (string host, uint16 port, string password, ConsoleProtocol protocol, NetTransport transport) {
    Object (
      host : host,
      port : port,
      password : password,
      protocol : protocol,
      transport : transport
    );
  }
```

In `client.vala` `setup_console_client ()`:

```vala
      var proto = (ConsoleProtocol) protocol_registry.create (console_protocol);
      var transport = transport_registry.create_net_transport (proto.info.transport, server.host, console_port);

      if (transport == null)
        return;

      console_client = new ConsoleClient (server.host, console_port, console_password, proto, transport);
```

- [x] **Step 5: Fix** `about.vala`

In `format_debug_info`, delete the line printing `transport.class_type.name ()` (keep the id line).

- [x] **Step 6: Compile and run all tests**

Run: `meson compile -C build && meson test -C build --print-errorlogs`
Expected: PASS.

- [x] **Step 7: STOP — user reviews, tests (run the app: querying over UDP/TCP and the remote console must still work), and commits**

Suggested message: `refactor: transport factories with explicit dependencies`

---



### Task 12: Composition root — remove all singletons

**Files:**

- Modify: `src/core/querier-manager.vala`, `src/core/protocol/protocol-registry.vala`, `src/core/transport/transport-registry.vala`, `src/core/transport/udp-transport-manager.vala`, `src/core/geo-location-resolver/maxmind-location-resolver.vala`
- Modify: `src/application.vala`
- Modify: `src/ui/main-window.vala`, `src/ui/main-window.ui`, `src/ui/server-list.vala`, `src/ui/server-info.vala`
- Test: `src/core/server-list_test.vala`, `tests/test-server-watcher.vala` (constructor updates)

**Interfaces:**

- Produces: `QuerierManager (ProtocolRegistry protocol_registry, TransportRegistry transport_registry, GameDb game_db)` with those three as `{ get; construct; }`; plain `new ProtocolRegistry ()`, `new TransportRegistry ()`, `new UdpTransportManager ()`, `new MaxMindLocationResolver ()`; `MainWindow (Gtk.Application application, Gsw.ServerList client_list, Gsw.BuddyList buddy_list, GeoLocationResolver location_resolver)`; `location_resolver` settable property (`GeoLocationResolver?`) on `Ui.ServerList` and `Ui.ServerInfo`.

- [x] **Step 1: De-singleton the services**

In each of `QuerierManager`, `ProtocolRegistry`, `TransportRegistry`, `UdpTransportManager`, `MaxMindLocationResolver`: remove the `[SingleInstance]` attribute and the `public static X get_instance ()` method.

`QuerierManager` properties become construct-only with a real constructor:

```vala
  public GameDb game_db { get; construct; }
  public ProtocolRegistry protocol_registry { get; construct; }
  public TransportRegistry transport_registry { get; construct; }

  public QuerierManager (ProtocolRegistry protocol_registry, TransportRegistry transport_registry, GameDb game_db) {
    Object (
      protocol_registry : protocol_registry,
      transport_registry : transport_registry,
      game_db : game_db
    );
  }
```

- [x] **Step 2: Rebuild** `Application`**'s field block and startup order**

```vala
  private ProtocolRegistry protocol_registry = new ProtocolRegistry ();
  private TransportRegistry transport_registry = new TransportRegistry ();
  private UdpTransportManager udp_transport_manager = new UdpTransportManager ();
  private GameDb game_db = new GameDb ();
  private QuerierManager querier_manager;
  private MaxMindLocationResolver location_resolver;
```

In `startup ()`, after the transport registrations replace the Task 9 block with:

```vala
    // games
    game_db.load.begin ();

    // queriers
    querier_manager = new QuerierManager (protocol_registry, transport_registry, game_db);

    // geoip
    location_resolver = new MaxMindLocationResolver ();
```

(The `preferences.bind (..., querier_manager, ...)` line later in `startup` now refers to this instance; order is already correct.)

In `activate ()`, replace the `var location_resolver = new MaxMindLocationResolver ();` line — use the field instead (the `init_error` dialog block stays), and pass it to the window:

```vala
      main_window = new Ui.MainWindow (this, server_list, buddy_list, location_resolver);
```

- [x] **Step 3: Thread the resolver through the UI**

`src/ui/main-window.ui`: give the server info widget an id — `<object class="GswUiServerInfo">` (line ~220) → `<object class="GswUiServerInfo" id="server_info_ctl">`.

`src/ui/main-window.vala`:

```vala
  public GeoLocationResolver location_resolver { get; construct; }

  [GtkChild]
  private unowned Ui.ServerList server_list_ctl;
  [GtkChild]
  private unowned Ui.ServerInfo server_info_ctl;
```

extend `construct`:

```vala
    server_list_ctl.location_resolver = location_resolver;
    server_info_ctl.location_resolver = location_resolver;
```

and the constructor:

```vala
  public MainWindow (Gtk.Application application, Gsw.ServerList client_list, Gsw.BuddyList buddy_list, GeoLocationResolver location_resolver) {
    Object (
      application : application,
      client_list : client_list,
      buddy_list : buddy_list,
      location_resolver : location_resolver
    );
  }
```

`src/ui/server-list.vala` and `src/ui/server-info.vala`: add to each widget class

```vala
  public GeoLocationResolver? location_resolver { get; set; }
```

and update the callbacks:

```vala
  // ui/server-list.vala
  private string? format_country_icon (string? ip_address) {
    if (ip_address == null || location_resolver == null)
      return null;

    var country_code = location_resolver.country_code_by_ip (ip_address).ascii_down ();
```

```vala
  // ui/server-info.vala
  private string? format_location (string? ip_address) {
    if (ip_address == null || location_resolver == null)
      return null;

    return location_resolver.location_by_ip (ip_address);
  }
```

(Check `GeoLocationResolver` in `src/core/geo-location-resolver/geo-location-resolver.vala` declares both methods; if `country_code_by_ip`/`location_by_ip` are MaxMind-specific, type the properties as `MaxMindLocationResolver?` instead.)

- [x] **Step 4: Update the test helper**

In both test files, the helper from Task 10 becomes:

```vala
ServerList make_server_list () {
  var protocol_registry = new ProtocolRegistry ();
  var transport_registry = new TransportRegistry ();
  var game_db = new GameDb ();
  game_db.finish_loading ();

  var querier_manager = new QuerierManager (protocol_registry, transport_registry, game_db);

  return new ServerList (querier_manager, protocol_registry, transport_registry, game_db);
}
```

- [x] **Step 5: Sweep for leftovers**

Run: `rg -n "get_instance|SingleInstance" src tests`
Expected: no matches. Fix any stragglers the search finds.

- [x] **Step 6: Compile and run all tests**

Run: `meson compile -C build && meson test -C build --print-errorlogs`
Expected: PASS.

- [x] **Step 7: STOP — user reviews, tests the app end to end (server list, flags/locations, game icons, console, notifications, preferences bindings for query interval and UDP port), and commits**

Suggested message: `refactor: explicit composition root, no singletons`

---



### Task 13: Ship generic definitions

**Files:**

- Create: `data/games/generic-source.ini`
- Create: `data/games/generic-quake3.ini`

`data/meson.build` already installs the whole `games/` subdir — no build change needed.

- [ ] **Step 1: Create** `data/games/generic-source.ini`

```ini
[Game]
id = generic-source
protocol = source
port = 27015

[Features]
console = source-console

[Info]
game-name = inf["game"]

[Extra]
```

- [ ] **Step 2: Create** `data/games/generic-quake3.ini`

```ini
[Game]
id = generic-quake3
protocol = quake3
port = 27960

[Info]
game-name = inf["gamename"]
server-name = toMarkup(inf["sv_hostname"], "quake-color-code")

[Extra]
```

**Verify the** `inf` **field names against your protocol implementations** (`data/protocols/*.lua`) — these two files are drafts based on the documented examples. Add generics for further protocols (gamespy, etc.) the same way as you validate them against real servers.

- [ ] **Step 3: Manual test**

Run the app against a known Source server whose game has no specific definition — it should now appear with `game-id = generic-source` and a name from `inf["game"]`. A server *with* a specific definition (e.g. CS2) must be unaffected except for any `[Info]` keys only the generic defines.

- [ ] **Step 4: STOP — user reviews, tests, and commits**

Suggested message: `feat: generic game definitions for source and quake3`

---



## Deferred (spec: future work)

- Audit test loading every shipped `data/games/*.ini` (parseability + no specificity ties).
- Recorded server-response fixtures asserting end-to-end resolution.

