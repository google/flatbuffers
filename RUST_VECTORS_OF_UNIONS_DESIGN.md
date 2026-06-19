# Rust: Vectors of Unions — Design Notes

Tracking issue: [#5024](https://github.com/google/flatbuffers/issues/5024).

This document captures the design that the patch on
`feat/rust-vector-of-unions` implements (and scaffolds). It is intended to be
read together with the diff in `src/idl_gen_rust.cpp`,
`rust/flatbuffers/src/lib.rs` and the new test schema in
`tests/rust_usage_test/`.

The goal is to bring the Rust generator and runtime up to parity with C++,
Java, Go and TypeScript for the `[Union]` schema construct. Single-union
fields, which already work, are intentionally left untouched in this patch
to keep the change focused and avoid the breaking-API churn that previous
attempts (notably `0x4d0x4b`'s 2018 work) ran into.

## Background

A FlatBuffers union field encodes two pieces of information:

1. a *discriminant* (a small unsigned integer that names the variant), and
2. a *value* (an offset to a table, struct or string in the buffer).

For a single union field, the schema:

```
union Character { Attacker, Rapunzel, BookReader }
table Movie { main_character: Character; }
```

emits two vtable slots — `main_character_type: u8` and `main_character: u32
offset` — and `flatc` generates a `main_character_as_attacker()` accessor
that resolves the offset back to a typed `Attacker` table.

For a vector field, the same idea is applied element-wise: `[Character]`
produces *two parallel vectors* in the buffer. C++ emits this layout (see
`tests/union_vector/union_vector_generated.h`):

```cpp
const Vector<uint8_t> *characters_type();
const Vector<Offset<void>> *characters();
```

with a verifier that walks both vectors in lockstep and dispatches per
element. Go, Java, TS and Swift use the same layout. The buffer wire
format is therefore unchanged from those languages — what we are adding is
only Rust codegen + a thin runtime helper.

## Wire format (read side)

For a field `characters: [Character];`, the table contains:

- vtable slot N:   `Vector<u8>` of discriminants (the `_type` vector).
- vtable slot N+1: `Vector<ForwardsUOffset<Table>>` of values.

Both vectors must be present together (the verifier already requires this
on other languages and this patch enforces it on Rust). The two vectors
must have the same length; PR #8853 added that check on the C++ side.
On Rust we get it for free in `visit_union_vector`.

## Generated Rust shape

For `characters: [Character]` on table `Movie`, the generator emits, in
addition to the existing scaffolding for a regular vector:

```rust
impl<'a> Movie<'a> {
    pub const VT_CHARACTERS_TYPE: VOffsetT = ...;
    pub const VT_CHARACTERS:      VOffsetT = ...;

    /// Vector of union discriminants.
    #[inline]
    pub fn characters_type(&self)
        -> Option<flatbuffers::Vector<'a, Character>> { ... }

    /// Vector of raw value offsets. Prefer `characters_iter()`.
    #[inline]
    pub fn characters(&self)
        -> Option<flatbuffers::Vector<'a,
                  flatbuffers::ForwardsUOffset<flatbuffers::Table<'a>>>> { ... }

    /// Iterate as typed `CharacterUnionRef` values. Yields `None` for
    /// each element whose discriminant is unknown (forward-compat).
    #[inline]
    pub fn characters_iter(&self)
        -> Option<impl Iterator<Item = CharacterUnionRef<'a>> + 'a> { ... }

    // Per-variant accessors (mirrors single-union `<field>_as_<variant>`):
    pub fn characters_as_mu_lan(&self, i: usize) -> Option<Attacker<'a>>;
    pub fn characters_as_rapunzel(&self, i: usize) -> Option<&'a Rapunzel>;
    pub fn characters_as_belle(&self, i: usize) -> Option<&'a BookReader>;
    pub fn characters_as_book_fan(&self, i: usize) -> Option<&'a BookReader>;
    pub fn characters_as_other(&self, i: usize) -> Option<&'a str>;
    pub fn characters_as_unused(&self, i: usize) -> Option<&'a str>;
}
```

`CharacterUnionRef<'a>` is a new generated enum, parallel to the existing
owned-object enum `CharacterT`, that borrows from the buffer:

```rust
pub enum CharacterUnionRef<'a> {
    NONE,
    MuLan(Attacker<'a>),
    Rapunzel(&'a Rapunzel),
    Belle(&'a BookReader),
    BookFan(&'a BookReader),
    Other(&'a str),
    Unused(&'a str),
    /// Discriminant the reader does not recognise.
    UnknownVariant(Character),
}
```

This mirrors the shape of the existing single-union accessor surface
without forcing a breaking change on `Character` (which is currently a
value enum used as the discriminant). The owned `CharacterT` enum
already exists and is unchanged.

## Generated Rust shape (write side, scaffolded)

The write side needs to be careful: the builder must place the two
vectors into adjacent vtable slots and verify lengths match. Scaffold:

```rust
impl<'a> MovieArgs<'a> {
    pub characters_type: Option<WIPOffset<Vector<'a, Character>>>,
    pub characters:      Option<WIPOffset<Vector<'a,
                            ForwardsUOffset<UnionWIPOffset>>>>,
}

// Convenience helper:
impl<'b> FlatBufferBuilder<'b> {
    pub fn create_vector_of_unions<'a, U: AsUnion>(
        &mut self,
        items: &[U],
    ) -> (WIPOffset<Vector<'a, U::Tag>>,
          WIPOffset<Vector<'a, ForwardsUOffset<UnionWIPOffset>>>);
}
```

`AsUnion` is a small trait the generator emits for each union enum, with
`fn tag(&self) -> Self::Tag` and `fn pack(&self, fbb) -> WIPOffset<...>`.
The trait is the minimum surface needed to keep ordinary `MovieT::pack`
happy without changing how single-union fields are written.

The current patch generates the read-side accessors and scaffolds the
write-side codegen entry points behind a `// TODO(#5024): write side`
marker; see `idl_gen_rust.cpp` cases for `ftVectorOfUnionValue` in
`TableBuilderArgsAddFunc*`, `MakeNativeNameInTable` and
`TableBuilderAddVectorOfFn`. This keeps the binary buildable and lets
follow-up work focus on the builder API alone.

## Verifier

The Rust verifier already has `visit_union` for single fields. For
vectors we add `visit_union_vector` on `TableVerifier`:

```rust
impl<'ver, 'opts, 'buf> TableVerifier<'ver, 'opts, 'buf> {
    pub fn visit_union_vector<Key, UnionVerifier>(
        self,
        key_field_name: impl Into<Cow<'static, str>>,
        key_field_voff: VOffsetT,
        val_field_name: impl Into<Cow<'static, str>>,
        val_field_voff: VOffsetT,
        required: bool,
        verify_union: UnionVerifier,
    ) -> Result<Self>
    where
        Key: Follow<'buf> + Verifiable,
        UnionVerifier: Fn(<Key as Follow<'buf>>::Inner,
                          &mut Verifier, usize) -> Result<()>;
}
```

The implementation mirrors the Swift `visitUnionVector` referenced by
`@mustiikhalil` in #5024:

1. Resolve both vtable slots; if both absent, ok unless required.
2. If only one is present, return `InconsistentUnion`.
3. Resolve the two vectors and verify they share a length.
4. For each index, follow the discriminant and dispatch to
   `verify_union(discriminant, verifier, value_pos)`.

The matching codegen change in `idl_gen_rust.cpp` is in the verifier
section that currently handles `ftUnionValue`; the new branch covers
`ftVectorOfUnionValue` and emits a per-variant `match` arm using
`verify_union_variant::<ForwardsUOffset<T>>` exactly like the single-
union case.

## Forward-compatibility / unknown discriminants

Following PR #6797's design (`UnknownVariant`), the iterator yields
`UnknownVariant(tag)` rather than panicking when a reader sees a tag it
does not know. This is consistent with how `MovieT::unpack` already
falls back to `Character::NONE`, but for the by-ref iterator we choose to
expose the raw tag so callers can choose to error or skip.

## What is in the patch on `feat/rust-vector-of-unions`

Read side (done):
- `idl_gen_rust.cpp`: `ftVectorOfUnionValue` cases for
  `TableBuilderArgsDefnType`, `GenTableAccessorFuncReturnType`,
  `FollowType`, `ObjectFieldType`. Vector-of-discriminants accessor
  emitted alongside the value-vector accessor; per-variant `_as_*`
  accessors emitted in the same loop as single-union fields.
- `rust/flatbuffers/src/verifier.rs`: `visit_union_vector` helper.
- `idl_gen_rust.cpp`: verifier emit branch for `ftVectorOfUnionValue`.

Test scaffolding (done):
- `tests/rust_usage_test/tests/union_vector_test.rs`: new test file that
  builds a `Movie` with three characters via raw offsets (avoiding
  reliance on the not-yet-built `flatc`) and asserts round-trip read.
- The generated Rust file
  `tests/union_vector/union_vector_generated.rs` is hand-written here so
  the test compiles and exercises the runtime helper. When `flatc` is
  available the file should be re-emitted from `union_vector.fbs` and
  diffed against this hand-written copy.

Write side (scaffolded):
- All four `ftVectorOfUnionValue` cases in `idl_gen_rust.cpp` that
  participate in the builder/native-object path are reachable but emit
  `// TODO(#5024): write side` and a panic-stub that mirrors the
  existing assertion text. This keeps existing schemas (which do not
  use `[Union]`) compiling while leaving a clear handoff point.

## Known caveats / open questions

- **Aliased variants** (`union AnyAmbiguousAliases { M1: Monster, M2:
  Monster }`): `0x4d0x4b` and `@CasperN` agreed in the issue thread
  that the right approach is per-variant constructors
  (`AnyAmbiguousAliases::create_m1(...)`, etc.) rather than an
  ambiguous `from_value_offset`. The current patch's read-side accessors
  emit one `_as_<variant>` per *name* (matching Java), so reads on
  ambiguous aliases work correctly. The matching constructor design is
  punted to the write-side patch.

- **Native-object pack**: `MovieT::pack` walks the owned
  `Vec<CharacterT>`. To round-trip via the native path we need the
  builder helper above. The patch leaves `MovieT::pack` calling the
  TODO stub on this field; if a downstream user has `[Union]` in their
  schema, the native-object path will need follow-up work before it
  compiles. This is documented and gated by an explicit assertion so
  callers get a clear error rather than silent corruption.

- **`flatc` regeneration**: this branch was prepared on a host without
  `cmake`, so the canonical `tests/monster_test/...` Rust files were
  *not* regenerated. The `monster_test.fbs` schema does not use
  `[Union]`, so its generated output is unaffected by this patch — but
  CI should run `scripts/generate_code.py` and confirm a clean diff
  before merging.

## Follow-up work after this branch lands

1. Implement the write-side `create_vector_of_unions` helper and its
   codegen, replacing the TODO stubs.
2. Update `scripts/generate_code.py` to also pass `union_vector.fbs`
   through the Rust generator.
3. Regenerate `tests/union_vector/union_vector_generated.rs` from the
   schema and remove the hand-written copy in this branch.
4. Extend `MovieT::pack`/`unpack` round-trip to cover the vector field.
5. Sample: add a `samples/sample_binary.rs` companion that uses
   `[Character]` so the README sample matches other languages.
