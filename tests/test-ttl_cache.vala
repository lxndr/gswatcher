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
        var cache = new TtlCache<string, string> (5 * TimeSpan.SECOND);

        // Test initial state
        assert (cache.size == 0);

        // Test setting and getting values
        cache.set ("key1", "value1");
        assert (cache.size == 1);
        assert (cache.get ("key1") == "value1");

        cache.set ("key2", "value2");
        assert (cache.size == 2);
        assert (cache.get ("key2") == "value2");

        // Test updating existing key
        cache.set ("key1", "updated_value1");
        assert (cache.get ("key1") == "updated_value1");
        assert (cache.size == 2);

        // Test getting non-existent key
        assert (cache.get ("nonexistent") == null);

        // Test has method
        assert (cache.has ("key1"));
        assert (cache.has ("key2"));
        assert (!cache.has ("nonexistent"));
    });

    Test.add_func ("/core/utils/ttl_cache/custom_ttl", () => {
        var cache = new TtlCache<string, string> (5 * TimeSpan.SECOND);

        // Test setting with custom TTL
        cache.set_with_ttl ("key1", "value1", TimeSpan.SECOND);
        cache.set_with_ttl ("key2", "value2", 10 * TimeSpan.SECOND);
        assert (cache.get ("key1") == "value1");
        assert (cache.get ("key2") == "value2");

        // Sleep for 1.5 seconds to let key1 expire but not key2
        Thread.usleep ((ulong) (1500 * TimeSpan.MILLISECOND));

        // key1 should be expired, key2 should still be valid
        assert (cache.get ("key1") == null);
        assert (cache.get ("key2") == "value2");
        assert (cache.size == 1);
    });

    Test.add_func ("/core/utils/ttl_cache/expiration", () => {
        var cache = new TtlCache<string, string> (TimeSpan.SECOND);

        cache.set ("key1", "value1");
        cache.set ("key2", "value2");

        assert (cache.get ("key1") == "value1");
        assert (cache.get ("key2") == "value2");
        assert (cache.size == 2);

        // Sleep for 1.5 seconds to let entries expire
        Thread.usleep ((ulong) (1500 * TimeSpan.MILLISECOND));

        // Both entries should be expired
        assert (cache.get ("key1") == null);
        assert (cache.get ("key2") == null);
        assert (cache.size == 0);

        // Add entries again and test has method after expiration
        cache.set ("key3", "value3");
        assert (cache.has ("key3"));
        Thread.usleep ((ulong) (1500 * TimeSpan.MILLISECOND));
        assert (!cache.has ("key3"));
    });

    Test.add_func ("/core/utils/ttl_cache/unset", () => {
        var cache = new TtlCache<string, string> (5 * TimeSpan.SECOND);

        cache.set ("key1", "value1");
        cache.set ("key2", "value2");

        assert (cache.size == 2);
        assert (cache.has ("key1"));
        assert (cache.has ("key2"));

        // Test unset returns true for existing key
        bool result = cache.unset ("key1");
        assert (result == true);
        assert (cache.size == 1);
        assert (!cache.has ("key1"));
        assert (cache.get ("key1") == null);

        // Test unset returns false for non-existent key
        result = cache.unset ("nonexistent");
        assert (result == false);
        assert (cache.size == 1);

        // Test that unset key cannot be retrieved
        assert (cache.get ("key1") == null);
    });

    Test.add_func ("/core/utils/ttl_cache/clear", () => {
        var cache = new TtlCache<string, string> (5 * TimeSpan.SECOND);

        cache.set ("key1", "value1");
        cache.set ("key2", "value2");
        cache.set ("key3", "value3");

        assert (cache.size == 3);
        assert (cache.has ("key1"));
        assert (cache.has ("key2"));
        assert (cache.has ("key3"));

        cache.clear ();
        assert (cache.size == 0);
        assert (!cache.has ("key1"));
        assert (!cache.has ("key2"));
        assert (!cache.has ("key3"));
        assert (cache.get ("key1") == null);
        assert (cache.get ("key2") == null);
        assert (cache.get ("key3") == null);
    });

    Test.add_func ("/core/utils/ttl_cache/default_ttl_validation", () => {
        // Test that default TTL is clamped to minimum of 1000ms
        var cache = new TtlCache<string, string> (500 * TimeSpan.MILLISECOND); // Below minimum
        assert (cache.default_ttl >= TimeSpan.SECOND);

        // Test with a very small TTL (should be clamped)
        var cache2 = new TtlCache<string, string> (100 * TimeSpan.MILLISECOND);
        assert (cache2.default_ttl >= TimeSpan.SECOND);

        // Test with a reasonable TTL
        var cache3 = new TtlCache<string, string> (3 * TimeSpan.SECOND);
        assert (cache3.default_ttl == 3 * TimeSpan.SECOND);

        // Test with a very large TTL
        var cache4 = new TtlCache<string, string> (1000 * TimeSpan.DAY);
        assert (cache4.default_ttl == 1000 * TimeSpan.DAY);
    });

    Test.add_func ("/core/utils/ttl_cache/set_with_ttl_validation", () => {
        var cache = new TtlCache<string, string> (5 * TimeSpan.SECOND);

        // Test that TTL is clamped to minimum of 1000ms when using set_with_ttl
        cache.set_with_ttl ("key1", "value1", 500 * TimeSpan.MILLISECOND); // Below minimum
        assert (cache.get ("key1") == "value1"); // Should still work but with at least 1000ms TTL

        // Sleep for 0.5 seconds to ensure it's less than 1000ms minimum
        Thread.usleep ((ulong) (500 * TimeSpan.MILLISECOND));
        assert (cache.get ("key1") == "value1"); // Should still be there
    });

    Test.add_func ("/core/utils/ttl_cache/generic_types", () => {
        // Test with different generic types
        var string_cache = new TtlCache<string, string> (5 * TimeSpan.SECOND);
        string_cache.set ("str_key", "str_value");
        assert (string_cache.get ("str_key") == "str_value");

        var int_cache = new TtlCache<int, int> (5 * TimeSpan.SECOND);
        int_cache.set (123, 456);
        assert (int_cache.get (123) == 456);

        var mixed_cache = new TtlCache<string, int> (5 * TimeSpan.SECOND);
        mixed_cache.set ("num_key", 789);
        assert (mixed_cache.get ("num_key") == 789);
    });

    Test.add_func ("/core/utils/ttl_cache/size_property", () => {
        var cache = new TtlCache<string, string> (TimeSpan.SECOND);

        assert (cache.size == 0);

        cache.set ("key1", "value1");
        assert (cache.size == 1);

        cache.set ("key2", "value2");
        assert (cache.size == 2);

        // Add a third key and then expire one
        cache.set ("key3", "value3");
        assert (cache.size == 3);

        // Sleep to expire all entries
        Thread.usleep ((ulong) (1500 * TimeSpan.MILLISECOND));
        assert (cache.size == 0); // All should be expired due to cleanup in size property

        // Add keys again
        cache.set ("key4", "value4");
        assert (cache.size == 1);
    });

    Test.add_func ("/core/utils/ttl_cache/ttl_cache_cleanup", () => {
        var cache = new TtlCache<string, string> (TimeSpan.SECOND);

        cache.set ("key1", "value1");
        cache.set ("key2", "value2");
        cache.set ("key3", "value3");

        assert (cache.size == 3);

        // Sleep to expire all entries
        Thread.usleep ((ulong) (1500 * TimeSpan.MILLISECOND));

        // Accessing size should trigger cleanup of expired entries
        assert (cache.size == 0);

        // Add new entries
        cache.set ("key4", "value4");
        cache.set ("key5", "value5");
        assert (cache.size == 2);
        assert (cache.get ("key4") == "value4");
        assert (cache.get ("key5") == "value5");
    });

    Test.run ();

    return 0;
}
