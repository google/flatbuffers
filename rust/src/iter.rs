use std::iter;
use std::mem;
use std::marker::PhantomData;
use byteorder::{ByteOrder, LittleEndian};

use table;
use table::{Table,TableObject};

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

impl<'a, T: LittleEndianReader<'a>>  ExactSizeIterator for Iterator<'a, T>  {
    fn len(&self) -> usize {
        self.len
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

macro_rules! little_reader {
    ($ty:ty) => {
        impl<'a> LittleEndianReader<'a> for $ty {
            fn read(buffer: &[u8], index: usize) -> $ty { buffer[index] as $ty }
            fn size() -> usize { mem::size_of::<$ty>() }
        }
    };
    ($ty:ty, $fun:ident) => {
        impl<'a> LittleEndianReader<'a> for $ty {
            fn read(buffer: &[u8], index: usize) -> $ty { LittleEndian::$fun(&buffer[index..]) }
            fn size() -> usize { mem::size_of::<$ty>() }
        }
    }
}

little_reader!(u8);
little_reader!(i8);
little_reader!(u16, read_u16);
little_reader!(i16, read_i16);
little_reader!(u32, read_u32);
little_reader!(i32, read_i32);
little_reader!(u64, read_u64);
little_reader!(i64, read_i64);
little_reader!(f32, read_f32);
little_reader!(f64, read_f64);

impl<'a> LittleEndianReader<'a> for bool {
    fn read(buffer: &[u8], index: usize) -> bool { buffer[index] == 0 }
    fn size() -> usize { 1 }
}

impl<'a> LittleEndianReader<'a> for &'a str {
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


impl<'a, T: TableObject<'a>> LittleEndianReader<'a> for T {
    fn read(buffer: &'a [u8], index: usize) -> T {
        let table = if T::is_struct() {
            Table::with_pos(&buffer, index as usize)
        } else {
            Table::from_offset(buffer, index as usize)
        };
        table.into()
    }
    fn size() -> usize { T::inline_size() }
}



