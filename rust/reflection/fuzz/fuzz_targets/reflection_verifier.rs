#![no_main]

use libfuzzer_sys::fuzz_target;
use flatbuffers::VerifierOptions;
use flatbuffers_reflection::reflection::root_as_schema;
use flatbuffers_reflection::SafeBuffer;

/// Fuzz target for the reflection verifier.
///
/// Feeds arbitrary byte sequences to SafeBuffer::new_with_options,
/// which internally calls verify_with_options. The verifier must:
/// 1. Never panic
/// 2. Never allocate > max_tables entries (DoS protection)
/// 3. Always terminate in bounded time
/// 4. Return Ok only for structurally valid buffers
fuzz_target!(|data: &[u8]| {
    // Use a real schema to verify against.
    // The fuzz input is the DATA buffer, not the schema.
    static SCHEMA_BYTES: &[u8] = include_bytes!("../../../../tests/monster_test.bfbs");

    let schema = match root_as_schema(SCHEMA_BYTES) {
        Ok(s) => s,
        Err(_) => return,
    };

    let opts = VerifierOptions {
        max_depth: 64,
        max_tables: 1000,
        max_apparent_size: data.len().saturating_add(1024),
        ignore_missing_null_terminator: false,
    };

    // Must not panic regardless of input
    let _ = SafeBuffer::new_with_options(data, &schema, &opts);
});
