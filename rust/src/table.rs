use std::{ops, str};
use byteorder::{ByteOrder, LittleEndian};

use types::*;
use iter::{Iter, VectorType};

/// Table<T> provides functions to read Flatbuffer data.
#[derive(Debug, Clone, PartialEq, Eq)]
pub struct Table<T: AsRef<[u8]>> {
    inner: T,
    pos: usize,
}

impl<T: AsRef<[u8]>> Table<T> {
    /// Create a table using from a slice using data starting at `pos`.
    /// First `UOffsetT` should indicate the offset from the root
    /// of the table to the begining of the object.
    ///
    /// In the following example pos should be 0:
    /// ```
    /// 16, 0, 0, 0, // root of table: points to object   <---------
    /// 0, 0,        // padding
    /// 10, 0,       // vtable bytes
    /// 8, 0,        // size of object
    /// 7, 0,        // start of value 0
    /// 6, 0,        // start of value 1
    /// 5, 0,        // start of value 2
    /// 10, 0, 0, 0, // start of object..ie. vtable offset
    /// 0,           // padding
    /// 77,          // value 2
    /// 66,          // value 1
    /// 55,          // value 0
    /// ```
    pub fn get_indirect_root(buffer: T, pos: usize) -> Table<T> {
        let offset = Table::<T>::read_uoffset(&buffer.as_ref()[pos..]) as usize;
        Table {
            inner: buffer,
            pos: offset + pos,
        }
    }

    /// Create a table for an object.
    /// `pos` should be the offset to the first byte of
    /// useable data.
    ///
    /// In the following example pos should be 16:
    /// ```
    /// 16, 0, 0, 0, // root of table: points to object
    /// 0, 0,        // padding
    /// 10, 0,       // vtable bytes
    /// 8, 0,        // size of object
    /// 7, 0,        // start of value 0
    /// 6, 0,        // start of value 1
    /// 5, 0,        // start of value 2
    /// 10, 0, 0, 0, // start of object..ie. vtable offset <---------
    /// 0,           // padding
    /// 77,          // value 2
    /// 66,          // value 1
    /// 55,          // value 0
    /// ```
    pub fn get_root(buffer: T, pos: usize) -> Table<T> {
        Table {
            inner: buffer,
            pos: pos,
        }
    }

    // Returns a slice of table including the vtable.
    //
    // First element of root table is the vtable offset
    // and is by default a negative direction.
    #[inline(always)]
    fn vtable(&self) -> &[u8] {
        let vt_offset = Table::<T>::read_soffset(&self[self.pos..]);
        let vtable = (self.pos as i32 - vt_offset) as usize;
        &self.as_bytes()[vtable..]
    }

    /// Returns the field offset or 0 if the field was not present.
    pub fn field_offset(&self, field: VOffsetT) -> VOffsetT {
        let vtable = self.vtable();
        let vt_size = LittleEndian::read_u16(vtable);
        if field < vt_size {
            return LittleEndian::read_u16(&vtable[field as usize..]);
        }
        0
    }

    /// Returns the `bool` value of the field at the offset written in the
    /// vtable `slot`.
    pub fn get_slot_bool(&self, slot: VOffsetT, default: bool) -> bool {
        let offset = self.field_offset(slot);
        if offset != 0 {
            return self.get_bool(offset);
        }
        default
    }

    /// Returns the `u8` value of the field at the offset written in the
    /// vtable `slot`.
    pub fn get_slot_u8(&self, slot: VOffsetT, default: u8) -> u8 {
        let offset = self.field_offset(slot);
        if offset != 0 {
            return self.get_u8(offset);
        }
        default
    }

    /// Returns the `i8` value of the field at the offset written in the
    /// vtable `slot`.
    pub fn get_slot_i8(&self, slot: VOffsetT, default: i8) -> i8 {
        let offset = self.field_offset(slot);
        if offset != 0 {
            return self.get_i8(offset);
        }
        default
    }

    /// Returns the `u16` value of the field at the offset written in the
    /// vtable `slot`.
    pub fn get_slot_u16(&self, slot: VOffsetT, default: u16) -> u16 {
        let offset = self.field_offset(slot);
        if offset != 0 {
            return self.get_u16(offset);
        }
        default
    }

    /// Returns the `i16` value of the field at the offset written in the
    /// vtable `slot`.
    pub fn get_slot_i16(&self, slot: VOffsetT, default: i16) -> i16 {
        let offset = self.field_offset(slot);
        if offset != 0 {
            return self.get_i16(offset);
        }
        default
    }

    /// Returns the `u32` value of the field at the offset written in the
    /// vtable `slot`.
    pub fn get_slot_u32(&self, slot: VOffsetT, default: u32) -> u32 {
        let offset = self.field_offset(slot);
        if offset != 0 {
            return self.get_u32(offset);
        }
        default
    }

