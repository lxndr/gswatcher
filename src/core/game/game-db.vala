/*
  Game Server Watcher
  Copyright (C) 2010-2026 Alexander Burobin

  This program is free software: you can redistribute it and/or modify
  it under the terms of the GNU Affero General Public License as
  published by the Free Software Foundation, either version 3 of the
  License, or (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU Affero General Public License for more details.

  You should have received a copy of the GNU Affero General Public License
  along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

namespace Gsw {

const int64 GAME_DB_CACHE_TTL = 30 * TimeSpan.MINUTE;

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
  // Non-owning cache of the specific match; Games stay alive via `games`.
  // Do not cache ephemeral wrappers here — TtlCache does not ref values.
  private TtlCache<ServerDetails, Game> cache = new TtlCache<ServerDetails, Game> (GAME_DB_CACHE_TTL);
  private bool loaded;
  private Gee.List<Waiter> waiters = new Gee.ArrayList<Waiter> ();

  /**
   * Adds a game definition. A second generic definition for the same
   * protocol is rejected with a warning; the first one wins.
   */
  public void add_game (Game game) {
    if (game.is_generic) {
      if (generics.has_key (game.protocol)) {
        log (Config.LOG_DOMAIN, LEVEL_WARNING,
             "duplicate generic game definition '%s' for protocol '%s' ('%s' is already registered)",
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

          add_game (GameDef.load_from_bytes (new Bytes.static (ini_file.data)));
        } catch (Error err) {
          log (Config.LOG_DOMAIN, LEVEL_WARNING,
               "failed to load game definition from '%s': %s",
               ini_file.file.get_path (), err.message);
        }
      }
    } catch (Error err) {
      log (Config.LOG_DOMAIN, LEVEL_WARNING, "failed to load game definitions: %s", err.message);
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

  /**
   * Identifies the game for the given server details and applies its
   * information to sinfo (generic layer first, then the specific game).
   * Returns false when nothing could be resolved.
   */
  public virtual bool resolve (string protocol_id, ServerInfo sinfo, ServerDetails details) {
    var generic = generics[protocol_id];
    var specific = cache.get (details);

    if (specific == null) {
      specific = find_best_match (protocol_id, details);

      if (generic == null && specific == null)
        return false;

      if (specific != null)
        cache.set (details, specific);
    }

    if (generic != null)
      generic.enhance_sinfo (sinfo, details);

    if (specific != null)
      specific.enhance_sinfo (sinfo, details);

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
}

}
