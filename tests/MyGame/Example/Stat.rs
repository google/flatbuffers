//! Automatically generated, do not modify.

use flatbuffers;
use super::*;

flatbuffers_object!{Table => Stat [
 field => { name = id,
            typeOf = string,
            slot = 4,
            default = 0 }, 
 field => { name = val,
            typeOf = i64,
            slot = 6,
            default = 0 }, 
 field => { name = count,
            typeOf = u16,
            slot = 8,
            default = 0 }]}

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

