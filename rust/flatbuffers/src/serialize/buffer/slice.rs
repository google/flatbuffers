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
use core::mem::{self, MaybeUninit};

pub struct SliceBuffer<'slice> {
    slice: &'slice mut [u8],
    position: usize,
}

impl<'slice> SliceBuffer<'slice> {
    #[inline]
    pub fn new(slice: &'slice mut [u8]) -> SliceBuffer<'slice> {
        SliceBuffer {
            position: slice.len(),
            slice,
        }
    }
}

pub struct SliceAllocator<'a> {
    slice: &'a mut [u8],
    position: &'a mut usize,
}

#[cfg(feature = "std")]
static_assertions::assert_not_impl_any!(SliceBuffer: std::panic::UnwindSafe);

#[cfg(feature = "std")]
static_assertions::assert_not_impl_any!(SliceAllocator<'static>: std::panic::UnwindSafe);

pub struct Iter<'a> {
    slice: Option<&'a [u8]>,
}

impl<'a, 'slice> Buffer<'a> for SliceBuffer<'slice> {
    type Allocator = SliceAllocator<'a>;
    type Iterator = Iter<'a>;
    #[inline]
    fn allocator(&'a mut self) -> SliceAllocator<'a> {
        self.position = self.slice.len();
        SliceAllocator {
            slice: self.slice,
            position: &mut self.position,
        }
    }

    #[inline]
    fn iter_bytes(&'a mut self) -> Iter<'a> {
        Iter {
            slice: Some(&self.slice[self.position..]),
        }
    }
}

impl<'a> BufferAllocator<'a> for SliceAllocator<'a> {
    #[inline]
    unsafe fn allocate_uninitialized(
        &mut self,
        count: usize,
    ) -> Result<&'a mut [MaybeUninit<u8>], OutOfBufferSpace> {
        let slice = mem::replace(&mut self.slice, &mut []);
        if count < slice.len() {
            let new_len = slice.len().wrapping_sub(count);
            let (remaining, cur) = slice.split_at_mut(new_len);
            if cur.len() != count {
                panic!("waaat1");
            }
            self.slice = remaining;
            *self.position = new_len;

            // This is okay, since we are only casting the slice from &[u8] to
            // &[MaybeUninit<u8>]
            let slice = core::slice::from_raw_parts_mut(
                cur.as_mut_ptr() as *mut MaybeUninit<u8>,
                cur.len(),
            );
            if slice.len() != count {
                panic!("waaat2");
            }
            Ok(slice)
        } else {
            self.slice = slice;
            Err(OutOfBufferSpace)
        }
    }
}

impl<'a> Iterator for Iter<'a> {
    type Item = &'a [u8];

    #[inline]
    fn next(&mut self) -> Option<&'a [u8]> {
        self.slice.take()
    }

    #[inline]
    fn size_hint(&self) -> (usize, Option<usize>) {
        let len = if self.slice.is_some() { 1 } else { 0 };
        (len, Some(len))
    }
}

impl<'a> core::iter::ExactSizeIterator for Iter<'a> {}
impl<'a> core::iter::FusedIterator for Iter<'a> {}
