//! Compile + behaviour coverage for `--rust-object-api-hashable-floats`.
//!
//! The flag wraps Object API float fields in `flatbuffers::ordered_float::OrderedFloat`
//! so the generated `*T` structs can derive `Eq` and `Hash`. This test exists to keep
//! that output compiling across upstream merges and to prove the derived `Eq`/`Hash`
//! actually work (the entire point of the flag). `optional_scalars` covers
//! just/optional/defaulted `f32` and `f64`, i.e. every float code path the flag touches.
#[allow(dead_code, unused_imports)]
#[path = "../../optional_scalars_ordered_floats/mod.rs"]
mod optional_scalars_ordered_floats_generated;

use crate::optional_scalars_ordered_floats_generated::optional_scalars::*;
use flatbuffers::ordered_float::OrderedFloat;
use std::collections::HashSet;

fn sample() -> ScalarStuffT {
    ScalarStuffT {
        just_f32: OrderedFloat(1.5),
        maybe_f32: Some(OrderedFloat(2.5)),
        default_f32: OrderedFloat(42.0),
        just_f64: OrderedFloat(3.5),
        maybe_f64: Some(OrderedFloat(4.5)),
        default_f64: OrderedFloat(42.0),
        ..Default::default()
    }
}

#[test]
fn object_api_floats_derive_eq_and_hash() {
    // `Eq` + `Hash` on `ScalarStuffT` are only derivable because the float fields
    // are wrapped in `OrderedFloat`; this would not compile without the flag.
    let mut set: HashSet<ScalarStuffT> = HashSet::new();
    set.insert(sample());
    assert!(set.contains(&sample()));
    assert_eq!(sample(), sample());
}

#[test]
fn object_api_floats_round_trip() {
    let obj = sample();

    let mut builder = flatbuffers::FlatBufferBuilder::new();
    let off = obj.pack(&mut builder);
    builder.finish(off, None);

    let read = flatbuffers::root::<ScalarStuff>(builder.finished_data()).unwrap();
    let unpacked = read.unpack();

    assert_eq!(unpacked, obj);
    assert_eq!(unpacked.just_f32.into_inner(), 1.5);
    assert_eq!(unpacked.maybe_f32, Some(OrderedFloat(2.5)));
    assert_eq!(unpacked.maybe_f64, Some(OrderedFloat(4.5)));
}
