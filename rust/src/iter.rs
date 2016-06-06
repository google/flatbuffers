use std::iter;
use std::rc::Rc;
use std::marker::PhantomData;
use byteorder::{ByteOrder, LittleEndian};

use table;
use table::{Table,TableObject};

/// An iterator over flatbuffer vectors.
#[derive(Debug)]
pub struct Iterator<'a, T> {
    buffer: Option<&'a Rc<Vec<u8>>>,
    index: usize,
    len: usize,
    i: usize,
    _marker: PhantomData<T>
}

impl<'a, T> Iterator<'a, T> {
    /// Create a new Iterator for type `T` starting at `index` with
    /// a length of `len` items of size `T`.
    pub fn new(buffer: &Rc<Vec<u8>>, index: usize, len: usize) -> Iterator<T> {
        Iterator {
            buffer: Some(buffer),
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
            buffer: None,
            index: 0,
            len: 0,
            i: 0,
            _marker: PhantomData
        }
    }
}

macro_rules! size_hint {
    () => {
        fn size_hint(&self) -> (usize, Option<usize>) {
            (self.len, Some(self.len))
        }
    }
}

impl<'a> iter::Iterator for Iterator<'a, &'a str> {
    type Item = &'a str;

    fn next(&mut self) -> Option<&'a str> {
        use std::str;
        if self.i < self.len {
            self.i += 1;
            if self.buffer.unwrap().len() >= self.index + 4 {
                let offset = LittleEndian::read_u32(&self.buffer.unwrap()[self.index..]) as usize + self.index;
                let start =  offset as usize + 4;
                let length = table::read_uoffset(self.buffer.unwrap(), offset as usize) as usize;
                let s = &self.buffer.unwrap()[start..start+length];
                let res = unsafe { str::from_utf8_unchecked(s) };
                self.index += 4;
                return Some(res)
            }
        }
        None
    }
    size_hint!();
}

impl<'a, T: TableObject> iter::Iterator for Iterator<'a, T> {
    type Item = T;

    fn next(&mut self) -> Option<T> {
        if self.i < self.len {
            self.i += 1;
            if self.buffer.unwrap().len() >= self.index + T::inline_size() {
                let table = if T::is_struct() {
                    Table::with_pos(self.buffer.unwrap().clone(), self.index as usize)
                } else {
                    Table::from_offset(self.buffer.unwrap().clone(), self.index as usize)
                };
                let res = table.into();
                self.index += T::inline_size();
                return Some(res)
            }
        }
        None
    }
    size_hint!();
}

impl<'a> iter::Iterator for Iterator<'a, bool> {
    type Item = bool;

    fn next(&mut self) -> Option<bool> {
        if self.i < self.len {
            self.i += 1;
            if self.buffer.unwrap().len() >= self.index + 1 {
                let res = self.buffer.unwrap()[self.index] == 0;
                self.index += 1;
                return Some(res)
            }
        }
        None
    }
    size_hint!();
}

macro_rules! little_iterator {
    ($ty:ty) => {
        impl<'a> iter::Iterator for Iterator<'a, $ty> {
            type Item = $ty;

            fn next(&mut self) -> Option<$ty> {
                if self.i < self.len {
                    self.i += 1;
                    if self.buffer.unwrap().len() >= self.index + 1 {
                        let res = self.buffer.unwrap()[self.index] as $ty;
                        self.index += 1;
                        return Some(res)
                    }
                }
                None
            }
            size_hint!();
        }
    };
    ($ty:ty, $size:expr, $fun:ident) => {
        impl<'a> iter::Iterator for Iterator<'a, $ty> {
            type Item = $ty;

            fn next(&mut self) -> Option<$ty> {
                if self.i < self.len {
                    self.i += 1;
                    if self.buffer.unwrap().len() >= self.index + $size {
                        let res = LittleEndian::$fun(&self.buffer.unwrap()[self.index..]);
                        self.index += $size;
                        return Some(res)
                    }
                }
                None
            }
            size_hint!();
        }
    }
}

little_iterator!(u8);
little_iterator!(i8);
little_iterator!(u16, 2, read_u16);
little_iterator!(i16, 2, read_i16);
little_iterator!(u32, 4, read_u32);
little_iterator!(i32, 4, read_i32);
little_iterator!(u64, 8, read_u64);
little_iterator!(i64, 8, read_i64);
little_iterator!(f32, 4, read_f32);
little_iterator!(f64, 8, read_f64);


