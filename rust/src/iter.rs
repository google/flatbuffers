use std::iter;
use std::marker::PhantomData;

use table;
use types::*;

/// An iterator over flatbuffer vectors.
#[derive(Debug)]
pub struct Iterator<'a, T> {
    is_table: bool,
    table: &'a table::Table<'a>,
    index: usize,
    len: usize,
    i: usize,
    _marker: PhantomData<[T]>
}

/// Create a new iterator over a flatbuffer vector of string.
pub fn new_string_iterator<'a>(table: &'a table::Table, offset: usize) -> Iterator<'a, &'a str> {
    let len =  table.get_u32(offset as u32);
    Iterator {
        is_table: false,
        table: table,
        index: offset + UOFFSETT_SIZE,
        len: len as usize,
        i: 0,
        _marker: PhantomData,
    }
}

/// Create a new iterator over a flatbuffer vector of structs.
pub fn new_struct_iterator<'a, T: From<table::Table<'a>>>(table: &'a table::Table, offset: usize) -> Iterator<'a, T> {
    let len =  table.get_u32(offset as u32);
    Iterator {
        is_table: false,
        table: table,
        index: offset + UOFFSETT_SIZE,
        len: len as usize,
        i: 0,
        _marker: PhantomData,
    }
}
/// Create a new iterator over a flatbuffer vector of tables.
pub fn new_table_iterator<'a, T: From<table::Table<'a>>>(table: &'a table::Table, offset: usize) -> Iterator<'a, T> {
    let len =  table.get_u32(offset as u32);
    Iterator {
        is_table: true,
        table: table,
        index: offset,
        len: len as usize,
        i: 0,
        _marker: PhantomData,
    }
}

#[doc(hide)]
/// Helper function. Constructs a zero iterator.
pub fn empty_iterator<'a, T>(table: &'a table::Table) -> Iterator<'a, T> {
    Iterator {
        is_table: false,
        table: table,
        index: 0,
        len: 0,
        i: 100,
        _marker: PhantomData
    }
}


impl<'a> iter::Iterator for Iterator<'a, &'a str> {
    type Item = &'a str;

    fn next(&mut self) -> Option<&'a str> {
        if self.i < self.len {
            let offset = self.index as u32;
            let res = self.table.get_str(offset);
            self.index +=  UOFFSETT_SIZE;
            self.i += 1;
            return Some(res)
        }
        None
    }

    fn size_hint(&self) -> (usize, Option<usize>) {
        let len = self.table.get_u32(0);
        (0, Some(len as usize))
    }
}

impl<'a> ExactSizeIterator for Iterator<'a, &'a str> {
    fn len(&self) -> usize {
        self.len
    }
}

impl<'a, T: From<table::Table<'a>>> iter::Iterator for Iterator<'a, T> {
    type Item = T;

    fn next(&mut self) -> Option<T> {
        if self.i < self.len {
            self.i += 1;
            if self.is_table {
                //let offset = self.table.get_u32(self.index as u32);
                let offset = self.index as u32;
                let res = self.table.get_indirect_root(offset);
                self.index +=  UOFFSETT_SIZE;
                return Some(res.into())
            } else {
                let index = self.index;
                let res = self.table.get_struct(index as u32);
                self.index +=  UOFFSETT_SIZE;
                return Some(res)
                    
            }
        }
        None
    }

    fn size_hint(&self) -> (usize, Option<usize>) {
        let len = self.table.get_u32(0);
        (0, Some(len as usize))
    }
}

impl<'a,  T: From<table::Table<'a>>> ExactSizeIterator for Iterator<'a, T> {
    fn len(&self) -> usize {
        self.len
    }
}
