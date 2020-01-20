// Copyright 2019 Google LLC
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

use crate::bitwidth::BitWidth;
use crate::flexbuffer_type::FlexBufferType;
use crate::Blob;
use std::convert::{TryFrom, TryInto};
use std::fmt;
use std::ops::Rem;
use std::str::FromStr;
mod de;
mod iter;
mod map;
mod vector;
pub use de::DeserializationError;
pub use iter::ReaderIterator;
pub use map::{MapReader, MapReaderIndexer};
pub use vector::VectorReader;

/// All the possible errors when reading a flexbuffer.
#[derive(Debug, PartialEq, Eq, Clone, Copy)]
pub enum Error {
    /// One of the following data errors occured:
    ///
    /// *    The read flexbuffer had an offset that pointed outside the flexbuffer.
    /// *    The 'negative indicies' where length and map keys are stored were out of bounds
    /// *    The buffer was too small to contain a flexbuffer root.
    FlexbufferOutOfBounds,
    /// Failed to parse a valid FlexbufferType and Bitwidth from a type byte.
    InvalidPackedType,
    /// Flexbuffer type of the read data does not match function used.
    UnexpectedFlexbufferType {
        expected: FlexBufferType,
        actual: FlexBufferType,
    },
    /// BitWidth type of the read data does not match function used.
    UnexpectedBitWidth {
        expected: BitWidth,
        actual: BitWidth,
    },
    /// Read a flexbuffer offset or length that overflowed usize.
    ReadUsizeOverflowed,
    /// Tried to index a type that's not one of the Flexbuffer vector types.
    CannotIndexAsVector,
    /// Tried to index a Flexbuffer vector or map out of bounds.
    IndexOutOfBounds,
    /// A Map was indexed with a key that it did not contain.
    KeyNotFound,
    /// Failed to parse a Utf8 string.
    Utf8Error,
    /// get_slice failed because the given data buffer is misaligned.
    AlignmentError,
    InvalidRootWidth,
    InvalidMapKeysVectorWidth,
}

pub trait ReadLE: crate::private::Sealed + std::marker::Sized {
    const VECTOR_TYPE: FlexBufferType;
    const WIDTH: BitWidth;
}
macro_rules! rle {
    ($T: ty, $VECTOR_TYPE: ident, $WIDTH: ident) => {
        impl ReadLE for $T {
            const VECTOR_TYPE: FlexBufferType = FlexBufferType::$VECTOR_TYPE;
            const WIDTH: BitWidth = BitWidth::$WIDTH;
        }
    };
}
rle!(u8, VectorUInt, W8);
rle!(u16, VectorUInt, W16);
rle!(u32, VectorUInt, W32);
rle!(u64, VectorUInt, W64);
rle!(i8, VectorInt, W8);
rle!(i16, VectorInt, W16);
rle!(i32, VectorInt, W32);
rle!(i64, VectorInt, W64);
rle!(f32, VectorFloat, W32);
rle!(f64, VectorFloat, W64);

macro_rules! as_default {
    ($as: ident, $get: ident, $T: ty) => {
        pub fn $as(&self) -> $T {
            self.$get().unwrap_or_default()
        }
    };
}

/// Lazily parses a flexbuffer, one value at a time. Start a reader with
/// `get_root`. The `get_T` methods succeeds if and only if both
/// the flexbuffer type and bitwidth match `T`. The `as_T` methods will try
/// their best to return a value of type `T` (by casting or even parsing a
/// string if necessary) but ultimately returns `T::default` if it fails.
#[derive(DebugStub, Default, Clone)]
pub struct Reader<'de> {
    fxb_type: FlexBufferType,
    width: BitWidth,
    address: usize,
    #[debug_stub = "&[..]"]
    buffer: &'de [u8],
}

macro_rules! try_cast_fn {
    ($name: ident, $full_width: ident, $Ty: ident) => {
        pub fn $name(&self) -> $Ty {
            self.$full_width().try_into().unwrap_or_default()
        }
    }
}

fn safe_sub(a: usize, b: usize) -> Result<usize, Error> {
    a.checked_sub(b).ok_or(Error::FlexbufferOutOfBounds)
}

fn deref_offset(buffer: &[u8], address: usize, width: BitWidth) -> Result<usize, Error> {
    let off = read_usize(buffer, address, width);
    safe_sub(address, off)
}

