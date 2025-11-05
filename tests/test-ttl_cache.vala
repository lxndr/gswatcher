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

using Gsw;

int main (string[] args) {
  Test.init (ref args);

  Test.add_func ("/core/utils/ttl_cache/basic_functionality", () => {
    var cache = new TtlCache<string, string> (1000);

    // Test initial state
    assert (cache.size () == 0);
    assert (cache.get ("nonexistent") == null);
    assert (!cache.has ("nonexistent"));

    // Test adding elements
    cache.set ("key1", "value1");
    cache.set ("key2", "value2");

    assert (cache.size () == 2);
    assert (cache.get ("key1") == "value1");
    assert (cache.get ("key2") == "value2");
    assert (cache.has ("key1"));
    assert (cache.has ("key2"));

    // Test updating existing key
    cache.set ("key1", "updated_value1");
    assert (cache.get ("key1") == "updated_value1");
    assert (cache.size () == 2);

    // Test remove
    bool removed = cache.remove ("key2");
    assert (removed == true);
    assert (cache.get ("key2") == null);
    assert (!cache.has ("key2"));
    assert (cache.size () == 1);

    // Test remove non-existent
    removed = cache.remove ("nonexistent");
    assert (removed == false);
    assert (cache.size () == 1);

    // Test clear
    cache.set ("key2", "value2");
    cache.clear ();
    assert (cache.size () == 0);
    assert (cache.get ("key1") == null);
    assert (cache.get ("key2") == null);
    assert (!cache.has ("key1"));
    assert (!cache.has ("key2"));
  });

  Test.add_func ("/core/utils/ttl_cache/custom_ttl", () => {
    var cache = new TtlCache<string, string> (1000);

    // Test setting with custom TTL
    cache.set_with_ttl ("short_key", "short_value", 100);
    cache.set_with_ttl ("long_key", "long_value", 2000);

    assert (cache.get ("short_key") == "short_value");
    assert (cache.get ("long_key") == "long_value");
    assert (cache.size () == 2);

    // Test default TTL setting
    cache.set ("default_key", "default_value");
    assert (cache.get ("default_key") == "default_value");
    assert (cache.size () == 3);
  });

  Test.add_func ("/core/utils/ttl_cache/expiry", () => {
    var cache = new TtlCache<string, string> (1000);

    // Set values with very short TTLs
    cache.set_with_ttl ("key1", "value1", 50);
    cache.set_with_ttl ("key2", "value2", 200);

    // Initially both should be available
    assert (cache.get ("key1") == "value1");
    assert (cache.get ("key2") == "value2");
    assert (cache.has ("key1"));
    assert (cache.has ("key2"));

    // Wait for first key to expire
    GLib.Thread.usleep (60000); // 60ms

    // First key should be expired, second should still be available
    assert (cache.get ("key1") == null);
    assert (!cache.has ("key1"));
    assert (cache.get ("key2") == "value2");
    assert (cache.has ("key2"));

    // Wait for second key to expire
    GLib.Thread.usleep (250000); // 250ms

    // Both keys should be expired
    assert (cache.get ("key1") == null);
    assert (!cache.has ("key1"));
    assert (cache.get ("key2") == null);
    assert (!cache.has ("key2"));
  });

  Test.add_func ("/core/utils/ttl_cache/cleanup_expired", () => {
    var cache = new TtlCache<string, string> (1000);

    // Add expired and non-expired entries
    cache.set_with_ttl ("expired_key", "expired_value", 50);
    cache.set ("valid_key", "valid_value");

    // Wait for first key to expire
    GLib.Thread.usleep (60000); // 60ms

    // Both keys should still be in cache (lazy cleanup)
    assert (cache.size () == 2);
    assert (cache.get ("expired_key") == null);
    assert (!cache.has ("expired_key"));
    assert (cache.get ("valid_key") == "valid_value");
    assert (cache.has ("valid_key"));

    // Explicit cleanup should remove expired entries
    cache.cleanup_expired ();
    assert (cache.size () == 1);
    assert (cache.get ("expired_key") == null);
    assert (!cache.has ("expired_key"));
    assert (cache.get ("valid_key") == "valid_value");
    assert (cache.has ("valid_key"));
  });

  Test.add_func ("/core/utils/ttl_cache/default_ttl_setting", () => {
    var cache = new TtlCache<string, string> (1000);

    // Set default TTL
    cache.set_default_ttl (500);

    // Add entries with default TTL
    cache.set ("key1", "value1");
    cache.set ("key2", "value2");

    // Both should be available initially
    assert (cache.get ("key1") == "value1");
    assert (cache.get ("key2") == "value2");

    // Wait for entries to expire
    GLib.Thread.usleep (600000); // 600ms

    // Both should be expired
    assert (cache.get ("key1") == null);
    assert (!cache.has ("key1"));
    assert (cache.get ("key2") == null);
    assert (!cache.has ("key2"));
  });

  Test.add_func ("/core/utils/ttl_cache/min_ttl_validation", () => {
    var cache = new TtlCache<string, string> (500);

    // Test that TTL is clamped to minimum (1000ms)
    cache.set ("key", "value");

    // Should be available for at least 1000ms
    assert (cache.get ("key") == "value");
    assert (cache.has ("key"));

    // Wait 800ms (less than minimum TTL)
    GLib.Thread.usleep (800000); // 800ms

    // Should still be available
    assert (cache.get ("key") == "value");
    assert (cache.has ("key"));

    // Wait additional 300ms (total 1100ms, more than minimum TTL)
    GLib.Thread.usleep (300000); // 300ms

    // Should now be expired
    assert (cache.get ("key") == null);
    assert (!cache.has ("key"));
  });

  Test.add_func ("/core/utils/ttl_cache/generic_types", () => {
    // Test with string keys and int values
    var cache1 = new TtlCache<string, int> (1000);
    cache1.set ("count", 42);
    assert (cache1.get ("count") == 42);

    // Test with int keys and string values
    var cache2 = new TtlCache<int, string> (1000);
    cache2.set (123, "hello");
    assert (cache2.get (123) == "hello");

    // Test with custom object keys
    var cache3 = new TtlCache<TestKey, string> (1000);
    var key1 = new TestKey ("test1");
    var key2 = new TestKey ("test2");
    cache3.set (key1, "value1");
    cache3.set (key2, "value2");
    assert (cache3.get (key1) == "value1");
    assert (cache3.get (key2) == "value2");
  });

  Test.add_func ("/core/utils/ttl_cache/edge_cases", () => {
    var cache = new TtlCache<string, string> (1000);

    // Test null values
    cache.set ("null_key", null);
    assert (cache.get ("null_key") == null);
    assert (cache.has ("null_key"));

    // Test empty string values
    cache.set ("empty_key", "");
    assert (cache.get ("empty_key") == "");
    assert (cache.has ("empty_key"));

    // Test zero TTL (should be clamped to minimum)
    cache.set_with_ttl ("zero_ttl_key", "value", 0);
    assert (cache.get ("zero_ttl_key") == "value");
    assert (cache.has ("zero_ttl_key"));

    // Test negative TTL (should be clamped to minimum)
    cache.set_with_ttl ("negative_ttl_key", "value", -100);
    assert (cache.get ("negative_ttl_key") == "value");
    assert (cache.has ("negative_ttl_key"));
  });

  Test.add_func ("/core/utils/ttl_cache/overwrite_behavior", () => {
    var cache = new TtlCache<string, string> (1000);

    // Set initial value
    cache.set ("key", "original_value");
    assert (cache.get ("key") == "original_value");

    // Overwrite with same TTL
    cache.set ("key", "updated_value");
    assert (cache.get ("key") == "updated_value");

    // Overwrite with custom TTL
    cache.set_with_ttl ("key", "custom_ttl_value", 2000);
    assert (cache.get ("key") == "custom_ttl_value");

    // Wait and verify custom TTL is respected
    GLib.Thread.usleep (1500000); // 1500ms
    assert (cache.get ("key") == "custom_ttl_value");

    // Wait for it to expire
    GLib.Thread.usleep (600000); // 600ms
    assert (cache.get ("key") == null);
    assert (!cache.has ("key"));
  });
  
  Test.run ();
  
  return 0;
}

// Helper class for testing generic types
class TestKey {
  public string value { get; private set; }
  
  public TestKey (string value) {
      this.value = value;
  }
  
  public bool equal_to (TestKey other) {
      return this.value == other.value;
  }
  
  public uint hash () {
      return str_hash (value);
  }
}