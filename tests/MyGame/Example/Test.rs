//! Automatically generated, do not modify.

use flatbuffers;
use super::*;

struct_object!{Test, 4, [
    (a,get_i16, i16, 0, 0), 
    (b,get_i8, i8, 2, 0)]}

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

