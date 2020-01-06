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
    serialize::{
        buffer::{Buffer, BufferAllocator},
        cache::{Cache, CacheAllocator},
        offsets::{Offset, RawOffset, VtableOffset},
        traits::{FlatbufferPrimitive, FlatbufferTable},
    },
};
use core::{borrow::Borrow, convert::TryInto, mem::MaybeUninit, num::NonZeroUsize};

pub struct FlatbufferBuilder<'a, B, C>
where
    B: Buffer<'a>,
    C: Cache<'a, 'a>,
{
    writer: Writer<B::Allocator, C::Allocator>,
    message_size: &'a mut Option<NonZeroUsize>,
}

impl<'a, B, C> FlatbufferBuilder<'a, B, C>
where
    B: Buffer<'a>,
    C: Cache<'a, 'a>,
{
    #[inline]
    pub(crate) fn new(
        message_size: &'a mut Option<NonZeroUsize>,
        allocator: B::Allocator,
        cache: C::Allocator,
    ) -> FlatbufferBuilder<'a, B, C> {
        FlatbufferBuilder {
            message_size,
            writer: Writer {
                cache,
                allocator,
                bytes_written: 0,
                current_alignment_offset: 0,
                current_alignment_mask: 3,
            },
        }
    }

    #[inline]
    pub fn create_table<T>(&mut self, table: &T) -> Result<Offset<T>, OutOfBufferSpace>
    where
        T: FlatbufferTable,
    {
        table.validate_required();
        self.create_table_no_validate(table)
    }

    #[inline]
    pub fn create_table_no_validate<T>(&mut self, table: &T) -> Result<Offset<T>, OutOfBufferSpace>
    where
        T: FlatbufferTable,
    {
        Ok(Offset::new(table.serialize(&mut self.writer)?))
    }

    #[inline]
    pub fn create_vector<T>(&mut self, slice: &[T]) -> Result<Offset<[T]>, OutOfBufferSpace>
    where
        T: FlatbufferPrimitive,
    {
        self.writer.create_vector_reverse_iter(slice.iter().rev())
    }

    #[inline]
    pub fn create_vector_iter<T, I>(&mut self, iter: I) -> Result<Offset<[T]>, OutOfBufferSpace>
    where
        T: FlatbufferPrimitive,
        I: IntoIterator,
        I::IntoIter: DoubleEndedIterator + ExactSizeIterator,
        I::Item: Borrow<T>,
    {
        self.writer
            .create_vector_reverse_iter(iter.into_iter().rev())
    }

    #[inline]
    pub fn create_vector_reverse_iter<T, I>(
        &mut self,
        iter: I,
    ) -> Result<Offset<[T]>, OutOfBufferSpace>
    where
        T: FlatbufferPrimitive,
        I: IntoIterator,
        I::IntoIter: ExactSizeIterator,
        I::Item: Borrow<T>,
    {
        self.writer.create_vector_reverse_iter(iter.into_iter())
    }

    #[inline]
    pub fn create_string(&mut self, s: &str) -> Result<Offset<str>, OutOfBufferSpace> {
        self.writer.create_string(s)
    }

    #[inline]
    pub fn finish<T>(mut self, offset: Offset<T>) -> Result<(), OutOfBufferSpace> {
        self.writer.finish(offset)?;
        *self.message_size = NonZeroUsize::new(self.writer.bytes_written);
        Ok(())
    }
}

struct Writer<Allocator, Cache> {
    allocator: Allocator,
    bytes_written: usize,
    current_alignment_offset: usize,
    current_alignment_mask: usize,
    cache: Cache,
}

