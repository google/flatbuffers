//! Runtime tests for vectors of unions support (issue #5024).
//!
//! These tests exercise the runtime helper `Verifier::visit_union_vector`
//! directly, without requiring `flatc`-generated code. They construct a
//! buffer that mirrors the wire format used by C++/Java/Go/TS for the
//! schema:
//!
//! ```fbs
//! table A { x: int; }
//! table B { y: int; }
//! union AB { A, B }
//! table Movie { actors: [AB]; }
//! ```
//!
//! The full codegen path for `[Union]` is exercised by the schema-driven
//! integration test once `flatc` is available; this file pins the
//! runtime contract independently so a regression in
//! `visit_union_vector` is caught even without rerunning the generator.

use flatbuffers::{
    FlatBufferBuilder, Follow, InvalidFlatbuffer, Verifier, VerifierOptions, WIPOffset,
};

#[derive(Copy, Clone, Eq, PartialEq, Debug)]
#[repr(u8)]
enum Tag {
    None = 0,
    A = 1,
    B = 2,
}

// Mimic what the generator emits for a union discriminant:
// follow as u8, verify as u8 (in-buffer scalar).
impl<'a> Follow<'a> for Tag {
    type Inner = Tag;
    unsafe fn follow(buf: &'a [u8], loc: usize) -> Self::Inner {
        match buf[loc] {
            1 => Tag::A,
            2 => Tag::B,
            _ => Tag::None,
        }
    }
}
impl flatbuffers::Verifiable for Tag {
    #[inline]
    fn run_verifier(v: &mut Verifier, pos: usize) -> Result<(), InvalidFlatbuffer> {
        v.in_buffer::<u8>(pos)
    }
}
// SAFETY: Tag is a single-byte repr(u8) enum with no padding. The
// generator emits this exact impl for every union discriminant enum.
impl flatbuffers::SimpleToVerifyInSlice for Tag {}

/// Build a Movie table that has paired (`actors_type`, `actors`) vectors
/// containing two elements: A{x=7} and B{y=11}. Returns the finished
/// buffer.
fn build_two_element_movie() -> Vec<u8> {
    let mut fbb = FlatBufferBuilder::new();

    // Build the two leaf tables.
    let a_off: WIPOffset<flatbuffers::UnionWIPOffset> = {
        let start = fbb.start_table();
        // x: int @ vtable slot 4 (first user field after vtable header).
        fbb.push_slot::<i32>(4, 7, 0);
        WIPOffset::new(fbb.end_table(start).value())
    };
    let b_off: WIPOffset<flatbuffers::UnionWIPOffset> = {
        let start = fbb.start_table();
        // y: int @ vtable slot 4.
        fbb.push_slot::<i32>(4, 11, 0);
        WIPOffset::new(fbb.end_table(start).value())
    };

    // The two paired vectors: types (u8) and values (offsets).
    let types = fbb.create_vector(&[Tag::A as u8, Tag::B as u8]);
    let values = fbb.create_vector(&[a_off, b_off]);

    // Movie table:
    //   slot 4 -> actors_type (Vector<u8>)
    //   slot 6 -> actors      (Vector<ForwardsUOffset<Table>>)
    let start = fbb.start_table();
    fbb.push_slot_always(4, types);
    fbb.push_slot_always(6, values);
    let movie = fbb.end_table(start);

    fbb.finish_minimal(movie);
    fbb.finished_data().to_vec()
}

