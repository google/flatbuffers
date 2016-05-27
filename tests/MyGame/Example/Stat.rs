//! Automatically generated, do not modify.

use flatbuffers;
use super::*;

#[derive(Debug)]
pub struct Stat<'a> {
    table: flatbuffers::Table<'a>,
}

enum VT {
    ID = 4,
    VAL = 6,
    COUNT = 8,
}

impl<'a> Stat<'a> {
    pub fn new(buf: &[u8], offset: flatbuffers::UOffsetT) -> Stat {
        Stat {
            table: flatbuffers::Table::from_offset(buf, offset),
        }
    }

    pub fn id(&self) -> &str {
        let offset = self.table.field_offset(VT::ID as u16);
        if offset != 0 {
            return self.table.get_str(offset)
        }
        ""
    }

    pub fn val(&self) -> i64 {
        let offset = self.table.field_offset(VT::VAL as u16);
        if offset != 0 {
            return self.table.get_i64(offset)
        }
        0
    }

    pub fn count(&self) -> u16 {
        let offset = self.table.field_offset(VT::COUNT as u16);
        if offset != 0 {
            return self.table.get_u16(offset)
        }
        0
    }

}

impl<'a> From<flatbuffers::Table<'a>> for Stat<'a> {
    fn  from(table: flatbuffers::Table) -> Stat {
        Stat{
            table: table,
        }
    }
}

/// Builder Object for `Stat` tables.
pub struct Builder {
    inner: flatbuffers::Builder,
}

impl Builder {
    /// Create a new builder.
    pub fn with_capacity(size: usize) -> Self {
        Builder {
            inner: flatbuffers::Builder::with_capacity(size),
        }
    }

    /// Initialize a new `Builder` for a `Stat` table.
    pub fn start(&mut self) {
        self.inner.start_object(3);
    }

    /// Finalize the current object and return the offset.
    pub fn end(&mut self) -> flatbuffers::UOffsetT {
        return self.inner.end_object()
    }

    /// Set the value for field `id`.
    pub fn add_id(&mut self, id: flatbuffers::UOffsetT) {
        self.inner.add_slot_uoffset(0, id, 0)
    }

    /// Set the value for field `val`.
    pub fn add_val(&mut self, val: i64) {
        self.inner.add_slot_i64(1, val, 0)
    }

    /// Set the value for field `count`.
    pub fn add_count(&mut self, count: u16) {
        self.inner.add_slot_u16(2, count, 0)
    }

    /// Finish the buffer.
    pub fn finish(&mut self, r: flatbuffers::UOffsetT) {
        return self.inner.finish(r)
    }

    pub fn create_string(&mut self, v: &str) -> flatbuffers::UOffsetT { self.inner.create_string(v) }
    pub fn get_bytes(&self) -> &[u8] { self.inner.get_bytes() }
    pub fn len(&self) -> usize { self.inner.len() }
    pub fn offset(&self) -> usize { self.inner.offset() }
    pub fn reset(&mut self) { self.inner.reset() }
    pub fn prep(&mut self, s:usize, a: usize) { self.inner.prep(s,a) }
    pub fn pad(&mut self, n:usize) { self.inner.pad(n) }
    pub fn end_vector(&mut self) -> flatbuffers::UOffsetT { self.inner.end_vector() }
    pub fn add_bool(&mut self, v:bool) { self.inner.add_bool(v) }
    pub fn add_u8(&mut self, v:u8) { self.inner.add_u8(v) }
    pub fn add_i8(&mut self, v:i8) { self.inner.add_i8(v) }
    pub fn add_u16(&mut self, v:u16) { self.inner.add_u16(v) }
    pub fn add_i16(&mut self, v:i16) { self.inner.add_i16(v) }
    pub fn add_u32(&mut self, v:u32) { self.inner.add_u32(v) }
    pub fn add_i32(&mut self, v:i32) { self.inner.add_i32(v) }
    pub fn add_u64(&mut self, v:u64) { self.inner.add_u64(v) }
    pub fn add_i64(&mut self, v:i64) { self.inner.add_i64(v) }
    pub fn add_f32(&mut self, v:f32) { self.inner.add_f32(v) }
    pub fn add_f64(&mut self, v:f64) { self.inner.add_f64(v) }
    pub fn add_uoffset(&mut self, v: flatbuffers::UOffsetT) { self.inner.add_uoffset(v) }
    pub fn add_slot_bool(&mut self, o: usize, v: bool, d: bool) { self.inner.add_slot_bool(o,v,d) }
    pub fn add_slot_u8(&mut self, o: usize, v: u8, d: u8) { self.inner.add_slot_u8(o,v,d) }
    pub fn add_slot_i8(&mut self, o: usize, v: i8, d: i8) { self.inner.add_slot_i8(o,v,d) }
    pub fn add_slot_u16(&mut self, o: usize, v: u16, d: u16) { self.inner.add_slot_u16(o,v,d) }
    pub fn add_slot_i16(&mut self, o: usize, v: i16, d: i16) { self.inner.add_slot_i16(o,v,d) }
    pub fn add_slot_i32(&mut self, o: usize, v: i32, d: i32) { self.inner.add_slot_i32(o,v,d) }
    pub fn add_slot_u32(&mut self, o: usize, v: u32, d: u32) { self.inner.add_slot_u32(o,v,d) }
    pub fn add_slot_u64(&mut self, o: usize, v: u64, d: u64) { self.inner.add_slot_u64(o,v,d) }
    pub fn add_slot_i64(&mut self, o: usize, v: i64, d: i64) { self.inner.add_slot_i64(o,v,d) }
    pub fn add_slot_f32(&mut self, o: usize, v: f32, d: f32) { self.inner.add_slot_f32(o,v,d) }
    pub fn add_slot_f64(&mut self, o: usize, v: f64, d: f64) { self.inner.add_slot_f64(o,v,d) }
    pub fn add_slot_uoffset(&mut self, o: usize, v: flatbuffers::UOffsetT, d: flatbuffers::UOffsetT) { self.inner.add_slot_uoffset(o,v,d) }
    pub fn add_slot_struct(&mut self, o: usize, v: flatbuffers::UOffsetT, d: flatbuffers::UOffsetT) { self.inner.add_slot_struct(o,v,d) }
}

impl Default for Builder {
    fn default() -> Builder {
        Builder::with_capacity(1024)
    }
}

impl From<flatbuffers::Builder> for Builder {
    fn from(b: flatbuffers::Builder) -> Builder {
        Builder {
            inner: b,
        }
    }
}

impl Into<flatbuffers::Builder> for Builder {
    fn into(self) -> flatbuffers::Builder {
        self.inner
    }
}

impl flatbuffers::ObjectBuilder for Builder {}