impl<'allocator, A, C> Writer<A, C>
where
    A: BufferAllocator<'allocator>,
    C: CacheAllocator<'allocator>,
{
    #[inline]
    fn make_offset_at_current_position<T: ?Sized>(&self) -> Offset<T> {
        Offset::new(RawOffset::new(self.bytes_written))
    }

    #[inline]
    fn create_vector_reverse_iter<T, I>(&mut self, iter: I) -> Result<Offset<[T]>, OutOfBufferSpace>
    where
        T: FlatbufferPrimitive,
        I: Iterator + ExactSizeIterator,
        I::Item: Borrow<T>,
    {
        assert_eq!(T::SIZE % T::ALIGNMENT, 0);
        assert!(T::ALIGNMENT.is_power_of_two());

        let len: usize = iter.len();
        let len_u32: u32 = len.try_into().map_err(|_| OutOfBufferSpace)?;

        // We need the length is a u32, which needs to be aligned
        let align = T::ALIGNMENT.max(u32::ALIGNMENT);
        assert!(align.is_power_of_two());
        assert!(align <= 64);
        let align_mask = align - 1;
        let size = T::SIZE.checked_mul(len).ok_or(OutOfBufferSpace)?;
        self.align_before_write(size, align_mask)?;

        let mut count = 0;
        for item in iter {
            count += 1;
            self.write_primitive(item.borrow())?;
        }

        debug_assert_eq!(len, count);

        self.write_raw_slice(&len_u32.to_le_bytes())?;
        debug_assert_eq!(self.current_alignment_offset % align, 0);
        Ok(self.make_offset_at_current_position())
    }

    #[inline]
    fn create_string(&mut self, s: &str) -> Result<Offset<str>, OutOfBufferSpace> {
        let s = s.as_bytes();
        let len = s.len();
        let len_u32: u32 = s.len().try_into().map_err(|_| OutOfBufferSpace)?;

        // Write at least one null byte
        self.write_raw_slice(&[0])?;
        // But potentially more, because we need to have the count field be 4-byte
        // aligned
        self.align_before_write(len, 3)?;

        self.write_raw_slice(s)?;
        self.write_raw_slice(&len_u32.to_le_bytes())?;

        debug_assert_eq!(self.current_alignment_offset % 4, 0);
        Ok(self.make_offset_at_current_position())
    }

    #[inline]
    fn finish<T>(&mut self, offset: Offset<T>) -> Result<(), OutOfBufferSpace> {
        self.align_before_write(4, self.current_alignment_mask)?;
        self.write_primitive(&offset)?;
        Ok(())
    }

    #[inline]
    fn write_raw_slice(&mut self, data: &[u8]) -> Result<&'allocator [u8], OutOfBufferSpace> {
        // The allocate_uninitialized function requires that we immediately initialize
        // the slice which we do. Afterwards we cast the slice from
        // `&'allocated [MaybeUinit<u8>]` to `&'allocated [u8]`, which is also safe,
        // since we just initialized it
        unsafe {
            let slice: &'allocator mut [MaybeUninit<u8>] =
                self.allocate_uninitialized(data.len())?;

            // This can safely be removed, but we keep it to make sure that the
            // compiler optimizes out a panic. If we turn it into a assert_eq, then
            // the compiler is for some reason unable to infer this
            if slice.len() != data.len() {
                panic!("allocated slice len did not match requested len");
            }
            let ptr = slice.as_mut_ptr() as *mut u8;
            ptr.copy_from(data.as_ptr(), data.len());
            Ok(core::slice::from_raw_parts_mut(ptr, slice.len()))
        }
    }

    #[inline]
    unsafe fn allocate_uninitialized(
        &mut self,
        size: usize,
    ) -> Result<&'allocator mut [MaybeUninit<u8>], OutOfBufferSpace> {
        let new_bytes_written = self
            .bytes_written
            .checked_add(size)
            .ok_or(OutOfBufferSpace)?;
        if new_bytes_written > crate::serialize::FLATBUFFERS_MAX_BUFFER_SIZE {
            return Err(OutOfBufferSpace);
        }

        // Make sure to update the bytes written only if
        // the allocation goes okay. If we don't, then future
        // offsets are going to be wrong
        let result = self.allocator.allocate_uninitialized(size)?;

        self.bytes_written = new_bytes_written;
        self.current_alignment_offset =
            self.current_alignment_offset.wrapping_add(size) & self.current_alignment_mask;
        Ok(result)
    }
}

#[doc(hidden)]
pub trait FlatbufferWriter {
    fn write_primitive<T>(&mut self, primitive: &T) -> Result<(), OutOfBufferSpace>
    where
        T: FlatbufferPrimitive;

    fn write_vtable_and_offset(
        &mut self,
        vtable: &mut [u8],
        object_size: usize,
    ) -> Result<RawOffset, OutOfBufferSpace>;

