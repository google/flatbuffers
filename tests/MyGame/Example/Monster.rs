//! Automatically generated, do not modify.

use flatbuffers;
use super::*;

/// an example documentation comment: monster object
#[derive(Debug)]
pub struct Monster<'a> {
    table: flatbuffers::Table<'a>,
}

enum VT {
    POS = 4,
    MANA = 6,
    HP = 8,
    NAME = 10,
    INVENTORY = 14,
    COLOR = 16,
    TEST_TYPE = 18,
    TEST = 20,
    TEST4 = 22,
    TESTARRAYOFSTRING = 24,
    TESTARRAYOFTABLES = 26,
    ENEMY = 28,
    TESTNESTEDFLATBUFFER = 30,
    TESTEMPTY = 32,
    TESTBOOL = 34,
    TESTHASHS32_FNV1 = 36,
    TESTHASHU32_FNV1 = 38,
    TESTHASHS64_FNV1 = 40,
    TESTHASHU64_FNV1 = 42,
    TESTHASHS32_FNV1A = 44,
    TESTHASHU32_FNV1A = 46,
    TESTHASHS64_FNV1A = 48,
    TESTHASHU64_FNV1A = 50,
    TESTARRAYOFBOOLS = 52,
    TESTF = 54,
    TESTF2 = 56,
    TESTF3 = 58,
}

impl<'a> Monster<'a> {
    pub fn new(buf: &[u8], offset: flatbuffers::UOffsetT) -> Monster {
        Monster {
            table: flatbuffers::Table::from_offset(buf, offset),
        }
    }

    pub fn pos(&self) -> Option<Vec3> {
        let offset = self.table.field_offset(VT::POS as u16);
        if offset != 0 {
             return Some(self.table.get_struct::<Vec3>(offset))
        };
        None
    }

    pub fn mana(&self) -> i16 {
        let offset = self.table.field_offset(VT::MANA as u16);
        if offset != 0 {
            return self.table.get_i16(offset)
        }
        150
    }

    pub fn hp(&self) -> i16 {
        let offset = self.table.field_offset(VT::HP as u16);
        if offset != 0 {
            return self.table.get_i16(offset)
        }
        100
    }

    pub fn name(&self) -> &str {
        let offset = self.table.field_offset(VT::NAME as u16);
        if offset != 0 {
            return self.table.get_str(offset)
        }
        ""
    }

    pub fn inventory(&self) -> &[u8] {
        let offset = self.table.field_offset(VT::INVENTORY as u16);
        if offset != 0 {
            return self.table.byte_vector(offset);
        }
        &[]
    }

    pub fn color(&self) -> Color {
        let offset = self.table.field_offset(VT::COLOR as u16);
        if offset != 0 {
            return self.table.get_i8(offset).into()
        }
        8.into()
    }

    pub fn test_type(&self) -> Any {
        let offset = self.table.field_offset(VT::TEST_TYPE as u16);
        if offset != 0 {
            return self.table.get_u8(offset).into()
        }
        0.into()
    }

    pub fn test(&self) -> AnyUnion {
        let offset = self.table.field_offset(VT::TEST as u16);
        if offset != 0 {
            let t = self.test_type();
                        return AnyUnion::from_type(&self.table, t, offset as usize);
        }
        AnyUnion::None
    }

    pub fn test4(&self) -> flatbuffers::Iterator<Test> {
        let offset = self.table.field_offset(VT::TEST4 as u16);
        if offset != 0 {
            return self.table.struct_vector(offset)
        }
        flatbuffers::empty_iterator(&self.table)
    }

    pub fn testarrayofstring(&self) -> flatbuffers::Iterator<&str> {
        let offset = self.table.field_offset(VT::TESTARRAYOFSTRING as u16);
        if offset != 0 {
            return self.table.str_vector(offset);
        }
        flatbuffers::empty_iterator(&self.table)    }

