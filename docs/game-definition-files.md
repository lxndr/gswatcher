# Game Definition Files

## Overview

Game definition files are INI-formatted configuration files that define how Game Server Watcher recognizes and handles different games. These files contain information about game protocols, server matching rules, and data transformation expressions.

## Location

Game definition files are loaded from both user-specific and system data directories. The application searches for `.ini` files in the following locations in order:

1. **User-specific directory**: `~/.local/share/gswatcher/games/`
2. **System directories**: `/usr/local/share/gswatcher/games/`, `/usr/share/gswatcher/games/` (or other system data directories)
3. **Override directory**: If the `GSW_GAMES_DIR` environment variable is set, that directory is searched first

The application will load all `.ini` files found in these directories, with user-specific files taking precedence over system files in case of id conflicts.

## File Structure

Game definition files use standard INI format with the following sections:

### [Game] Section

The `[Game]` section contains basic game identification information:

```ini
[Game]
id = cs
protocol = source
port = 27015
qport-diff = 1
```

- `id`: A unique identifier for the game (used internally)
- `protocol`: The protocol used to query the game server (e.g., `source`, `quake3`, `gamespy4`)
- `port`: The default port number for the game server
- `qport-diff`: Optional port offset for query port (if different from game port)

### [Features] Section

The `[Features]` section defines additional features available for the game:

```ini
[Features]
console = source-console
```

- `console`: Specifies the console protocol to use for server administration

### [Match] Section

The `[Match]` section defines rules for identifying game servers:

```ini
[Match]
inf.appid = 730
inf.game = "Counter-Strike 2"
```

These rules match against server response data to determine if a server belongs to this game. The `inf` prefix refers to server information fields returned by the query protocol.

### [Info] Section

The `[Info]` section defines how to transform and display server information:

```ini
[Info]
game-name = "Counter-Strike 2"
game-mode = mapKeyword(inf["keywords"], ",", "Modes", "")
```

Values in this section can be literal strings or expressions that transform server data.

### [Player] Section

The `[Player]` section defines how player list fields are displayed:

```ini
[Player]
Name = toMarkup(2, player)|string|main
Score = score|number
Ping = ping|number
```

Format: `Display Name = field_name|field_type|optional_main_indicator`

The Player section supports expressions, allowing for dynamic data transformation similar to the `[Info]` section. This enables complex player data processing and formatting.

#### Field Types

- `string`: Text field
- `number`: Numeric field
- `duration`: Duration field (time duration in seconds, automatically formatted as HH:MM:SS)

#### Main Indicator

The third field (optional) indicates if this is the main player field for display purposes. Only one player field should be marked as `main`.

### [Extra] Section

The `[Extra]` section contains additional metadata:

```ini
[Extra]
release-year = 2023
icon-source = https://example.com/icon.png
```

### Custom Mapping Sections

Additional sections can be defined to create lookup tables for use with expressions:

```ini
[Modes]
competitive = Competitive
```

## Expression Usage

Game definition files support expressions in the `[Info]` section and certain other contexts. These expressions allow for dynamic data transformation.

### Value Access

Access server information fields using the `inf` object:

```
inf["field_name"]
```

For example:
```
game-mode = inf["g_gametype"]
```

### Map Lookup Expression

Use the `Modes[inf["g_gametype"]]` syntax to look up values in custom sections:

```
game-mode = Modes[inf["g_gametype"]]
```

This looks up the value of `inf["g_gametype"]` in the `[Modes]` section and returns the corresponding mapped value.

### Function Expressions

#### regex() Function

The `regex()` function extracts information using regular expressions:

```
game-mode = regex("^L4D - ([a-zA-z-]+)", inf["game"])
```

Syntax: `regex(pattern, input)`
- `pattern`: Regular expression with capturing groups
- `input`: String to apply the regex to
- Returns the content of the first capturing group, or empty string if no match

#### mapKeyword() Function

The `mapKeyword()` function maps keywords to display values:

```
game-mode = mapKeyword(inf["keywords"], ",", "Modes", "")
```

Syntax: `mapKeyword(keywords, delimiter, map_section, default)`
- `keywords`: String containing keywords to check
- `delimiter`: Character used to separate keywords
- `map_section`: Section name to look up values in
- `default`: Default value if no match is found

### Or Expression (used only for matching)

Multiple values can be matched using the `or` operator:

```
inf.game = "game1" or "game2" or "game3"
```

This allows matching if the server's game field equals any of the specified values.

## Conflict Resolution

When multiple game definitions could potentially match the same server, Game Server Watcher uses a conflict resolution mechanism:

### Matching Process

1. **Protocol Matching**: First, the system filters games that use the same protocol as the server response
2. **Rule Matching**: For each game definition, all `[Match]` rules are evaluated against the server's response data
3. **Weight Calculation**: Each game definition is assigned a weight based on the complexity and length of its matching rules
4. **Priority Selection**: The game with the highest weight is selected

### Weight System

The weight of a game definition is calculated based on the length of expression strings in `[Match]` rules.

## Examples

### Simple Game Definition (Counter-Strike)

```ini
[Game]
id = cs
protocol = source
port = 27015

[Features]
console = source-console

[Match]
inf.dir = "cstrike"

[Info]
game-name = "Counter-Strike"
```

### Complex Game Definition (OpenArena with Mapping)

```ini
[Game]
id = openarena
protocol = quake3
port = 27960

[Match]
inf.gamename = "baseoa"

[Info]
game-name = "OpenArena"
game-mode = Modes[inf["g_gametype"]]

[Modes]
0 = Deathmatch
1 = Tournament
2 = Single Player Deathmatch
3 = Team Deathmatch
4 = Capture The Flag
5 = One Flag Capture
6 = Overload
7 = Harvester
8 = Elimination
9 = CTF Elimination
10 = Last Man Standing
11 = Double Domination
12 = Domination
```

### Game with Regex Expression (Left 4 Dead)

```ini
[Game]
id = l4d
protocol = source
port = 27015

[Features]
console = source-console

[Match]
inf.appid = 500

[Info]
game-name = "Left 4 Dead"
game-mode = regex("^L4D - ([a-zA-z-]+)", inf["game"])
```

### Game with Keyword Mapping (Counter-Strike 2)

```ini
[Game]
id = cs2
protocol = source
port = 27015

[Features]
console = source-console

[Match]
inf.appid = 730
inf.game = "Counter-Strike 2"

[Info]
game-name = "Counter-Strike 2"
game-mode = mapKeyword(inf["keywords"], ",", "Modes", "")

[Modes]
competitive = Competitive
```

### Game with Advanced Player Section (Call of Duty)

```ini
[Game]
id = cod
protocol = quake3
port = 28960

[Match]
inf.gamename = "main"

[Info]
game-name = "Call of Duty"
game-mode = inf["g_gametype"]
server-name = toMarkup(inf["g_gametype"], "quake-color-code")

[Player]
Name = toMarkup(2, "quake-color-code")|string|main
Score = 0|number
Ping = 1|number

[Extra]
release-year = 2003
```

This example shows:
1. Updated Player section format using pipe separators
2. Expression support in Player section (`toMarkup()` function)
3. Main field indicator (`main`)
4. Different field types (`string`, `number`)