    /// Returns the `i32` value of the field at the offset written in the
    /// vtable `slot`.
    pub fn get_slot_i32(&self, slot: VOffsetT, default: i32) -> i32 {
        let offset = self.field_offset(slot);
        if offset != 0 {
            return self.get_i32(offset);
        }
        default
    }

    /// Returns the `u64` value of the field at the offset written in the
    /// vtable `slot`.
    pub fn get_slot_u64(&self, slot: VOffsetT, default: u64) -> u64 {
        let offset = self.field_offset(slot);
        if offset != 0 {
            return self.get_u64(offset);
        }
        default
    }

    /// Returns the `i64` value of the field at the offset written in the
    /// vtable `slot`.
    pub fn get_slot_i64(&self, slot: VOffsetT, default: i64) -> i64 {
        let offset = self.field_offset(slot);
        if offset != 0 {
            return self.get_i64(offset);
        }
        default
    }

    /// Returns the `f32` value of the field at the offset written in the
    /// vtable `slot`.
    pub fn get_slot_f32(&self, slot: VOffsetT, default: f32) -> f32 {
        let offset = self.field_offset(slot);
        if offset != 0 {
            return self.get_f32(offset);
        }
        default
    }

    /// Returns the `f64` value of the field at the offset written in the
    /// vtable `slot`.
    pub fn get_slot_f64(&self, slot: VOffsetT, default: f64) -> f64 {
        let offset = self.field_offset(slot);
        if offset != 0 {
            return self.get_f64(offset);
        }
        default
    }

    /// Returns the `&str` value of the field at the offset written in the
    /// vtable `slot`.
    pub fn get_slot_str(&self, slot: VOffsetT) -> &str {
        let offset = self.field_offset(slot);
        if offset != 0 {
            return self.get_str(offset);
        }
        ""
    }

