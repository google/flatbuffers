//! Automatically generated, do not modify.

use flatbuffers;
use super::*;

struct_object!{Vec3, 32, [
    (x,get_f32, f32, 0, 0.0), 
    (y,get_f32, f32, 4, 0.0), 
    (z,get_f32, f32, 8, 0.0), 
    (test1,get_f64, f64, 16, 0.0), 
    (test2,simple_enum,get_i8, i8, Color, 24, 0), 
    (test3,get_struct, Test, 26)]}

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

