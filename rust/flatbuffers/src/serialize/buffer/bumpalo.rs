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

use crate::{
    errors::OutOfBufferSpace,
    serialize::buffer::{Buffer, BufferAllocator},
};
use bumpalo::Bump;
use core::mem::MaybeUninit;

#[derive(Default)]
pub struct BumpaloBuffer {
    bump: Bump,
}

pub struct BumpaloAllocator<'a> {
    bump: &'a Bump,
}

#[cfg(feature = "std")]
static_assertions::assert_not_impl_any!(BumpaloBuffer: std::panic::UnwindSafe);

#[cfg(feature = "std")]
static_assertions::assert_not_impl_any!(BumpaloAllocator<'static>: std::panic::UnwindSafe);

impl BumpaloBuffer {
    #[inline]
    pub fn new(mut bump: Bump) -> BumpaloBuffer {
        bump.reset();
        BumpaloBuffer { bump }
    }

    #[inline]
    pub fn with_capacity(capacity: usize) -> BumpaloBuffer {
        BumpaloBuffer {
            bump: Bump::with_capacity(capacity),
        }
    }
}

impl<'a> Buffer<'a> for BumpaloBuffer {
    type Allocator = BumpaloAllocator<'a>;
    type Iterator = Iter<'a>;

    #[inline]
    fn allocator(&'a mut self) -> BumpaloAllocator<'a> {
        self.bump.reset();
        BumpaloAllocator { bump: &self.bump }
    }

    #[inline]
    fn iter_bytes(&'a mut self) -> Iter<'a> {
        Iter {
            iter: self.bump.iter_allocated_chunks(),
        }
    }
}

impl<'a> BufferAllocator<'a> for BumpaloAllocator<'a> {
    #[inline]
    unsafe fn allocate_uninitialized(
        &mut self,
        count: usize,
    ) -> Result<&'a mut [MaybeUninit<u8>], OutOfBufferSpace> {
        Ok(self
            .bump
            .alloc_slice_fill_with(count, |_| MaybeUninit::uninit()))
    }
}

pub struct Iter<'a> {
    iter: bumpalo::ChunkIter<'a>,
}

impl<'a> Iterator for Iter<'a> {
    type Item = &'a [u8];

    #[inline]
    fn next(&mut self) -> Option<&'a [u8]> {
        self.iter.next().map(|s: &[MaybeUninit<u8>]| {
            let (ptr, len) = (s.as_ptr(), s.len());
            // This is safe, because we fulfill the requirements written on
            // bumpalo::Bump::iter_allocated_chunks. Specifically we only
            // ever allocate byte slices, which all have 1-byte alignment and
            // no internal padding.
            //
            // However for this to be valid we also need to make sure that all
            // the slices handed out by allocate_uninitialized have in fact
            // been initialized.
            unsafe { core::slice::from_raw_parts(ptr as *const u8, len) }
        })
    }
}

impl<'a> core::iter::FusedIterator for Iter<'a> {}
