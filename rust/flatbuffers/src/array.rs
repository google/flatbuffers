/*
 * Copyright 2021 Google Inc. All rights reserved.
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

use crate::follow::Follow;
use crate::vector::VectorIter;
use crate::EndianScalar;
use std::fmt::{Debug, Formatter, Result};
use std::marker::PhantomData;
use std::mem::size_of;

#[derive(Copy, Clone)]
pub struct Array<'a, T: 'a, const N: usize>(&'a [u8], PhantomData<T>);

impl<'a, T: 'a, const N: usize> Debug for Array<'a, T, N>
where
    T: 'a + Follow<'a>,
    <T as Follow<'a>>::Inner: Debug,
{
    fn fmt(&self, f: &mut Formatter) -> Result {
        f.debug_list().entries(self.iter()).finish()
    }
}

#[allow(clippy::len_without_is_empty)]
#[allow(clippy::from_over_into)] // TODO(caspern): Go from From to Into.
impl<'a, T: 'a, const N: usize> Array<'a, T, N> {
    #[inline(always)]
    pub fn new(buf: &'a [u8]) -> Self {
        debug_assert!(size_of::<T>() * N == buf.len());

        Array {
            0: buf,
            1: PhantomData,
        }
    }

    #[inline(always)]
    pub const fn len(&self) -> usize {
        N
    }
    pub fn as_ptr(&self) -> *const u8 {
        self.0.as_ptr()
    }
}

impl<'a, T: Follow<'a> + 'a, const N: usize> Array<'a, T, N> {
    #[inline(always)]
    pub fn get(&self, idx: usize) -> T::Inner {
        debug_assert!(idx < N);
        let sz = size_of::<T>();
        T::follow(self.0, sz * idx)
    }

    #[inline(always)]
    pub fn iter(&self) -> VectorIter<'a, T> {
        VectorIter::from_slice(self.0, self.len())
    }
}

impl<'a, T: Follow<'a> + Debug, const N: usize> Into<[T::Inner; N]> for Array<'a, T, N> {
    #[inline(always)]
    fn into(self) -> [T::Inner; N] {
        array_init(|i| self.get(i))
    }
}

// TODO(caspern): Implement some future safe version of SafeSliceAccess.

/// Implement Follow for all possible Arrays that have Follow-able elements.
impl<'a, T: Follow<'a> + 'a, const N: usize> Follow<'a> for Array<'a, T, N> {
    type Inner = Array<'a, T, N>;
    #[inline(always)]
    fn follow(buf: &'a [u8], loc: usize) -> Self::Inner {
        Array::new(&buf[loc..loc + N * size_of::<T>()])
    }
}

pub fn emplace_scalar_array<T: EndianScalar, const N: usize>(
    buf: &mut [u8],
    loc: usize,
    src: &[T; N],
) {
    let mut buf_ptr = buf[loc..].as_mut_ptr();
    for item in src.iter() {
        let item_le = item.to_little_endian();
        unsafe {
            core::ptr::copy_nonoverlapping(
                &item_le as *const T as *const u8,
                buf_ptr,
                size_of::<T>(),
            );
            buf_ptr = buf_ptr.add(size_of::<T>());
        }
    }
}

impl<'a, T: Follow<'a> + 'a, const N: usize> IntoIterator for Array<'a, T, N> {
    type Item = T::Inner;
    type IntoIter = VectorIter<'a, T>;
    #[inline]
    fn into_iter(self) -> Self::IntoIter {
        self.iter()
    }
}

#[inline]
pub fn array_init<F, T, const N: usize>(mut initializer: F) -> [T; N]
where
    F: FnMut(usize) -> T,
{
    let mut array: core::mem::MaybeUninit<[T; N]> = core::mem::MaybeUninit::uninit();
    let mut ptr_i = array.as_mut_ptr() as *mut T;

    unsafe {
        for i in 0..N {
            let value_i = initializer(i);
            ptr_i.write(value_i);
            ptr_i = ptr_i.add(1);
        }
        array.assume_init()
    }
}
