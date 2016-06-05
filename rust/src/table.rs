use byteorder::{ByteOrder, LittleEndian};

use types::*;
use iter::Iterator;

/// A trait for Structs and Tables to implement.
pub trait TableObject<'a>: From<Table<'a>> {
    /// Table Objects require indirection. Structs do not.
    fn is_struct() -> bool;
    /// The size of the object in the vector.
    fn inline_size() -> usize;
}


/// Table provides functions to read Flatbuffer data.
#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub struct Table<'a> {
    buffer: &'a [u8],
    pos: usize
}

impl<'a> Table<'a> {
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
    pub fn from_offset(buffer: &[u8], pos: usize) -> Table {
        let n = read_uoffset(&buffer, pos);
	    Table {
            buffer: buffer,
            pos: n as usize + pos 
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
    pub fn with_pos(buffer: &[u8], pos: usize) -> Table {
        Table {
            buffer: buffer,
            pos: pos
        }
    }

    /// Return an object table at offset.
    /// See `from_offset`.
    pub fn get_root(&self, offset: UOffsetT) -> Table {
        Table::from_offset(self.buffer, self.pos + offset as usize)
    }

    /// Return an object table at offset specified by offset.
    pub fn get_indirect_root(&self, offset: UOffsetT) -> Table {
        let actual_offset = self.pos as u32 + offset + self.get_u32(offset);
        Table::with_pos(&self.buffer, actual_offset as usize)
    }


    // Returns a slice of table including the vtable.
    //
    // First element of root table is Offset to vtable used
    // and is by default a negative direction.
    #[inline(always)]
    fn vtable(&self) -> &[u8] {
        let vt_offset = read_soffset(&self.buffer, self.pos);
        let vtable = (self.pos as i32 - vt_offset) as usize;
        &self.buffer[vtable..]
    }
    
    /// Returns the field offset or 0 if the field was not present.
    pub fn field_offset(&self, field: VOffsetT) -> UOffsetT {
        let vtable = self.vtable();
        let vt_size = LittleEndian::read_u16(vtable);
        if field < vt_size {
            return LittleEndian::read_u16(&vtable[field as usize..]) as u32
        }
        0
    }

    /// Returns the `bool` value of the field at the offset written in the
    /// vtable `slot`. 
    pub fn get_slot_bool(&self, slot: VOffsetT, default: bool) -> bool {
        let offset = self.field_offset(slot);
        if offset != 0 {
            return self.get_bool(offset)
        }
        default
    }

    /// Returns the `u8` value of the field at the offset written in the
    /// vtable `slot`.
    pub fn get_slot_u8(&self, slot: VOffsetT, default: u8) -> u8 {
        let offset = self.field_offset(slot);
        if offset != 0 {
            return self.get_u8(offset)
        }
        default
    }

    /// Returns the `i8` value of the field at the offset written in the
    /// vtable `slot`.
    pub fn get_slot_i8(&self, slot: VOffsetT, default: i8) -> i8 {
        let offset = self.field_offset(slot);
        if offset != 0 {
            return self.get_i8(offset)
        }
        default
    }

    /// Returns the `u16` value of the field at the offset written in the
    /// vtable `slot`.
    pub fn get_slot_u16(&self, slot: VOffsetT, default: u16) -> u16 {
        let offset = self.field_offset(slot);
        if offset != 0 {
            return self.get_u16(offset)
        }
        default
    }

    /// Returns the `i16` value of the field at the offset written in the
    /// vtable `slot`.
    pub fn get_slot_i16(&self, slot: VOffsetT, default: i16) -> i16 {
        let offset = self.field_offset(slot);
        if offset != 0 {
            return self.get_i16(offset)
        }
        default
    }

    /// Returns the `u32` value of the field at the offset written in the
    /// vtable `slot`.
    pub fn get_slot_u32(&self, slot: VOffsetT, default: u32) -> u32 {
        let offset = self.field_offset(slot);
        if offset != 0 {
            return self.get_u32(offset)
        }
        default
    }

    /// Returns the `i32` value of the field at the offset written in the
    /// vtable `slot`.
    pub fn get_slot_i32(&self, slot: VOffsetT, default: i32) -> i32 {
        let offset = self.field_offset(slot);
        if offset != 0 {
            return self.get_i32(offset)
        }
        default
    }

    /// Returns the `u64` value of the field at the offset written in the
    /// vtable `slot`.
    pub fn get_slot_u64(&self, slot: VOffsetT, default: u64) -> u64 {
        let offset = self.field_offset(slot);
        if offset != 0 {
            return self.get_u64(offset)
        }
        default
    }

    /// Returns the `i64` value of the field at the offset written in the
    /// vtable `slot`.
    pub fn get_slot_i64(&self, slot: VOffsetT, default: i64) -> i64 {
        let offset = self.field_offset(slot);
        if offset != 0 {
            return self.get_i64(offset)
        }
        default
    }

    /// Returns the `f32` value of the field at the offset written in the
    /// vtable `slot`.
    pub fn get_slot_f32(&self, slot: VOffsetT, default: f32) -> f32 {
        let offset = self.field_offset(slot);
        if offset != 0 {
            return self.get_f32(offset)
        }
        default
    }

    /// Returns the `f64` value of the field at the offset written in the
    /// vtable `slot`.
    pub fn get_slot_f64(&self, slot: VOffsetT, default: f64) -> f64 {
        let offset = self.field_offset(slot);
        if offset != 0 {
            return self.get_f64(offset)
        }
        default
    }

    /// Returns the `&str` value of the field at the offset written in the
    /// vtable `slot`.
    pub fn get_slot_str(&self, slot: VOffsetT) -> &str {
        let offset = self.field_offset(slot);
        if offset != 0 {
            return self.get_str(offset)
        }
        ""
    }

    /// Returns the struct `T` value of the field at the offset written in the
    /// vtable `slot`.
    pub fn get_slot_struct<T: From<Table<'a>>>(&self, slot: VOffsetT) -> Option<T> {
        let offset = self.field_offset(slot);
        if offset != 0 {
            return Some(self.get_struct(offset))
        }
        None
    }

    /// Returns the unsigned byte vector value of the field at the offset written in the
    /// vtable `slot`.
    pub fn get_slot_vector<T>(&self, slot: VOffsetT) -> Iterator<'a, T> {
        let offset = self.field_offset(slot);
        if offset != 0 {
            return self.get_vector::<T>(offset)
        }
        Default::default()
    }

