//! Automatically generated, do not modify.

use flatbuffers;
use super::*;

/// an example documentation comment: monster object
table_object!{Monster, 4, [
    (pos,get_struct, Vec3, 4), 
    (mana,get_i16, i16, 6, 150), 
    (hp,get_i16, i16, 8, 100), 
    (name,get_str, &str, 10, 0), 
    (inventory,vector, u8, 14), 
    (color,simple_enum,get_i8, i8, Color, 16, 8), 
    (test_type,simple_enum,get_u8, u8, AnyType, 18, 0), 
    (test,union,test_type, Any, Any, 20, 0), 
    (test4,vector, Test, 22), 
    (testarrayofstring,vector, &str, 24), 
    (testarrayoftables,vector, Monster, 26), 
    (enemy,get_struct, Monster, 28), 
    (testnestedflatbuffer,vector, u8, 30), 
    (testempty,get_struct, Stat, 32), 
    (testbool,get_bool, bool, 34, false), 
    (testhashs32_fnv1,get_i32, i32, 36, 0), 
    (testhashu32_fnv1,get_u32, u32, 38, 0), 
    (testhashs64_fnv1,get_i64, i64, 40, 0), 
    (testhashu64_fnv1,get_u64, u64, 42, 0), 
    (testhashs32_fnv1a,get_i32, i32, 44, 0), 
    (testhashu32_fnv1a,get_u32, u32, 46, 0), 
    (testhashs64_fnv1a,get_i64, i64, 48, 0), 
    (testhashu64_fnv1a,get_u64, u64, 50, 0), 
    (testarrayofbools,vector, bool, 52), 
    (testf,get_f32, f32, 54, 3.14159), 
    (testf2,get_f32, f32, 56, 3.0), 
    (testf3,get_f32, f32, 58, 0.0)]}

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