    fn align_before_write(
        &mut self,
        size: usize,
        alignment_mask: usize,
    ) -> Result<(), OutOfBufferSpace>;

    fn write_zeros(&mut self, count: usize) -> Result<(), OutOfBufferSpace>;
}

impl<'allocator, A, C> FlatbufferWriter for Writer<A, C>
where
    A: BufferAllocator<'allocator>,
    C: CacheAllocator<'allocator>,
{
    #[inline]
    fn write_primitive<T>(&mut self, primitive: &T) -> Result<(), OutOfBufferSpace>
    where
        T: FlatbufferPrimitive,
    {
        // The allocate_uninitialized function requires that we immediately initialize
        // the slice which we do. The FlatbufferPrimitive::serialize requires
        // that the buffer is at least T::SIZE bytes large, which it is
        unsafe {
            let slice = self.allocate_uninitialized(T::SIZE)?;
            debug_assert_eq!(slice.len(), T::SIZE as usize);
            debug_assert_eq!(self.current_alignment_offset % T::ALIGNMENT, 0);
            primitive.serialize(
                slice.as_mut_ptr() as *mut u8,
                RawOffset::new(self.bytes_written),
            );
        }
        Ok(())
    }

    #[inline]
    fn write_vtable_and_offset(
        &mut self,
        vtable: &mut [u8],
        object_size: usize,
    ) -> Result<RawOffset, OutOfBufferSpace> {
        assert!(vtable.len() >= 4);
        debug_assert_eq!(vtable.len() % 2, 0);

        let vtable_len = vtable.len();
        vtable[0..2].copy_from_slice(&(vtable_len as u16).to_le_bytes());
        vtable[2..4].copy_from_slice(&(object_size as u16).to_le_bytes());

        if let Some(vtable_offset) = self.cache.get(vtable) {
            self.write_primitive(&vtable_offset)?;
            Ok(RawOffset::new(self.bytes_written))
        } else {
            // This is the offset of the vtable, after writing 4 bytes for the offset itself
            // followed by entire vtable
            let vtable_offset = VtableOffset::new(self.bytes_written + 4 + vtable_len);
            self.write_primitive(&vtable_offset)?;
            let result = RawOffset::new(self.bytes_written);
            let allocated = self.write_raw_slice(vtable)?;
            self.cache.insert(allocated, vtable_offset);
            Ok(result)
        }
    }

    #[inline]
    fn align_before_write(
        &mut self,
        size: usize,
        alignment_mask: usize,
    ) -> Result<(), OutOfBufferSpace> {
        debug_assert!(self.current_alignment_mask <= 63);
        debug_assert!((self.current_alignment_mask + 1).is_power_of_two());
        debug_assert!(alignment_mask <= 63);
        debug_assert!((alignment_mask + 1).is_power_of_two());
        debug_assert!(
            self.current_alignment_offset & self.current_alignment_mask
                == self.current_alignment_offset
        );

        let min_alignment_mask = self.current_alignment_mask.min(alignment_mask);
        let needed_zeros = 0usize
            .wrapping_sub(size)
            .wrapping_sub(self.current_alignment_offset)
            & min_alignment_mask;

        debug_assert_eq!(
            self.current_alignment_offset
                .wrapping_add(needed_zeros)
                .wrapping_add(size)
                & min_alignment_mask,
            0
        );

        self.write_zeros(needed_zeros)?;

        if alignment_mask > self.current_alignment_mask {
            self.current_alignment_offset = 0usize.wrapping_sub(size) & alignment_mask;
            self.current_alignment_mask = alignment_mask;
        }
        Ok(())
    }

    #[inline]
    fn write_zeros(&mut self, count: usize) -> Result<(), OutOfBufferSpace> {
        // The allocate_uninitialized function requires that we immediately initialize
        // the slice which we do
        unsafe {
            let slice: &'allocator mut [MaybeUninit<u8>] = self.allocate_uninitialized(count)?;

            // This can safely be removed, but we keep it to make sure that the
            // compiler optimizes out a panic. If we turn it into a assert_eq, then
            // the compiler is for some reason unable to infer this
            if slice.len() != count {
                panic!("allocated slice len did not match requested len");
            }
            let ptr = slice.as_mut_ptr() as *mut u8;
            ptr.write_bytes(0, slice.len());
        }
        Ok(())
    }
}
