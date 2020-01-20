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

use crate::serialize::offsets::VtableOffset;

#[cfg(feature = "hashbrown")]
mod hashbrown;
#[cfg(feature = "lru-cache")]
mod lru_cache;
mod noop;
#[cfg(feature = "uluru")]
mod uluru;

#[cfg(feature = "hashbrown")]
pub use crate::serialize::cache::hashbrown::HashMapCache;
#[cfg(feature = "lru-cache")]
pub use crate::serialize::cache::lru_cache::LruCache;
pub use crate::serialize::cache::noop::NoopCache;
#[cfg(feature = "uluru")]
pub use crate::serialize::cache::uluru::UluruCache;

pub trait Cache<'allocator, 'cache> {
    type Allocator: CacheAllocator<'allocator>;
    fn initialize(&'cache mut self) -> Self::Allocator;
}

#[doc(hidden)]
pub trait CacheAllocator<'allocator> {
    fn get(&mut self, data: &[u8]) -> Option<VtableOffset>;
    fn insert(&mut self, data: &'allocator [u8], offset: VtableOffset);
}
