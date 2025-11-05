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
    
    Test.add_func ("/core/utils/hashable_string_map/basic_functionality", () => {
        var map = new HashableStringMap ();
        
        // Test initial state
        assert (map.size == 0);
        assert (map.is_empty);
        
        // Test adding elements
        map.set ("key1", "value1");
        map.set ("key2", "value2");
        
        assert (map.size == 2);
        assert (!map.is_empty);
        assert (map.get ("key1") == "value1");
        assert (map.get ("key2") == "value2");
        
        // Test updating existing key
        map.set ("key1", "updated_value1");
        assert (map.get ("key1") == "updated_value1");
        
        // Test unset
        string value;
        bool result = map.unset ("key2", out value);
        assert (result == true);
        assert (value == "value2");
        assert (map.size == 1);
        assert (!map.has_key ("key2"));
        
        // Test clear
        map.set ("key2", "value2");
        map.clear ();
        assert (map.size == 0);
        assert (map.is_empty);
    });
    
    Test.add_func ("/core/utils/hashable_string_map/equality", () => {
        var map1 = new HashableStringMap ();
        var map2 = new HashableStringMap ();
        
        // Test equality of empty maps
        assert (map1.equal_to (map2));
        assert (map2.equal_to (map1));
        
        // Test with same content
        map1.set ("key1", "value1");
        map1.set ("key2", "value2");
        
        map2.set ("key1", "value1");
        map2.set ("key2", "value2");
        
        assert (map1.equal_to (map2));
        assert (map2.equal_to (map1));
        
        // Test with different content
        map2.set ("key3", "value3");
        assert (!map1.equal_to (map2));
        assert (!map2.equal_to (map1));
        
        // Test with same keys but different values
        map2.unset ("key3");
        map2.set ("key2", "different_value");
        assert (!map1.equal_to (map2));
        assert (!map2.equal_to (map1));
        
        // Test with different sizes
        map2.unset ("key2");
        assert (!map1.equal_to (map2));
        assert (!map2.equal_to (map1));        
    });
    
    Test.add_func ("/core/utils/hashable_string_map/hash_consistency", () => {
        var map = new HashableStringMap ();
        
        // Test that hash is consistent when map hasn't changed
        map.set ("key1", "value1");
        map.set ("key2", "value2");
        
        uint hash1 = map.hash ();
        uint hash2 = map.hash ();
        
        assert (hash1 == hash2);
        
        // Test that hash changes when content changes
        map.set ("key1", "new_value1");
        uint hash3 = map.hash ();
        
        assert (hash1 != hash3);
        
        // Test that same content produces same hash regardless of insertion order
        var map_a = new HashableStringMap ();
        map_a.set ("first", "value1");
        map_a.set ("second", "value2");
        uint hash_a = map_a.hash ();
        
        var map_b = new HashableStringMap ();
        map_b.set ("second", "value2");
        map_b.set ("first", "value1");
        uint hash_b = map_b.hash ();
        
        assert (hash_a == hash_b);
        
        // Test that maps with same content have same hash
        var map_c = new HashableStringMap ();
        map_c.set ("first", "value1");
        map_c.set ("second", "value2");
        uint hash_c = map_c.hash ();
        
        assert (hash_a == hash_c);
    });
    
    Test.add_func ("/core/utils/hashable_string_map/hash_invalidation", () => {
        var map = new HashableStringMap ();
        map.set ("key1", "value1");
        map.set ("key2", "value2");
        
        uint original_hash = map.hash ();
        
        // Test that adding a key invalidates hash
        map.set ("key3", "value3");
        uint new_hash = map.hash ();
        assert (original_hash != new_hash);
        
        // Reset map
        map.clear ();
        map.set ("key1", "value1");
        map.set ("key2", "value2");
        original_hash = map.hash ();
        
        // Test that updating a key invalidates hash
        map.set ("key1", "updated_value1");
        new_hash = map.hash ();
        assert (original_hash != new_hash);
        
        // Reset map
        map.clear ();
        map.set ("key1", "value1");
        map.set ("key2", "value2");
        original_hash = map.hash ();
        
        // Test that removing a key invalidates hash
        map.unset ("key1");
        new_hash = map.hash ();
        assert (original_hash != new_hash);
        
        // Reset map
        map.clear ();
        map.set ("key1", "value1");
        map.set ("key2", "value2");
        original_hash = map.hash ();
        
        // Test that clearing invalidates hash
        map.clear ();
        new_hash = map.hash ();
        // Note: hash of empty map should be consistent, but different from non-empty
        map.set ("key1", "value1");
        uint post_clear_hash = map.hash ();
        assert (new_hash != post_clear_hash);
    });
    
    Test.add_func ("/core/utils/hashable_string_map/hash_order_independence", () => {
        // Test that the hash algorithm is order-independent for the same content
        var map1 = new HashableStringMap ();
        map1.set ("key1", "value1");
        map1.set ("key2", "value2");
        map1.set ("key3", "value3");
        
        var map2 = new HashableStringMap ();
        map2.set ("key3", "value3");
        map2.set ("key1", "value1");
        map2.set ("key2", "value2");
        
        var map3 = new HashableStringMap ();
        map3.set ("key2", "value2");
        map3.set ("key3", "value3");
        map3.set ("key1", "value1");
        
        uint hash1 = map1.hash ();
        uint hash2 = map2.hash ();
        uint hash3 = map3.hash ();
        
        assert (hash1 == hash2);
        assert (hash2 == hash3);
        assert (hash1 == hash3);
    });
    
    Test.add_func ("/core/utils/hashable_string_map/hash_different_content", () => {
        var map1 = new HashableStringMap ();
        map1.set ("key1", "value1");
        map1.set ("key2", "value2");
        
        var map2 = new HashableStringMap ();
        map2.set ("key1", "value1");
        map2.set ("key2", "different_value");
        
        var map3 = new HashableStringMap ();
        map3.set ("key1", "value1");
        map3.set ("key3", "value2");
        
        uint hash1 = map1.hash ();
        uint hash2 = map2.hash ();
        uint hash3 = map3.hash ();
        
        // Different values should produce different hashes
        assert (hash1 != hash2);
        
        // Different keys should produce different hashes
        assert (hash1 != hash3);
    });
    
    Test.run ();
    
    return 0;
}