/// Run the verifier exactly the way generated code would: visit the
/// table, then call `visit_union_vector` with a per-variant dispatch.
///
/// In real generated code the per-variant dispatch calls
/// `verify_union_variant::<ForwardsUOffset<TableT>>`; we don't have a
/// schema-derived table type here, so the dispatcher just returns Ok
/// for known variants. That's enough to exercise the lockstep logic in
/// `visit_union_vector` itself — length checks, presence-pairing,
/// inconsistent-union detection.
fn verify_movie(buf: &[u8]) -> Result<(), InvalidFlatbuffer> {
    let opts = VerifierOptions::default();
    let mut v = Verifier::new(&opts, buf);
    let root_pos =
        unsafe { flatbuffers::read_scalar::<flatbuffers::UOffsetT>(&buf[..4]) }
            as usize;
    let tv = v.visit_table(root_pos)?;
    tv.visit_union_vector::<Tag, _>(
        "actors_type",
        4,
        "actors",
        6,
        /*required=*/ false,
        |key, _v, _pos| match key {
            Tag::A | Tag::B | Tag::None => Ok(()),
        },
    )?
    .finish();
    Ok(())
}

#[test]
fn vector_of_unions_round_trips_two_elements() {
    let buf = build_two_element_movie();
    verify_movie(&buf).expect("verifier accepts a valid vector of unions");
}

#[test]
fn vector_of_unions_rejects_inconsistent_lengths() {
    // Construct a movie with a length mismatch between types and
    // values: types has 2 entries, values has 1. The verifier must
    // reject this with an InconsistentUnion error (matches the C++
    // PR #8853 fix).
    let mut fbb = FlatBufferBuilder::new();
    let a_off: WIPOffset<flatbuffers::UnionWIPOffset> = {
        let start = fbb.start_table();
        fbb.push_slot::<i32>(4, 7, 0);
        WIPOffset::new(fbb.end_table(start).value())
    };

    let types = fbb.create_vector(&[Tag::A as u8, Tag::B as u8]); // 2
    let values = fbb.create_vector(&[a_off]); // 1

    let start = fbb.start_table();
    fbb.push_slot_always(4, types);
    fbb.push_slot_always(6, values);
    let movie = fbb.end_table(start);
    fbb.finish_minimal(movie);
    let buf = fbb.finished_data().to_vec();

    let err = verify_movie(&buf).expect_err("length mismatch must fail verification");
    match err {
        InvalidFlatbuffer::InconsistentUnion { .. } => {}
        other => panic!("expected InconsistentUnion, got {:?}", other),
    }
}

#[test]
fn vector_of_unions_rejects_orphan_type_vector() {
    // Discriminant vector present but value vector absent — must be
    // reported as InconsistentUnion just like the single-union case.
    let mut fbb = FlatBufferBuilder::new();
    let types = fbb.create_vector(&[Tag::A as u8]);
    let start = fbb.start_table();
    fbb.push_slot_always(4, types);
    // Intentionally do not push slot 6.
    let movie = fbb.end_table(start);
    fbb.finish_minimal(movie);
    let buf = fbb.finished_data().to_vec();

    let err = verify_movie(&buf).expect_err("orphan _type vector must fail verification");
    match err {
        InvalidFlatbuffer::InconsistentUnion { .. } => {}
        other => panic!("expected InconsistentUnion, got {:?}", other),
    }
}

#[test]
fn vector_of_unions_accepts_empty_vectors() {
    // Both vectors are present and length 0 — that's a perfectly
    // valid empty vector-of-unions.
    let mut fbb = FlatBufferBuilder::new();
    let types = fbb.create_vector::<u8>(&[]);
    let values =
        fbb.create_vector::<WIPOffset<flatbuffers::UnionWIPOffset>>(&[]);

    let start = fbb.start_table();
    fbb.push_slot_always(4, types);
    fbb.push_slot_always(6, values);
    let movie = fbb.end_table(start);
    fbb.finish_minimal(movie);
    let buf = fbb.finished_data().to_vec();

    verify_movie(&buf).expect("empty paired vectors are valid");
}

#[test]
fn vector_of_unions_optional_field_can_be_absent() {
    // Both vectors absent and required=false — must be Ok.
    let mut fbb = FlatBufferBuilder::new();
    let start = fbb.start_table();
    let movie = fbb.end_table(start);
    fbb.finish_minimal(movie);
    let buf = fbb.finished_data().to_vec();

    verify_movie(&buf).expect("absent optional [Union] is valid");
}
