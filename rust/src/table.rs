use byteorder::{ByteOrder, LittleEndian};

use types::*;
use iter;

/// A wrapper object around Flatbuffer table data.
#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub struct Table<'a> {
    buffer: &'a [u8],
    pos: UOffsetT
}

impl<'a> Table<'a> {
    /// Create a table using from a slice using data starting at offset.
    pub fn from_offset(buffer: &[u8], offset: UOffsetT) -> Table {
        let n = read_uoffset(&buffer, offset as UOffsetT);
	    Table {
            buffer: buffer,
            pos: n + offset
        }
    }

    /// Create a table for a simple inline struct.
    pub fn with_pos(buffer: &[u8], pos: UOffsetT) -> Table {
        Table {
            buffer: buffer,
            pos: pos
        }
    }

    /// Return an object table at offset.
    pub fn get_root(&self, offset: UOffsetT) -> Table {
        Table::from_offset(self.buffer, self.pos + offset)
    }

    /// Return an object table at offset specified by offset.
    pub fn get_indirect_root(&self, offset: UOffsetT) -> Table {
        let actual_offset = self.pos as u32 + offset + self.get_u32(offset);
        Table::with_pos(&self.buffer, actual_offset)
    }


    // Returns a slice of table including the vtable.
    //
    // First element of root table is Offset to vtable used
    // and is by default a negative direction.
    #[inline(always)]
    fn vtable(&self) -> &[u8] {
        let pos = self.pos;
        let vt_offset = read_soffset(&self.buffer, self.pos as UOffsetT);
        let vtable = (self.pos as i32 - vt_offset) as usize;
        &self.buffer[vtable..pos as usize]
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

    /// ByteVector gets an unsigned byte slice from data stored inside
    /// the flatbuffer.
    pub fn byte_vector(&self, offset: UOffsetT) -> &[u8] {
        let mut offset = offset + self.pos as UOffsetT;
	    offset += read_uoffset(self.buffer, offset);
	    let start =  offset as usize + 4 as usize;
	    let length = read_uoffset(self.buffer, offset) as usize;
	    &self.buffer[start..start+length]
    }

    /// ByteVector gets a signed byte slice from data stored inside
    /// the flatbuffer.
    pub fn ibyte_vector(&self, offset: UOffsetT) -> &[i8] {
        let slice = self.byte_vector(offset);
        let len = slice.len();
        unsafe {
            use std::slice;
            let z = slice.as_ptr() as *const i8;
            slice::from_raw_parts(z, len)
        }
    }

    /// ByteVector gets a slice of bool from data stored inside the flatbuffer.
    pub fn bool_vector(&self, offset: UOffsetT) -> &[bool] {
        let slice = self.byte_vector(offset);
        let len = slice.len();
        unsafe {
            use std::slice;
            let z = slice.as_ptr() as *const bool;
            slice::from_raw_parts(z, len)
        }
    }

    /// TODO
    pub fn struct_vector<T: From<Table<'a>>>(&'a self, offset: UOffsetT) -> iter::Iterator<T> {
        let mut offset = offset;
        offset += self.get_u32(offset);
        iter::new_struct_iterator::<T>(&self,  offset as usize)
    }

    /// TODO
    pub fn table_vector<T: From<Table<'a>>>(&'a self, offset: UOffsetT) -> iter::Iterator<T> {
        let mut offset = offset;
        offset += self.get_u32(offset);
        iter::new_table_iterator::<T>(&self, offset as usize)
    }

    /// TODO
    pub fn str_vector(&'a self, offset: UOffsetT) -> iter::Iterator<&str> {
        let mut offset = offset;
        offset += self.get_u32(offset);
        iter::new_string_iterator(&self,  offset as usize)
    }

    /// TODO
    pub fn get_bool(&self, offset: UOffsetT) -> bool {
        let pos = self.pos as usize;
        self.buffer[pos + offset as usize] == 1
    }

    /// TODO
    pub fn get_u8(&self, offset: UOffsetT) -> u8 {
        let pos = self.pos as usize;
        self.buffer[pos + offset as usize]
    }

    /// TODO
    pub fn get_i8(&self, offset: UOffsetT) -> i8 {
        let pos = self.pos as usize;
        self.buffer[pos + offset as usize] as i8
    }

    /// TODO
    pub fn get_u16(&self, offset: UOffsetT) -> u16 {
        let pos = self.pos as usize;
        LittleEndian::read_u16(&self.buffer[pos + offset as usize..])
    }

    /// TODO
    pub fn get_i16(&self, offset: UOffsetT) -> i16 {
        let pos = self.pos as usize;
        LittleEndian::read_i16(&self.buffer[pos + offset as usize..])
    }

    /// TODO
    pub fn get_u32(&self, offset: UOffsetT) -> u32 {
        let pos = self.pos as usize;
        LittleEndian::read_u32(&self.buffer[pos + offset as usize..])
    }

    /// TODO
    pub fn get_i32(&self, offset: UOffsetT) -> i32 {
        let pos = self.pos as usize;
        LittleEndian::read_i32(&self.buffer[pos + offset as usize..])
    }

    /// TODO
    pub fn get_u64(&self, offset: UOffsetT) -> u64 {
        let pos = self.pos as usize;
        LittleEndian::read_u64(&self.buffer[pos + offset as usize..])
    }

    /// TODO
    pub fn get_i64(&self, offset: UOffsetT) -> i64 {
        let pos = self.pos as usize;
        LittleEndian::read_i64(&self.buffer[pos + offset as usize..])
    }

    /// TODO
    pub fn get_f32(&self, offset: UOffsetT) -> f32 {
        let pos = self.pos as usize;
        LittleEndian::read_f32(&self.buffer[pos + offset as usize..])
    }

    /// TODO
    pub fn get_f64(&self, offset: UOffsetT) -> f64 {
        let pos = self.pos as usize;
        LittleEndian::read_f64(&self.buffer[pos + offset as usize..])
    }

    /// TODO
    pub fn get_str(&self, offset: UOffsetT) -> &str {
        use std::str;
        let s = self.byte_vector(offset);
        unsafe { str::from_utf8_unchecked(s) }
    }

    /// Retrieve a struct table from offset.
    pub fn get_struct<T: From<Table<'a>>>(&self, offset: UOffsetT) -> T {
        let pos = self.pos as UOffsetT + offset;
        let table = Table::with_pos(self.buffer, pos);
        table.into()
    }

    /// Accesor function for the tables position in the buffer.
    #[inline]
    pub fn get_pos(&self) -> UOffsetT {
        self.pos as UOffsetT
    }

    /// Reads an offset at exact position.
    pub fn read_uoffset(&self, offset: UOffsetT) -> UOffsetT {
        read_uoffset(self.buffer, offset)
    }
}

/// Read a Unsigned offset value at given offset
pub fn read_uoffset(buf: &[u8], offset: UOffsetT) -> UOffsetT {
    LittleEndian::read_u32(&buf[offset as usize..])
}

/// Read a Signed offset value at given offset
pub fn read_soffset(buf: &[u8], offset: UOffsetT) -> SOffsetT {
    LittleEndian::read_i32(&buf[offset as usize..])
}

impl<'a> From<&'a [u8]> for Table<'a> {
    fn from(buf: &[u8]) -> Table {
        Table::from_offset(buf, 0)
    }
}
