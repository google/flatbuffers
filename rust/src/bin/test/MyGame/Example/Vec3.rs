//! Automatically generated, do not modify.

use flatbuffers;
use super::*;

#[derive(Debug)]
pub struct Vec3<'a> {
    table: flatbuffers::Table<'a>,
}

enum VT {
    X = 0,
    Y = 4,
    Z = 8,// Padding4
    TEST1 = 16,
    TEST2 = 24,// Padding1
    TEST3 = 26,// Padding2
}

impl<'a> Vec3<'a> {
    pub fn new(buf: &[u8], offset: flatbuffers::UOffsetT) -> Vec3 {
        Vec3 {
            table: flatbuffers::Table::with_pos(buf, offset),
        }
    }

    pub fn x(&self) -> f32 {
        let offset = VT::X as u32;
        return self.table.get_f32(offset)
    }

    pub fn y(&self) -> f32 {
        let offset = VT::Y as u32;
        return self.table.get_f32(offset)
    }

    pub fn z(&self) -> f32 {
        let offset = VT::Z as u32;
        return self.table.get_f32(offset)
    }

    pub fn test1(&self) -> f64 {
        let offset = VT::TEST1 as u32;
        return self.table.get_f64(offset)
    }

    pub fn test2(&self) -> Color {
        let offset = VT::TEST2 as u32;
        return self.table.get_i8(offset).into()
    }

    pub fn test3(&self) -> Test {
        let offset = VT::TEST3 as u32;
         return self.table.get_struct::<Test>(offset)
    }

}

impl<'a> From<flatbuffers::Table<'a>> for Vec3<'a> {
    fn  from(table: flatbuffers::Table) -> Vec3 {
        Vec3{
            table: table,
        }
    }
}

pub fn build_vec3(builder: &mut flatbuffers::Builder, x: f32, y: f32, z: f32, test1: f64, test2: i8, test3_a: i16, test3_b: i8) -> flatbuffers::UOffsetT {
    builder.prep(16, 32);
    builder.pad(2);
    builder.prep(2, 4);
    builder.pad(1);
    builder.add_i8(test3_b);
    builder.add_i16(test3_a);
    builder.pad(1);
    builder.add_i8(test2);
    builder.add_f64(test1);
    builder.pad(4);
    builder.add_f32(z);
    builder.add_f32(y);
    builder.add_f32(x);
    builder.offset() as flatbuffers::UOffsetT 
}