    /// an example documentation comment: this will end up in the generated code
    /// multiline too
    pub fn testarrayoftables(&self) -> flatbuffers::Iterator<Monster> {
        let offset = self.table.field_offset(VT::TESTARRAYOFTABLES as u16);
        if offset != 0 {
            return self.table.table_vector(offset)
        }
        flatbuffers::empty_iterator(&self.table)
    }

    pub fn enemy(&self) -> Option<Monster> {
        let offset = self.table.field_offset(VT::ENEMY as u16);
        if offset != 0 {
            return Some(self.table.get_indirect_root(offset).into())
        };
        None
    }

    pub fn testnestedflatbuffer(&self) -> &[u8] {
        let offset = self.table.field_offset(VT::TESTNESTEDFLATBUFFER as u16);
        if offset != 0 {
            return self.table.byte_vector(offset);
        }
        &[]
    }

    pub fn testempty(&self) -> Option<Stat> {
        let offset = self.table.field_offset(VT::TESTEMPTY as u16);
        if offset != 0 {
            return Some(self.table.get_indirect_root(offset).into())
        };
        None
    }

    pub fn testbool(&self) -> bool {
        let offset = self.table.field_offset(VT::TESTBOOL as u16);
        if offset != 0 {
            return self.table.get_bool(offset)
        }
        false
    }

    pub fn testhashs32_fnv1(&self) -> i32 {
        let offset = self.table.field_offset(VT::TESTHASHS32_FNV1 as u16);
        if offset != 0 {
            return self.table.get_i32(offset)
        }
        0
    }

    pub fn testhashu32_fnv1(&self) -> u32 {
        let offset = self.table.field_offset(VT::TESTHASHU32_FNV1 as u16);
        if offset != 0 {
            return self.table.get_u32(offset)
        }
        0
    }

    pub fn testhashs64_fnv1(&self) -> i64 {
        let offset = self.table.field_offset(VT::TESTHASHS64_FNV1 as u16);
        if offset != 0 {
            return self.table.get_i64(offset)
        }
        0
    }

    pub fn testhashu64_fnv1(&self) -> u64 {
        let offset = self.table.field_offset(VT::TESTHASHU64_FNV1 as u16);
        if offset != 0 {
            return self.table.get_u64(offset)
        }
        0
    }

    pub fn testhashs32_fnv1a(&self) -> i32 {
        let offset = self.table.field_offset(VT::TESTHASHS32_FNV1A as u16);
        if offset != 0 {
            return self.table.get_i32(offset)
        }
        0
    }

    pub fn testhashu32_fnv1a(&self) -> u32 {
        let offset = self.table.field_offset(VT::TESTHASHU32_FNV1A as u16);
        if offset != 0 {
            return self.table.get_u32(offset)
        }
        0
    }

    pub fn testhashs64_fnv1a(&self) -> i64 {
        let offset = self.table.field_offset(VT::TESTHASHS64_FNV1A as u16);
        if offset != 0 {
            return self.table.get_i64(offset)
        }
        0
    }

    pub fn testhashu64_fnv1a(&self) -> u64 {
        let offset = self.table.field_offset(VT::TESTHASHU64_FNV1A as u16);
        if offset != 0 {
            return self.table.get_u64(offset)
        }
        0
    }

    pub fn testarrayofbools(&self) -> &[bool] {
        let offset = self.table.field_offset(VT::TESTARRAYOFBOOLS as u16);
        if offset != 0 {
            return self.table.bool_vector(offset);
        }
        &[]
    }

    pub fn testf(&self) -> f32 {
        let offset = self.table.field_offset(VT::TESTF as u16);
        if offset != 0 {
            return self.table.get_f32(offset)
        }
        3.14159
    }

    pub fn testf2(&self) -> f32 {
        let offset = self.table.field_offset(VT::TESTF2 as u16);
        if offset != 0 {
            return self.table.get_f32(offset)
        }
        3.0
    }

    pub fn testf3(&self) -> f32 {
        let offset = self.table.field_offset(VT::TESTF3 as u16);
        if offset != 0 {
            return self.table.get_f32(offset)
        }
        0.0
    }

}

