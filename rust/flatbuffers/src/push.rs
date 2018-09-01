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

use std::cmp::max;
use std::mem::size_of;
use std::slice::from_raw_parts;

use endian_scalar::emplace_scalar;
use primitives::*;
use vector::{SafeSliceAccess, Vector};

/// Trait to abstract over functionality needed to write values. Used in
/// FlatBufferBuilder and implemented for generated types.
pub trait Push: Sized {
    type Output;
    fn push(&self, dst: &mut [u8], _rest: &[u8]);

    #[inline]
    fn size(&self) -> usize {
        size_of::<Self>()
    }

    #[inline]
    fn alignment(&self) -> usize {
        self.size()
    }
}

/// Push-able wrapper for slices of types that implement SafeSliceAccess.
impl<'a, T: SafeSliceAccess + Sized> Push for &'a [T] {
    type Output = Vector<'a, u8>;

    #[inline]
    fn push(&self, dst: &mut [u8], _rest: &[u8]) {
        let elem_sz = size_of::<T>();
        let data = {
            let ptr = self.as_ptr() as *const T as *const u8;
            unsafe {
                from_raw_parts(ptr, self.len() * elem_sz)
            }
        };
        emplace_scalar::<UOffsetT>(&mut dst[..SIZE_UOFFSET], self.len() as UOffsetT);
        dst[SIZE_UOFFSET..SIZE_UOFFSET+data.len()].copy_from_slice(data);
    }

    #[inline]
    fn size(&self) -> usize {
        SIZE_UOFFSET + self.len() * size_of::<T>()
    }

    #[inline]
    fn alignment(&self) -> usize {
        max(SIZE_UOFFSET, size_of::<T>())
    }
}

/// Macro to implement Push for EndianScalar types.
macro_rules! impl_push_for_endian_scalar {
    ($ty:ident) => (
        impl Push for $ty {
            type Output = $ty;

            #[inline]
            fn push(&self, dst: &mut [u8], _rest: &[u8]) {
                emplace_scalar::<$ty>(dst, *self);
            }
        }
    )
}

impl_push_for_endian_scalar!(bool);
impl_push_for_endian_scalar!(u8);
impl_push_for_endian_scalar!(i8);
impl_push_for_endian_scalar!(u16);
impl_push_for_endian_scalar!(i16);
impl_push_for_endian_scalar!(u32);
impl_push_for_endian_scalar!(i32);
impl_push_for_endian_scalar!(u64);
impl_push_for_endian_scalar!(i64);
impl_push_for_endian_scalar!(f32);
impl_push_for_endian_scalar!(f64);
