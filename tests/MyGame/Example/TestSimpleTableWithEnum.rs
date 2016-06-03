//! Automatically generated, do not modify.

use flatbuffers;
use super::*;

table_object!{TestSimpleTableWithEnum, 4, [
    (color,simple_enum,get_i8, i8, Color, 4, 2)]}

/// Builder Trait for `TestSimpleTableWithEnum` tables.
pub trait TestSimpleTableWithEnumBuilder {
    fn start_testsimpletablewithenum(&mut self);
    /// Set the value for field `color`.
    fn add_color(&mut self, color: i8);
}

impl TestSimpleTableWithEnumBuilder for flatbuffers::Builder {
    fn start_testsimpletablewithenum(&mut self) {
        self.start_object(1);
    }

    fn add_color(&mut self, color: i8) {
        self.add_slot_i8(0, color, 2)
    }

}

