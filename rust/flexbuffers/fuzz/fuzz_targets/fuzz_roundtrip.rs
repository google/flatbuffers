// Fuzz target: roundtrip encode→decode must never panic on any Reader accessor.
//
// Run with:
//   cargo +nightly fuzz run fuzz_roundtrip

#![no_main]

use libfuzzer_sys::{arbitrary, fuzz_target, Corpus};
use flexbuffers::Reader;

fuzz_target!(|data: &[u8]| -> Corpus {
    // Only test parseable inputs.
    if Reader::get_root(data).is_err() {
        return Corpus::Reject;
    }
    let reader = Reader::get_root(data).unwrap();

    // Any of these panicking is a bug in the Reader.
    let _ = reader.as_bool();
    let _ = reader.as_u64();
    let _ = reader.as_i64();
    let _ = reader.as_f64();
    let _ = reader.as_str();
    let _ = reader.length();

    Corpus::Keep
});
