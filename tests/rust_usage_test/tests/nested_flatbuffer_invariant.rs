//! Differential check for the invariant the Rust docs promise:
//!
//!   "All of the safe Rust APIs ensure the verifier is run over these
//!    flatbuffers before accessing them."
//!
//! Concretely: if `root::<Monster>()` accepts a buffer, then reading the nested
//! flatbuffer out of it through the generated accessor must not panic and must
//! not produce a `&str` that is not valid UTF-8.
//!
//! The nested payload is entirely attacker-controlled, so this mutates it and
//! checks the invariant over every mutant the verifier accepts. Mutation is
//! driven by a fixed-seed LCG so failures are reproducible and CI is stable.
#![allow(dead_code, unused_imports)]

#[allow(dead_code, unused_imports, clippy::approx_constant)]
#[path = "../../monster_test/mod.rs"]
mod monster_test_generated;
pub use monster_test_generated::my_game;
use my_game::example::{Monster, MonsterArgs};

use std::panic::{catch_unwind, AssertUnwindSafe};

fn valid_nested(name: &str) -> Vec<u8> {
    let mut b = flatbuffers::FlatBufferBuilder::new();
    let n = b.create_string(name);
    let inv = b.create_vector::<u8>(&[1, 2, 3, 4]);
    let m = Monster::create(
        &mut b,
        &MonsterArgs { name: Some(n), hp: 1234, inventory: Some(inv), ..Default::default() },
    );
    b.finish(m, None);
    b.finished_data().to_vec()
}

fn outer_with(payload: &[u8]) -> Vec<u8> {
    let mut b = flatbuffers::FlatBufferBuilder::new();
    let name = b.create_string("outer");
    let nested = b.create_vector::<u8>(payload);
    let m = Monster::create(
        &mut b,
        &MonsterArgs { name: Some(name), testnestedflatbuffer: Some(nested), ..Default::default() },
    );
    b.finish(m, None);
    b.finished_data().to_vec()
}

/// Exercises the nested buffer the way an application would. Returns Err if the
/// product misbehaved: a panic, or a `&str` that is not valid UTF-8.
fn traverse(data: &[u8], opts: &flatbuffers::VerifierOptions) -> Result<bool, String> {
    let monster = match flatbuffers::root_with_opts::<Monster>(opts, data) {
        Ok(m) => m,
        Err(_) => return Ok(false), // rejected: nothing to check
    };
    let res = catch_unwind(AssertUnwindSafe(|| {
        let nested = match monster.testnestedflatbuffer_nested_flatbuffer() {
            Some(n) => n,
            None => return Ok(()),
        };
        // Ordinary field reads, the same as any application would do.
        let name: &str = nested.name();
        if core::str::from_utf8(name.as_bytes()).is_err() {
            return Err(format!("accessor produced non-UTF-8 &str: {:02X?}", name.as_bytes()));
        }
        let _ = nested.hp();
        let _ = nested.mana();
        if let Some(inv) = nested.inventory() {
            for i in 0..inv.len() {
                let _ = inv.get(i);
            }
        }
        if let Some(v) = nested.testarrayofstring() {
            for i in 0..v.len() {
                let s = v.get(i);
                if core::str::from_utf8(s.as_bytes()).is_err() {
                    return Err("vector element produced non-UTF-8 &str".to_string());
                }
            }
        }
        Ok(())
    }));
    match res {
        Err(_) => Err("panicked while reading a verified buffer".to_string()),
        Ok(Err(e)) => Err(e),
        Ok(Ok(())) => Ok(true),
    }
}

/// Result of one mutation sweep.
struct Sweep {
    /// Buffers the verifier accepted and which traversed cleanly.
    accepted: usize,
    /// Buffers the verifier accepted but which then misbehaved. This is the
    /// number that must be zero.
    misbehaved: usize,
    /// First few distinct failures, for the assertion message. Kept separate
    /// from `misbehaved` so the count is never truncated by the sample limit.
    examples: Vec<String>,
}

