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
use byteorder::{ByteOrder, LittleEndian};
use std::convert::{From, TryFrom, TryInto};
use std::ops::Rem;
use std::str::FromStr;
mod de;
mod iter;
mod map;
mod vector;
pub use iter::ReaderIterator;
pub use map::{MapReader, MapReaderIndexer};
pub use vector::VectorReader;

/// All the possible errors when reading a flexbuffer.
#[derive(Debug, PartialEq, Eq)]
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
    Utf8Error(std::str::Utf8Error),
    /// Catch all for Serde errors.
    Serde(String),
    /// Catch all for std::io Errors.
    IO(String),
    /// get_slice failed because the given data buffer is misaligned.
    AlignmentError,
}
impl From<std::io::Error> for Error {
    fn from(err: std::io::Error) -> Self {
        Error::IO(format!("IO error: {}", err))
    }
}
impl std::fmt::Display for Error {
    fn fmt(&self, _: &mut std::fmt::Formatter) -> Result<(), std::fmt::Error> {
        unimplemented!()
    }
}
impl std::error::Error for Error {}
impl serde::de::Error for Error {
    fn custom<T>(msg: T) -> Self
    where
        T: std::fmt::Display,
    {
        Error::Serde(format!("Serde error: {}", msg))
    }
}

// TODO(cneo): Either to do this trait or use byteorder reader functions consistently.
pub trait ReadLE: crate::private::Sealed {
    fn read_le(sl: &[u8]) -> Self;
}
macro_rules! rle {
    ($T: ty, $read: path) => {
        impl ReadLE for $T {
            fn read_le(sl: &[u8]) -> Self {
                $read(sl)
            }
        }
    };
    ($T: ty) => {
        impl ReadLE for $T {
            fn read_le(sl: &[u8]) -> Self {
                sl[0] as Self
            }
        }
    };
}
rle!(u8);
rle!(u16, LittleEndian::read_u16);
rle!(u32, LittleEndian::read_u32);
rle!(u64, LittleEndian::read_u64);
rle!(i8);
rle!(i16, LittleEndian::read_i16);
rle!(i32, LittleEndian::read_i32);
rle!(i64, LittleEndian::read_i64);
rle!(f32, LittleEndian::read_f32);
rle!(f64, LittleEndian::read_f64);

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

macro_rules! get_fn {
    ($get_ty: ident, $Ty:ident, $read: ident, $FTy: ident, W8) => {
        #[inline(always)]
        pub fn $get_ty(&self) -> Result<$Ty, Error> {
            self.expect_type(FlexBufferType::$FTy)?;
            self.expect_bw(BitWidth::W8)?;
            Ok(self.buffer[self.address] as $Ty)
        }
    };

    ($get_ty: ident, $Ty:ident, $FTy: ident, $Width: ident) => {
        #[inline(always)]
        pub fn $get_ty(&self) -> Result<$Ty, Error> {
            self.expect_type(FlexBufferType::$FTy)?;
            self.expect_bw(BitWidth::$Width)?;
            Ok(<$Ty>::read_le(&self.buffer[self.address..]))
        }
    };
    ($get_tys: ident, [$Ty: ty], $Width: ident, $FTy: ident, $VecTy: ident) => {
        pub fn $get_tys(&self) -> Result<&'de [$Ty], Error> {
            if let Some(FlexBufferType::$FTy) = self.fxb_type.typed_vector_type() {
                self.expect_bw(BitWidth::$Width)?;
                let end = self.address + self.length() * self.width.n_bytes();
                let slice = &self.buffer[self.address..end];
                let (pre, mid, suf) = unsafe { slice.align_to::<$Ty>() };
                if pre.is_empty() && suf.is_empty() {
                    Ok(mid)
                } else {
                    Err(Error::AlignmentError)
                }
            } else {
                Err(self.expect_type(FlexBufferType::$VecTy).unwrap_err())
            }
        }
    };
}

fn safe_sub(a: usize, b: usize) -> Result<usize, Error> {
    a.checked_sub(b).ok_or(Error::FlexbufferOutOfBounds)
}

