#![no_main]

//! Fuzz target for the safe reader path of the FlatBuffers Rust runtime.
//!
//! The C++ fuzzers under `tests/fuzzer/` exercise the C++ verifier, but the
//! Rust runtime (`rust/flatbuffers`) had no fuzz coverage at all. Its
//! `flatbuffers::root::<T>()` entry point promises that any buffer it accepts
//! can then be read through the *safe* accessor API without out-of-bounds
//! accesses. This target closes that gap: it feeds arbitrary bytes to
//! `root::<T>()` and, whenever verification succeeds, reads the resulting
//! `Monster` back through the safe (non-`unsafe`) accessors and through the
//! safe `Table`/`VTable` API. A hole in the verifier therefore surfaces here as
//! a panic or an AddressSanitizer report instead of as silent memory
//! corruption in a downstream user's program.
//!
//! Build and run (from `rust/flatbuffers`):
//!
//! ```sh
//! cargo +nightly fuzz build rust_verifier
//! cargo +nightly fuzz run rust_verifier fuzz/corpus/rust_verifier
//! ```

use flatbuffers::Table;
use libfuzzer_sys::fuzz_target;

// Reuse the schema-generated types from the Rust test-suite
// (`tests/monster_test/mod.rs`) so the fuzzer exercises exactly the code that
// `flatc` emits for users: the same `Verifiable` impls, the same union
// machinery and the same safe accessors.
#[allow(dead_code, unused_imports, clippy::all)]
#[path = "../../../../tests/monster_test/mod.rs"]
mod monster_test_generated;

use monster_test_generated::my_game::example::Monster;

/// Walks the safe `Table`/`VTable` API. `VTable::num_fields`,
/// `VTable::object_inline_num_bytes` and `VTable::get_field` all assume a
/// well-formed vtable; a degenerate vtable that the verifier accepted makes
/// these reads escape the buffer.
fn probe_table(t: &Table) {
    let vtable = t.vtable();
    let _ = vtable.num_bytes();
    let _ = vtable.object_inline_num_bytes();
    // `num_fields` wraps on a degenerate (short) vtable, so do not use it as
    // the loop bound.
    let num_fields = vtable.num_fields();
    for i in 0..8usize.min(num_fields.saturating_add(1)) {
        let _ = vtable.get_field(i);
    }
    let _ = vtable.as_bytes();
}

/// Reads a `Monster` through the safe accessor API. Every scalar read goes
/// through `Table::get` -> `VTable::get`, and struct/vector/table fields follow
/// offsets that the verifier is supposed to have checked.
fn read_monster(m: &Monster, depth: u32) {
    if depth > 16 {
        return;
    }

    // Scalars / enums.
    let _ = m.hp();
    let _ = m.mana();
    let _ = m.color();
    let _ = m.signed_enum();
    let _ = m.testbool();
    let _ = m.testf();
    let _ = m.testf2();
    let _ = m.testf3();

    // Inline struct field.
    if let Some(p) = m.pos() {
        let _ = (p.x(), p.y(), p.z(), p.test1());
    }

    // Vectors (including vectors of strings and of tables).
    if let Some(v) = m.inventory() {
        for x in v {
            let _ = x;
        }
    }
    if let Some(v) = m.testarrayofstring() {
        for s in v {
            let _ = s.len();
        }
    }
    if let Some(v) = m.testarrayoftables() {
        for t in v {
            read_monster(&t, depth + 1);
        }
    }
    if let Some(v) = m.test4() {
        for e in v {
            let _ = (e.a(), e.b());
        }
    }
    if let Some(v) = m.vector_of_longs() {
        for x in v {
            let _ = x;
        }
    }
    if let Some(v) = m.vector_of_doubles() {
        for x in v {
            let _ = x;
        }
    }
    if let Some(v) = m.vector_of_referrables() {
        for r in v {
            let _ = r.id();
        }
    }

    // Union values. The value side of a union is only fully verified when the
    // reader knows its discriminant; for unknown discriminants the runtime
    // still hands back a `Table`, so probe it through the safe API.
    if let Some(t) = m.test() {
        probe_table(&t);
    }
    if let Some(t) = m.any_unique() {
        probe_table(&t);
    }
    if let Some(t) = m.any_ambiguous() {
        probe_table(&t);
    }
    if let Some(x) = m.test_as_monster() {
        read_monster(&x, depth + 1);
    }
    if let Some(x) = m.test_as_test_simple_table_with_enum() {
        let _ = x.color();
    }
    if let Some(x) = m.any_unique_as_m() {
        read_monster(&x, depth + 1);
    }
    if let Some(x) = m.any_unique_as_ts() {
        let _ = x.color();
    }
    if let Some(x) = m.any_ambiguous_as_m1() {
        read_monster(&x, depth + 1);
    }
    if let Some(x) = m.any_ambiguous_as_m2() {
        read_monster(&x, depth + 1);
    }
    if let Some(x) = m.any_ambiguous_as_m3() {
        read_monster(&x, depth + 1);
    }

    // The root table itself, through the safe Table/VTable API.
    probe_table(&m._tab);
}

fn harness(data: &[u8]) {
    if let Ok(m) = flatbuffers::root::<Monster>(data) {
        read_monster(&m, 0);
    }
    if let Ok(m) = flatbuffers::size_prefixed_root::<Monster>(data) {
        read_monster(&m, 0);
    }
}

fuzz_target!(|data: &[u8]| {
    harness(data);
});
