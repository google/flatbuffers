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

use crate::errors::OutOfBufferSpace;
use core::mem::MaybeUninit;

#[cfg(feature = "bumpalo")]
pub mod bumpalo;
mod slice;

#[cfg(feature = "bumpalo")]
pub use crate::serialize::buffer::bumpalo::BumpaloBuffer;
pub use crate::serialize::buffer::slice::SliceBuffer;

pub trait Buffer<'allocator> {
    type Allocator: BufferAllocator<'allocator>;
    type Iterator: Iterator<Item = &'allocator [u8]>;
    fn allocator(&'allocator mut self) -> Self::Allocator;
    fn iter_bytes(&'allocator mut self) -> Self::Iterator;
}

#[doc(hidden)]
pub trait BufferAllocator<'allocator> {
    /// Allocate a new slice in the buffer
    ///
    /// # Safety
    ///
    /// This function is only safe to call, if you immediately initialize
    /// the slice returned to you.
    unsafe fn allocate_uninitialized(
        &mut self,
        count: usize,
    ) -> Result<&'allocator mut [MaybeUninit<u8>], OutOfBufferSpace>;
}