/// Deterministic mutation sweep over the attacker-controlled nested payload.
fn sweep(opts: &flatbuffers::VerifierOptions) -> Sweep {
    let base = valid_nested("AAAA");
    let mut state: u64 = 0x5EED_1234_ABCD_0001;
    let mut next = || {
        state = state.wrapping_mul(6364136223846793005).wrapping_add(1442695040888963407);
        (state >> 33) as usize
    };
    let mut out = Sweep { accepted: 0, misbehaved: 0, examples: Vec::new() };
    for _ in 0..4000 {
        let mut payload = base.clone();
        // Corrupt one to three bytes anywhere in the nested buffer.
        let n = 1 + next() % 3;
        for _ in 0..n {
            let idx = next() % payload.len();
            payload[idx] = (next() % 256) as u8;
        }
        let data = outer_with(&payload);
        match traverse(&data, opts) {
            Ok(true) => out.accepted += 1,
            Ok(false) => {}
            Err(e) => {
                out.misbehaved += 1;
                if out.examples.len() < 5 && !out.examples.contains(&e) {
                    out.examples.push(e);
                }
            }
        }
    }
    out
}

/// With nested verification on (the default), every buffer the verifier accepts
/// must survive a full traversal.
#[test]
#[cfg(not(miri))] // slow.
fn accepted_buffers_are_safe_to_traverse() {
    let opts = flatbuffers::VerifierOptions::default();
    assert!(opts.check_nested_flatbuffers, "nested checking must default to on");
    let s = sweep(&opts);
    assert!(s.accepted > 0, "no buffer was accepted; the sweep would prove nothing");
    assert_eq!(
        s.misbehaved, 0,
        "verifier accepted {} buffers, {} of which misbehaved when read: {:?}",
        s.accepted, s.misbehaved, s.examples
    );
}

/// The verifier must never panic, whatever it is handed. The sweep above only
/// corrupts the nested payload, leaving the outer buffer well-formed, so it
/// never exercises a hostile *vector header* -- and the length field of that
/// header is what determines the range handed to the nested verifier.
///
/// This corrupts the whole outer buffer instead, so the length, the offsets and
/// the vtable are all attacker-controlled. Any panic here is a denial-of-service
/// bug, so the assertion is simply that verification always returns.
#[test]
#[cfg(not(miri))] // slow.
fn verifier_never_panics_on_a_corrupt_outer_buffer() {
    let base = outer_with(&valid_nested("AAAA"));
    let mut state: u64 = 0xA11C_E501_2345_6789;
    let mut next = || {
        state = state.wrapping_mul(6364136223846793005).wrapping_add(1442695040888963407);
        (state >> 33) as usize
    };

    let opts = flatbuffers::VerifierOptions::default();
    let mut accepted = 0usize;
    let mut rejected = 0usize;
    for _ in 0..20000 {
        let mut data = base.clone();
        let n = 1 + next() % 4;
        for _ in 0..n {
            let idx = next() % data.len();
            data[idx] = (next() % 256) as u8;
        }
        let res = catch_unwind(AssertUnwindSafe(|| {
            flatbuffers::root_with_opts::<Monster>(&opts, &data).is_ok()
        }));
        match res {
            Ok(true) => accepted += 1,
            Ok(false) => rejected += 1,
            Err(_) => panic!(
                "verifier panicked on a corrupt buffer; this is a DoS. Input: {:02X?}",
                data
            ),
        }
    }
    // Non-vacuity: the sweep has to actually reach both outcomes, otherwise it
    // is not exercising the verifier's decision paths at all.
    assert!(
        accepted > 0 && rejected > 0,
        "sweep reached only one outcome (accepted={}, rejected={}), so it is not \
         exercising the verifier's decision paths",
        accepted,
        rejected
    );
}

/// Control: the same sweep with nested verification disabled reproduces the
/// old behaviour, which does misbehave. This keeps the test above honest -- if
/// it ever passes for the wrong reason, this one will stop failing to fail.
#[test]
#[cfg(not(miri))] // slow.
fn control_disabling_nested_check_reintroduces_the_problem() {
    let mut opts = flatbuffers::VerifierOptions::default();
    opts.check_nested_flatbuffers = false;
    let s = sweep(&opts);
    assert!(
        s.misbehaved > 0,
        "expected the unverified path to misbehave on at least one mutant. If it \
         no longer does, the sweep has stopped reaching the behaviour it is meant \
         to guard and the test above proves nothing."
    );
}
