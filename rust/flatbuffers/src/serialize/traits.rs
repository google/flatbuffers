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
    serialize::{builder, offsets::RawOffset},
};
use core::mem;

/// Marker trait for flatbuffer tables
///
/// Types implementing this trait can be serialized as flatbuffer
/// tables.
///
/// This trait should not be implemented manually, and methods
/// on it should not be called outside the flatbuffer library.
///
/// It is used as a trait bound for some methods on the `FlatbufferBuilder`.
pub unsafe trait FlatbufferTable {
    #[doc(hidden)]
    fn validate_required(&self);

    #[doc(hidden)]
    fn serialize<F: builder::FlatbufferWriter>(
        &self,
        writer: &mut F,
    ) -> Result<RawOffset, OutOfBufferSpace>;
}

/// Marker trait for primitive flatbuffer types
///
/// Types implementing this trait can be serialized as primitive
/// flatbuffer types, i.e. numbers, booleans, structs and enums.
///
/// This trait should not be implemented manually. The (hidden)
/// methods this trait should not be called outside the flatbuffer library.
///
/// It is used as a trait bound for some methods on the `FlatbufferBuilder`.
pub unsafe trait FlatbufferPrimitive {
    /// The size of the serialized object in bytes.
    #[doc(hidden)]
    const SIZE: usize;

    /// The alignment needed for objects of this type.
    ///
    /// The value is only used to make the final output aligned. It has no
    /// influence on the buffer provided to `serialize`, which does not need
    /// to be aligned.
    #[doc(hidden)]
    const ALIGNMENT: usize;

    /// Serializes the value into the provided buffer.
    ///
    /// This function is not meant to be called from outside
    /// this library.
    ///
    /// The provided `buffer` must be `SIZE` bytes large, and
    /// does not need to be initialized. After the call to the
    /// function, all `SIZE` bytes will be initialized.
    ///
    /// The buffer does **not** need to be aligned. The `ALIGNMENT`
    /// const is only used to make the value aligned in the output.
    ///
    /// # Safety
    ///
    /// This function is safe as long as the provided as buffer is
    /// large enough.
    #[doc(hidden)]
    unsafe fn serialize(&self, buffer: *mut u8, offset: RawOffset);
}

/// Marker trait for primitive flatbuffer types for which it is safe to
/// simply mempy the value into the target buffer.
///
/// Note that this depends on the endianness of the host machine, so any
/// code using these functions might not be portable.
///
/// It is used as a trait bound for some methods on the `FlatbufferBuilder`.
pub unsafe trait MemcpySafe: FlatbufferPrimitive {}

macro_rules! number_impl {
    ($typ:ty) => {
        unsafe impl FlatbufferPrimitive for $typ {
            const SIZE: usize = mem::size_of::<$typ>();
            const ALIGNMENT: usize = mem::size_of::<$typ>();

            #[inline(always)]
            unsafe fn serialize(&self, buffer: *mut u8, _offset: RawOffset) {
                buffer.copy_from(self.to_le_bytes().as_ptr(), Self::SIZE);
            }
        }

        #[cfg(target_endian = "little")]
        unsafe impl MemcpySafe for $typ {}
    };
}

number_impl!(u8);
number_impl!(u16);
number_impl!(u32);
number_impl!(u64);

number_impl!(i8);
number_impl!(i16);
number_impl!(i32);
number_impl!(i64);

number_impl!(f32);
number_impl!(f64);

#[cfg(not(target_endian = "little"))]
unsafe impl MemcpySafe for u8 {}

#[cfg(not(target_endian = "little"))]
unsafe impl MemcpySafe for u8 {}

unsafe impl FlatbufferPrimitive for bool {
    const SIZE: usize = 1;
    const ALIGNMENT: usize = 1;

    #[inline(always)]
    unsafe fn serialize(&self, buffer: *mut u8, _offset: RawOffset) {
        buffer.write(if *self { 1 } else { 0 });
    }
}

unsafe impl MemcpySafe for bool {}
