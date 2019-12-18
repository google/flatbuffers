// Copyright 2019 Google LLC
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

use crate::*;
use quickcheck;

#[test]
fn store_13() {
    let buf = singleton(13i32);
    assert_eq!(
        &buf,
        &[13, 4, 0] // TODO(cneo): Documentation says root width is 1. I don't belive it.
    );
}
#[test]
fn store_2pow20() {
    let buf = singleton(1_048_576i32);
    assert_eq!(
        &buf,
        &[
            0, 0, 16, 0,  // 2^20 in LE bytes.
            1 << 2 | 2,  // Int 32bit
            2  // Root width 32 bit
        ]
    );
}

#[test]
fn heterogenous_vector_of_string_because_width() {
    // Each string is 32 characters. They are 256 bytes altogether.
    // This forces the vector to be W16 because of the large offsets.
    let test_data = [
        "0aaabbbbccccddddeeeeffffgggghhh",
        "1aaabbbbccccddddeeeeffffgggghhh",
        "2aaabbbbccccddddeeeeffffgggghhh",
        "3aaabbbbccccddddeeeeffffgggghhh",
        "4aaabbbbccccddddeeeeffffgggghhh",
        "5aaabbbbccccddddeeeeffffgggghhh",
        "6aaabbbbccccddddeeeeffffgggghhh",
        "7aaabbbbccccddddeeeeffffgggghhh",
    ];
    let mut fxb = Builder::default();
    let mut v = fxb.build_vector();
    for &s in test_data.iter() {
        v.push(s);
    }
    v.end();
    let mut expected = vec!();
    for &s in test_data.iter() {
        expected.push(s.len() as u8);
        expected.extend(s.bytes());
        expected.push(b'\0');
    }
    expected.extend(8u16.to_le_bytes().iter());  // Length.
    for i in 0..test_data.len() as u16 {
        let offset = 32 * (8 - i) + 9 + i;
        expected.extend(offset.to_le_bytes().iter());
    }
    for _ in 0..test_data.len() {
        expected.push(5 << 2 | 0);  // String, W8.
    }
    expected.push(24); // Offset to Vector.
    expected.push(10 << 2 | 1);  // Vector, W16.
    expected.push(0);  // Root width W8.
    assert_eq!(fxb.view(), expected.as_slice());
}


#[test]
fn store_vec_uint_16() {
    let mut fxb = Builder::default();
    let mut v = fxb.build_vector();
    v.push(256u16);
    v.push(257u16);
    v.push(258u16);
    v.push(259u16);
    v.push(0u8);  // This still becomes u16.
    v.end();
    assert_eq!(
        fxb.view(),
        &[
            5, 0, 0, 1, 1, 1, 2, 1, 3, 1, 0, 0,  // Data
            10,             // Vector offset.
            12 << 2 | 1,    // (VectorUInt, W16 - referring to data).
            0,              // Root width W8 - referring to vector.
        ]
    );
}

quickcheck! {
    fn qc_f32(x: f32) -> bool {
        let fxb = singleton(x);
        let mut expected = x.to_le_bytes().to_vec();
        expected.push(3 << 2 | 2);  // Float W32.
        expected.push(2); // Root width W32.
        println!("{:?}: {:?} vs {:?} cmp {:?}", x, &fxb, &expected, fxb==expected);
        fxb == expected
    }
}


#[test]
fn singleton_vector_uint_4_16bit() {
    let buf= singleton(&[4u16, 16, 64, 256]);
    assert_eq!(
        &buf,
        &[
            4, 0, 16, 0, 64, 0, 0, 1,  // Data
            8,              // Vector offset.
            23 << 2 | 1,    // (VectorUInt, W16 - referring to data).
            0,              // Root width W8 - referring to vector.
        ]
    );
}
#[test]
fn store_u64() {
    let buf = singleton(u64::max_value() - 10);
    assert_eq!(
        &buf,
        &[
            245, 255, 255, 255, 255, 255, 255, 255,  // max value - 10.
            2 << 2 | 3,                             // (UInt, W64)
            3,                                      // Root width W64.
        ]
    );
}
#[test]
fn vector_uint4() {
    let mut fxb = Builder::default();
    let mut v = fxb.build_vector();
    v.push(2u8);
    v.push(3u8);
    v.push(5u8);
    v.push(7u8);
    v.end();
    assert_eq!(
        &fxb.view(),
        &[
            2, 3, 5, 7,  // data
            4,           // Root (offset)
            23 << 2 | 0,  // Root type VectorUInt4, BitWidth::W8
            0,           // Root bitwidth W8
        ]
    );
}
#[test]
fn nested_vector() {
    let mut fxb = Builder::default();
    let mut v = fxb.build_vector();
    v.push(0u8);
    {
        let mut nested = v.nest_vector();
        nested.push(1u8);
        nested.push(2u8);
        nested.push(3u8);
    }
    v.push(-42i8);
    v.end();
    assert_eq!(
        fxb.view(),
        &[
            1, 2, 3,        // Nested vector
            3, 0, 5, 214,   // Root Vector: size, v[0], v[1] (offset), v[2] as u8
            2 << 2 | 0,     // v[0]: (UInt, W8)
            20 << 2 | 0,    // v[1]: (VectorUInt3, W8)
            1 << 2 | 0,     // v[2]: (Int, W8)
            6,              // Root points to Root vector
            10 << 2 | 0,    // Root type and width (Vector, W8)
            0,              // Root bit width
        ]
    )
}