    /// Returns the struct `T` value of the field at the offset written in the
    /// vtable `slot`.
    pub fn get_slot_struct<'a, O>(&'a self, slot: VOffsetT) -> Option<O>
        where O: From<Table<&'a [u8]>> + AsRef<[u8]>
    {
        let offset = self.field_offset(slot);
        if offset != 0 {
            return Some(self.get_struct(offset));
        }
        None
    }

    /// Returns the a `Table` value of the field at the offset written in the
    /// vtable `slot`.
    pub fn get_slot_table(&self, slot: VOffsetT) -> Option<Table<&[u8]>> {
        let offset = self.field_offset(slot);
        if offset != 0 {
            return Some(self.get_table(offset));
        }
        None
    }


    /// Returns the vector value of the field at the offset
    /// written in the vtable `slot`.
    pub fn get_slot_vector<'a, V>(&'a self, slot: VOffsetT) -> Iter<V>
        where V: VectorType<'a>
    {
        let offset = self.field_offset(slot);
        if offset != 0 {
            return self.get_vector::<V>(offset);
        }
        Default::default()
    }

    /// Returns byte slice from data stored inside
    /// the flatbuffer.
    pub fn get_vector<'a, V>(&'a self, offset: VOffsetT) -> Iter<V>
        where V: VectorType<'a>
    {
        let mut offset = offset as usize + self.pos;
        let length = Table::<T>::read_uoffset(&self[offset..]) as usize;
        offset += length;
        let start = offset + UOFFSETT_SIZE;
        Iter::new(&self.as_bytes(), start, length)
    }

    /// Returns a value of `bool` at `offset`.
    pub fn get_bool(&self, offset: VOffsetT) -> bool {
        let pos = self.pos as usize;
        self.as_bytes()[pos + offset as usize] == 1
    }

    /// Returns a value of `u8` at `offset`.
    pub fn get_u8(&self, offset: VOffsetT) -> u8 {
        let pos = self.pos as usize;
        self.as_bytes()[pos + offset as usize]
    }

    /// Returns a value of `i8` at `offset`.
    pub fn get_i8(&self, offset: VOffsetT) -> i8 {
        let pos = self.pos as usize;
        self.as_bytes()[pos + offset as usize] as i8
    }

    /// Returns a value of `u16` at `offset`.
    pub fn get_u16(&self, offset: VOffsetT) -> u16 {
        let pos = self.pos as usize;
        LittleEndian::read_u16(&self[pos + offset as usize..])
    }

    /// Returns a value of `i16` at `offset`.
    pub fn get_i16(&self, offset: VOffsetT) -> i16 {
        let pos = self.pos as usize;
        LittleEndian::read_i16(&self[pos + offset as usize..])
    }

    /// Returns a value of `u32` at `offset`.
    pub fn get_u32(&self, offset: VOffsetT) -> u32 {
        let pos = self.pos as usize;
        LittleEndian::read_u32(&self[pos + offset as usize..])
    }

    /// Returns a value of `i32` at `offset`.
    pub fn get_i32(&self, offset: VOffsetT) -> i32 {
        let pos = self.pos as usize;
        LittleEndian::read_i32(&self[pos + offset as usize..])
    }

    /// Returns a value of `u64` at `offset`.
    pub fn get_u64(&self, offset: VOffsetT) -> u64 {
        let pos = self.pos as usize;
        LittleEndian::read_u64(&self[pos + offset as usize..])
    }

    /// Returns a value of `i64` at `offset`.
    pub fn get_i64(&self, offset: VOffsetT) -> i64 {
        let pos = self.pos as usize;
        LittleEndian::read_i64(&self[pos + offset as usize..])
    }

    /// Returns a value of `f32` at `offset`.
    pub fn get_f32(&self, offset: VOffsetT) -> f32 {
        let pos = self.pos as usize;
        LittleEndian::read_f32(&self[pos + offset as usize..])
    }

    /// Returns a value of `f64` at `offset`.
    pub fn get_f64(&self, offset: VOffsetT) -> f64 {
        let pos = self.pos as usize;
        LittleEndian::read_f64(&self[pos + offset as usize..])
    }

    /// Returns a value of `&str` at `offset`.
    pub fn get_str(&self, offset: VOffsetT) -> &str {
        let offset = offset as usize + self.pos;
        Table::<T>::read_str(&self[offset..])
    }

    /// Retrieve a struct table from offset. Offset should point to the
    /// first usable byte of data i.e. the Vtable offset or start
    /// of struct
    pub fn get_struct<'a, O>(&'a self, offset: VOffsetT) -> O
        where O: From<Table<&'a [u8]>>
    {
        let table = self.get_table(offset);
        table.into()
    }

    /// Retrieve a `Table` from offset. Offset should point to the
    /// first usable byte of data i.e. the Vtable offset or start
    /// of struct
    pub fn get_table(&self, offset: VOffsetT) -> Table<&[u8]> {
        let pos = self.pos as usize + offset as usize;
        Table::get_root(self.inner.as_ref(), pos as usize)
    }

    /// Accesor function for the tables position in the buffer.
    #[inline]
    pub fn get_pos(&self) -> UOffsetT {
        self.pos as UOffsetT
    }

    /// Reads an `UOffsetT` at exact position.
    pub fn get_uoffset(&self, pos: usize) -> UOffsetT {
        Table::<T>::read_uoffset(&self[pos..])
    }

    /// Reads an `VOffsetT` at exact position.
    pub fn get_voffset(&self, pos: usize) -> VOffsetT {
        Table::<T>::read_voffset(&self[pos..])
    }

    /// Read a Signed offset value at given pos.
    pub fn get_soffset(&self, pos: usize) -> SOffsetT {
        Table::<T>::read_soffset(&self[pos..])
    }

    /// Get a reference to the raw buffer.
    #[inline]
    pub fn as_bytes(&self) -> &[u8] {
        self.inner.as_ref()
    }

    /// Return the backing buffer.
    pub fn into_inner(self) -> T {
        self.inner
    }

    /// Read a string starting at the beginning of the buffer.
    pub fn read_str(buffer: &[u8]) -> &str {
        let offset = Table::<T>::read_uoffset(buffer) as usize;
        let start = offset + UOFFSETT_SIZE;
        let length = Table::<T>::read_uoffset(&buffer[offset..]) as usize;
        let s = &buffer[start..start + length];
        unsafe { str::from_utf8_unchecked(s) }
    }

    /// Read a `UOffsetT` starting at the beginning of the buffer.
    pub fn read_uoffset(buffer: &[u8]) -> UOffsetT {
        LittleEndian::read_u32(buffer)
    }

    /// Read a `VOffsetT` starting at the beginning of the buffer.
    pub fn read_voffset(buffer: &[u8]) -> VOffsetT {
        LittleEndian::read_u16(buffer)
    }

    /// Read a `SOffsetT` starting at the beginning of the buffer.
    pub fn read_soffset(buffer: &[u8]) -> SOffsetT {
        LittleEndian::read_i32(buffer)
    }
}

impl From<Vec<u8>> for Table<Vec<u8>> {
    fn from(buf: Vec<u8>) -> Table<Vec<u8>> {
        Table {
            inner: buf,
            pos: 0,
        }
    }
}

impl<T: AsRef<[u8]>> ops::Deref for Table<T> {
    type Target = [u8];

    fn deref(&self) -> &[u8] {
        self.inner.as_ref()
    }
}

impl ops::DerefMut for Table<Vec<u8>> {
    fn deref_mut<'a>(&'a mut self) -> &'a mut [u8] {
        &mut self.inner
    }
}