impl<'de> Reader<'de> {
    fn new(
        buffer: &'de [u8],
        mut address: usize,
        mut fxb_type: FlexBufferType,
        width: BitWidth,
        parent_width: BitWidth,
    ) -> Result<Self, Error> {
        if fxb_type.is_reference() {
            address = deref_offset(buffer, address, parent_width)?;
            // Indirects were dereferenced.
            if let Some(t) = fxb_type.to_direct() {
                fxb_type = t;
            }
        }
        Ok(Reader {
            address,
            fxb_type,
            width,
            buffer,
        })
    }
    /// Parses the flexbuffer from the given buffer. Assumes the flexbuffer root is the last byte
    /// of the buffer.
    pub fn get_root(buffer: &'de [u8]) -> Result<Self, Error> {
        let end = buffer.len();
        if end < 3 {
            return Err(Error::FlexbufferOutOfBounds);
        }
        // Last byte is the root width.
        let root_width = BitWidth::from_nbytes(buffer[end - 1]).ok_or(Error::InvalidRootWidth)?;
        // Second last byte is root type.
        let (fxb_type, width) = unpack_type(buffer[end - 2])?;
        // Location of root data. (BitWidth bits before root type)
        let address = safe_sub(end - 2, root_width.n_bytes())?;
        Self::new(buffer, address, fxb_type, width, root_width)
    }
    /// Returns the FlexBufferType of this Reader.
    pub fn flexbuffer_type(&self) -> FlexBufferType {
        self.fxb_type
    }
    /// Returns the bitwidth of this Reader.
    pub fn bitwidth(&self) -> BitWidth {
        self.width
    }
    /// Returns the length of the Flexbuffer. If the type has no length, or if an error occurs,
    /// 0 is returned.
    pub fn length(&self) -> usize {
        if let Some(len) = self.fxb_type.fixed_length_vector_length() {
            len
        } else if self.fxb_type.has_length_slot() && self.address >= self.width.n_bytes() {
            read_usize(self.buffer, self.address - self.width.n_bytes(), self.width)
        } else {
            0
        }
    }
    /// Returns true if the flexbuffer is aligned to 8 bytes. This guarantees, for valid
    /// flexbuffers, that the data is correctly aligned in memory and slices can be read directly
    /// e.g. with `get_f64s` or `get_i16s`.
    pub fn is_aligned(&self) -> bool {
        (self.buffer.as_ptr() as usize).rem(8) == 0
    }
    as_default!(as_vector, get_vector, VectorReader<'de>);
    as_default!(as_map, get_map, MapReader<'de>);

    fn expect_type(&self, ty: FlexBufferType) -> Result<(), Error> {
        if self.fxb_type == ty {
            Ok(())
        } else {
            Err(Error::UnexpectedFlexbufferType {
                expected: ty,
                actual: self.fxb_type,
            })
        }
    }
    fn expect_bw(&self, bw: BitWidth) -> Result<(), Error> {
        if self.width == bw {
            Ok(())
        } else {
            Err(Error::UnexpectedBitWidth {
                expected: bw,
                actual: self.width,
            })
        }
    }
    /// Directly reads a slice of type `T`where `T` is one of `u8,u16,u32,u64,i8,i16,i32,i64,f32,f64`.
    /// Returns Err if the type, bitwidth, or memory alignment does not match. Since the bitwidth is
    /// dynamic, its better to use a VectorReader unless you know your data and performance is critical.
    #[cfg(target_endian = "little")]
    pub fn get_slice<T: ReadLE>(&self) -> Result<&'de [T], Error> {
        if self.flexbuffer_type().typed_vector_type() != T::VECTOR_TYPE.typed_vector_type() {
            self.expect_type(T::VECTOR_TYPE)?;
        }
        if self.bitwidth().n_bytes() != std::mem::size_of::<T>() {
            self.expect_bw(T::WIDTH)?;
        }
        let end = self.address + self.length() * std::mem::size_of::<T>();
        let slice = &self
            .buffer
            .get(self.address..end)
            .ok_or(Error::FlexbufferOutOfBounds)?;
        let (pre, mid, suf) = unsafe { slice.align_to::<T>() };
        if pre.is_empty() && suf.is_empty() {
            Ok(mid)
        } else {
            Err(Error::AlignmentError)
        }
    }

    pub fn get_bool(&self) -> Result<bool, Error> {
        self.expect_type(FlexBufferType::Bool)?;
        Ok(
            self.buffer[self.address..self.address + self.width.n_bytes()]
                .iter()
                .any(|&b| b != 0),
        )
    }
    pub fn get_key(&self) -> Result<&'de str, Error> {
        self.expect_type(FlexBufferType::Key)?;
        let (length, _) = self.buffer[self.address..]
            .iter()
            .enumerate()
            .find(|(_, &b)| b == b'\0')
            .unwrap_or((0, &0));
        std::str::from_utf8(&self.buffer[self.address..self.address + length])
            .map_err(|_| Error::Utf8Error)
    }
    pub fn get_blob(&self) -> Result<Blob<'de>, Error> {
        self.expect_type(FlexBufferType::Blob)?;
        Ok(Blob(
            &self.buffer[self.address..self.address + self.length()],
        ))
    }
    pub fn get_str(&self) -> Result<&'de str, Error> {
        self.expect_type(FlexBufferType::String)?;
        std::str::from_utf8(&self.buffer[self.address..self.address + self.length()])
            .map_err(|_| Error::Utf8Error)
    }
    fn get_map_info(&self) -> Result<(usize, BitWidth), Error> {
        self.expect_type(FlexBufferType::Map)?;
        if 3 * self.width.n_bytes() >= self.address {
            return Err(Error::FlexbufferOutOfBounds);
        }
        let keys_offset_address = self.address - 3 * self.width.n_bytes();
        let keys_width = {
            let kw_addr = self.address - 2 * self.width.n_bytes();
            let kw = read_usize(self.buffer, kw_addr, self.width);
            BitWidth::from_nbytes(kw).ok_or(Error::InvalidMapKeysVectorWidth)
        }?;
        Ok((keys_offset_address, keys_width))
    }
    pub fn get_map(&self) -> Result<MapReader<'de>, Error> {
        let (keys_offset_address, keys_width) = self.get_map_info()?;
        let keys_address = deref_offset(self.buffer, keys_offset_address, self.width)?;
        // TODO(cneo): Check that vectors length equals keys length.
        Ok(MapReader {
            buffer: self.buffer,
            values_address: self.address,
            values_width: self.width,
            keys_address,
            keys_width,
            length: self.length(),
        })
    }
    /// Tries to read a FlexBufferType::UInt. Returns Err if the type is not a UInt or if the
    /// address is out of bounds.
    pub fn get_u64(&self) -> Result<u64, Error> {
        self.expect_type(FlexBufferType::UInt)?;
        let cursor = self
            .buffer
            .get(self.address..self.address + self.width.n_bytes());
        match self.width {
            BitWidth::W8 => cursor.map(|s| s[0] as u8).map(Into::into),
            BitWidth::W16 => cursor
                .and_then(|s| s.try_into().ok())
                .map(<u16>::from_le_bytes)
                .map(Into::into),
            BitWidth::W32 => cursor
                .and_then(|s| s.try_into().ok())
                .map(<u32>::from_le_bytes)
                .map(Into::into),
            BitWidth::W64 => cursor
                .and_then(|s| s.try_into().ok())
                .map(<u64>::from_le_bytes),
        }
        .ok_or(Error::FlexbufferOutOfBounds)
    }
    /// Tries to read a FlexBufferType::Int. Returns Err if the type is not a UInt or if the
    /// address is out of bounds.
    pub fn get_i64(&self) -> Result<i64, Error> {
        self.expect_type(FlexBufferType::Int)?;
        let cursor = self
            .buffer
            .get(self.address..self.address + self.width.n_bytes());
        match self.width {
            BitWidth::W8 => cursor.map(|s| s[0] as i8).map(Into::into),
            BitWidth::W16 => cursor
                .and_then(|s| s.try_into().ok())
                .map(<i16>::from_le_bytes)
                .map(Into::into),
            BitWidth::W32 => cursor
                .and_then(|s| s.try_into().ok())
                .map(<i32>::from_le_bytes)
                .map(Into::into),
            BitWidth::W64 => cursor
                .and_then(|s| s.try_into().ok())
                .map(<i64>::from_le_bytes),
        }
        .ok_or(Error::FlexbufferOutOfBounds)
    }
    /// Tries to read a FlexBufferType::Float. Returns Err if the type is not a UInt, if the
    /// address is out of bounds, or if its a f16 or f8 (not currently supported).
    pub fn get_f64(&self) -> Result<f64, Error> {
        self.expect_type(FlexBufferType::Float)?;
        let cursor = self
            .buffer
            .get(self.address..self.address + self.width.n_bytes());
        match self.width {
            BitWidth::W8 | BitWidth::W16 => return Err(Error::InvalidPackedType),
            BitWidth::W32 => cursor
                .and_then(|s| s.try_into().ok())
                .map(<f32>::from_le_bytes)
                .map(Into::into),
            BitWidth::W64 => cursor
                .and_then(|s| s.try_into().ok())
                .map(<f64>::from_le_bytes),
        }
        .ok_or(Error::FlexbufferOutOfBounds)
    }
    pub fn as_bool(&self) -> bool {
        use FlexBufferType::*;
        match self.fxb_type {
            Bool => self.get_bool().unwrap_or_default(),
            UInt => self.as_u64() != 0,
            Int => self.as_i64() != 0,
            Float => self.as_f64().abs() > std::f64::EPSILON,
            String | Key => !self.as_str().is_empty(),
            Null => false,
            Blob => self.length() != 0,
            ty if ty.is_vector() => self.length() != 0,
            _ => unreachable!(),
        }
    }
    /// Returns a u64, casting if necessary. For Maps and Vectors, their length is
    /// returned. If anything fails, 0 is returned.
    pub fn as_u64(&self) -> u64 {
        match self.fxb_type {
            FlexBufferType::UInt => self.get_u64().unwrap_or_default(),
            FlexBufferType::Int => self
                .get_i64()
                .unwrap_or_default()
                .try_into()
                .unwrap_or_default(),
            FlexBufferType::Float => self.get_f64().unwrap_or_default() as u64,
            FlexBufferType::String => {
                if let Ok(s) = self.get_str() {
                    if let Ok(f) = u64::from_str(s) {
                        return f;
                    }
                }
                0
            }
            _ if self.fxb_type.is_vector() => self.length() as u64,
            _ => 0,
        }
    }
    try_cast_fn!(as_u32, as_u64, u32);
    try_cast_fn!(as_u16, as_u64, u16);
    try_cast_fn!(as_u8, as_u64, u8);

    /// Returns an i64, casting if necessary. For Maps and Vectors, their length is
    /// returned. If anything fails, 0 is returned.
    pub fn as_i64(&self) -> i64 {
        match self.fxb_type {
            FlexBufferType::Int => self.get_i64().unwrap_or_default(),
            FlexBufferType::UInt => self
                .get_u64()
                .unwrap_or_default()
                .try_into()
                .unwrap_or_default(),
            FlexBufferType::Float => self.get_f64().unwrap_or_default() as i64,
            FlexBufferType::String => {
                if let Ok(s) = self.get_str() {
                    if let Ok(f) = i64::from_str(s) {
                        return f;
                    }
                }
                0
            }
            _ if self.fxb_type.is_vector() => self.length() as i64,
            _ => 0,
        }
    }
    try_cast_fn!(as_i32, as_i64, i32);
    try_cast_fn!(as_i16, as_i64, i16);
    try_cast_fn!(as_i8, as_i64, i8);

    /// Returns an f64, casting if necessary. For Maps and Vectors, their length is
    /// returned. If anything fails, 0 is returned.
    pub fn as_f64(&self) -> f64 {
        match self.fxb_type {
            FlexBufferType::Int => self.get_i64().unwrap_or_default() as f64,
            FlexBufferType::UInt => self.get_u64().unwrap_or_default() as f64,
            FlexBufferType::Float => self.get_f64().unwrap_or_default(),
            FlexBufferType::String => {
                if let Ok(s) = self.get_str() {
                    if let Ok(f) = f64::from_str(s) {
                        return f;
                    }
                }
                0.0
            }
            _ if self.fxb_type.is_vector() => self.length() as f64,
            _ => 0.0,
        }
    }
    pub fn as_f32(&self) -> f32 {
        self.as_f64() as f32
    }

    /// Returns empty string if you're not trying to read a string.
    pub fn as_str(&self) -> &'de str {
        match self.fxb_type {
            FlexBufferType::String => self.get_str().unwrap_or_default(),
            FlexBufferType::Key => self.get_key().unwrap_or_default(),
            _ => "",
        }
    }
    pub fn get_vector(&self) -> Result<VectorReader<'de>, Error> {
        if !self.fxb_type.is_vector() {
            self.expect_type(FlexBufferType::Vector)?;
        };
        Ok(VectorReader {
            reader: self.clone(),
            length: self.length(),
        })
    }
}

