# Use in C++

Assuming you have written a schema using the above language in say
`mygame.fbs` (FlatBuffer Schema, though the extension doesn't matter),
you've generated a C++ header called `mygame_generated.h` using the
compiler (e.g. `flatc -c mygame.fbs`), you can now start using this in
your program by including the header. As noted, this header relies on
`flatbuffers/flatbuffers.h`, which should be in your include path.

### Writing in C++

To start creating a buffer, create an instance of `FlatBufferBuilder`
which will contain the buffer as it grows:

    FlatBufferBuilder fbb;

Before we serialize a Monster, we need to first serialize any objects
that are contained there-in, i.e. we serialize the data tree using
depth first, pre-order traversal. This is generally easy to do on
any tree structures. For example:

    auto name = fbb.CreateString("MyMonster");

    unsigned char inv[] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 };
    auto inventory = fbb.CreateVector(inv, 10);

`CreateString` and `CreateVector` serialize these two built-in
datatypes, and return offsets into the serialized data indicating where
they are stored, such that `Monster` below can refer to them.

`CreateString` can also take an `std::string`, or a `const char *` with
an explicit length, and is suitable for holding UTF-8 and binary
data if needed.

`CreateVector` can also take an `std::vector`. The
offset it returns is typed, i.e. can only be used to set fields of the
correct type below. To create a vector of struct objects (which will
be stored as contiguous memory in the buffer, use `CreateVectorOfStructs`
instead.

    Vec3 vec(1, 2, 3);

`Vec3` is the first example of code from our generated
header. Structs (unlike tables) translate to simple structs in C++, so
we can construct them in a familiar way.

We have now serialized the non-scalar components of of the monster
example, so we could create the monster something like this:

    auto mloc = CreateMonster(fbb, &vec, 150, 80, name, inventory, Color_Red, 0, Any_NONE);

Note that we're passing `150` for the `mana` field, which happens to be the
default value: this means the field will not actually be written to the buffer,
since we'll get that value anyway when we query it. This is a nice space
savings, since it is very common for fields to be at their default. It means
we also don't need to be scared to add fields only used in a minority of cases,
since they won't bloat up the buffer sizes if they're not actually used.

We do something similarly for the union field `test` by specifying a `0` offset
and the `NONE` enum value (part of every union) to indicate we don't actually
want to write this field. You can use `0` also as a default for other
non-scalar types, such as strings, vectors and tables.

Tables (like `Monster`) give you full flexibility on what fields you write
(unlike `Vec3`, which always has all fields set because it is a `struct`).
If you want even more control over this (i.e. skip fields even when they are
not default), instead of the convenient `CreateMonster` call we can also
build the object field-by-field manually:

    MonsterBuilder mb(fbb);
    mb.add_pos(&vec);
    mb.add_hp(80);
    mb.add_name(name);
    mb.add_inventory(inventory);
    auto mloc = mb.Finish();

We start with a temporary helper class `MonsterBuilder` (which is
defined in our generated code also), then call the various `add_`
methods to set fields, and `Finish` to complete the object. This is
pretty much the same code as you find inside `CreateMonster`, except
we're leaving out a few fields. Fields may also be added in any order,
though orderings with fields of the same size adjacent
to each other most efficient in size, due to alignment. You should
not nest these Builder classes (serialize your
data in pre-order).

Regardless of whether you used `CreateMonster` or `MonsterBuilder`, you
now have an offset to the root of your data, and you can finish the
buffer using:

    fbb.Finish(mloc);

The buffer is now ready to be stored somewhere, sent over the network,
be compressed, or whatever you'd like to do with it. You can access the
start of the buffer with `fbb.GetBufferPointer()`, and it's size from
`fbb.GetSize()`.

`samples/sample_binary.cpp` is a complete code sample similar to
the code above, that also includes the reading code below.

### Reading in C++

If you've received a buffer from somewhere (disk, network, etc.) you can
directly start traversing it using:

    auto monster = GetMonster(buffer_pointer);

`monster` is of type `Monster *`, and points to somewhere inside your
buffer. If you look in your generated header, you'll see it has
convenient accessors for all fields, e.g.

    assert(monster->hp() == 80);
    assert(monster->mana() == 150);  // default
    assert(strcmp(monster->name()->c_str(), "MyMonster") == 0);

These should all be true. Note that we never stored a `mana` value, so
it will return the default.

To access sub-objects, in this case the `Vec3`:

    auto pos = monster->pos();
    assert(pos);
    assert(pos->z() == 3);

If we had not set the `pos` field during serialization, it would be
`NULL`.

Similarly, we can access elements of the inventory array:

    auto inv = monster->inventory();
    assert(inv);
    assert(inv->Get(9) == 9);

### Direct memory access

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

### Access of untrusted buffers

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

	bool ok = VerifyMonsterBuffer(Verifier(buf, len));

if `ok` is true, the buffer is safe to read.

Besides untrusted data, this function may be useful to call in debug
mode, as extra insurance against data being corrupted somewhere along
the way.

While verifying a buffer isn't "free", it is typically faster than
a full traversal (since any scalar data is not actually touched),
and since it may cause the buffer to be brought into cache before
reading, the actual overhead may be even lower than expected.

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

### Using the compiler as a conversion tool

This is the preferred path, as it doesn't require you to add any new
code to your program, and is maximally efficient since you can ship with
binary data. The disadvantage is that it is an extra step for your
users/developers to perform, though you might be able to automate it.

    flatc -b myschema.fbs mydata.json

This will generate the binary file `mydata_wire.bin` which can be loaded
as before.

### Making your program capable of loading text directly

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

    flatbuffers::Parser parser;

Now you can parse any number of text files in sequence:

    parser.Parse(text_file.c_str());

This works similarly to how the command-line compiler works: a sequence
of files parsed by the same `Parser` object allow later files to
reference definitions in earlier files. Typically this means you first
load a schema file (which populates `Parser` with definitions), followed
by one or more JSON files.

If there were any parsing errors, `Parse` will return `false`, and
`Parser::err` contains a human readable error string with a line number
etc, which you should present to the creator of that file.

After each JSON file, the `Parser::fbb` member variable is the
`FlatBufferBuilder` that contains the binary buffer version of that
file, that you can access as described above.

`samples/sample_text.cpp` is a code sample showing the above operations.

### Threading

None of the code is thread-safe, by design. That said, since currently a
FlatBuffer is read-only and entirely `const`, reading by multiple threads
is possible.

