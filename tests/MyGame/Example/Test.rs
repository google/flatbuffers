//! Automatically generated, do not modify.

use flatbuffers;
use super::*;

#[derive(Debug)]
pub struct Test<'a> {
    table: flatbuffers::Table<'a>,
}

enum VT {
    A = 0,
    B = 2,// Padding1
}

impl<'a> Test<'a> {
    pub fn new(buf: &[u8], offset: flatbuffers::UOffsetT) -> Test {
        Test {
            table: flatbuffers::Table::with_pos(buf, offset),
        }
    }

    pub fn a(&self) -> i16 {
        let offset = VT::A as u32;
        return self.table.get_i16(offset)
    }

    pub fn b(&self) -> i8 {
        let offset = VT::B as u32;
        return self.table.get_i8(offset)
    }

}

impl<'a> From<flatbuffers::Table<'a>> for Test<'a> {
    fn  from(table: flatbuffers::Table) -> Test {
        Test{
            table: table,
        }
    }
}

pub fn build_test(builder: &mut flatbuffers::Builder, a: i16, b: i8) -> flatbuffers::UOffsetT {
    builder.prep(2, 4);
    builder.pad(1);
    builder.add_i8(b);
    builder.add_i16(a);
    builder.offset() as flatbuffers::UOffsetT 
}
