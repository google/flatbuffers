/*
 * Copyright 2018 Google Inc. All rights reserved.
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

use std::fmt::{Debug, Formatter, Result};
use std::iter::{DoubleEndedIterator, ExactSizeIterator, FusedIterator};
use std::marker::PhantomData;
use std::mem::size_of;
use std::slice::from_raw_parts;
use std::str::from_utf8_unchecked;

use crate::endian_scalar::read_scalar_at;
#[cfg(target_endian = "little")]
use crate::endian_scalar::EndianScalar;
use crate::follow::Follow;
use crate::primitives::*;

pub struct Vector<'a, T: 'a>(&'a [u8], usize, PhantomData<T>);

impl<'a, T: 'a> Default for Vector<'a, T> {
    fn default() -> Self {
        // Static, length 0 vector.
        // Note that derived default causes UB due to issues in read_scalar_at /facepalm.
        Self(
            &[0; core::mem::size_of::<UOffsetT>()],
            0,
            Default::default(),
        )
    }
}

impl<'a, T> Debug for Vector<'a, T>
where
    T: 'a + Follow<'a>,
    <T as Follow<'a>>::Inner: Debug,
{
    fn fmt(&self, f: &mut Formatter) -> Result {
        f.debug_list().entries(self.iter()).finish()
    }
}

// We cannot use derive for these two impls, as it would only implement Copy
// and Clone for `T: Copy` and `T: Clone` respectively. However `Vector<'a, T>`
// can always be copied, no matter that `T` you have.
impl<'a, T> Copy for Vector<'a, T> {}
impl<'a, T> Clone for Vector<'a, T> {
    fn clone(&self) -> Self {
        *self
    }
}

impl<'a, T: 'a> Vector<'a, T> {
    #[inline(always)]
    pub fn new(buf: &'a [u8], loc: usize) -> Self {
        Vector {
            0: buf,
            1: loc,
            2: PhantomData,
        }
    }

    #[inline(always)]
    pub fn len(&self) -> usize {
        unsafe { read_scalar_at::<UOffsetT>(&self.0, self.1) as usize }
    }
    #[inline(always)]
    pub fn is_empty(&self) -> bool {
        self.len() == 0
    }
}

impl<'a, T: Follow<'a> + 'a> Vector<'a, T> {
    #[inline(always)]
    pub fn get(&self, idx: usize) -> T::Inner {
        debug_assert!(idx < self.len() as usize);
        let sz = size_of::<T>();
        debug_assert!(sz > 0);
        T::follow(self.0, self.1 as usize + SIZE_UOFFSET + sz * idx)
    }

    #[inline(always)]
    pub fn iter(&self) -> VectorIter<'a, T> {
        VectorIter::from_vector(*self)
    }
}

pub trait SafeSliceAccess {}
impl<'a, T: SafeSliceAccess + 'a> Vector<'a, T> {
    pub fn safe_slice(self) -> &'a [T] {
        let buf = self.0;
        let loc = self.1;
        let sz = size_of::<T>();
        debug_assert!(sz > 0);
        let len = unsafe { read_scalar_at::<UOffsetT>(&buf, loc) } as usize;
        let data_buf = &buf[loc + SIZE_UOFFSET..loc + SIZE_UOFFSET + len * sz];
        let ptr = data_buf.as_ptr() as *const T;
        let s: &'a [T] = unsafe { from_raw_parts(ptr, len) };
        s
    }
}

impl SafeSliceAccess for u8 {}
impl SafeSliceAccess for i8 {}
impl SafeSliceAccess for bool {}

// TODO(caspern): Get rid of this. Conditional compliation is unnecessary complexity.
// Vectors of primitives just don't work on big endian machines!!!
#[cfg(target_endian = "little")]
mod le_safe_slice_impls {
    impl super::SafeSliceAccess for u16 {}
    impl super::SafeSliceAccess for u32 {}
    impl super::SafeSliceAccess for u64 {}

    impl super::SafeSliceAccess for i16 {}
    impl super::SafeSliceAccess for i32 {}
    impl super::SafeSliceAccess for i64 {}

    impl super::SafeSliceAccess for f32 {}
    impl super::SafeSliceAccess for f64 {}
}

#[cfg(target_endian = "little")]
pub use self::le_safe_slice_impls::*;

pub fn follow_cast_ref<'a, T: Sized + 'a>(buf: &'a [u8], loc: usize) -> &'a T {
    let sz = size_of::<T>();
    let buf = &buf[loc..loc + sz];
    let ptr = buf.as_ptr() as *const T;
    unsafe { &*ptr }
}

impl<'a> Follow<'a> for &'a str {
    type Inner = &'a str;
    fn follow(buf: &'a [u8], loc: usize) -> Self::Inner {
        let len = unsafe { read_scalar_at::<UOffsetT>(&buf, loc) } as usize;
        let slice = &buf[loc + SIZE_UOFFSET..loc + SIZE_UOFFSET + len];
        unsafe { from_utf8_unchecked(slice) }
    }
}

#[cfg(target_endian = "little")]
fn follow_slice_helper<T>(buf: &[u8], loc: usize) -> &[T] {
    let sz = size_of::<T>();
    debug_assert!(sz > 0);
    let len = unsafe { read_scalar_at::<UOffsetT>(&buf, loc) as usize };
    let data_buf = &buf[loc + SIZE_UOFFSET..loc + SIZE_UOFFSET + len * sz];
    let ptr = data_buf.as_ptr() as *const T;
    let s: &[T] = unsafe { from_raw_parts(ptr, len) };
    s
}

/// Implement direct slice access if the host is little-endian.
#[cfg(target_endian = "little")]
impl<'a, T: EndianScalar> Follow<'a> for &'a [T] {
    type Inner = &'a [T];
    fn follow(buf: &'a [u8], loc: usize) -> Self::Inner {
        follow_slice_helper::<T>(buf, loc)
    }
}

/// Implement Follow for all possible Vectors that have Follow-able elements.
impl<'a, T: Follow<'a> + 'a> Follow<'a> for Vector<'a, T> {
    type Inner = Vector<'a, T>;
    fn follow(buf: &'a [u8], loc: usize) -> Self::Inner {
        Vector::new(buf, loc)
    }
}

/// An iterator over a `Vector`.
#[derive(Debug)]
pub struct VectorIter<'a, T: 'a> {
    buf: &'a [u8],
    loc: usize,
    remaining: usize,
    phantom: PhantomData<T>,
}

impl<'a, T: 'a> VectorIter<'a, T> {
    #[inline]
    pub fn from_vector(inner: Vector<'a, T>) -> Self {
        VectorIter {
            buf: inner.0,
            // inner.1 is the location of the data for the vector.
            // The first SIZE_UOFFSET bytes is the length. We skip
            // that to get to the actual vector content.
            loc: inner.1 + SIZE_UOFFSET,
            remaining: inner.len(),
            phantom: PhantomData,
        }
    }

    #[inline]
    pub fn from_slice(buf: &'a [u8], items_num: usize) -> Self {
        VectorIter {
            buf,
            loc: 0,
            remaining: items_num,
            phantom: PhantomData,
        }
    }
}

impl<'a, T: Follow<'a> + 'a> Clone for VectorIter<'a, T> {
    #[inline]
    fn clone(&self) -> Self {
        VectorIter {
            buf: self.buf,
            loc: self.loc,
            remaining: self.remaining,
            phantom: self.phantom,
        }
    }
}

impl<'a, T: Follow<'a> + 'a> Iterator for VectorIter<'a, T> {
    type Item = T::Inner;

    #[inline]
    fn next(&mut self) -> Option<T::Inner> {
        let sz = size_of::<T>();
        debug_assert!(sz > 0);

        if self.remaining == 0 {
            None
        } else {
            let result = T::follow(self.buf, self.loc);
            self.loc += sz;
            self.remaining -= 1;
            Some(result)
        }
    }

    #[inline]
    fn nth(&mut self, n: usize) -> Option<T::Inner> {
        let sz = size_of::<T>();
        debug_assert!(sz > 0);

        self.remaining = self.remaining.saturating_sub(n);

        // Note that this might overflow, but that is okay because
        // in that case self.remaining will have been set to zero.
        self.loc = self.loc.wrapping_add(sz * n);

        self.next()
    }

    #[inline]
    fn size_hint(&self) -> (usize, Option<usize>) {
        (self.remaining, Some(self.remaining))
    }
}

impl<'a, T: Follow<'a> + 'a> DoubleEndedIterator for VectorIter<'a, T> {
    #[inline]
    fn next_back(&mut self) -> Option<T::Inner> {
        let sz = size_of::<T>();
        debug_assert!(sz > 0);

        if self.remaining == 0 {
            None
        } else {
            self.remaining -= 1;
            Some(T::follow(self.buf, self.loc + sz * self.remaining))
        }
    }

    #[inline]
    fn nth_back(&mut self, n: usize) -> Option<T::Inner> {
        self.remaining = self.remaining.saturating_sub(n);
        self.next_back()
    }
}

impl<'a, T: 'a + Follow<'a>> ExactSizeIterator for VectorIter<'a, T> {
    #[inline]
    fn len(&self) -> usize {
        self.remaining
    }
}

impl<'a, T: 'a + Follow<'a>> FusedIterator for VectorIter<'a, T> {}

impl<'a, T: Follow<'a> + 'a> IntoIterator for Vector<'a, T> {
    type Item = T::Inner;
    type IntoIter = VectorIter<'a, T>;
    #[inline]
    fn into_iter(self) -> Self::IntoIter {
        self.iter()
    }
}

impl<'a, 'b, T: Follow<'a> + 'a> IntoIterator for &'b Vector<'a, T> {
    type Item = T::Inner;
    type IntoIter = VectorIter<'a, T>;
    fn into_iter(self) -> Self::IntoIter {
        self.iter()
    }
}
