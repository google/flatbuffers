/*
 * Copyright 2024 Google Inc. All rights reserved.
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

//! Regression test for https://github.com/google/flatbuffers/issues/8894
//!
//! Tests that vtable memory is properly zeroed when building FlatBuffers.

extern crate alloc;
extern crate flatbuffers;

use alloc::vec;
use alloc::vec::Vec;
use core::convert::Infallible;
use core::cmp::max;
use core::ops::{Deref, DerefMut};
use core::ptr::write_bytes;

use flatbuffers::{Allocator, FlatBufferBuilder};

/// Custom allocator that pre-fills buffer with garbage (0xAA) to detect
/// uninitialized memory bugs.
struct GarbageFilledAllocator(Vec<u8>);

impl GarbageFilledAllocator {
    fn new(size: usize) -> Self {
        Self(vec![0xAA; size])
    }
}

impl Deref for GarbageFilledAllocator {
    type Target = [u8];
    fn deref(&self) -> &Self::Target {
        &self.0
    }
}

impl DerefMut for GarbageFilledAllocator {
    fn deref_mut(&mut self) -> &mut Self::Target {
        &mut self.0
    }
}

// SAFETY: grow_downwards properly moves data and the new space is filled with garbage
// (intentionally, to detect bugs where code assumes zeroed memory)
unsafe impl Allocator for GarbageFilledAllocator {
    type Error = Infallible;

    fn grow_downwards(&mut self) -> Result<(), Self::Error> {
        let old_len = self.0.len();
        let new_len = max(1, old_len * 2);

        // Resize and fill new space with garbage
        self.0.resize(new_len, 0xAA);

        if new_len == 1 {
            return Ok(());
        }

        // Move old data to the end
        let middle = new_len / 2;
        {
            let (left, right) = &mut self.0[..].split_at_mut(middle);
            right.copy_from_slice(left);
        }
        // Fill old space with garbage (NOT zeros)
        {
            let ptr = self.0[..middle].as_mut_ptr();
            unsafe {
                write_bytes(ptr, 0xAA, middle);
            }
        }
        Ok(())
    }

    fn len(&self) -> usize {
        self.0.len()
    }
}

/// Regression test for https://github.com/google/flatbuffers/issues/8894
///
/// The bug: write_vtable() called make_space() which only reserves memory
/// but doesn't zero it. If the allocator's buffer contains garbage data,
/// vtable entries for fields with default values would contain garbage
/// instead of zero (which indicates "use default").
///
/// This test uses a garbage-filled allocator to detect if vtable memory
/// is properly zeroed before being written.
#[test]
fn test_vtable_zeroed_with_garbage_allocator() {
    // Create a builder with garbage-filled allocator
    let allocator = GarbageFilledAllocator::new(256);
    let mut builder: FlatBufferBuilder<GarbageFilledAllocator> =
        FlatBufferBuilder::new_in(allocator);

    // Start a table
    let table_start = builder.start_table();

    // Set a field at a HIGH slot ID (14) to force a larger vtable.
    // This leaves slots 4, 6, 8, 10, 12 unset (should be zero).
    // VTable layout: [vtable_size:2][table_size:2][field0:2][field1:2][field2:2][field3:2][field4:2][field5:2]
    // Offsets:           0              2            4         6         8        10        12        14
    builder.push_slot::<u32>(14, 42, 0); // Set field 5 (at vtable offset 14) to 42

    // End the table - this calls write_vtable()
    let table_end = builder.end_table(table_start);

    // Finish the buffer
    builder.finish(table_end, None);

    let data = builder.finished_data();

    // Read the root table offset (first 4 bytes, little-endian)
    let root_offset = u32::from_le_bytes([data[0], data[1], data[2], data[3]]) as usize;
    let table_pos = root_offset;

    // Read the vtable offset (signed, at table position)
    let vtable_offset = i32::from_le_bytes([
        data[table_pos],
        data[table_pos + 1],
        data[table_pos + 2],
        data[table_pos + 3],
    ]);
    let vtable_pos = (table_pos as i32 - vtable_offset) as usize;

    // Read vtable size (first 2 bytes of vtable)
    let vtable_size = u16::from_le_bytes([data[vtable_pos], data[vtable_pos + 1]]) as usize;

    // Verify vtable structure is as expected (16 bytes total for 6 fields + header)
    assert_eq!(
        vtable_size, 16,
        "VTable should be 16 bytes (4 header + 6*2 fields)"
    );

    // Check that unset fields (at offsets 4, 6, 8, 10, 12) are zero.
    // Only field at offset 14 was set.
    // If the bug exists, unset fields would be 0xAAAA instead of 0.
    for i in [4_usize, 6, 8, 10, 12] {
        let field_offset = u16::from_le_bytes([data[vtable_pos + i], data[vtable_pos + i + 1]]);
        assert_eq!(
            field_offset, 0,
            "Vtable entry at offset {} should be 0 (default), but was 0x{:04X}. \
             This indicates uninitialized vtable memory (issue #8894).",
            i, field_offset
        );
    }

    // Verify the field we DID set has a non-zero offset
    let field5_offset = u16::from_le_bytes([data[vtable_pos + 14], data[vtable_pos + 14 + 1]]);
    assert_ne!(field5_offset, 0, "Field 5 should have a non-zero offset");
}