fn deref_offset(buffer: &[u8], address: usize, width: BitWidth) -> Result<usize, Error> {
    let off = read_usize(buffer, address, width)?;
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
        let (_, root_width) = unpack_type(buffer[end - 1])?;
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
                .unwrap_or_default()
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
    get_fn!(get_f64, f64, Float, W64);
    get_fn!(get_f32, f32, Float, W32);
    get_fn!(get_u8, u8, UInt, W8);
    get_fn!(get_u16, u16, UInt, W16);
    get_fn!(get_u32, u32, UInt, W32);
    get_fn!(get_u64, u64, UInt, W64);
    get_fn!(get_i8, i8, Int, W8);
    get_fn!(get_i16, i16, Int, W16);
    get_fn!(get_i32, i32, Int, W32);
    get_fn!(get_i64, i64, Int, W64);
    get_fn!(get_u8s, [u8], W8, UInt, VectorUInt);
    get_fn!(get_u16s, [u16], W16, UInt, VectorUInt);
    get_fn!(get_u32s, [u32], W32, UInt, VectorUInt);
    get_fn!(get_u64s, [u64], W64, UInt, VectorUInt);
    get_fn!(get_i8s, [i8], W8, Int, VectorInt);
    get_fn!(get_i16s, [i16], W16, Int, VectorInt);
    get_fn!(get_i32s, [i32], W32, Int, VectorInt);
    get_fn!(get_i64s, [i64], W64, Int, VectorInt);
    get_fn!(get_f32s, [f32], W32, Float, VectorFloat);
    get_fn!(get_f64s, [f64], W64, Float, VectorFloat);

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
            .map_err(Error::Utf8Error)
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
            .map_err(Error::Utf8Error)
    }
    fn get_map_info(&self) -> Result<(usize, BitWidth), Error> {
        self.expect_type(FlexBufferType::Map)?;
        if 3 * self.width.n_bytes() >= self.address {
            return Err(Error::FlexbufferOutOfBounds);
        }
        let keys_offset_address = self.address - 3 * self.width.n_bytes();
        let keys_width = unpack_type(self.buffer[self.address - 2 * self.width.n_bytes()])?.1;
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
    fn read_u64(&self) -> u64 {
        let cursor = &self.buffer[self.address..];
        match self.width {
            BitWidth::W8 => <u64>::from(<u8>::read_le(cursor)),
            BitWidth::W16 => <u64>::from(<u16>::read_le(cursor)),
            BitWidth::W32 => <u64>::from(<u32>::read_le(cursor)),
            BitWidth::W64 => <u64>::read_le(cursor),
        }
    }
    fn read_i64(&self) -> i64 {
        let cursor = &self.buffer[self.address..];
        match self.width {
            BitWidth::W8 => <i64>::from(<i8>::read_le(cursor)),
            BitWidth::W16 => <i64>::from(<i16>::read_le(cursor)),
            BitWidth::W32 => <i64>::from(<i32>::read_le(cursor)),
            BitWidth::W64 => <i64>::read_le(cursor) as i64,
        }
    }
    fn read_f64(&self) -> f64 {
        let cursor = &self.buffer[self.address..];
        match self.width {
            BitWidth::W32 => <f64>::from(<f32>::read_le(cursor)),
            BitWidth::W64 => <f64>::read_le(cursor),
            _ => unreachable!("Flexbuffers does not support 8 or 16 bit floats."),
        }
    }

    pub fn as_bool(&self) -> bool {
        match self.fxb_type {
            FlexBufferType::Bool => self.get_bool().unwrap_or_default(),
            FlexBufferType::UInt => self.as_u64() != 0,
            FlexBufferType::Int => self.as_i64() != 0,
            FlexBufferType::Float => self.as_f64().abs() > std::f64::EPSILON,
            FlexBufferType::String => !self.as_str().is_empty(),
            FlexBufferType::Null => false,
            ty if ty.is_vector() => self.length() != 0,
            ty => unimplemented!("TODO(cneo): as_bool for {:?}", ty),
        }
    }
    /// Returns a u64, casting if necessary. For Maps and Vectors, their length is
    /// returned. If anything fails, 0 is returned.
    pub fn as_u64(&self) -> u64 {
        match self.fxb_type {
            FlexBufferType::UInt => self.read_u64(),
            FlexBufferType::Int => self.read_i64().try_into().unwrap_or_default(),
            FlexBufferType::Float => self.read_f64() as u64,
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
            FlexBufferType::Int => self.read_i64(),
            FlexBufferType::UInt => self.read_u64().try_into().unwrap_or_default(),
            FlexBufferType::Float => self.read_f64() as i64,
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
            FlexBufferType::Int => self.read_i64() as f64,
            FlexBufferType::UInt => self.read_u64() as f64,
            FlexBufferType::Float => self.read_f64(),
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
    as_default!(as_i8s, get_i8s, &'de [i8]);
    as_default!(as_i16s, get_i16s, &'de [i16]);
    as_default!(as_i32s, get_i32s, &'de [i32]);
    as_default!(as_i64s, get_i64s, &'de [i64]);
    as_default!(as_u8s, get_u8s, &'de [u8]);
    as_default!(as_u16s, get_u16s, &'de [u16]);
    as_default!(as_u32s, get_u32s, &'de [u32]);
    as_default!(as_u64s, get_u64s, &'de [u64]);
    as_default!(as_f32s, get_f32s, &'de [f32]);
    as_default!(as_f64s, get_f64s, &'de [f64]);

    /// Iterates over the values of a Vector or map.
    /// Any errors are defaulted to Null Readers.
    pub fn iter(&self) -> ReaderIterator<'de> {
        ReaderIterator::new(self.as_vector())
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

    pub fn get_map_key_vector(&self) -> Result<Self, Error> {
        let (keys_offset_address, keys_width) = self.get_map_info()?;
        Reader::new(
            self.buffer,
            keys_offset_address,
            FlexBufferType::VectorKey,
            keys_width,
            self.width,
        )
    }
}

fn read_usize(buffer: &[u8], address: usize, width: BitWidth) -> Result<usize, Error> {
    let cursor = &buffer[address..];
    match width {
        BitWidth::W8 => {
            <usize>::try_from(<u8>::read_le(cursor)).map_err(|_| Error::ReadUsizeOverflowed)
        }
        BitWidth::W16 => {
            <usize>::try_from(<u16>::read_le(cursor)).map_err(|_| Error::ReadUsizeOverflowed)
        }
        BitWidth::W32 => {
            <usize>::try_from(<u32>::read_le(cursor)).map_err(|_| Error::ReadUsizeOverflowed)
        }
        BitWidth::W64 => {
            <usize>::try_from(<u64>::read_le(cursor)).map_err(|_| Error::ReadUsizeOverflowed)
        }
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
        let mut m = fxb.build_map();
        m.push("a", &[-1i8, -2, -3, -4]);
        m.push("b", 250i64);
        m.push("c", 5000u16);
        m.end();

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
        let mut v = fxb.build_vector();
        v.push("3.1415");
        v.push("9.001e3");
        v.push("42");
        v.end();
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
        assert!(n.get_i8().is_err());
        assert!(n.get_i16().is_err());
        assert!(n.get_i32().is_err());
        assert!(n.get_i64().is_err());
        assert!(n.get_u8().is_err());
        assert!(n.get_u16().is_err());
        assert!(n.get_u32().is_err());
        assert!(n.get_u64().is_err());
        assert!(n.get_f32().is_err());
        assert!(n.get_f64().is_err());
        assert!(n.as_vector().is_empty());
        assert!(n.as_map().is_empty());
        assert_eq!(n.as_vector().idx(1).flexbuffer_type(), FlexBufferType::Null);
        assert_eq!(n.as_map().idx("1").flexbuffer_type(), FlexBufferType::Null);
    }
}
