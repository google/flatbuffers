//! Automatically generated, do not modify.

use flatbuffers;
use super::*;

/// an example documentation comment: monster object
flatbuffers_object!{Table => Monster [
 field => { name = pos,
            typeOf = Vec3,
            slot = 4 }, 
 field => { name = mana,
            typeOf = i16,
            slot = 6,
            default = 150 }, 
 field => { name = hp,
            typeOf = i16,
            slot = 8,
            default = 100 }, 
 field => { name = name,
            typeOf = string,
            slot = 10,
            default = 0 }, 
 field => { name = inventory,
            typeOf = [u8],
            slot = 14 }, 
 field => { name = color,
            typeOf = enum Color i8,
            slot = 16,
            default = 8 }, 
 field => { name = test_type,
            typeOf = enum AnyType u8,
            slot = 18,
            default = 0 }, 
 field => { name = test,
            typeOf = union Any,
            slot = 20,
            default = 0 }, 
 field => { name = test4,
            typeOf = [Test],
            slot = 22 }, 
 field => { name = testarrayofstring,
            typeOf = [string],
            slot = 24 }, 
 field => { name = testarrayoftables,
            typeOf = [Monster],
            slot = 26 }, 
 field => { name = enemy,
            typeOf = Monster,
            slot = 28 }, 
 field => { name = testnestedflatbuffer,
            typeOf = [u8],
            slot = 30 }, 
 field => { name = testempty,
            typeOf = Stat,
            slot = 32 }, 
 field => { name = testbool,
            typeOf = bool,
            slot = 34,
            default = false }, 
 field => { name = testhashs32_fnv1,
            typeOf = i32,
            slot = 36,
            default = 0 }, 
 field => { name = testhashu32_fnv1,
            typeOf = u32,
            slot = 38,
            default = 0 }, 
 field => { name = testhashs64_fnv1,
            typeOf = i64,
            slot = 40,
            default = 0 }, 
 field => { name = testhashu64_fnv1,
            typeOf = u64,
            slot = 42,
            default = 0 }, 
 field => { name = testhashs32_fnv1a,
            typeOf = i32,
            slot = 44,
            default = 0 }, 
 field => { name = testhashu32_fnv1a,
            typeOf = u32,
            slot = 46,
            default = 0 }, 
 field => { name = testhashs64_fnv1a,
            typeOf = i64,
            slot = 48,
            default = 0 }, 
 field => { name = testhashu64_fnv1a,
            typeOf = u64,
            slot = 50,
            default = 0 }, 
 field => { name = testarrayofbools,
            typeOf = [bool],
            slot = 52 }, 
 field => { name = testf,
            typeOf = f32,
            slot = 54,
            default = 3.14159 }, 
 field => { name = testf2,
            typeOf = f32,
            slot = 56,
            default = 3.0 }, 
 field => { name = testf3,
            typeOf = f32,
            slot = 58,
            default = 0.0 }]}