impl<'a> From<flatbuffers::Table<'a>> for Monster<'a> {
    fn  from(table: flatbuffers::Table) -> Monster {
        Monster{
            table: table,
        }
    }
}

/// Builder Object for `Monster` tables.
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

    /// Initialize a new `Builder` for a `Monster` table.
    pub fn start(&mut self) {
        self.inner.start_object(28);
    }

    /// Finalize the current object and return the offset.
    pub fn end(&mut self) -> flatbuffers::UOffsetT {
        return self.inner.end_object()
    }

    /// Set the value for field `pos`.
    pub fn add_pos(&mut self, pos: flatbuffers::UOffsetT) {
        self.inner.add_slot_struct(0, pos, 0)
    }

    /// Set the value for field `mana`.
    pub fn add_mana(&mut self, mana: i16) {
        self.inner.add_slot_i16(1, mana, 150)
    }

    /// Set the value for field `hp`.
    pub fn add_hp(&mut self, hp: i16) {
        self.inner.add_slot_i16(2, hp, 100)
    }

    /// Set the value for field `name`.
    pub fn add_name(&mut self, name: flatbuffers::UOffsetT) {
        self.inner.add_slot_uoffset(3, name, 0)
    }

    /// Set the value for field `inventory`.
    pub fn add_inventory(&mut self, inventory: flatbuffers::UOffsetT) {
        self.inner.add_slot_uoffset(5, inventory, 0)
    }

    /// Initializes bookkeeping for writing a new `inventory` vector.
    pub fn start_inventory_vector(&mut self, numElems: usize) {
        self.inner.start_vector(1, numElems, 1)
    }

    /// Set the value for field `color`.
    pub fn add_color(&mut self, color: i8) {
        self.inner.add_slot_i8(6, color, 8)
    }

    /// Set the value for field `test_type`.
    pub fn add_test_type(&mut self, test_type: u8) {
        self.inner.add_slot_u8(7, test_type, 0)
    }

    /// Set the value for field `test`.
    pub fn add_test(&mut self, test: flatbuffers::UOffsetT) {
        self.inner.add_slot_uoffset(8, test, 0)
    }

    /// Set the value for field `test4`.
    pub fn add_test4(&mut self, test4: flatbuffers::UOffsetT) {
        self.inner.add_slot_uoffset(9, test4, 0)
    }

    /// Initializes bookkeeping for writing a new `test4` vector.
    pub fn start_test4_vector(&mut self, numElems: usize) {
        self.inner.start_vector(4, numElems, 2)
    }

    /// Set the value for field `testarrayofstring`.
    pub fn add_testarrayofstring(&mut self, testarrayofstring: flatbuffers::UOffsetT) {
        self.inner.add_slot_uoffset(10, testarrayofstring, 0)
    }

    /// Initializes bookkeeping for writing a new `testarrayofstring` vector.
    pub fn start_testarrayofstring_vector(&mut self, numElems: usize) {
        self.inner.start_vector(4, numElems, 4)
    }

    /// Set the value for field `testarrayoftables`.
    pub fn add_testarrayoftables(&mut self, testarrayoftables: flatbuffers::UOffsetT) {
        self.inner.add_slot_uoffset(11, testarrayoftables, 0)
    }

    /// Initializes bookkeeping for writing a new `testarrayoftables` vector.
    pub fn start_testarrayoftables_vector(&mut self, numElems: usize) {
        self.inner.start_vector(4, numElems, 4)
    }

    /// Set the value for field `enemy`.
    pub fn add_enemy(&mut self, enemy: flatbuffers::UOffsetT) {
        self.inner.add_slot_uoffset(12, enemy, 0)
    }

    /// Set the value for field `testnestedflatbuffer`.
    pub fn add_testnestedflatbuffer(&mut self, testnestedflatbuffer: flatbuffers::UOffsetT) {
        self.inner.add_slot_uoffset(13, testnestedflatbuffer, 0)
    }

    /// Initializes bookkeeping for writing a new `testnestedflatbuffer` vector.
    pub fn start_testnestedflatbuffer_vector(&mut self, numElems: usize) {
        self.inner.start_vector(1, numElems, 1)
    }

    /// Set the value for field `testempty`.
    pub fn add_testempty(&mut self, testempty: flatbuffers::UOffsetT) {
        self.inner.add_slot_uoffset(14, testempty, 0)
    }

    /// Set the value for field `testbool`.
    pub fn add_testbool(&mut self, testbool: bool) {
        self.inner.add_slot_bool(15, testbool, false)
    }

    /// Set the value for field `testhashs32_fnv1`.
    pub fn add_testhashs32_fnv1(&mut self, testhashs32_fnv1: i32) {
        self.inner.add_slot_i32(16, testhashs32_fnv1, 0)
    }

    /// Set the value for field `testhashu32_fnv1`.
    pub fn add_testhashu32_fnv1(&mut self, testhashu32_fnv1: u32) {
        self.inner.add_slot_u32(17, testhashu32_fnv1, 0)
    }

    /// Set the value for field `testhashs64_fnv1`.
    pub fn add_testhashs64_fnv1(&mut self, testhashs64_fnv1: i64) {
        self.inner.add_slot_i64(18, testhashs64_fnv1, 0)
    }

    /// Set the value for field `testhashu64_fnv1`.
    pub fn add_testhashu64_fnv1(&mut self, testhashu64_fnv1: u64) {
        self.inner.add_slot_u64(19, testhashu64_fnv1, 0)
    }

    /// Set the value for field `testhashs32_fnv1a`.
    pub fn add_testhashs32_fnv1a(&mut self, testhashs32_fnv1a: i32) {
        self.inner.add_slot_i32(20, testhashs32_fnv1a, 0)
    }

    /// Set the value for field `testhashu32_fnv1a`.
    pub fn add_testhashu32_fnv1a(&mut self, testhashu32_fnv1a: u32) {
        self.inner.add_slot_u32(21, testhashu32_fnv1a, 0)
    }

    /// Set the value for field `testhashs64_fnv1a`.
    pub fn add_testhashs64_fnv1a(&mut self, testhashs64_fnv1a: i64) {
        self.inner.add_slot_i64(22, testhashs64_fnv1a, 0)
    }

    /// Set the value for field `testhashu64_fnv1a`.
    pub fn add_testhashu64_fnv1a(&mut self, testhashu64_fnv1a: u64) {
        self.inner.add_slot_u64(23, testhashu64_fnv1a, 0)
    }

    /// Set the value for field `testarrayofbools`.
    pub fn add_testarrayofbools(&mut self, testarrayofbools: flatbuffers::UOffsetT) {
        self.inner.add_slot_uoffset(24, testarrayofbools, 0)
    }

    /// Initializes bookkeeping for writing a new `testarrayofbools` vector.
    pub fn start_testarrayofbools_vector(&mut self, numElems: usize) {
        self.inner.start_vector(1, numElems, 1)
    }

    /// Set the value for field `testf`.
    pub fn add_testf(&mut self, testf: f32) {
        self.inner.add_slot_f32(25, testf, 3.14159)
    }

    /// Set the value for field `testf2`.
    pub fn add_testf2(&mut self, testf2: f32) {
        self.inner.add_slot_f32(26, testf2, 3.0)
    }

    /// Set the value for field `testf3`.
    pub fn add_testf3(&mut self, testf3: f32) {
        self.inner.add_slot_f32(27, testf3, 0.0)
    }

    pub fn build_vec3(&mut self , x: f32, y: f32, z: f32, test1: f64, test2: i8, test3_a: i16, test3_b: i8) -> flatbuffers::UOffsetT {
        vec3::build_vec3(&mut self.inner, x, y, z, test1, test2, test3_a, test3_b)
    }

    pub fn build_test(&mut self , a: i16, b: i8) -> flatbuffers::UOffsetT {
        test::build_test(&mut self.inner, a, b)
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