impl<'de> fmt::Display for Reader<'de> {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        use FlexBufferType::*;
        match self.flexbuffer_type() {
            Null => write!(f, "null"),
            UInt => write!(f, "{}", self.as_u64()),
            Int => write!(f, "{}", self.as_i64()),
            Float => write!(f, "{}", self.as_f64()),
            Key | String => write!(f, "{:?}", self.as_str()),
            Bool => write!(f, "{}", self.as_bool()),
            Blob => write!(f, "blob"),
            Map => {
                write!(f, "{{")?;
                let m = self.as_map();
                let mut pairs = m.iter_keys().zip(m.iter_values());
                if let Some((k, v)) = pairs.next() {
                    write!(f, "{:?}: {}", k, v)?;
                    for (k, v) in pairs {
                        write!(f, ", {:?}: {}", k, v)?;
                    }
                }
                write!(f, "}}")
            }
            t if t.is_vector() => {
                write!(f, "[")?;
                let mut elems = self.as_vector().iter();
                if let Some(first) = elems.next() {
                    write!(f, "{}", first)?;
                    for e in elems {
                        write!(f, ", {}", e)?;
                    }
                }
                write!(f, "]")
            }
            _ => unreachable!("Display not implemented for {:?}", self),
        }
    }
}

fn read_usize(buffer: &[u8], address: usize, width: BitWidth) -> usize {
    let cursor = &buffer[address..];
    match width {
        BitWidth::W8 => cursor[0] as usize,
        BitWidth::W16 => cursor
            .get(0..2)
            .and_then(|s| s.try_into().ok())
            .map(<u16>::from_le_bytes)
            .unwrap_or_default() as usize,
        BitWidth::W32 => cursor
            .get(0..4)
            .and_then(|s| s.try_into().ok())
            .map(<u32>::from_le_bytes)
            .unwrap_or_default() as usize,
        BitWidth::W64 => cursor
            .get(0..8)
            .and_then(|s| s.try_into().ok())
            .map(<u64>::from_le_bytes)
            .unwrap_or_default() as usize,
    }
}

