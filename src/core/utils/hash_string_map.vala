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

private const uint64 FNV_OFFSET_BASIS = 14695981039346656037ul;
private const uint64 FNV_PRIME = 1099511628211ul;

public uint64 hash_string_map (Gee.Map<string, string> hash_map) {
  // Use FNV-1a hash algorithm for consistent hashing
  uint64 hash = FNV_OFFSET_BASIS;

  // Process each key-value pair and combine using XOR to ensure order independence
  foreach (var key in hash_map.keys) {
    var value = hash_map.get (key);

    // Calculate hash for this individual key-value pair
    uint64 pair_hash = hash_key_value_pair (key, value);

    // Combine this pair's hash with the overall hash using XOR
    // XOR is commutative, so the order of key-value pairs doesn't matter
    hash ^= pair_hash;
  }

  return hash;
}

private uint64 hash_key_value_pair (string key, string value) {
  // Calculate hash for a single key-value pair using FNV-1a algorithm
  // We hash the key first, then a null separator, then the value
  // This ensures unambiguous separation between key and value
  uint64 pair_hash = FNV_OFFSET_BASIS;

  // Hash the key
  pair_hash = hash_string (pair_hash, key);

  // Add a null byte separator (0) to distinguish between key and value
  // Since strings in Vala are null-terminated, a null byte inside the content is impossible
  pair_hash ^= (uint64) 0;
  pair_hash *= FNV_PRIME;

  // Hash the value
  pair_hash = hash_string (pair_hash, value);

  return pair_hash;
}

private uint64 hash_string (uint64 initial_hash, string str) {
  uint64 hash = initial_hash;
  
  // Hash the string byte by byte using FNV-1a algorithm
  // We use str.data to get the raw bytes, which properly handles UTF-8 encoding
  var bytes = str.data;

  foreach (uchar byte_val in bytes) {
    hash ^= (uint64) byte_val;
    hash *= FNV_PRIME;
  }

  return hash;
}

}
