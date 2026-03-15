// Fuzz target: exhaustively exercise the FlexBuffers Reader API with arbitrary input.
//
// Exercises every public accessor on Reader so that any panic from malformed
// data is caught by the fuzzer. Before the fix for read_usize, this harness
// would quickly find the crash with a 4-byte payload.
//
// Run with:
//   cargo +nightly fuzz run fuzz_reader

#![no_main]

use libfuzzer_sys::fuzz_target;
use flexbuffers::Reader;

fuzz_target!(|data: &[u8]| {
    let Ok(reader) = Reader::get_root(data) else { return };

    // Exercise every accessor — any panic here is a bug.
    let _ = reader.as_bool();
    let _ = reader.as_u8();
    let _ = reader.as_u16();
    let _ = reader.as_u32();
    let _ = reader.as_u64();
    let _ = reader.as_i8();
    let _ = reader.as_i16();
    let _ = reader.as_i32();
    let _ = reader.as_i64();
    let _ = reader.as_f32();
    let _ = reader.as_f64();
    let _ = reader.as_str();
    let _ = reader.as_bytes();
    let _ = reader.length();
    let _ = reader.flexbuffer_type();
    let _ = reader.bitwidth();

    // Exercise map reader if applicable.
    if let Ok(map) = reader.as_map().ok().map(|_| reader.as_map()) {
        if let Ok(m) = map {
            for i in 0..m.len().min(16) {
                let _ = m.idx(i);
            }
        }
    }

    // Exercise vector reader if applicable.
    if let Ok(vec) = reader.as_vector().ok().map(|_| reader.as_vector()) {
        if let Ok(v) = vec {
            for i in 0..v.len().min(16) {
                let _ = v.idx(i);
            }
        }
    }
});
