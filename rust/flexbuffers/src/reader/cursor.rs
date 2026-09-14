// Copyright 2026 Google LLC
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     https://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

use super::Error;
use crate::{BitWidth, Buffer};
use byteorder::{ByteOrder, LittleEndian};
use std::cmp::Ordering;
use std::convert::TryFrom;
use std::fmt;
use std::ops::Range;

/// An opaque, bounds-checked position in a FlexBuffer.
#[derive(Clone, Copy)]
pub(super) struct BufferAddress(usize);

impl fmt::Debug for BufferAddress {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        self.0.fmt(f)
    }
}

/// Owns a buffer handle and the only address from which reader code may navigate.
pub(super) struct BufferCursor<B> {
    buffer: B,
    address: BufferAddress,
}

impl<B> BufferCursor<B> {
    pub(super) fn address(&self) -> BufferAddress {
        self.address
    }
}

impl<B: Buffer> Clone for BufferCursor<B> {
    fn clone(&self) -> Self {
        Self {
            buffer: self.buffer.shallow_copy(),
            address: self.address,
        }
    }
}

impl<B: Buffer> Default for BufferCursor<B> {
    fn default() -> Self {
        Self {
            buffer: B::empty(),
            address: BufferAddress(0),
        }
    }
}

impl<B: Buffer> BufferCursor<B> {
    pub(super) fn new(buffer: B, address: usize) -> Result<Self, Error> {
        if address <= buffer.len() {
            Ok(Self {
                buffer,
                address: BufferAddress(address),
            })
        } else {
            Err(Error::FlexbufferOutOfBounds)
        }
    }

    pub(super) fn at(&self, address: BufferAddress) -> Result<Self, Error> {
        Self::new(self.buffer.shallow_copy(), address.0)
    }

    pub(super) fn buffer(&self) -> B {
        self.buffer.shallow_copy()
    }

    pub(super) fn buffer_ptr(&self) -> *const u8 {
        self.buffer.as_ptr()
    }

    pub(super) fn add(&self, offset: usize) -> Result<Self, Error> {
        let address = self
            .address
            .0
            .checked_add(offset)
            .ok_or(Error::FlexbufferOutOfBounds)?;
        Self::new(self.buffer.shallow_copy(), address)
    }

    pub(super) fn sub(&self, offset: usize) -> Result<Self, Error> {
        let address = self
            .address
            .0
            .checked_sub(offset)
            .ok_or(Error::FlexbufferOutOfBounds)?;
        Self::new(self.buffer.shallow_copy(), address)
    }

    pub(super) fn index(&self, index: usize, stride: usize) -> Result<Self, Error> {
        let offset = index
            .checked_mul(stride)
            .ok_or(Error::FlexbufferOutOfBounds)?;
        self.add(offset)
    }

    fn range(&self, len: usize) -> Result<Range<usize>, Error> {
        let end = self
            .address
            .0
            .checked_add(len)
            .ok_or(Error::FlexbufferOutOfBounds)?;
        if end <= self.buffer.len() {
            Ok(self.address.0..end)
        } else {
            Err(Error::FlexbufferOutOfBounds)
        }
    }

    pub(super) fn bytes(&self, len: usize) -> Result<&[u8], Error> {
        self.buffer
            .get(self.range(len)?)
            .ok_or(Error::FlexbufferOutOfBounds)
    }

    pub(super) fn read_u8(&self) -> Result<u8, Error> {
        self.buffer
            .get(self.address.0)
            .copied()
            .ok_or(Error::FlexbufferOutOfBounds)
    }

    pub(super) fn slice(&self, len: usize) -> Result<B, Error> {
        self.buffer
            .slice(self.range(len)?)
            .ok_or(Error::FlexbufferOutOfBounds)
    }

    pub(super) fn nul_terminated_len(&self) -> Result<usize, Error> {
        self.buffer
            .get(self.address.0..)
            .and_then(|bytes| bytes.iter().position(|&byte| byte == b'\0'))
            .ok_or(Error::FlexbufferOutOfBounds)
    }

    pub(super) fn compare_nul_terminated(&self, value: &[u8]) -> Result<Ordering, Error> {
        let len = self.nul_terminated_len()?;
        Ok(self.bytes(len)?.iter().cmp(value.iter()))
    }

    pub(super) fn read_usize(&self, width: BitWidth) -> Result<usize, Error> {
        usize::try_from(self.read_u64(width)?).map_err(|_| Error::ReadUsizeOverflowed)
    }

    pub(super) fn read_u64(&self, width: BitWidth) -> Result<u64, Error> {
        match width {
            BitWidth::W8 => Ok(self.read_u8()? as u64),
            BitWidth::W16 => Ok(LittleEndian::read_u16(self.bytes(2)?) as u64),
            BitWidth::W32 => Ok(LittleEndian::read_u32(self.bytes(4)?) as u64),
            BitWidth::W64 => Ok(LittleEndian::read_u64(self.bytes(8)?)),
        }
    }

    pub(super) fn read_i64(&self, width: BitWidth) -> Result<i64, Error> {
        match width {
            BitWidth::W8 => Ok(self.read_u8()? as i8 as i64),
            BitWidth::W16 => Ok(LittleEndian::read_i16(self.bytes(2)?) as i64),
            BitWidth::W32 => Ok(LittleEndian::read_i32(self.bytes(4)?) as i64),
            BitWidth::W64 => Ok(LittleEndian::read_i64(self.bytes(8)?)),
        }
    }

    pub(super) fn read_f64(&self, width: BitWidth) -> Result<f64, Error> {
        match width {
            BitWidth::W8 | BitWidth::W16 => Err(Error::InvalidPackedType),
            BitWidth::W32 => Ok(LittleEndian::read_f32(self.bytes(4)?) as f64),
            BitWidth::W64 => Ok(LittleEndian::read_f64(self.bytes(8)?)),
        }
    }

    pub(super) fn deref_offset(&self, width: BitWidth) -> Result<Self, Error> {
        self.sub(self.read_usize(width)?)
    }
}
