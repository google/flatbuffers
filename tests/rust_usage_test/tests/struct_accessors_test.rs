//! Tests for the accessors `flatc` generates for Rust structs.
//!
//! Struct getters and setters are generated without raw pointer operations:
//! every field access is a bounds-checked slice of the struct's `[u8; N]`
//! storage. These tests pin down the exact little-endian wire bytes those
//! accessors produce, so they also catch endianness bugs when run on a
//! big-endian target (for example `cargo miri test --target
//! s390x-unknown-linux-gnu`, or the project's MIPS CI job).
#![no_std]

#[cfg(not(feature = "no_std"))]
extern crate std;

extern crate alloc;

#[allow(dead_code, unused_imports, clippy::all)]
#[path = "../../rust_struct_accessors_test/mod.rs"]
mod rust_struct_accessors_test_generated;

use crate::rust_struct_accessors_test_generated::rust_struct_accessors_test::*;

/// `HugeEnum::Y` in little-endian byte order.
const HUGE_Y_LE: [u8; 8] = [0x08, 0x07, 0x06, 0x05, 0x04, 0x03, 0x02, 0x01];

fn example() -> Accessors {
    Accessors::new(
        &[WideEnum::B, WideEnum::C, WideEnum::A],
        &[HugeEnum::Y, HugeEnum::X],
        &[Inner::new(0xA1B2), Inner::new(0x0304)],
        1.5,
        &[true, false],
        WideEnum::C,
        &Inner::new(0xBEEF),
        -2.0,
    )
}

/// The wire bytes of `example()`, written out by hand from the schema layout.
#[rustfmt::skip]
const EXAMPLE_BYTES: [u8; 48] = [
    // e: [WideEnum; 3] at 0 = [B (0x0102), C (-2), A (1)]
    0x02, 0x01, 0xFE, 0xFF, 0x01, 0x00,
    // padding
    0x00, 0x00,
    // h: [HugeEnum; 2] at 8 = [Y, X]
    0x08, 0x07, 0x06, 0x05, 0x04, 0x03, 0x02, 0x01,
    0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    // inners: [Inner; 2] at 24
    0xB2, 0xA1, 0x04, 0x03,
    // f: f32 at 28 = 1.5 (0x3FC00000)
    0x00, 0x00, 0xC0, 0x3F,
    // flags: [bool; 2] at 32
    0x01, 0x00,
    // one: WideEnum at 34 = C (-2)
    0xFE, 0xFF,
    // nested: Inner at 36
    0xEF, 0xBE,
    // padding
    0x00, 0x00,
    // d: f64 at 40 = -2.0 (0xC000000000000000)
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xC0,
];

fn check_example_getters(a: &Accessors) {
    assert_eq!(a.e().iter().collect::<alloc::vec::Vec<_>>(), [WideEnum::B, WideEnum::C, WideEnum::A]);
    assert_eq!(a.h().get(0), HugeEnum::Y);
    assert_eq!(a.h().get(1), HugeEnum::X);
    assert_eq!(a.inners().get(0).v(), 0xA1B2);
    assert_eq!(a.inners().get(1).v(), 0x0304);
    assert_eq!(a.f(), 1.5);
    assert_eq!(a.flags().get(0), true);
    assert_eq!(a.flags().get(1), false);
    assert_eq!(a.one(), WideEnum::C);
    assert_eq!(a.nested().v(), 0xBEEF);
    assert_eq!(a.d(), -2.0);
}

#[test]
fn struct_setters_write_little_endian_wire_bytes() {
    assert_eq!(example().0, EXAMPLE_BYTES);
}

#[test]
fn struct_getters_read_little_endian_wire_bytes() {
    check_example_getters(&Accessors(EXAMPLE_BYTES));
}

#[test]
fn struct_enum_array_setter_converts_each_element_to_little_endian() {
    // Regression test: arrays of enums used to be copied with a raw
    // `ptr::copy` of their in-memory (host-endian) representation, which
    // produced big-endian wire bytes on big-endian hosts.
    let mut a = Accessors::default();
    a.set_e(&[WideEnum::B, WideEnum::C, WideEnum::A]);
    assert_eq!(a.0[0..6], [0x02, 0x01, 0xFE, 0xFF, 0x01, 0x00]);
    a.set_h(&[HugeEnum::Y, HugeEnum::X]);
    assert_eq!(a.0[8..16], HUGE_Y_LE);
    assert_eq!(a.0[16..24], [1, 0, 0, 0, 0, 0, 0, 0]);
}

#[test]
fn struct_setters_only_touch_their_own_bytes() {
    let mut a = Accessors([0x55; 48]);
    a.set_one(WideEnum::B);
    a.set_nested(&Inner::new(0x0A0B));
    a.set_flags(&[false, true]);
    for (i, byte) in a.0.iter().enumerate() {
        let expected = match i {
            32 => 0x00,
            33 => 0x01,
            34 => 0x02,
            35 => 0x01,
            36 => 0x0B,
            37 => 0x0A,
            _ => 0x55,
        };
        assert_eq!(*byte, expected, "unexpected byte at offset {}", i);
    }
}

#[test]
fn struct_round_trips_through_builder_and_verifier() {
    let mut builder = flatbuffers::FlatBufferBuilder::new();
    let a = example();
    let root = Holder::create(&mut builder, &HolderArgs { a: Some(&a) });
    builder.finish(root, None);

    let holder = root_as_holder(builder.finished_data()).unwrap();
    let read = holder.a().unwrap();
    assert_eq!(read.0, EXAMPLE_BYTES);
    check_example_getters(read);
}

#[test]
fn struct_object_api_round_trips() {
    let unpacked = example().unpack();
    assert_eq!(unpacked.e, [WideEnum::B, WideEnum::C, WideEnum::A]);
    assert_eq!(unpacked.h, [HugeEnum::Y, HugeEnum::X]);
    assert_eq!(unpacked.pack().0, EXAMPLE_BYTES);
}

#[test]
fn generated_struct_accessors_do_not_use_raw_pointer_operations() {
    // Guards the code generator: struct accessors must stay expressible as
    // bounds-checked slice operations on the struct's byte array.
    let sources = [
        include_str!("../../rust_struct_accessors_test/rust_struct_accessors_test/accessors_generated.rs"),
        include_str!("../../rust_struct_accessors_test/rust_struct_accessors_test/inner_generated.rs"),
    ];
    for source in sources.iter() {
        for forbidden in [
            "copy_nonoverlapping",
            "ptr::copy",
            "from_raw_parts",
            "MaybeUninit",
            "emplace_scalar_array",
            "as *const",
            "as *mut",
        ]
        .iter()
        {
            assert!(!source.contains(forbidden), "generated struct code uses `{}`", forbidden);
        }
    }
}
