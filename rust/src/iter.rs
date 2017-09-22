use std::marker::PhantomData;
use byteorder::{ByteOrder, LittleEndian};

use types::*;
use table::Table;

/// An iterator over vectors of flatbuffer objects.
#[derive(Debug)]
pub struct Iter<'a, T: 'a> {
    buffer: &'a [u8],
    start: usize,
    end: usize,
    _marker: PhantomData<&'a [T]>,
}

impl<'a, T: VectorType<'a>> Iter<'a, T> {
    /// Create a new Iterator for type `T` starting at `index` with
    /// a length of `len` items of size `T`.
    pub fn new<'b>(buffer: &'b [u8], index: usize, len: usize) -> Iter<'b, T> {
        Iter {
            buffer: buffer,
            start: index,
            end: len * T::inline_size(),
            _marker: PhantomData,
        }
    }
}

// Its akward to have a default empty iterator but
// it means that the Table module can return
// `Iter` rather than `Option<Iter>` for
// table vector slots (with defaults).
impl<'a, T> Default for Iter<'a, T> {
    fn default() -> Iter<'a, T> {
        Iter {
            buffer: &[],
            start: 0,
            end: 0,
            _marker: PhantomData,
        }
    }
}

impl<'a> Iterator for Iter<'a, &'a str> {
    type Item = &'a str;

    fn next(&mut self) -> Option<&'a str> {
        if self.start == self.end {
            return None;
        }
        let ret = Table::<&[u8]>::read_str(&self.buffer[self.start..]);
        self.start += UOFFSETT_SIZE;
        Some(ret)
    }

    fn size_hint(&self) -> (usize, Option<usize>) {
        let len = (self.end - self.start) / 4;
        (len, Some(len))
    }
}

impl<'a> DoubleEndedIterator for Iter<'a, &'a str> {
    fn next_back(&mut self) -> Option<&'a str> {
        if self.start == self.end {
            return None;
        }
        self.end -= UOFFSETT_SIZE;
        let ret = Table::<&[u8]>::read_str(&self.buffer[self.end..]);
        Some(ret)
    }
}

impl<'a> ExactSizeIterator for Iter<'a, &'a str> {}

impl<'a, T: VectorType<'a>> Iterator for Iter<'a, T> {
    type Item = T::Item;

    fn next(&mut self) -> Option<T::Item> {
        if self.start == self.end {
            return None;
        }
        let ret = T::read_next(&self.buffer[self.start..]);
        self.start += T::inline_size();
        Some(ret)
    }

    fn size_hint(&self) -> (usize, Option<usize>) {
        let len = (self.end - self.start) / T::inline_size();
        (len, Some(len))
    }
}

impl<'a, T: VectorType<'a>> DoubleEndedIterator for Iter<'a, T> {
    fn next_back(&mut self) -> Option<T::Item> {
        if self.start == self.end {
            return None;
        }
        self.end -= T::inline_size();
        let ret = T::read_next(&self.buffer[self.end..]);
        Some(ret)
    }
}

impl<'a, T: VectorType<'a>> ExactSizeIterator for Iter<'a, T> {}

/// A trait for any Flatbuffer type that can occur in a vector.
/// Flatbuffer tables and structs wil need to implemente this trait.
pub trait VectorType<'a> {
    /// The type returned by the itertator over this `ScalarVectorType`
    type Item;
    /// Return the linline length in bytes of the `ScalarVectorType`.
    fn inline_size() -> usize;
    /// Read one value of `ScalarVectorType` from the front of the
    /// buffer.
    fn read_next(buffer: &'a [u8]) -> Self::Item;
}

macro_rules! vector_type {
    ($ty:ty, $size:expr, $fun:ident) => {
        impl<'a> VectorType<'a> for $ty {
            type Item = $ty;
            fn inline_size() -> usize { $size }
            fn read_next(buffer: &[u8]) -> $ty {
                $fun(buffer)
            }
        }
    };
    ($ty:ty, $size:expr, $md:ident::$fun:ident) => {
        impl<'a> VectorType<'a> for $ty {
            type Item = $ty;
            fn inline_size() -> usize { $size }
            fn read_next(buffer: &[u8]) -> $ty {
                $md::$fun(buffer)
            }
        }
    }
}

vector_type!(bool, 1, read_bool);
vector_type!(u8, 1, read_u8);
vector_type!(i8, 1, read_i8);
vector_type!(u16, 2, LittleEndian::read_u16);
vector_type!(i16, 2, LittleEndian::read_i16);
vector_type!(u32, 4, LittleEndian::read_u32);
vector_type!(i32, 4, LittleEndian::read_i32);
vector_type!(u64, 8, LittleEndian::read_u64);
vector_type!(i64, 8, LittleEndian::read_i64);
vector_type!(f32, 4, LittleEndian::read_f32);
vector_type!(f64, 8, LittleEndian::read_f64);

fn read_bool(buffer: &[u8]) -> bool {
    buffer[0] == 1
}

fn read_u8(buffer: &[u8]) -> u8 {
    buffer[0]
}


fn read_i8(buffer: &[u8]) -> i8 {
    buffer[0] as i8
}