#[test]
fn nested_vector_push_direct() {
    let mut fxb = Builder::default();
    let mut v = fxb.build_vector();
    v.push(0u8);
    v.push(&[1u8, 2, 3]);
    v.push(-42i8);
    v.end();
    assert_eq!(
        fxb.view(),
        &[
            1, 2, 3,        // Nested VectorUInt3
            3, 0, 5, 214,   // Root Vector: size, v[0], v[1] (offset), v[2] as u8
            2 << 2 | 0,     // v[0]: (UInt, W8)
            20 << 2 | 0,    // v[1]: (VectorUInt3, W8)
            1 << 2 | 0,     // v[2]: (Int, W8)
            6,              // Root points to Root vector
            10 << 2 | 0,    // Root type and width (Vector, W8)
            0,              // Root bit width
        ]
    )
}
#[test]
fn store_map_index_into_it() {
    let mut fxb = Builder::default();
    {
    let mut m = fxb.build_map();
    m.push("foo", 17u8);
    m.push("bar", 33u16);
    m.push("baz", 41u32);
    }
    assert_eq!(
        fxb.view(),
        &[
            b'f', b'o', b'o', b'\0',
            b'b', b'a', b'r', b'\0',
            b'b', b'a', b'z', b'\0',
            3, 9, 6, 15,    // Keys vector (note "bar" < "baz" < "foo").
            3, 0, 3,        // map prefix
            33, 41, 17,     // values
            8, 8, 8,        // types (UInt, W8) ~ (2 << 2 | 0)
            6,              // Offset to map (root)
            9 << 2 | 0,     // Root type (map)
            0,              // Root width
        ]
    );
}
#[test]
fn utf8_snowman() {
    let buf = singleton("snowman ☃︎");
    assert_eq!(
        &buf,
        &[
            14,     // Byte length (besides extra null terminator).
            b's', b'n', b'o', b'w', b'm', b'a', b'n', b' ',
            226, 152, 131,  // snowman bytes
            239, 184, 142,  // UTF Variation selector 15
            0,      // extra null terminator.
            15,     // Offset to string start.
            5 << 2,  // String, W8
            0       // Root width
        ]
    );
    let r = Reader::get_root(&buf).unwrap();
    assert_eq!(r.get_str(), Ok("snowman ☃︎"));
}
#[test]
fn indirect_numbers() {
    let mut fxb = Builder::default();
    let mut v = fxb.build_vector();
    v.push(IndirectUInt(u64::max_value()));
    v.push(IndirectInt(i64::min_value()));
    // TODO(cneo): Something about Float EPSILON and casting leads to a different binary format.
    v.push(IndirectFloat(std::f64::consts::PI));
    v.push(0u32);  // This is stored in 8 bits instead of 64 because of indirection.
    v.end();
    assert_eq!(
        fxb.view(),
        vec![
            255, 255, 255, 255, 255, 255, 255, 255,  // u64 max
            0, 0, 0, 0, 0, 0, 0, 128,  // i64 min value
            24, 45, 68, 84, 251, 33, 9, 64,  // f64 PI.
            4,  // Vector length
            25, 18, 11, 0,  // offsets to the indirect numbers and zero.
            7 << 2 | 3,  // IndirectUInt 64 bit
            6 << 2 | 3,  // IndirectInt 64 bit
            8 << 2 | 3,  // IndirectFloat 64 bit
            2 << 2 | 0,  // (inline) UInt 8 bit
            8,  // Offset to Root.
            10 << 2 | 0,  // Vector 8 bit
            0u8,  // 8 bit root
        ].as_slice()
    )
}
#[test]
fn indirect_2p5x_smaller() {
    let mut builder = Builder::default();
    let mut v = builder.build_vector();
    for i in 0..512 {
        v.push(i);
    }
    v.push(i64::max_value());
    v.end();
    let len_without_indirect = builder.view().len() as f32;

    let mut v = builder.build_vector();
    for i in 0..512 {
        v.push(i);
    }
    v.push(IndirectInt(i64::max_value()));
    v.end();
    let len_with_indirect = builder.view().len() as f32;
    dbg!(len_with_indirect, len_without_indirect);
    assert!(len_with_indirect * 2.5 < len_without_indirect);
}
#[test]
fn key_pool() {
    let mut builder = Builder::new();
    let mut vector = builder.build_vector();
    for _ in 0..2 {
        let mut m = vector.nest_map();
        m.push("a", 42u8);
        m.push("b", 42u8);
        m.push("c", 42u8);
    }
    vector.end();

    assert_eq!(
        builder.view(),
        vec![
            b'a', b'\0',
            b'b', b'\0',
            b'c', b'\0',
            3, 7, 6, 5,  // Key vector 0
            3, 0, 3, 42, 42, 42, 2<<2, 2<<2, 2<<2,  // Map 0.
            3, 20, 19, 18,  // Key vector 1 (shares keys with key vector 0).
            3, 0, 3, 42, 42, 42, 2<<2, 2<<2, 2<<2,  // Map 1.
            2, 20, 8, 9<<2, 9<<2,  // Vector containing the maps.
            4, 10 << 2, 0,  // Root.
        ].as_slice()
    );
}
