Use in C++    {#flatbuffers_guide_use_cpp}
==========

## Before you get started

Before diving into the FlatBuffers usage in C++, it should be noted that
the [Tutorial](@ref flatbuffers_guide_tutorial) page has a complete guide
to general FlatBuffers usage in all of the supported languages (including C++).
This page is designed to cover the nuances of FlatBuffers usage, specific to
C++.

#### Prerequisites

This page assumes you have written a FlatBuffers schema and compiled it
with the Schema Compiler. If you have not, please see
[Using the schema compiler](@ref flatbuffers_guide_using_schema_compiler)
and [Writing a schema](@ref flatbuffers_guide_writing_schema).

Assuming you wrote a schema, say `mygame.fbs` (though the extension doesn't
matter), you've generated a C++ header called `mygame_generated.h` using the
compiler (e.g. `flatc -c mygame.fbs`), you can now start using this in
your program by including the header. As noted, this header relies on
`flatbuffers/flatbuffers.h`, which should be in your include path.

## FlatBuffers C++ library code location

The code for the FlatBuffers C++ library can be found at
`flatbuffers/include/flatbuffers`. You can browse the library code on the
[FlatBuffers GitHub page](https://github.com/google/flatbuffers/tree/master/include/flatbuffers).

## Testing the FlatBuffers C++ library

The code to test the C++ library can be found at `flatbuffers/tests`.
The test code itself is located in
[test.cpp](https://github.com/google/flatbuffers/blob/master/tests/test.cpp).

This test file is built alongside `flatc`. To review how to build the project,
please read the [Building](@ref flatbuffers_guide_building) documenation.

To run the tests, execute `flattests` from the root `flatbuffers/` directory.
For example, on [Linux](https://en.wikipedia.org/wiki/Linux), you would simply
run: `./flattests`.

## Using the FlatBuffers C++ library

*Note: See [Tutorial](@ref flatbuffers_guide_tutorial) for a more in-depth
example of how to use FlatBuffers in C++.*

FlatBuffers supports both reading and writing FlatBuffers in C++.

To use FlatBuffers in your code, first generate the C++ classes from your
schema with the `--cpp` option to `flatc`. Then you can include both FlatBuffers
and the generated code to read or write FlatBuffers.

For example, here is how you would read a FlatBuffer binary file in C++:
First, include the library and generated code. Then read the file into
a `char *` array, which you pass to `GetMonster()`.

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~{.cpp}
    #include "flatbuffers/flatbuffers.h"
    #include "monster_test_generate.h"
    #include <cstdio> // For printing and file access.

    FILE* file = fopen("monsterdata_test.mon", "rb");
    fseek(file, 0L, SEEK_END);
    int length = ftell(file);
    fseek(file, 0L, SEEK_SET);
    char *data = new char[length];
    fread(data, sizeof(char), length, file);
    fclose(file);

    auto monster = GetMonster(data);
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

`monster` is of type `Monster *`, and points to somewhere *inside* your
buffer (root object pointers are not the same as `buffer_pointer` !).
If you look in your generated header, you'll see it has
convenient accessors for all fields, e.g. `hp()`, `mana()`, etc:

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~{.cpp}
    printf("%d\n", monster->hp());            // `80`
    printf("%d\n", monster->mana());          // default value of `150`
    printf("%s\n", monster->name()->c_str()); // "MyMonster"
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

*Note: That we never stored a `mana` value, so it will return the default.*

## Reflection (& Resizing)

There is experimental support for reflection in FlatBuffers, allowing you to
read and write data even if you don't know the exact format of a buffer, and
even allows you to change sizes of strings and vectors in-place.

The way this works is very elegant; there is actually a FlatBuffer schema that
describes schemas (!) which you can find in `reflection/reflection.fbs`.
The compiler, `flatc`, can write out any schemas it has just parsed as a binary
FlatBuffer, corresponding to this meta-schema.

Loading in one of these binary schemas at runtime allows you traverse any
FlatBuffer data that corresponds to it without knowing the exact format. You
can query what fields are present, and then read/write them after.

For convenient field manipulation, you can include the header
`flatbuffers/reflection.h` which includes both the generated code from the meta
schema, as well as a lot of helper functions.

And example of usage, for the time being, can be found in
`test.cpp/ReflectionTest()`.

## Storing maps / dictionaries in a FlatBuffer

FlatBuffers doesn't support maps natively, but there is support to
emulate their behavior with vectors and binary search, which means you
can have fast lookups directly from a FlatBuffer without having to unpack
your data into a `std::map` or similar.

To use it:
-   Designate one of the fields in a table as they "key" field. You do this
    by setting the `key` attribute on this field, e.g.
    `name:string (key)`.
    You may only have one key field, and it must be of string or scalar type.
-   Write out tables of this type as usual, collect their offsets in an
    array or vector.
-   Instead of `CreateVector`, call `CreateVectorOfSortedTables`,
    which will first sort all offsets such that the tables they refer to
    are sorted by the key field, then serialize it.
-   Now when you're accessing the FlatBuffer, you can use `Vector::LookupByKey`
    instead of just `Vector::Get` to access elements of the vector, e.g.:
    `myvector->LookupByKey("Fred")`, which returns a pointer to the
    corresponding table type, or `nullptr` if not found.
    `LookupByKey` performs a binary search, so should have a similar speed to
    `std::map`, though may be faster because of better caching. `LookupByKey`
    only works if the vector has been sorted, it will likely not find elements
    if it hasn't been sorted.

## Direct memory access

As you can see from the above examples, all elements in a buffer are
accessed through generated accessors. This is because everything is
stored in little endian format on all platforms (the accessor
performs a swap operation on big endian machines), and also because
the layout of things is generally not known to the user.

For structs, layout is deterministic and guaranteed to be the same
accross platforms (scalars are aligned to their
own size, and structs themselves to their largest member), and you
are allowed to access this memory directly by using `sizeof()` and
`memcpy` on the pointer to a struct, or even an array of structs.

To compute offsets to sub-elements of a struct, make sure they
are a structs themselves, as then you can use the pointers to
figure out the offset without having to hardcode it. This is
handy for use of arrays of structs with calls like `glVertexAttribPointer`
in OpenGL or similar APIs.

It is important to note is that structs are still little endian on all
machines, so only use tricks like this if you can guarantee you're not
shipping on a big endian machine (an `assert(FLATBUFFERS_LITTLEENDIAN)`
would be wise).

## Access of untrusted buffers

The generated accessor functions access fields over offsets, which is
very quick. These offsets are not verified at run-time, so a malformed
buffer could cause a program to crash by accessing random memory.

When you're processing large amounts of data from a source you know (e.g.
your own generated data on disk), this is acceptable, but when reading
data from the network that can potentially have been modified by an
attacker, this is undesirable.

For this reason, you can optionally use a buffer verifier before you
access the data. This verifier will check all offsets, all sizes of
fields, and null termination of strings to ensure that when a buffer
is accessed, all reads will end up inside the buffer.

Each root type will have a verification function generated for it,
e.g. for `Monster`, you can call:

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~{.cpp}
	bool ok = VerifyMonsterBuffer(Verifier(buf, len));
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

if `ok` is true, the buffer is safe to read.

Besides untrusted data, this function may be useful to call in debug
mode, as extra insurance against data being corrupted somewhere along
the way.

While verifying a buffer isn't "free", it is typically faster than
a full traversal (since any scalar data is not actually touched),
and since it may cause the buffer to be brought into cache before
reading, the actual overhead may be even lower than expected.

In specialized cases where a denial of service attack is possible,
the verifier has two additional constructor arguments that allow
you to limit the nesting depth and total amount of tables the
verifier may encounter before declaring the buffer malformed. The default is
`Verifier(buf, len, 64 /* max depth */, 1000000, /* max tables */)` which
should be sufficient for most uses.

## Text & schema parsing

Using binary buffers with the generated header provides a super low
overhead use of FlatBuffer data. There are, however, times when you want
to use text formats, for example because it interacts better with source
control, or you want to give your users easy access to data.

Another reason might be that you already have a lot of data in JSON
format, or a tool that generates JSON, and if you can write a schema for
it, this will provide you an easy way to use that data directly.

(see the schema documentation for some specifics on the JSON format
accepted).

There are two ways to use text formats:

#### Using the compiler as a conversion tool

This is the preferred path, as it doesn't require you to add any new
code to your program, and is maximally efficient since you can ship with
binary data. The disadvantage is that it is an extra step for your
users/developers to perform, though you might be able to automate it.

    flatc -b myschema.fbs mydata.json

This will generate the binary file `mydata_wire.bin` which can be loaded
as before.

#### Making your program capable of loading text directly

This gives you maximum flexibility. You could even opt to support both,
i.e. check for both files, and regenerate the binary from text when
required, otherwise just load the binary.

This option is currently only available for C++, or Java through JNI.

As mentioned in the section "Building" above, this technique requires
you to link a few more files into your program, and you'll want to include
`flatbuffers/idl.h`.

Load text (either a schema or json) into an in-memory buffer (there is a
convenient `LoadFile()` utility function in `flatbuffers/util.h` if you
wish). Construct a parser:

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~{.cpp}
    flatbuffers::Parser parser;
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Now you can parse any number of text files in sequence:

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~{.cpp}
    parser.Parse(text_file.c_str());
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

This works similarly to how the command-line compiler works: a sequence
of files parsed by the same `Parser` object allow later files to
reference definitions in earlier files. Typically this means you first
load a schema file (which populates `Parser` with definitions), followed
by one or more JSON files.

As optional argument to `Parse`, you may specify a null-terminated list of
include paths. If not specified, any include statements try to resolve from
the current directory.

If there were any parsing errors, `Parse` will return `false`, and
`Parser::err` contains a human readable error string with a line number
etc, which you should present to the creator of that file.

After each JSON file, the `Parser::fbb` member variable is the
`FlatBufferBuilder` that contains the binary buffer version of that
file, that you can access as described above.

`samples/sample_text.cpp` is a code sample showing the above operations.

## Threading

Reading a FlatBuffer does not touch any memory outside the original buffer,
and is entirely read-only (all const), so is safe to access from multiple
threads even without synchronisation primitives.

Creating a FlatBuffer is not thread safe. All state related to building
a FlatBuffer is contained in a FlatBufferBuilder instance, and no memory
outside of it is touched. To make this thread safe, either do not
share instances of FlatBufferBuilder between threads (recommended), or
manually wrap it in synchronisation primites. There's no automatic way to
accomplish this, by design, as we feel multithreaded construction
of a single buffer will be rare, and synchronisation overhead would be costly.

<br>
