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

using Gee;

namespace Gsw {

/**
 * A generic TTL (Time To Live) cache that stores key-value pairs and automatically
 * removes entries after a specified time period.
 */
public class TtlCache<K, V> {
  private struct Entry<V> {
    public V value;
    public int64 expiry_time;
  }

  private HashMap<K, Entry?> _cache = new HashMap<K, Entry?> ();
  private int64 _default_ttl;
  private uint _cleanup_interval_id;

  /**
   * Creates a new TTL cache with the specified default time-to-live.
   *
   * @param default_ttl Default time-to-live in milliseconds for entries
   */
  public TtlCache (int64 default_ttl = 60000) {
    _default_ttl = default_ttl.clamp (1000, int64.MAX);

    start_periodic_cleanup ();
  }

  /**
   * Disposes the TTL cache and stops the periodic cleanup timer.
   */
  ~TtlCache () {
    stop_periodic_cleanup ();
  }

  /**
   * Sets a key-value pair with the default TTL.
   *
   * @param key The key to store the value under
   * @param value The value to store
   */
  public void set (K key, V value) {
    set_with_ttl (key, value, _default_ttl);
  }

  /**
   * Sets a key-value pair with a custom TTL.
   *
   * @param key The key to store the value under
   * @param value The value to store
   * @param ttl Time to live in milliseconds
   */
  public void set_with_ttl (K key, V value, int64 ttl) {
    int64 expiry_time = get_current_time_ms () + ttl.clamp (1000, int64.MAX);
    _cache.set (key, { value, expiry_time });
  }

  /**
   * Gets the value for a key, or null if the key doesn't exist or has expired.
   *
   * @param key The key to look up
   * @return The value if it exists and hasn't expired, null otherwise
   */
  public V? get(K key) {
    Entry<V>? entry = _cache.get (key);

    if (entry == null) {
      return null;
    }

    if (is_expired (entry)) {
      _cache.unset (key);
      return null;
    }

    return entry.value;
  }

  /**
   * Checks if a key exists and hasn't expired.
   *
   * @param key The key to check
   * @return true if the key exists and hasn't expired, false otherwise
   */
  public bool has (K key) {
    Entry? entry = _cache.get (key);

    if (entry == null) {
      return false;
    }
    
    return !is_expired (entry);
  }

  /**
   * Removes a key from the cache.
   *
   * @param key The key to remove
   * @return true if the key was removed, false if it didn't exist
   */
  public bool remove (K key) {
    return _cache.unset (key);
  }

  /**
   * Clears all entries from the cache.
   */
  public void clear () {
    _cache.clear ();
  }

  /**
   * Returns the number of entries in the cache (including expired ones that haven't been cleaned up yet).
   *
   * @return The number of entries in the cache
   */
  public int size () {
    return _cache.size;
  }

  /**
   * Sets the default TTL for this cache.
   *
   * @param ttl The new default TTL in milliseconds
   */
  public void set_default_ttl (int64 ttl) {
    _default_ttl = ttl;
  }

  /**
   * Starts the periodic cleanup timer.
   */
  private void start_periodic_cleanup () {
    if (_cleanup_interval_id == 0) {
      _cleanup_interval_id = GLib.Timeout.add((uint) _default_ttl, () => {
        cleanup_expired ();
        return Source.CONTINUE;
      });
    }
  }

  /**
   * Stops the periodic cleanup timer.
   */
  private void stop_periodic_cleanup () {
    if (_cleanup_interval_id != 0) {
      GLib.Source.remove (_cleanup_interval_id);
      _cleanup_interval_id = 0;
    }
  }

  /**
   * Removes all expired entries from the cache.
   */
  public void cleanup_expired () {
    var iter = _cache.map_iterator ();

    while (iter.next ()) {
      if (is_expired (iter.get_value ())) {
        iter.unset ();
      }
    }
  }

  /**
   * Checks if an entry has expired.
   *
   * @param entry The entry to check
   * @return true if the entry has expired, false otherwise
   */
  private bool is_expired (Entry entry) {
    return get_current_time_ms () > entry.expiry_time;
  }

  /**
   * Gets the current time in milliseconds since the Unix epoch.
   *
   * @return Current time in milliseconds
   */
  private int64 get_current_time_ms () {
    return GLib.get_real_time () / 1000;
  }
}

}
