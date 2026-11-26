/*
 * Copyright 2018 Google Inc. All rights reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

use crate::endian_scalar::read_scalar_at;
use crate::follow::Follow;
use crate::primitives::*;

/// VTable encapsulates read-only usage of a vtable. It is only to be used
/// by generated code.
#[derive(Debug)]
pub struct VTable<'a> {
    buf: &'a [u8],
    loc: usize,
}

impl<'a> PartialEq for VTable<'a> {
    fn eq(&self, other: &VTable) -> bool {
        self.as_bytes().eq(other.as_bytes())
    }
}

impl<'a> VTable<'a> {
    /// SAFETY
    /// `buf` must contain a valid vtable at `loc`
    ///
    /// This consists of a number of `VOffsetT`
    /// - size of vtable in bytes including size element
    /// - size of object in bytes including the vtable offset
    /// - n fields where n is the number of fields in the table's schema when the code was compiled
    pub unsafe fn init(buf: &'a [u8], loc: usize) -> Self {
        VTable { buf, loc }
    }

    pub fn num_fields(&self) -> usize {
        (self.num_bytes() / SIZE_VOFFSET) - 2
    }

    pub fn num_bytes(&self) -> usize {
        // Safety:
        // Valid VTable at time of construction
        unsafe { read_scalar_at::<VOffsetT>(self.buf, self.loc) as usize }
    }

    pub fn object_inline_num_bytes(&self) -> usize {
        // Safety:
        // Valid VTable at time of construction
        let n = unsafe { read_scalar_at::<VOffsetT>(self.buf, self.loc + SIZE_VOFFSET) };
        n as usize
    }

    pub fn get_field(&self, idx: usize) -> VOffsetT {
        // TODO(rw): distinguish between None and 0?
        // Fields occupy indices `0..num_fields()`; `num_fields()` itself is one
        // past the last entry. The `>` bound below accepted it and read a
        // VOffsetT at `loc + num_bytes`, one slot past the end of the vtable --
        // an out-of-bounds read when the vtable ends at the end of the buffer.
        if idx >= self.num_fields() {
            return 0;
        }

        // Safety:
        // Valid VTable at time of construction
        unsafe {
            read_scalar_at::<VOffsetT>(
                self.buf,
                self.loc + SIZE_VOFFSET + SIZE_VOFFSET + SIZE_VOFFSET * idx,
            )
        }
    }

    pub fn get(&self, byte_loc: VOffsetT) -> VOffsetT {
        // TODO(rw): distinguish between None and 0?
        if byte_loc as usize + 2 > self.num_bytes() {
            return 0;
        }
        // Safety:
        // byte_loc is within bounds of vtable, which was valid at time of construction
        unsafe { read_scalar_at::<VOffsetT>(self.buf, self.loc + byte_loc as usize) }
    }

    pub fn as_bytes(&self) -> &[u8] {
        let len = self.num_bytes();
        &self.buf[self.loc..self.loc + len]
    }
}

#[allow(dead_code)]
pub fn field_index_to_field_offset(field_id: VOffsetT) -> VOffsetT {
    // Should correspond to what end_table() below builds up.
    let fixed_fields = 2; // Vtable size and Object Size.
    ((field_id + fixed_fields) * (SIZE_VOFFSET as VOffsetT)) as VOffsetT
}

#[allow(dead_code)]
pub fn field_offset_to_field_index(field_o: VOffsetT) -> VOffsetT {
    debug_assert!(field_o >= 2);
    let fixed_fields = 2; // VTable size and Object Size.
    (field_o / (SIZE_VOFFSET as VOffsetT)) - fixed_fields
}

impl<'a> Follow<'a> for VTable<'a> {
    type Inner = VTable<'a>;
    unsafe fn follow(buf: &'a [u8], loc: usize) -> Self::Inner {
        VTable::init(buf, loc)
    }
}

#[cfg(test)]
mod tests {
    use super::*;
    use crate::table::Table;

    // A 14-byte root whose vtable has `num_bytes = 6` and a single field entry,
    // so the vtable ends exactly at the end of the buffer:
    //   [0..4]  uoffset -> root table @4
    //   [4..8]  soffset -> vtable @8
    //   [8..14] vtable: num_bytes=6, object size=4, one field entry
    const GET_FIELD_ONE_PAST_VTABLE: [u8; 14] = [
        0x04, 0x00, 0x00, 0x00, 0xfc, 0xff, 0xff, 0xff, 0x06, 0x00, 0x04, 0x00, 0x00, 0x00,
    ];

    #[test]
    fn get_field_one_past_the_vtable_is_rejected() {
        // Safety: `GET_FIELD_ONE_PAST_VTABLE` is a well-formed root table whose
        // vtable lies at offset 8 and ends at the end of the buffer.
        let tab = unsafe { Table::new(&GET_FIELD_ONE_PAST_VTABLE, 4) };
        let vt = tab.vtable();
        assert_eq!(vt.num_bytes(), 6);
        assert_eq!(vt.num_fields(), 1);
        // The one real entry is still readable ...
        assert_eq!(vt.get_field(0), 0);
        // ... but `num_fields()` is one past the last entry. Before the fix
        // this read a VOffsetT at offset `8 + 6 == 14`, past the buffer.
        assert_eq!(vt.get_field(vt.num_fields()), 0);
    }

    #[test]
    fn get_field_reads_present_fields_and_absent_ones_as_zero() {
        let buf: [u8; 8] = [6, 0, 4, 0, 0x34, 0x12, 0, 0];
        // Safety: `buf` contains a well-formed vtable at offset 0.
        let vt = unsafe { VTable::init(&buf, 0) };
        assert_eq!(vt.num_fields(), 1);
        assert_eq!(vt.get_field(0), 0x1234);
        assert_eq!(vt.get_field(1), 0);
        assert_eq!(vt.get_field(2), 0);
    }
}
