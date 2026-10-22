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

use crate::primitives::*;

/// VTableWriter compartmentalizes actions needed to create a vtable.
#[derive(Debug)]
pub struct VTableWriter<'a> {
    buf: &'a mut [u8],
}

impl<'a> VTableWriter<'a> {
    #[inline(always)]
    pub fn init(buf: &'a mut [u8]) -> Self {
        VTableWriter { buf }
    }

    /// Writes the vtable length (in bytes) into the vtable.
    ///
    /// Note that callers already need to have computed this to initialize
    /// a VTableWriter.
    ///
    /// In debug mode, asserts that the length of the underlying data is equal
    /// to the provided value.
    #[inline(always)]
    pub fn write_vtable_byte_length(&mut self, n: VOffsetT) {
        let buf = &mut self.buf[..SIZE_VOFFSET];
        buf.copy_from_slice(&n.to_le_bytes());
        debug_assert_eq!(n as usize, self.buf.len());
    }

    /// Writes an object length (in bytes) into the vtable.
    #[inline(always)]
    pub fn write_object_inline_size(&mut self, n: VOffsetT) {
        let buf = &mut self.buf[SIZE_VOFFSET..2 * SIZE_VOFFSET];
        buf.copy_from_slice(&n.to_le_bytes());
    }

    /// Writes an object field offset into the vtable.
    ///
    /// Note that this expects field offsets (which are like pointers), not
    /// field ids (which are like array indices).
    #[inline(always)]
    pub fn write_field_offset(&mut self, vtable_offset: VOffsetT, object_data_offset: VOffsetT) {
        let idx = vtable_offset as usize;
        let buf = &mut self.buf[idx..idx + SIZE_VOFFSET];
        buf.copy_from_slice(&object_data_offset.to_le_bytes());
    }

    /// Clears all data in this VTableWriter. Used to cleanly undo a
    /// vtable write.
    #[inline(always)]
    pub fn clear(&mut self) {
        self.buf.fill(0);
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn writes_little_endian_values_without_changing_other_bytes() {
        let mut buf = [0xa5; 10];
        let mut writer = VTableWriter::init(&mut buf[1..9]);
        writer.write_vtable_byte_length(8);
        writer.write_object_inline_size(0xabcd);
        writer.write_field_offset(4, 0x1234);
        assert_eq!(buf, [0xa5, 8, 0, 0xcd, 0xab, 0x34, 0x12, 0xa5, 0xa5, 0xa5]);
    }

    #[test]
    fn clears_only_the_vtable_slice() {
        let mut buf = [0xa5; 10];
        VTableWriter::init(&mut buf[1..9]).clear();
        assert_eq!(buf, [0xa5, 0, 0, 0, 0, 0, 0, 0, 0, 0xa5]);
    }

    #[test]
    fn clears_empty_and_single_byte_slices() {
        VTableWriter::init(&mut []).clear();
        let mut buf = [0xa5];
        VTableWriter::init(&mut buf).clear();
        assert_eq!(buf, [0]);
    }

    #[test]
    #[should_panic]
    fn rejects_truncated_object_size() {
        VTableWriter::init(&mut [0; 3]).write_object_inline_size(1);
    }

    #[test]
    #[should_panic]
    fn rejects_out_of_bounds_field_offset() {
        VTableWriter::init(&mut [0; 4]).write_field_offset(4, 1);
    }
}
