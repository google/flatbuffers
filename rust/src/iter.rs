use std::fmt::Debug;
use std::iter;
use std::marker::PhantomData;
use byteorder::{ByteOrder, LittleEndian};

use table;
use table::Table;

/// A trait for Structs and Tables to implement.
pub trait VectorItem {
    /// Table Objects require indirection. Structs do not.
    fn indirect_lookup() -> bool;
    /// The size of the object in the vector.
    fn inline_size() -> usize;
}

/// An iterator over flatbuffer vectors.
#[derive(Debug)]
pub struct Iterator<'a, T> {
    buffer: &'a [u8],
    index: usize,
    len: usize,
    i: usize,
    _marker: PhantomData<[T]>
}

/// A trait for handling little endian encoding of byte arrays.
pub trait LittleEndianReader<'a> {
    /// Read the first byte from buffer and return type `Self`.
    fn read(buffer: &'a [u8], index: usize) -> Self;
    /// Provide the size in bytes of type `Self`.
    fn size() -> usize;
}

impl<'a> LittleEndianReader<'a>for u8 {
    fn read(buffer: &[u8], index: usize) -> u8 { buffer[index] }
    fn size() -> usize { 1 }
}

impl<'a> LittleEndianReader<'a>for i8 {
    fn read(buffer: &[u8], index: usize) -> i8 { buffer[index] as i8 }
    fn size() -> usize { 1 }
}

impl<'a> LittleEndianReader<'a>for bool {
    fn read(buffer: &[u8], index: usize) -> bool { buffer[index] == 0 }
    fn size() -> usize { 1 }
}

impl<'a> LittleEndianReader<'a>for u16 {
    fn read(buffer: &[u8], index: usize) -> u16 { LittleEndian::read_u16(&buffer[index..]) }
    fn size() -> usize { 2 }
}

impl<'a> LittleEndianReader<'a>for i16 {
    fn read(buffer: &[u8], index: usize) -> i16 { LittleEndian::read_i16(&buffer[index..]) }
    fn size() -> usize { 2 }
}

impl<'a> LittleEndianReader<'a>for u32 {
    fn read(buffer: &[u8], index: usize) -> u32 { LittleEndian::read_u32(&buffer[index..]) }
    fn size() -> usize { 4 }
}

impl<'a> LittleEndianReader<'a>for i32 {
    fn read(buffer: &[u8], index: usize) -> i32 { LittleEndian::read_i32(&buffer[index..]) }
    fn size() -> usize { 4 }
}

impl<'a> LittleEndianReader<'a>for u64 {
    fn read(buffer: &[u8], index: usize) -> u64 { LittleEndian::read_u64(&buffer[index..]) }
    fn size() -> usize { 8 }
}

impl<'a> LittleEndianReader<'a>for i64 {
    fn read(buffer: &[u8], index: usize) -> i64 { LittleEndian::read_i64(&buffer[index..]) }
    fn size() -> usize { 8 }
}

impl<'a> LittleEndianReader<'a>for f32 {
    fn read(buffer: &[u8], index: usize) -> f32 { LittleEndian::read_f32(&buffer[index..]) }
    fn size() -> usize { 4 }
}

impl<'a> LittleEndianReader<'a>for f64 {
    fn read(buffer: &[u8], index: usize) -> f64 { LittleEndian::read_f64(&buffer[index..]) }
    fn size() -> usize { 8 }
}

impl<'a> LittleEndianReader<'a>for &'a str {
    fn read(buffer: &[u8], index:usize) -> &str {
        use std::str;
        let offset = LittleEndian::read_u32(&buffer[index..]) as usize + index;
        let start =  offset as usize + 4;
        let length = table::read_uoffset(buffer, offset as usize) as usize;
        let s = &buffer[start..start+length];
        unsafe { str::from_utf8_unchecked(s) }
    }
    fn size() -> usize { 4 }
}


impl<'a, T: From<Table<'a>> + VectorItem> LittleEndianReader<'a>for T {
    fn read(buffer: &'a [u8], index: usize) -> T {
        let table = if T::indirect_lookup() {
            Table::from_offset(buffer, index as usize)
        } else {
            Table::with_pos(&buffer, index as usize)
        };
        table.into()
    }
    fn size() -> usize { T::inline_size() }
}

impl<'a, T: LittleEndianReader<'a>> iter::Iterator for Iterator<'a, T> {
    type Item = T;

    fn next(&mut self) -> Option<T> {
        if self.i < self.len {
            self.i += 1;
            let slice = &self.buffer[self.index..];
            if slice.len() >= T::size() {
                let res = T::read(self.buffer, self.index);
                self.index += T::size();
                return Some(res)
            }
        }
        None
    }
}

impl<'a, T: LittleEndianReader<'a> + Debug>  ExactSizeIterator for Iterator<'a, T>  {
    fn len(&self) -> usize {
        self.len
    }
}

impl<'a, T> Default for Iterator<'a, T> {
    fn default() -> Iterator<'a, T> {
        Iterator {
            buffer: &[],
            index: 0,
            len: 0,
            i: 0,
            _marker: PhantomData
        }
    }
}

impl<'a, T> Iterator<'a, T> {
    /// Create a new Iterator for type `T` starting at `index` with
    /// a length of `len` items of size `T`.
    pub fn new(buffer: &[u8], index: usize, len: usize) -> Iterator<T> {
        Iterator {
            buffer: buffer,
            index: index,
            len: len,
            i: 0,
            _marker: PhantomData
        }
    }
}
