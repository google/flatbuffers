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
use core::{marker::PhantomData, ptr::NonNull};

#[derive(Copy, Clone)]
struct CacheValue {
    offset: VtableOffset,
    ptr: NonNull<[u8]>,
}

type Map = uluru::LRUCache<[uluru::Entry<CacheValue>; 16]>;

// Invariant: If an UluruAllocator<'allocator, 'cache> exists, then all the
// NonNull<[u8]> values inside can be considered to live for at least the
// lifetime 'allocator. If no UluruCache exists, then the NonNull<[u8]>
// values are considered dangling and should not be dereferenced.
#[derive(Default)]
pub struct UluruCache {
    map: Map,
}

pub struct UluruAllocator<'allocator, 'cache> {
    map: &'cache mut Map,
    marker: PhantomData<&'allocator [u8]>,
}

impl<'allocator, 'cache> Cache<'allocator, 'cache> for UluruCache {
    type Allocator = UluruAllocator<'allocator, 'cache>;

    #[inline]
    fn initialize(&'cache mut self) -> Self::Allocator {
        let map = &mut self.map;

        // Here we create a new UluruCache, which means that we have to make
        // sure that the invariant is kept. We do this by clearing the map
        map.evict_all();
        let cache: UluruAllocator<'allocator, 'cache> = UluruAllocator {
            map,
            marker: PhantomData,
        };
        cache
    }
}

impl<'allocator, 'cache> CacheAllocator<'allocator> for UluruAllocator<'allocator, 'cache> {
    #[inline]
    fn get(&mut self, data: &[u8]) -> Option<VtableOffset> {
        Some(
            self.map
                .find(|value| {
                    // This unsafe block is okay, because we know from the invariant
                    // that the pointer is alive
                    unsafe { value.ptr.as_ref() == data }
                })?
                .offset,
        )
    }

    #[inline]
    fn insert(&mut self, data: &'allocator [u8], offset: VtableOffset) {
        // This does not break the invariant: The data lives long enough
        self.map.insert(CacheValue {
            offset,
            ptr: data.into(),
        })
    }
}
