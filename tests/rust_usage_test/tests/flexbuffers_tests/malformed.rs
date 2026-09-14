// Copyright 2026 Google LLC
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     https://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

// NOTE: 32-bit tests are excluded because the active Rust CI targets are 64-bit.

use flexbuffers::{BitWidth, FlexBufferType, Reader, ReaderError};

fn packed_type(fxb_type: FlexBufferType, width: BitWidth) -> u8 {
    (fxb_type as u8) << 2 | width as u8
}

fn assert_out_of_bounds<T>(result: Result<T, ReaderError>) {
    assert_eq!(result.err(), Some(ReaderError::FlexbufferOutOfBounds));
}

fn w64_length_root(fxb_type: FlexBufferType, length: u64) -> [u8; 12] {
    let mut buffer = [0; 12];
    buffer[..8].copy_from_slice(&length.to_le_bytes());
    buffer[8] = 0; // First byte of the referenced value.
    buffer[9] = 1; // Root offset to byte 8.
    buffer[10] = packed_type(fxb_type, BitWidth::W64);
    buffer[11] = 1; // Root offset width.
    buffer
}

fn map_buffer() -> [u8; 12] {
    [
        b'a', // Key.
        0,
        1,      // Key count.
        3,      // Backward key offset.
        1,      // Key-vector offset.
        1,      // Key width.
        1,      // Value count.
        33,     // Value.
        2 << 2, // UInt/W8 type.
        2,      // Root offset.
        9 << 2, // Map/W8 type.
        1,      // Root width.
    ]
}

fn w64_map_with_max_lengths() -> [u8; 46] {
    let mut buffer = [0; 46];
    buffer[0] = b'a';
    buffer[2..10].copy_from_slice(&u64::MAX.to_le_bytes()); // Key count.
    buffer[10..18].copy_from_slice(&10u64.to_le_bytes()); // Key offset.
    buffer[18..26].copy_from_slice(&8u64.to_le_bytes()); // Key-vector offset.
    buffer[26..34].copy_from_slice(&8u64.to_le_bytes()); // Key width.
    buffer[34..42].copy_from_slice(&u64::MAX.to_le_bytes()); // Value count.
    buffer[43] = 1; // Root offset to byte 42.
    buffer[44] = packed_type(FlexBufferType::Map, BitWidth::W64);
    buffer[45] = 1; // Root offset width.
    buffer
}

#[test]
fn oversized_bool_is_rejected() {
    let buffer = [1, packed_type(FlexBufferType::Bool, BitWidth::W64), 1];
    let reader = Reader::get_root(buffer.as_ref()).unwrap();

    assert_out_of_bounds(reader.get_bool());
}

#[test]
fn string_end_overflow_is_rejected() {
    let buffer = w64_length_root(FlexBufferType::String, u64::max_value());
    let reader = Reader::get_root(buffer.as_ref()).unwrap();

    assert_eq!(
        reader.get_str().err(),
        Some(ReaderError::FlexbufferOutOfBounds)
    );
}

#[test]
fn blob_end_overflow_is_rejected() {
    let buffer = w64_length_root(FlexBufferType::Blob, u64::max_value());
    let reader = Reader::get_root(buffer.as_ref()).unwrap();

    assert_eq!(
        reader.get_blob().err(),
        Some(ReaderError::FlexbufferOutOfBounds)
    );
}

#[test]
fn unterminated_key_is_rejected() {
    let buffer = [b'a', 1, packed_type(FlexBufferType::Key, BitWidth::W8), 1];
    let reader = Reader::get_root(buffer.as_ref()).unwrap();

    assert_out_of_bounds(reader.get_key());
}

#[test]
fn vector_missing_length_slot_is_rejected() {
    let buffer = [0, packed_type(FlexBufferType::VectorUInt, BitWidth::W64), 1];
    let reader = Reader::get_root(buffer.as_ref()).unwrap();

    assert_out_of_bounds(reader.get_vector());
}

// NOTE: same bug existed on w16 and w32. w64 was chosen to demonstrates all.
#[test]
fn truncated_w64_offset_is_rejected() {
    let buffer = w64_length_root(FlexBufferType::VectorKey, 1);
    let vector = Reader::get_root(buffer.as_ref())
        .unwrap()
        .get_vector()
        .unwrap();

    assert_out_of_bounds(vector.index(0)); // used to return `Error::KeyNotFound`
}

#[test]
fn vector_index_overflow_is_rejected() {
    let buffer = w64_length_root(FlexBufferType::VectorUInt, u64::max_value());
    let vector = Reader::get_root(buffer.as_ref())
        .unwrap()
        .get_vector()
        .unwrap();

    assert_out_of_bounds(vector.index(usize::max_value() - 1));
}

#[test]
fn untyped_vector_type_table_overflow_is_rejected() {
    let buffer = w64_length_root(FlexBufferType::Vector, u64::MAX);
    let vector = Reader::get_root(buffer.as_ref())
        .unwrap()
        .get_vector()
        .unwrap();

    assert_out_of_bounds(vector.index(0));
}

#[cfg(target_endian = "little")]
#[test]
#[allow(deprecated)]
fn typed_slice_byte_length_overflow_is_rejected() {
    let buffer = w64_length_root(FlexBufferType::VectorUInt, u64::max_value());
    let reader = Reader::get_root(buffer.as_ref()).unwrap();

    assert_eq!(
        reader.get_slice::<u64>().err(),
        Some(ReaderError::FlexbufferOutOfBounds)
    );
}

#[test]
fn unterminated_map_key_is_rejected() {
    let mut buffer = map_buffer();
    buffer[1] = b'b';
    let key = std::str::from_utf8(&buffer).unwrap();
    let map = Reader::get_root(buffer.as_ref())
        .unwrap()
        .get_map()
        .unwrap();

    assert_out_of_bounds(map.index(key));
}

#[test]
fn map_key_and_value_length_mismatch_is_rejected() {
    let mut buffer = map_buffer();
    buffer[2] = 0;
    let reader = Reader::get_root(buffer.as_ref()).unwrap();

    assert_out_of_bounds(reader.get_map());
}

#[test]
fn invalid_map_key_offset_is_rejected() {
    let mut buffer = map_buffer();
    buffer[3] = u8::max_value();
    let map = Reader::get_root(buffer.as_ref())
        .unwrap()
        .get_map()
        .unwrap();

    assert_out_of_bounds(map.index("a"));
}

#[test]
fn map_key_offset_index_overflow_is_rejected() {
    let buffer = w64_map_with_max_lengths();
    let map = Reader::get_root(buffer.as_ref())
        .unwrap()
        .get_map()
        .unwrap();

    assert_out_of_bounds(map.index("a"));
}

#[test]
fn map_value_index_overflow_is_rejected() {
    let buffer = w64_map_with_max_lengths();
    let map = Reader::get_root(buffer.as_ref())
        .unwrap()
        .get_map()
        .unwrap();

    assert_out_of_bounds(map.index(usize::MAX - 1));
}

#[test]
fn map_type_table_overflow_is_rejected() {
    let buffer = w64_map_with_max_lengths();
    let map = Reader::get_root(buffer.as_ref())
        .unwrap()
        .get_map()
        .unwrap();

    assert_out_of_bounds(map.index(0usize));
}