    /// ByteVector gets an unsigned byte slice from data stored inside
    /// the flatbuffer.
    pub fn get_vector<T>(&self, offset: UOffsetT) ->  Iterator<'a, T> {
        let mut offset = offset as usize + self.pos;
	    offset += read_uoffset(self.buffer, offset) as usize;
	    let start =  offset + UOFFSETT_SIZE;
	    let length = read_uoffset(self.buffer, offset) as usize;
	    Iterator::new(&self.buffer, start, length)
    }

    /// Returns a value of `bool` at `offset`.
    pub fn get_bool(&self, offset: UOffsetT) -> bool {
        let pos = self.pos as usize;
        self.buffer[pos + offset as usize] == 1
    }

    /// Returns a value of `u8` at `offset`.
    pub fn get_u8(&self, offset: UOffsetT) -> u8 {
        let pos = self.pos as usize;
        self.buffer[pos + offset as usize]
    }

    /// Returns a value of `i8` at `offset`.
    pub fn get_i8(&self, offset: UOffsetT) -> i8 {
        let pos = self.pos as usize;
        self.buffer[pos + offset as usize] as i8
    }

    /// Returns a value of `u16` at `offset`.
    pub fn get_u16(&self, offset: UOffsetT) -> u16 {
        let pos = self.pos as usize;
        LittleEndian::read_u16(&self.buffer[pos + offset as usize..])
    }

    /// Returns a value of `i16` at `offset`.
    pub fn get_i16(&self, offset: UOffsetT) -> i16 {
        let pos = self.pos as usize;
        LittleEndian::read_i16(&self.buffer[pos + offset as usize..])
    }

    /// Returns a value of `u32` at `offset`.
    pub fn get_u32(&self, offset: UOffsetT) -> u32 {
        let pos = self.pos as usize;
        LittleEndian::read_u32(&self.buffer[pos + offset as usize..])
    }

    /// Returns a value of `i32` at `offset`.
    pub fn get_i32(&self, offset: UOffsetT) -> i32 {
        let pos = self.pos as usize;
        LittleEndian::read_i32(&self.buffer[pos + offset as usize..])
    }

    /// Returns a value of `u64` at `offset`.
    pub fn get_u64(&self, offset: UOffsetT) -> u64 {
        let pos = self.pos as usize;
        LittleEndian::read_u64(&self.buffer[pos + offset as usize..])
    }

    /// Returns a value of `i64` at `offset`.
    pub fn get_i64(&self, offset: UOffsetT) -> i64 {
        let pos = self.pos as usize;
        LittleEndian::read_i64(&self.buffer[pos + offset as usize..])
    }

    /// Returns a value of `f32` at `offset`.
    pub fn get_f32(&self, offset: UOffsetT) -> f32 {
        let pos = self.pos as usize;
        LittleEndian::read_f32(&self.buffer[pos + offset as usize..])
    }

    /// Returns a value of `f64` at `offset`.
    pub fn get_f64(&self, offset: UOffsetT) -> f64 {
        let pos = self.pos as usize;
        LittleEndian::read_f64(&self.buffer[pos + offset as usize..])
    }

    /// Returns a value of `&str` at `offset`.
    pub fn get_str(&self, offset: UOffsetT) -> &str {
        use std::str;
        let mut offset = offset + self.pos as UOffsetT;
        offset += read_uoffset(self.buffer, offset as usize);
        let start =  offset as usize + 4 as usize;
        let length = read_uoffset(self.buffer, offset as usize) as usize;
        let s = &self.buffer[start..start+length];
        unsafe { str::from_utf8_unchecked(s) }
    }

    /// Retrieve a struct table from offset. Offset should point to then
    /// first usable byte of data i.e. the Vtable offset.
    pub fn get_struct<T: From<Table<'a>>>(&self, offset: UOffsetT) -> T {
        let pos = self.pos as UOffsetT + offset;
        let table = Table::with_pos(self.buffer, pos  as usize);
        table.into()
    }

    /// Accesor function for the tables position in the buffer.
    #[inline]
    pub fn get_pos(&self) -> UOffsetT {
        self.pos as UOffsetT
    }

    /// Reads an `UOffsetT` at exact position.
    pub fn read_uoffset(&self, offset: UOffsetT) -> UOffsetT {
        read_uoffset(self.buffer, offset as usize)
    }

    /// Reads an `VOffsetT` at exact position.
    pub fn read_voffset(&self, offset: UOffsetT) -> VOffsetT {
        read_voffset(self.buffer, offset as usize)
    }

    /// Retrieves the `VOffsetT` in the vtable `slot`.
    /// If the vtable value is zero, the default value `d` will be returned.
    pub fn read_voffset_slot(&self, slot: VOffsetT, d: VOffsetT) -> VOffsetT {
        let offset = self.field_offset(slot);
        if offset != 0 {
            return offset as VOffsetT
        }
        d
    }

    /// Get a reference to the raw buffer
    #[inline]
    pub fn get_bytes(&self) -> &[u8] {
        self.buffer
    }
}

/// Read a Unsigned offset value at given offset
pub fn read_uoffset(buf: &[u8], offset: usize) -> UOffsetT {
    LittleEndian::read_u32(&buf[offset as usize..])
}

/// Read a Signed offset value at given offset
pub fn read_soffset(buf: &[u8], offset: usize) -> SOffsetT {
    LittleEndian::read_i32(&buf[offset as usize..])
}

/// Read a Signed offset value at given offset
pub fn read_voffset(buf: &[u8], offset: usize) -> VOffsetT {
    LittleEndian::read_u16(&buf[offset as usize..])
}

impl<'a> From<&'a [u8]> for Table<'a> {
    fn from(buf: &[u8]) -> Table {
        Table::from_offset(buf, 0)
    }
}
