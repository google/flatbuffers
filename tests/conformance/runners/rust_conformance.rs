//! Rust conformance runner for cross-language verifier testing.
//!
//! Reads all .bin files from the conformance corpus, runs the FlatBuffers
//! verifier on each, and outputs JSON results for comparison.
//!
//! Usage: cargo run --bin rust_conformance -- ../corpus/ > rust_results.json

use flatbuffers::{VerifierOptions, Verifier};
use std::collections::BTreeMap;
use std::fs;
use std::path::{Path, PathBuf};

fn main() {
    let corpus_dir = std::env::args()
        .nth(1)
        .unwrap_or_else(|| "tests/conformance/corpus".to_string());

    let opts = VerifierOptions {
        max_depth: 64,
        max_tables: 1_000_000,
        max_apparent_size: 1_073_741_824,
        ignore_missing_null_terminator: false,
    };

    let mut results: BTreeMap<String, serde_json::Value> = BTreeMap::new();

    for category in &["valid", "malicious", "edge"] {
        let dir = PathBuf::from(&corpus_dir).join(category);
        if !dir.exists() {
            continue;
        }

        let mut entries: Vec<_> = fs::read_dir(&dir)
            .expect("Failed to read corpus directory")
            .filter_map(|e| e.ok())
            .filter(|e| {
                e.path()
                    .extension()
                    .is_some_and(|ext| ext == "bin")
            })
            .collect();

        entries.sort_by_key(|e| e.file_name());

        for entry in entries {
            let path = entry.path();
            let buf = fs::read(&path).expect("Failed to read corpus file");
            let key = format!("{}/{}", category, path.file_name().unwrap().to_string_lossy());

            let result = verify_buffer(&buf, &opts);
            results.insert(key, result);
        }
    }

    println!("{}", serde_json::to_string_pretty(&results).unwrap());
}

fn verify_buffer(buf: &[u8], opts: &VerifierOptions) -> serde_json::Value {
    let mut verifier = Verifier::new(opts, buf);

    // Try to verify as a generic table (read root offset, verify table structure)
    match verifier.get_uoffset(0) {
        Ok(root_offset) => {
            let table_pos: usize = match root_offset.try_into() {
                Ok(pos) => pos,
                Err(_) => {
                    return serde_json::json!({
                        "accept": false,
                        "error_kind": "RangeOutOfBounds"
                    });
                }
            };

            match verifier.visit_table(table_pos) {
                Ok(tv) => {
                    tv.finish();
                    serde_json::json!({"accept": true})
                }
                Err(e) => {
                    serde_json::json!({
                        "accept": false,
                        "error_kind": classify_error(&e)
                    })
                }
            }
        }
        Err(e) => {
            serde_json::json!({
                "accept": false,
                "error_kind": classify_error(&e)
            })
        }
    }
}

fn classify_error(e: &flatbuffers::InvalidFlatbuffer) -> &'static str {
    match e {
        flatbuffers::InvalidFlatbuffer::MissingRequiredField { .. } => "MissingRequiredField",
        flatbuffers::InvalidFlatbuffer::InconsistentUnion { .. } => "InconsistentUnion",
        flatbuffers::InvalidFlatbuffer::Utf8Error { .. } => "Utf8Error",
        flatbuffers::InvalidFlatbuffer::MissingNullTerminator { .. } => "MissingNullTerminator",
        flatbuffers::InvalidFlatbuffer::Unaligned { .. } => "Unaligned",
        flatbuffers::InvalidFlatbuffer::RangeOutOfBounds { .. } => "RangeOutOfBounds",
        flatbuffers::InvalidFlatbuffer::SignedOffsetOutOfBounds { .. } => "SignedOffsetOutOfBounds",
        flatbuffers::InvalidFlatbuffer::TooManyTables => "TooManyTables",
        flatbuffers::InvalidFlatbuffer::ApparentSizeTooLarge => "ApparentSizeTooLarge",
        flatbuffers::InvalidFlatbuffer::DepthLimitReached => "DepthLimitReached",
    }
}
