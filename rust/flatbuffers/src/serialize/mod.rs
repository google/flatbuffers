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

pub mod buffer;
#[doc(hidden)]
pub mod builder;
pub mod cache;
mod iter;

mod offsets;
mod traits;

pub use self::{
    builder::FlatbufferBuilder,
    iter::FlatbufferIter,
    offsets::{Offset, RawOffset, VtableOffset},
    traits::{FlatbufferPrimitive, FlatbufferTable},
};

const FLATBUFFERS_MAX_BUFFER_SIZE: usize = (1u64 << 31) as usize;

pub struct Flatbuffer<B, C> {
    buffer: B,
    cache: C,
    last_message_size: Option<core::num::NonZeroUsize>,
}

impl<B, C> Flatbuffer<B, C>
where
    for<'allocator> B: buffer::Buffer<'allocator>,
    for<'allocator, 'cache> C: cache::Cache<'allocator, 'cache>,
{
    #[inline]
    pub fn new(buffer: B, cache: C) -> Flatbuffer<B, C> {
        Flatbuffer {
            buffer,
            cache,
            last_message_size: None,
        }
    }

    /// Construct builder for a new message.
    #[inline]
    pub fn new_message(&mut self) -> FlatbufferBuilder<'_, B, C> {
        let allocator = self.buffer.allocator();
        let cache = self.cache.initialize();
        self.last_message_size = None;
        FlatbufferBuilder::new(&mut self.last_message_size, allocator, cache)
    }

    /// Gets the size of the last message
    ///
    /// This will return `Some(size)` if the last message was finished
    /// correctly.
    ///
    /// If the last message was not finished correctly (e.g. if the builder was
    /// dropped), then this will return `None`.
    #[inline]
    pub fn last_message_size(&self) -> Option<usize> {
        self.last_message_size.map(|s| s.get())
    }

    /// Iterate over the byte-slices of the last constructed message.
    ///
    /// If the last message was not finished correctly (e.g. if the builder was
    /// dropped), then this will iterator will not return any slices.
    #[inline]
    pub fn iter_last_message(&mut self) -> iter::FlatbufferIter<'_, B> {
        iter::FlatbufferIter {
            iter: if self.last_message_size.is_some() {
                Some(self.buffer.iter_bytes())
            } else {
                None
            },
        }
    }

    /// Collects the byte-slices of the last constructed message into a `Vec<u8>`
    ///
    /// If the last message was not finished correctly (e.g. if the builder was
    /// dropped), then this will return `None`.
    #[inline]
    #[cfg(feature = "std")]
    pub fn collect_last_message(&mut self) -> Option<Vec<u8>> {
        if let Some(last_message_size) = self.last_message_size {
            let mut output = Vec::with_capacity(last_message_size.get());
            for slice in self.buffer.iter_bytes() {
                output.extend_from_slice(slice);
            }
            Some(output)
        } else {
            None
        }
    }
}