/// Builder Trait for `Monster` tables.
pub trait MonsterBuilder {
    fn start_monster(&mut self);
    /// Set the value for field `pos`.
    fn add_pos(&mut self, pos: flatbuffers::UOffsetT);
    /// Set the value for field `mana`.
    fn add_mana(&mut self, mana: i16);
    /// Set the value for field `hp`.
    fn add_hp(&mut self, hp: i16);
    /// Set the value for field `name`.
    fn add_name(&mut self, name: flatbuffers::UOffsetT);
    /// Set the value for field `inventory`.
    fn add_inventory(&mut self, inventory: flatbuffers::UOffsetT);
    /// Initializes bookkeeping for writing a new `inventory` vector.
    fn start_inventory_vector(&mut self, numElems: usize);
    /// Set the value for field `color`.
    fn add_color(&mut self, color: i8);
    /// Set the value for field `test_type`.
    fn add_test_type(&mut self, test_type: u8);
    /// Set the value for field `test`.
    fn add_test(&mut self, test: flatbuffers::UOffsetT);
    /// Set the value for field `test4`.
    fn add_test4(&mut self, test4: flatbuffers::UOffsetT);
    /// Initializes bookkeeping for writing a new `test4` vector.
    fn start_test4_vector(&mut self, numElems: usize);
    /// Set the value for field `testarrayofstring`.
    fn add_testarrayofstring(&mut self, testarrayofstring: flatbuffers::UOffsetT);
    /// Initializes bookkeeping for writing a new `testarrayofstring` vector.
    fn start_testarrayofstring_vector(&mut self, numElems: usize);
    /// Set the value for field `testarrayoftables`.
    fn add_testarrayoftables(&mut self, testarrayoftables: flatbuffers::UOffsetT);
    /// Initializes bookkeeping for writing a new `testarrayoftables` vector.
    fn start_testarrayoftables_vector(&mut self, numElems: usize);
    /// Set the value for field `enemy`.
    fn add_enemy(&mut self, enemy: flatbuffers::UOffsetT);
    /// Set the value for field `testnestedflatbuffer`.
    fn add_testnestedflatbuffer(&mut self, testnestedflatbuffer: flatbuffers::UOffsetT);
    /// Initializes bookkeeping for writing a new `testnestedflatbuffer` vector.
    fn start_testnestedflatbuffer_vector(&mut self, numElems: usize);
    /// Set the value for field `testempty`.
    fn add_testempty(&mut self, testempty: flatbuffers::UOffsetT);
    /// Set the value for field `testbool`.
    fn add_testbool(&mut self, testbool: bool);
    /// Set the value for field `testhashs32_fnv1`.
    fn add_testhashs32_fnv1(&mut self, testhashs32_fnv1: i32);
    /// Set the value for field `testhashu32_fnv1`.
    fn add_testhashu32_fnv1(&mut self, testhashu32_fnv1: u32);
    /// Set the value for field `testhashs64_fnv1`.
    fn add_testhashs64_fnv1(&mut self, testhashs64_fnv1: i64);
    /// Set the value for field `testhashu64_fnv1`.
    fn add_testhashu64_fnv1(&mut self, testhashu64_fnv1: u64);
    /// Set the value for field `testhashs32_fnv1a`.
    fn add_testhashs32_fnv1a(&mut self, testhashs32_fnv1a: i32);
    /// Set the value for field `testhashu32_fnv1a`.
    fn add_testhashu32_fnv1a(&mut self, testhashu32_fnv1a: u32);
    /// Set the value for field `testhashs64_fnv1a`.
    fn add_testhashs64_fnv1a(&mut self, testhashs64_fnv1a: i64);
    /// Set the value for field `testhashu64_fnv1a`.
    fn add_testhashu64_fnv1a(&mut self, testhashu64_fnv1a: u64);
    /// Set the value for field `testarrayofbools`.
    fn add_testarrayofbools(&mut self, testarrayofbools: flatbuffers::UOffsetT);
    /// Initializes bookkeeping for writing a new `testarrayofbools` vector.
    fn start_testarrayofbools_vector(&mut self, numElems: usize);
    /// Set the value for field `testf`.
    fn add_testf(&mut self, testf: f32);
    /// Set the value for field `testf2`.
    fn add_testf2(&mut self, testf2: f32);
    /// Set the value for field `testf3`.
    fn add_testf3(&mut self, testf3: f32);
}

impl MonsterBuilder for flatbuffers::Builder {
    fn start_monster(&mut self) {
        self.start_object(28);
    }

    fn add_pos(&mut self, pos: flatbuffers::UOffsetT) {
        self.add_slot_struct(0, pos, 0)
    }

    fn add_mana(&mut self, mana: i16) {
        self.add_slot_i16(1, mana, 150)
    }

    fn add_hp(&mut self, hp: i16) {
        self.add_slot_i16(2, hp, 100)
    }

    fn add_name(&mut self, name: flatbuffers::UOffsetT) {
        self.add_slot_uoffset(3, name, 0)
    }

    fn add_inventory(&mut self, inventory: flatbuffers::UOffsetT) {
        self.add_slot_uoffset(5, inventory, 0)
    }

    /// Initializes bookkeeping for writing a new `inventory` vector.
    fn start_inventory_vector(&mut self, numElems: usize) {
        self.start_vector(1, numElems, 1)
    }

    fn add_color(&mut self, color: i8) {
        self.add_slot_i8(6, color, 8)
    }

    fn add_test_type(&mut self, test_type: u8) {
        self.add_slot_u8(7, test_type, 0)
    }

