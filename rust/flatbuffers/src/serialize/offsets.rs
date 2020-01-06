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

use crate::serialize::traits::FlatbufferPrimitive;
use core::marker::PhantomData;

/// Offset to a location in a flatbuffer
///
/// Since the flatbuffer is constructed from the back,
/// this is internally implemented as the number of bytes
/// from the **end** of the buffer.
///
/// It is unlikely to be useful outside the flatbuffer
/// library, unless you are trying to implement the
/// FlatbufferPrimitive trait.
#[derive(Copy, Clone, PartialEq, Eq, Debug, Hash)]
#[doc(hidden)]
pub struct RawOffset(usize);

impl RawOffset {
    #[inline]
    pub(crate) fn new(offset: usize) -> RawOffset {
        RawOffset(offset)
    }
}

unsafe impl FlatbufferPrimitive for RawOffset {
    const SIZE: usize = 4;
    const ALIGNMENT: usize = 4;

    #[inline]
    unsafe fn serialize(&self, buffer: *mut u8, offset: RawOffset) {
        // Offset implemented as bytes from the end of the file, and
        // encoded relatively. `offset` is the location we are currently
        // writing to and `self` is the value to be written. This
        // means that `offset` is going to end up earlier in the buffer,
        // i.e. with a larger offset from the end
        debug_assert!(offset.0 >= self.0);
        let diff = offset.0 - self.0; // Encode as a relative offsets by subtracting
        let value_u32 = diff as u32;

        buffer.copy_from(value_u32.to_le_bytes().as_ptr(), Self::SIZE);
    }
}

impl core::ops::Add<usize> for RawOffset {
    type Output = RawOffset;

    #[allow(clippy::suspicious_arithmetic_impl)]
    #[inline]
    fn add(self, other: usize) -> RawOffset {
        // Since the internal offset is the number of bytes from the end of
        // the buffer, we need to **subtract** here to get a later offset
        debug_assert!(self.0 > other);
        RawOffset(self.0 - other)
    }
}

/// Offset to a flatbuffer vtable
///
/// Since the flatbuffer is constructed from the back,
/// this is internaly implemented as the number of bytes
/// from the **end** of the buffer.
///
/// It is unlikely to be useful outside the flatbuffer
/// library, unless you are trying to implement the
/// Cache trait.
#[derive(Copy, Clone, PartialEq, Eq, Debug, Hash)]
#[doc(hidden)]
pub struct VtableOffset(usize);

impl VtableOffset {
    #[inline]
    pub(crate) fn new(offset: usize) -> VtableOffset {
        VtableOffset(offset)
    }
}

/// Offset to a flatbuffer object
///
/// Since the flatbuffer is constructed from the back,
/// this is internally implemented as the number of bytes
/// from the **end** of the buffer.
pub struct Offset<T: ?Sized> {
    offset: RawOffset,
    marker: PhantomData<T>,
}

unsafe impl FlatbufferPrimitive for VtableOffset {
    const SIZE: usize = 4;
    const ALIGNMENT: usize = 4;

    #[inline]
    unsafe fn serialize(&self, buffer: *mut u8, offset: RawOffset) {
        // Offset implemented as bytes from the end of the file, and
        // encoded relatively. `offset` is the location we are currently
        // writing to and `self` is the value to be written. This
        // means that `offset` is going to end up earlier in the buffer,
        // i.e. with a larger offset from the end

        // Encode as a relative offsets by subtracting. Notice that we
        // subtracting the arguments in the opposite order compared to
        // RawOffset, because vtable offsets are subtracted instead of added
        let diff = (self.0 as isize) - (offset.0 as isize);
        let value_i32 = diff as i32;

        buffer.copy_from(value_i32.to_le_bytes().as_ptr(), Self::SIZE);
    }
}

unsafe impl<T: ?Sized> FlatbufferPrimitive for Offset<T> {
    const SIZE: usize = 4;
    const ALIGNMENT: usize = 4;

    #[inline]
    unsafe fn serialize(&self, buffer: *mut u8, offset: RawOffset) {
        self.offset.serialize(buffer, offset);
    }
}

impl<T: ?Sized> Offset<T> {
    #[inline]
    pub(crate) fn new(offset: RawOffset) -> Offset<T> {
        Offset {
            offset,
            marker: PhantomData,
        }
    }

    /// Throws out the type parameter and returns the intenal raw offset
    ///
    /// This is needed because unions need to return their internal offset,
    /// but the different union types do not have the same offset types.
    #[doc(hidden)]
    #[inline]
    pub fn raw_offset(self) -> RawOffset {
        self.offset
    }
}

// Below are trait impls for Offset<T> for are in principle derivable.
// However using derive(...) to implement them would place bounds on
// the T parameter, and we want these impls to exist independently of
// the choice of T
impl<T: ?Sized> Copy for Offset<T> {}
impl<T: ?Sized> Clone for Offset<T> {
    #[inline]
    fn clone(&self) -> Self {
        *self
    }
}
impl<T: ?Sized> PartialEq for Offset<T> {
    #[inline]
    fn eq(&self, other: &Offset<T>) -> bool {
        self.offset == other.offset
    }
}
impl<T: ?Sized> Eq for Offset<T> {}
impl<T: ?Sized> core::hash::Hash for Offset<T> {
    #[inline]
    fn hash<H>(&self, hasher: &mut H)
    where
        H: core::hash::Hasher,
    {
        self.offset.hash(hasher);
    }
}
impl<T: ?Sized> core::fmt::Debug for Offset<T> {
    fn fmt(&self, formatter: &mut core::fmt::Formatter) -> core::fmt::Result {
        formatter
            .debug_struct("Offset")
            .field("offset", &self.offset)
            .field(
                "marker",
                &format_args!("PhantomData<{}>", core::any::type_name::<T>()),
            )
            .finish()
    }
}
