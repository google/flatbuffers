Use in Rust    {#flatbuffers_guide_use_rust}
==========

## Before you get started

Before diving into the FlatBuffers usage in Rust, it should be noted that
the [Tutorial](@ref flatbuffers_guide_tutorial) page has a complete guide
to general FlatBuffers usage in all of the supported languages (including Rust).
This page is designed to cover the nuances of FlatBuffers usage, specific to
Rust.

#### Prerequisites

This page assumes you have written a FlatBuffers schema and compiled it
with the Schema Compiler. If you have not, please see
[Using the schema compiler](@ref flatbuffers_guide_using_schema_compiler)
and [Writing a schema](@ref flatbuffers_guide_writing_schema).

Assuming you wrote a schema, say `mygame.fbs` (though the extension doesn't
matter), you've generated a Rust file called `mygame_generated.rs` using the
compiler (e.g. `flatc --rust mygame.fbs` or via helpers listed in "Useful
tools created by others" section bellow), you can now start using this in
your program by including the file. As noted, this header relies on the crate
`flatbuffers`, which should be in your include `Cargo.toml`.

## FlatBuffers Rust library code location

The code for the FlatBuffers Rust library can be found at
`flatbuffers/rust`. You can browse the library code on the
[FlatBuffers GitHub page](https://github.com/google/flatbuffers/tree/master/rust).

## Testing the FlatBuffers Rust library

The code to test the Rust library can be found at `flatbuffers/tests/rust_usage_test`.
The test code itself is located in
[integration_test.rs](https://github.com/google/flatbuffers/blob/master/tests/rust_usage_test/tests/integration_test.rs)

This test file requires `flatc` to be present. To review how to build the project,
please read the [Building](@ref flatbuffers_guide_building) documentation.

To run the tests, execute `RustTest.sh` from the `flatbuffers/tests` directory.
For example, on [Linux](https://en.wikipedia.org/wiki/Linux), you would simply
run: `cd tests && ./RustTest.sh`.

*Note: The shell script requires [Rust](https://www.rust-lang.org) to
be installed.*

## Using the FlatBuffers Rust library

*Note: See [Tutorial](@ref flatbuffers_guide_tutorial) for a more in-depth
example of how to use FlatBuffers in Rust.*

FlatBuffers supports both reading and writing FlatBuffers in Rust.

To use FlatBuffers in your code, first generate the Rust modules from your
schema with the `--rust` option to `flatc`. Then you can import both FlatBuffers
and the generated code to read or write FlatBuffers.

For example, here is how you would read a FlatBuffer binary file in Rust:
First, include the library and generated code. Then read the file into
a `u8` vector, which you pass, as a byte slice, to `root_as_monster()`.

This full example program is available in the Rust test suite:
[monster_example.rs](https://github.com/google/flatbuffers/blob/master/tests/rust_usage_test/bin/monster_example.rs)

It can be run by `cd`ing to the `rust_usage_test` directory and executing: `cargo run monster_example`.

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~{.rs}
    extern crate flatbuffers;

    #[allow(dead_code, unused_imports)]
    #[path = "../../monster_test_generated.rs"]
    mod monster_test_generated;
    pub use monster_test_generated::my_game;

    use std::io::Read;

    fn main() {
        let mut f = std::fs::File::open("../monsterdata_test.mon").unwrap();
        let mut buf = Vec::new();
        f.read_to_end(&mut buf).expect("file reading failed");

        let monster = my_game::example::root_as_monster(&buf[..]);
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

`monster` is of type `Monster`, and points to somewhere *inside* your
buffer (root object pointers are not the same as `buffer_pointer` !).
If you look in your generated header, you'll see it has
convenient accessors for all fields, e.g. `hp()`, `mana()`, etc:

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~{.rs}
        println!("{}", monster.hp());     // `80`
        println!("{}", monster.mana());   // default value of `150`
        println!("{:?}", monster.name()); // Some("MyMonster")
    }
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

*Note: That we never stored a `mana` value, so it will return the default.*

## Direct memory access

As you can see from the above examples, all elements in a buffer are
accessed through generated accessors. This is because everything is
stored in little endian format on all platforms (the accessor
performs a swap operation on big endian machines), and also because
the layout of things is generally not known to the user.

For structs, layout is deterministic and guaranteed to be the same
across platforms (scalars are aligned to their
own size, and structs themselves to their largest member), and you
are allowed to access this memory directly by using `safe_slice`
on the reference to a struct, or even an array of structs.

To compute offsets to sub-elements of a struct, make sure they
are structs themselves, as then you can use the pointers to
figure out the offset without having to hardcode it. This is
handy for use of arrays of structs with calls like `glVertexAttribPointer`
in OpenGL or similar APIs.

It is important to note is that structs are still little endian on all
machines, so the functions to enable tricks like this are only exposed on little
endian machines. If you also ship on big endian machines, using an
`#[cfg(target_endian = "little")]` attribute would be wise or your code will not
compile.

The special function `safe_slice` is implemented on Vector objects that are
represented in memory the same way as they are represented on the wire. This
function is always available on vectors of struct, bool, u8, and i8. It is
conditionally-compiled on little-endian systems for all the remaining scalar
types.

The FlatBufferBuilder function `create_vector_direct` is implemented for all
types that are endian-safe to write with a `memcpy`. It is the write-equivalent
of `safe_slice`.

## Access of untrusted buffers

The safe Rust functions to interpret a slice as a table (`root`,
`size_prefixed_root`, `root_with_opts`, and `size_prefixed_root_with_opts`)
verify the data first. This has some performance cost, but is intended to be
safe for use on flatbuffers from untrusted sources. There are corresponding
`unsafe` versions with names ending in `_unchecked` which skip this
verification, and may access arbitrary memory.

The generated accessor functions access fields over offsets, which is
very quick. The current implementation uses these to access memory without any
further bounds checking. All of the safe Rust APIs ensure the verifier is run
over these flatbuffers before accessing them.

When you're processing large amounts of data from a source you know (e.g.
your own generated data on disk), the `_unchecked` versions are acceptable, but
when reading data from the network that can potentially have been modified by an
attacker, it is desirable to use the safe versions which use the verifier.

## Threading

Reading a FlatBuffer does not touch any memory outside the original buffer,
and is entirely read-only (all immutable), so is safe to access from multiple
threads even without synchronisation primitives.

Creating a FlatBuffer is not thread safe. All state related to building
a FlatBuffer is contained in a FlatBufferBuilder instance, and no memory
outside of it is touched. To make this thread safe, either do not
share instances of FlatBufferBuilder between threads (recommended), or
manually wrap it in synchronisation primitives. There's no automatic way to
accomplish this, by design, as we feel multithreaded construction
of a single buffer will be rare, and synchronisation overhead would be costly.

Unlike most other languages, in Rust these properties are exposed to and
enforced by the type system. `flatbuffers::Table` and the generated table types
are `Send + Sync`, indicating they may be freely shared across threads and data
may be accessed from any thread which receives a const (aka shared) reference.
There are no functions which require a mutable (aka exclusive) reference, which
means all the available functions may be called like this.
`flatbuffers::FlatBufferBuilder` is also `Send + Sync`, but all of the mutating
functions require a mutable (aka exclusive) reference which can only be created
when no other references to the `FlatBufferBuilder` exist, and may not be copied
within the same thread, let alone to a second thread.

## Useful tools created by others

* [flatc-rust](https://github.com/frol/flatc-rust) - FlatBuffers compiler
(flatc) as API for transparent `.fbs` to `.rs` code-generation via Cargo
build scripts integration.

<br>
