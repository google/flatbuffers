/*
 * Copyright 2020 Google Inc. All rights reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

use crate::serialize::{
    cache::{Cache, CacheAllocator},
    offsets::VtableOffset,
};
use static_assertions::assert_not_impl_any;
use std::collections::HashMap;

type Map<'allocator> = HashMap<&'allocator [u8], VtableOffset>;

#[derive(Default)]
pub struct HashMapCache {
    map: Map<'static>,
}

impl HashMapCache {
    #[inline]
    pub fn new() -> HashMapCache {
        HashMapCache::default()
    }

    #[inline]
    pub fn with_capacity(capacity: usize) -> HashMapCache {
        HashMapCache {
            map: Map::with_capacity(capacity),
        }
    }
}

pub struct HashMapAllocator<'allocator, 'cache> {
    borrowed_from: &'cache mut Map<'static>,
    map: Map<'allocator>,
}

assert_not_impl_any!(HashMapAllocator<'static, 'static>: std::panic::UnwindSafe);

impl<'allocator, 'cache> Cache<'allocator, 'cache> for HashMapCache {
    type Allocator = HashMapAllocator<'allocator, 'cache>;

    #[inline]
    fn initialize(&'cache mut self) -> Self::Allocator {
        let mut map = core::mem::replace(&mut self.map, HashMap::new());
        map.clear();
        HashMapAllocator {
            map,
            borrowed_from: &mut self.map,
        }
    }
}

impl<'allocator, 'cache> CacheAllocator<'allocator> for HashMapAllocator<'allocator, 'cache> {
    #[inline]
    fn get(&mut self, data: &[u8]) -> Option<VtableOffset> {
        self.map.get(data).cloned()
    }

    #[inline]
    fn insert(&mut self, data: &'allocator [u8], offset: VtableOffset) {
        self.map.insert(data, offset);
    }
}

impl<'allocator, 'cache> Drop for HashMapAllocator<'allocator, 'cache> {
    #[inline]
    fn drop(&mut self) {
        let mut map = core::mem::replace(&mut self.map, Map::new());
        map.clear();
        // This is safe, because the map is empty
        let map = unsafe { core::mem::transmute::<Map<'allocator>, Map<'static>>(map) };
        *self.borrowed_from = map;
    }
}
