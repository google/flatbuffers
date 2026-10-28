/*
 * Copyright 2026 Google LLC
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

//! Regression tests for the bounds-checked scalar access layer.
//!
//! Historically, out-of-range scalar reads and writes were undefined
//! behavior: `read_scalar`, `read_scalar_at`, `emplace_scalar`, and
//! `emplace_scalar_array` used raw `copy_nonoverlapping` calls without
//! checking the destination length. These tests assert that malformed
//! inputs panic instead. Under Miri they additionally assert the absence
//! of undefined behavior.

use flatbuffers::{
    emplace_scalar, emplace_scalar_array, read_scalar, read_scalar_at, Table, Vector,
};

#[test]
fn scalar_roundtrip_little_endian() {
    let mut b = [0u8; 8];
    unsafe { emplace_scalar::<u64>(&mut b, 0x0102_0304_0506_0708) };
    assert_eq!(b, [0x08, 0x07, 0x06, 0x05, 0x04, 0x03, 0x02, 0x01]);
    assert_eq!(unsafe { read_scalar::<u64>(&b) }, 0x0102_0304_0506_0708);

    let mut f = [0u8; 8];
    unsafe { emplace_scalar::<f64>(&mut f, -1.5) };
    assert_eq!(unsafe { read_scalar::<f64>(&f) }, -1.5);

    let mut c = [0u8; 1];
    unsafe { emplace_scalar::<bool>(&mut c, true) };
    assert_eq!(c, [1]);
    assert!(unsafe { read_scalar::<bool>(&c) });

    let mut w = [0u8; 2];
    unsafe { emplace_scalar::<i16>(&mut w, -2) };
    assert_eq!(w, [0xfe, 0xff]);
    assert_eq!(unsafe { read_scalar_at::<i16>(&w, 0) }, -2);
}

#[test]
#[should_panic]
fn read_scalar_on_short_slice_panics_instead_of_ub() {
    // 3 bytes for a u32: previously an out-of-bounds `copy_nonoverlapping`.
    let _ = unsafe { read_scalar::<u32>(&[1, 2, 3]) };
}

#[test]
#[should_panic]
fn read_scalar_at_truncated_tail_panics_instead_of_ub() {
    let buf = [0u8; 4];
    // A u32 read starting at offset 3 only has 1 byte available.
    let _ = unsafe { read_scalar_at::<u32>(&buf, 3) };
}

#[test]
#[should_panic]
fn read_scalar_past_end_of_buffer_panics_instead_of_ub() {
    let buf = [0u8; 4];
    // An 8-byte read from a 4-byte buffer.
    let _ = unsafe { read_scalar_at::<u64>(&buf, 0) };
}

#[test]
#[should_panic]
fn emplace_scalar_on_short_destination_panics_instead_of_ub() {
    let mut b = [0u8; 2];
    unsafe { emplace_scalar::<u32>(&mut b, 0xdead_beef) };
}

#[test]
#[should_panic]
fn emplace_scalar_array_past_end_of_buffer_panics_instead_of_ub() {
    // [u16; 4] needs 8 bytes; only 6 are available.
    let mut b = [0u8; 6];
    let src = [1u16, 2, 3, 4];
    unsafe { emplace_scalar_array(&mut b, 0, &src) };
}

#[test]
#[should_panic]
fn emplace_scalar_array_with_out_of_range_offset_panics_instead_of_ub() {
    // Writes bytes [4, 12) of a 10-byte buffer.
    let mut b = [0u8; 10];
    let src = [1u16, 2, 3, 4];
    unsafe { emplace_scalar_array(&mut b, 4, &src) };
}

/// Builds a buffer holding a table at offset 0 whose vtable lives at offset 8.
///
/// Layout: `[soffset -> vtable at 8][vtable: num_bytes=8, object_size=10,
/// field slots 4 and 6]`. The two bytes following the vtable are poisoned so
/// that an out-of-bounds read past the end of the vtable is observable.
fn table_with_vtable_at_end() -> Vec<u8> {
    let mut buf = vec![0u8; 18];
    buf[0..4].copy_from_slice(&(-8i32).to_le_bytes());
    buf[8..10].copy_from_slice(&8u16.to_le_bytes()); // vtable length
    buf[10..12].copy_from_slice(&10u16.to_le_bytes()); // object inline size
    buf[12..14].copy_from_slice(&4u16.to_le_bytes()); // field slot 0
    buf[14..16].copy_from_slice(&6u16.to_le_bytes()); // field slot 1
    buf[16] = 0x37; // poisoned bytes directly after the vtable
    buf[17] = 0xa5;
    buf
}

#[test]
fn vtable_get_field_never_reads_past_the_vtable() {
    let full = table_with_vtable_at_end();
    // Truncate the backing storage to the 16 bytes of the table + vtable so
    // that `get_field(num_fields)` would read outside of the slice.
    let buf = &full[..16];
    let table = unsafe { Table::new(buf, 0) };
    let vtable = table.vtable();

    assert_eq!(vtable.num_fields(), 2);
    assert_eq!(vtable.get_field(0), 4);
    assert_eq!(vtable.get_field(1), 6);
    // `idx == num_fields()` is out of range: must return the default value
    // without reading. Previously this read two bytes past the end of the
    // vtable (the poisoned bytes 0x37, 0xa5).
    assert_eq!(vtable.get_field(2), 0);
    assert_eq!(vtable.get_field(usize::MAX), 0);
}

#[test]
fn vtable_get_field_on_exact_sized_buffer_is_not_ub() {
    // Same table + vtable, but in an exact-size allocation so that an
    // out-of-bounds read past the vtable crosses the allocation boundary
    // (Miri flags this as undefined behavior).
    let full = table_with_vtable_at_end();
    let buf: Box<[u8]> = full[..16].to_vec().into_boxed_slice();
    let table = unsafe { Table::new(&buf, 0) };
    let vtable = table.vtable();

    assert_eq!(vtable.num_fields(), 2);
    assert_eq!(vtable.get_field(0), 4);
    assert_eq!(vtable.get_field(1), 6);
    assert_eq!(vtable.get_field(2), 0);
    assert_eq!(vtable.get_field(usize::MAX), 0);
}

#[test]
fn vtable_num_fields_does_not_underflow_on_short_vtable() {
    // Table at 0 with a soffset pointing at a 2-byte "vtable" at offset 4.
    // Such a vtable has no field slots; `num_fields` must saturate to 0
    // instead of underflowing (a debug panic / release wrap-around).
    let mut buf = [0u8; 8];
    buf[0..4].copy_from_slice(&(-4i32).to_le_bytes());
    buf[4..6].copy_from_slice(&2u16.to_le_bytes());

    let table = unsafe { Table::new(&buf, 0) };
    let vtable = table.vtable();
    assert_eq!(vtable.num_bytes(), 2);
    assert_eq!(vtable.num_fields(), 0);
    assert_eq!(vtable.get_field(0), 0);
}

#[test]
fn default_vector_stays_usable() {
    // `Vector`'s `Default` impl exists because a zero-length buffer cannot
    // satisfy the UOffsetT read in `len()`. Keep it panic-free.
    let v: Vector<'static, u32> = Default::default();
    assert_eq!(v.len(), 0);
    assert!(v.is_empty());
    assert_eq!(v.bytes(), &[] as &[u8]);
}
