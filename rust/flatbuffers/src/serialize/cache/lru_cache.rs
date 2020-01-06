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

type Map<'allocator> = lru_cache::LruCache<&'allocator [u8], VtableOffset>;

pub struct LruCache {
    map: Map<'static>,
}

pub struct LruAllocator<'allocator, 'cache> {
    borrowed_from: &'cache mut Map<'static>,
    map: Map<'allocator>,
}

impl LruCache {
    #[inline]
    pub fn new(capacity: usize) -> LruCache {
        LruCache {
            map: Map::new(capacity),
        }
    }
}

impl<'allocator, 'cache> Cache<'allocator, 'cache> for LruCache {
    type Allocator = LruAllocator<'allocator, 'cache>;

    #[inline]
    fn initialize(&'cache mut self) -> Self::Allocator {
        let mut map = core::mem::replace(&mut self.map, Map::new(0));
        map.clear();
        // This is safe, because the map is empty
        let map = unsafe { core::mem::transmute::<Map<'static>, Map<'allocator>>(map) };
        LruAllocator {
            map,
            borrowed_from: &mut self.map,
        }
    }
}

impl<'allocator, 'cache> CacheAllocator<'allocator> for LruAllocator<'allocator, 'cache> {
    #[inline]
    fn get(&mut self, data: &[u8]) -> Option<VtableOffset> {
        self.map.get_mut(data).cloned()
    }

    #[inline]
    fn insert(&mut self, data: &'allocator [u8], offset: VtableOffset) {
        self.map.insert(data, offset);
    }
}

impl<'allocator, 'cache> Drop for LruAllocator<'allocator, 'cache> {
    #[inline]
    fn drop(&mut self) {
        let mut map = core::mem::replace(&mut self.map, Map::new(0));
        map.clear();
        // This is safe, because the map is empty
        let map = unsafe { core::mem::transmute::<Map<'allocator>, Map<'static>>(map) };
        *self.borrowed_from = map;
    }
}
