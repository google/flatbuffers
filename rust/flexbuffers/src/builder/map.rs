// Copyright 2019 Google LLC
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     https://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

use super::{Builder, Pushable, Value, VectorBuilder};

/// Builds a Flexbuffer map. When this is dropped, or `end` is called, the map is
/// commited to the buffer. If this map is the root of the flexbuffer then the
/// root is written and the flexbuffer is complete.
/// WARNING: Duplicate keys results in a panic.
pub struct MapBuilder<'a> {
    pub(super) builder: &'a mut Builder,
    // If the root is this map then start == None. Otherwise start is the
    // number of values in the 'values stack' before adding this map.
    pub(super) start: Option<usize>,
}
impl<'a> MapBuilder<'a> {
    /// Push `p` onto this map with key `key`.
    /// This will panic if `key` contains internal nulls.
    pub fn push<P: Pushable>(&mut self, key: &str, p: P) {
        self.builder.push_key(key);
        self.builder.push(p);
    }
    /// Starts a nested vector which will be pushed onto this map
    /// with key `key`.
    /// This will panic if `key` contains internal nulls.
    pub fn nest_vector(&mut self, key: &str) -> VectorBuilder {
        // Push the key that refers to this nested vector.
        self.builder.push_key(key);
        // Nested vector.
        let start = Some(self.builder.values.len());
        VectorBuilder {
            builder: &mut self.builder,
            start,
        }
    }
    /// Starts a nested map which which will be pushed onto this map
    /// with key `key`.
    /// This will panic if `key` contains internal nulls.
    pub fn nest_map(&mut self, key: &str) -> MapBuilder {
        // Push the key that refers to this nested vector.
        self.builder.push_key(key);
        // Nested map.
        let start = Some(self.builder.values.len());
        MapBuilder {
            builder: &mut self.builder,
            start,
        }
    }
    /// `end` sorts the map by key and writes it to the buffer. This happens anyway
    /// when the map builder is dropped.
    pub fn end(self) {}
}
impl<'a> Drop for MapBuilder<'a> {
    fn drop(&mut self) {
        self.builder.end_map_or_vector(true, self.start);
    }
}

// Read a known key or string inside the buffer.
pub(super) unsafe fn read_trusted_str(buffer: &[u8], idx: usize, len: usize) -> &str {
    let str_ptr = buffer.as_ptr().add(idx);
    let slice = std::slice::from_raw_parts(str_ptr, len);
    std::str::from_utf8_unchecked(slice)
}

// `values` is assumed to be of the format [key1, value1, ..., keyN, valueN].
// The keys refer to cstrings in `buffer`. When this function returns,
// `values` is sorted in place by key.
pub(super) fn sort_map_by_keys(values: &mut [Value], buffer: &[u8]) {
    debug_assert_eq!(values.len() % 2, 0);
    debug_assert!(values.iter().step_by(2).all(Value::is_key));
    let pairs: &mut [[Value; 2]] = unsafe {
        let raw_pairs = values.as_mut_ptr() as *mut [Value; 2];
        std::slice::from_raw_parts_mut(raw_pairs, values.len() / 2)
    };
    #[rustfmt::skip]
    pairs.sort_unstable_by(|[key1, _], [key2, _]| {
        if let Value::Key { address: a1, length: l1 } = *key1 {
            if let Value::Key { address: a2, length: l2 } = *key2 {
                let s1 = unsafe { read_trusted_str(&buffer, a1, l1) };
                let s2 = unsafe { read_trusted_str(&buffer, a2, l2) };
                let ord = s1.cmp(s2);
                assert!(ord != std::cmp::Ordering::Equal, "Duplicated keys in map.");
                return ord;
            }
        }
        unreachable!();
    });
}

#[cfg(test)]
mod tests {
    use super::*;
    #[test]
    #[should_panic]
    fn panic_on_repeated_key() {
        let mut b = Builder::default();
        let mut m = b.build_map();
        m.push("foo", 5u8);
        m.push("foo", 6u8);
        m.end();
    }
    #[test]
    #[should_panic]
    fn panic_on_internal_null() {
        let mut b = Builder::default();
        let mut m = b.build_map();
        m.push("foo\0", 5u8);
        m.end();
    }
}
