//! Automatically generated, do not modify.

use flatbuffers;
use super::*;

flatbuffers_object!{Struct => Test ( size:4, align: 2) [
 field => { name = a,
            typeOf = i16,
            slot = 0,
            default = 0 }, 
 field => { name = b,
            typeOf = i8,
            slot = 2,
            default = 0,
            padding = 1 }]}

pub trait TestBuilder {
    fn build_test(&mut self, a: i16, b: i8) -> flatbuffers::UOffsetT;
}

impl TestBuilder for flatbuffers::Builder {
    fn build_test(&mut self, a: i16, b: i8) -> flatbuffers::UOffsetT {
        self.prep(2, 4);
        self.pad(1);
        self.add_i8(b);
        self.add_i16(a);
        self.offset() as flatbuffers::UOffsetT 
    }
}

