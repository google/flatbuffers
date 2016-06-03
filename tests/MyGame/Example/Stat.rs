//! Automatically generated, do not modify.

use flatbuffers;
use super::*;

table_object!{Stat, 4, [
    (id,get_str, &str, 4, 0), 
    (val,get_i64, i64, 6, 0), 
    (count,get_u16, u16, 8, 0)]}

/// Builder Trait for `Stat` tables.
pub trait StatBuilder {
    fn start_stat(&mut self);
    /// Set the value for field `id`.
    fn add_id(&mut self, id: flatbuffers::UOffsetT);
    /// Set the value for field `val`.
    fn add_val(&mut self, val: i64);
    /// Set the value for field `count`.
    fn add_count(&mut self, count: u16);
}

impl StatBuilder for flatbuffers::Builder {
    fn start_stat(&mut self) {
        self.start_object(3);
    }

    fn add_id(&mut self, id: flatbuffers::UOffsetT) {
        self.add_slot_uoffset(0, id, 0)
    }

    fn add_val(&mut self, val: i64) {
        self.add_slot_i64(1, val, 0)
    }

    fn add_count(&mut self, count: u16) {
        self.add_slot_u16(2, count, 0)
    }

}