    fn add_test(&mut self, test: flatbuffers::UOffsetT) {
        self.add_slot_uoffset(8, test, 0)
    }

    fn add_test4(&mut self, test4: flatbuffers::UOffsetT) {
        self.add_slot_uoffset(9, test4, 0)
    }

    /// Initializes bookkeeping for writing a new `test4` vector.
    fn start_test4_vector(&mut self, numElems: usize) {
        self.start_vector(4, numElems, 2)
    }

    fn add_testarrayofstring(&mut self, testarrayofstring: flatbuffers::UOffsetT) {
        self.add_slot_uoffset(10, testarrayofstring, 0)
    }

    /// Initializes bookkeeping for writing a new `testarrayofstring` vector.
    fn start_testarrayofstring_vector(&mut self, numElems: usize) {
        self.start_vector(4, numElems, 4)
    }

    fn add_testarrayoftables(&mut self, testarrayoftables: flatbuffers::UOffsetT) {
        self.add_slot_uoffset(11, testarrayoftables, 0)
    }

    /// Initializes bookkeeping for writing a new `testarrayoftables` vector.
    fn start_testarrayoftables_vector(&mut self, numElems: usize) {
        self.start_vector(4, numElems, 4)
    }

    fn add_enemy(&mut self, enemy: flatbuffers::UOffsetT) {
        self.add_slot_uoffset(12, enemy, 0)
    }

    fn add_testnestedflatbuffer(&mut self, testnestedflatbuffer: flatbuffers::UOffsetT) {
        self.add_slot_uoffset(13, testnestedflatbuffer, 0)
    }

    /// Initializes bookkeeping for writing a new `testnestedflatbuffer` vector.
    fn start_testnestedflatbuffer_vector(&mut self, numElems: usize) {
        self.start_vector(1, numElems, 1)
    }

    fn add_testempty(&mut self, testempty: flatbuffers::UOffsetT) {
        self.add_slot_uoffset(14, testempty, 0)
    }

    fn add_testbool(&mut self, testbool: bool) {
        self.add_slot_bool(15, testbool, false)
    }

    fn add_testhashs32_fnv1(&mut self, testhashs32_fnv1: i32) {
        self.add_slot_i32(16, testhashs32_fnv1, 0)
    }

    fn add_testhashu32_fnv1(&mut self, testhashu32_fnv1: u32) {
        self.add_slot_u32(17, testhashu32_fnv1, 0)
    }

    fn add_testhashs64_fnv1(&mut self, testhashs64_fnv1: i64) {
        self.add_slot_i64(18, testhashs64_fnv1, 0)
    }

    fn add_testhashu64_fnv1(&mut self, testhashu64_fnv1: u64) {
        self.add_slot_u64(19, testhashu64_fnv1, 0)
    }

    fn add_testhashs32_fnv1a(&mut self, testhashs32_fnv1a: i32) {
        self.add_slot_i32(20, testhashs32_fnv1a, 0)
    }

    fn add_testhashu32_fnv1a(&mut self, testhashu32_fnv1a: u32) {
        self.add_slot_u32(21, testhashu32_fnv1a, 0)
    }

    fn add_testhashs64_fnv1a(&mut self, testhashs64_fnv1a: i64) {
        self.add_slot_i64(22, testhashs64_fnv1a, 0)
    }

    fn add_testhashu64_fnv1a(&mut self, testhashu64_fnv1a: u64) {
        self.add_slot_u64(23, testhashu64_fnv1a, 0)
    }

    fn add_testarrayofbools(&mut self, testarrayofbools: flatbuffers::UOffsetT) {
        self.add_slot_uoffset(24, testarrayofbools, 0)
    }

    /// Initializes bookkeeping for writing a new `testarrayofbools` vector.
    fn start_testarrayofbools_vector(&mut self, numElems: usize) {
        self.start_vector(1, numElems, 1)
    }

    fn add_testf(&mut self, testf: f32) {
        self.add_slot_f32(25, testf, 3.14159)
    }

    fn add_testf2(&mut self, testf2: f32) {
        self.add_slot_f32(26, testf2, 3.0)
    }

    fn add_testf3(&mut self, testf3: f32) {
        self.add_slot_f32(27, testf3, 0.0)
    }

}