fn unpack_type(ty: u8) -> Result<(FlexBufferType, BitWidth), Error> {
    let w = BitWidth::try_from(ty & 3u8).map_err(|_| Error::InvalidPackedType)?;
    let t = FlexBufferType::try_from(ty >> 2).map_err(|_| Error::InvalidPackedType)?;
    Ok((t, w))
}

#[cfg(test)]
mod tests {
    use super::*;
    use crate::Builder;
    use quickcheck::QuickCheck;

    #[test]
    fn qc_reader_no_crash() {
        fn no_crash(xs: Vec<u8>) -> bool {
            let r = Reader::get_root(&xs);
            r.is_err() || r.is_ok()
        }
        QuickCheck::new()
            .min_tests_passed(10_000_000)
            .quicktest(no_crash as fn(Vec<u8>) -> bool)
            .unwrap();

        no_crash(vec![0, 10 << 2 | 2, 0]);
    }
    #[test]
    fn as_num() {
        let mut fxb = Builder::default();
        let mut m = fxb.start_map();
        m.push("a", &[-1i8, -2, -3, -4]);
        m.push("b", 250i64);
        m.push("c", 5000u16);
        m.end_map();

        let r = Reader::get_root(fxb.view()).unwrap();
        assert_eq!(r.as_i8(), 3); // length.
        assert_eq!(r.as_i16(), 3);
        assert_eq!(r.as_i32(), 3);
        assert_eq!(r.as_i64(), 3);
        assert_eq!(r.as_u8(), 3);
        assert_eq!(r.as_u16(), 3);
        assert_eq!(r.as_u32(), 3);
        assert_eq!(r.as_u64(), 3);
        assert_eq!(r.as_f32(), 3.0);
        assert_eq!(r.as_f64(), 3.0);

        let m = r.as_map();
        let a = m.index("a").unwrap();
        assert_eq!(a.as_f32(), 4.0); // length.
        assert_eq!(a.as_f64(), 4.0); // length.
        assert_eq!(a.as_vector().idx(0).as_i8(), -1);
        assert_eq!(a.as_vector().idx(1).as_i16(), -2);
        assert_eq!(a.as_vector().idx(2).as_i32(), -3);
        assert_eq!(a.as_vector().idx(3).as_i64(), -4);

        let b = m.index("b").unwrap();
        assert_eq!(b.as_u8(), 250);
        assert_eq!(b.as_u16(), 250);
        assert_eq!(b.as_u32(), 250);
        assert_eq!(b.as_u64(), 250);
        assert_eq!(b.as_i8(), 0); // overflow
        assert_eq!(b.as_i16(), 250);
        assert_eq!(b.as_i32(), 250);
        assert_eq!(b.as_i64(), 250);

        let c = m.index("c").unwrap();
        assert_eq!(c.as_i64(), 5000);
        assert_eq!(c.as_u64(), 5000);
        assert_eq!(c.as_f32(), 5000.0);
        assert_eq!(c.as_u8(), 0); // overflow
        assert_eq!(c.as_u16(), 5000);
        assert_eq!(c.as_u32(), 5000);
        assert_eq!(c.as_u64(), 5000);
        assert_eq!(c.as_i8(), 0); // overflow
        assert_eq!(c.as_i16(), 5000);
        assert_eq!(c.as_i32(), 5000);
        assert_eq!(c.as_i64(), 5000);
    }
    #[test]
    fn string_as_num() {
        let mut fxb = Builder::default();
        let mut v = fxb.start_vector();
        v.push("3.1415");
        v.push("9.001e3");
        v.push("42");
        v.end_vector();
        let r = Reader::get_root(fxb.view()).unwrap();

        let v0 = r.as_vector().idx(0);
        assert_eq!(v0.as_f64(), 3.1415);
        assert_eq!(v0.as_f32(), 3.1415);
        assert_eq!(v0.as_u8(), 0);
        assert_eq!(v0.as_u16(), 0);
        assert_eq!(v0.as_u32(), 0);
        assert_eq!(v0.as_u64(), 0);
        assert_eq!(v0.as_i8(), 0);
        assert_eq!(v0.as_i16(), 0);
        assert_eq!(v0.as_i32(), 0);
        assert_eq!(v0.as_i64(), 0);

        let v1 = r.as_vector().idx(1);
        assert_eq!(v1.as_f64(), 9001.0);
        assert_eq!(v1.as_f32(), 9001.0);
        assert_eq!(v1.as_u8(), 0);
        assert_eq!(v1.as_u16(), 0);
        assert_eq!(v1.as_u32(), 0);
        assert_eq!(v1.as_u64(), 0);
        assert_eq!(v1.as_i8(), 0);
        assert_eq!(v1.as_i16(), 0);
        assert_eq!(v1.as_i32(), 0);
        assert_eq!(v1.as_i64(), 0);
        assert_eq!(v1.as_i32(), 0);

        let v2 = r.as_vector().idx(2);
        assert_eq!(v2.as_f64(), 42.0);
        assert_eq!(v2.as_f32(), 42.0);
        assert_eq!(v2.as_u8(), 42);
        assert_eq!(v2.as_u16(), 42);
        assert_eq!(v2.as_u32(), 42);
        assert_eq!(v2.as_u64(), 42);
        assert_eq!(v2.as_i8(), 42);
        assert_eq!(v2.as_i16(), 42);
        assert_eq!(v2.as_i32(), 42);
        assert_eq!(v2.as_i64(), 42);
        assert_eq!(v2.as_i32(), 42);
    }
    #[test]
    fn null_reader() {
        let n = Reader::default();
        assert_eq!(n.as_i8(), 0);
        assert_eq!(n.as_i16(), 0);
        assert_eq!(n.as_i32(), 0);
        assert_eq!(n.as_i64(), 0);
        assert_eq!(n.as_u8(), 0);
        assert_eq!(n.as_u16(), 0);
        assert_eq!(n.as_u32(), 0);
        assert_eq!(n.as_u64(), 0);
        assert_eq!(n.as_f32(), 0.0);
        assert_eq!(n.as_f64(), 0.0);
        assert!(n.get_i64().is_err());
        assert!(n.get_u64().is_err());
        assert!(n.get_f64().is_err());
        assert!(n.as_vector().is_empty());
        assert!(n.as_map().is_empty());
        assert_eq!(n.as_vector().idx(1).flexbuffer_type(), FlexBufferType::Null);
        assert_eq!(n.as_map().idx("1").flexbuffer_type(), FlexBufferType::Null);
    }
    #[test]
    fn get_root_deref_oob() {
        let s = &[
            4, // Deref out of bounds
            (FlexBufferType::Vector as u8) << 2 | BitWidth::W8 as u8,
            1,
        ];
        assert!(Reader::get_root(s).is_err());
    }
    #[test]
    fn get_root_deref_u64() {
        let s = &[
            0,
            0,
            (FlexBufferType::IndirectUInt as u8) << 2 | BitWidth::W64 as u8,
            1,
        ];
        // The risk of crashing is reading 8 bytes from index 0.
        assert_eq!(Reader::get_root(s).unwrap().as_u64(), 0);
    }
}
