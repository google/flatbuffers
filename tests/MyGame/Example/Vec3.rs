//! Automatically generated, do not modify.

use flatbuffers;
use super::*;

flatbuffers_object!{Struct => Vec3 ( size:32, align: 16) [
 field => { name = x,
            typeOf = f32,
            slot = 0,
            default = 0.0 }, 
 field => { name = y,
            typeOf = f32,
            slot = 4,
            default = 0.0 }, 
 field => { name = z,
            typeOf = f32,
            slot = 8,
            default = 0.0,
            padding = 4 }, 
 field => { name = test1,
            typeOf = f64,
            slot = 16,
            default = 0.0 }, 
 field => { name = test2,
            typeOf = enum Color i8,
            slot = 24,
            default = 0,
            padding = 1 }, 
 field => { name = test3,
            typeOf = Test,
            slot = 26,
            padding = 2 }]}

pub trait Vec3Builder {
    fn build_vec3(&mut self, x: f32, y: f32, z: f32, test1: f64, test2: i8, test3_a: i16, test3_b: i8) -> flatbuffers::UOffsetT;
}

impl Vec3Builder for flatbuffers::Builder {
    fn build_vec3(&mut self, x: f32, y: f32, z: f32, test1: f64, test2: i8, test3_a: i16, test3_b: i8) -> flatbuffers::UOffsetT {
        self.prep(16, 32);
        self.pad(2);
        self.prep(2, 4);
        self.pad(1);
        self.add_i8(test3_b);
        self.add_i16(test3_a);
        self.pad(1);
        self.add_i8(test2);
        self.add_f64(test1);
        self.pad(4);
        self.add_f32(z);
        self.add_f32(y);
        self.add_f32(x);
        self.offset() as flatbuffers::UOffsetT 
    }
}

