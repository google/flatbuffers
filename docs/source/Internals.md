FlatBuffer Internals    {#flatbuffers_internals}
====================

This section is entirely optional for the use of FlatBuffers. In normal
usage, you should never need the information contained herein. If you're
interested however, it should give you more of an appreciation of why
FlatBuffers is both efficient and convenient.

### Format components

A FlatBuffer is a binary file and in-memory format consisting mostly of
scalars of various sizes, all aligned to their own size. Each scalar is
also always represented in little-endian format, as this corresponds to
all commonly used CPUs today. FlatBuffers will also work on big-endian
machines, but will be slightly slower because of additional
byte-swap intrinsics.

On purpose, the format leaves a lot of details about where exactly
things live in memory undefined, e.g. fields in a table can have any
order, and objects to some extent can be stored in many orders. This is
because the format doesn't need this information to be efficient, and it
leaves room for optimization and extension (for example, fields can be
packed in a way that is most compact). Instead, the format is defined in
terms of offsets and adjacency only. This may mean two different
implementations may produce different binaries given the same input
values, and this is perfectly valid.

### Format identification

The format also doesn't contain information for format identification
and versioning, which is also by design. FlatBuffers is a statically typed
system, meaning the user of a buffer needs to know what kind of buffer
it is. FlatBuffers can of course be wrapped inside other containers
where needed, or you can use its union feature to dynamically identify
multiple possible sub-objects stored. Additionally, it can be used
together with the schema parser if full reflective capabilities are
desired.

Versioning is something that is intrinsically part of the format (the
optionality / extensibility of fields), so the format itself does not
need a version number (it's a meta-format, in a sense). We're hoping
that this format can accommodate all data needed. If format breaking
changes are ever necessary, it would become a new kind of format rather
than just a variation.

### Offsets

The most important and generic offset type (see `flatbuffers.h`) is
`uoffset_t`, which is currently always a `uint32_t`, and is used to
refer to all tables/unions/strings/vectors (these are never stored
in-line). 32bit is
intentional, since we want to keep the format binary compatible between
32 and 64bit systems, and a 64bit offset would bloat the size for almost
all uses. A version of this format with 64bit (or 16bit) offsets is easy to set
when needed. Unsigned means they can only point in one direction, which
typically is forward (towards a higher memory location). Any backwards
offsets will be explicitly marked as such.

The format starts with an `uoffset_t` to the root object in the buffer.

We have two kinds of objects, structs and tables.

### Structs

These are the simplest, and as mentioned, intended for simple data that
benefits from being extra efficient and doesn't need versioning /
extensibility. They are always stored inline in their parent (a struct,
table, or vector) for maximum compactness. Structs define a consistent
memory layout where all components are aligned to their size, and
structs aligned to their largest scalar member. This is done independent
of the alignment rules of the underlying compiler to guarantee a cross
platform compatible layout. This layout is then enforced in the generated
code.

### Tables

Unlike structs, these are not stored in inline in their parent, but are
referred to by offset.

They start with an `soffset_t` to a vtable. This is a signed version of
`uoffset_t`, since vtables may be stored anywhere relative to the object.
This offset is substracted (not added) from the object start to arrive at
the vtable start. This offset is followed by all the
fields as aligned scalars (or offsets). Unlike structs, not all fields
need to be present. There is no set order and layout.

To be able to access fields regardless of these uncertainties, we go
through a vtable of offsets. Vtables are shared between any objects that
happen to have the same vtable values.

The elements of a vtable are all of type `voffset_t`, which is
a `uint16_t`. The first element is the size of the vtable in bytes,
including the size element. The second one is the size of the object, in bytes
(including the vtable offset). This size could be used for streaming, to know
how many bytes to read to be able to access all *inline* fields of the object.
The remaining elements are the N offsets, where N is the amount of fields
declared in the schema when the code that constructed this buffer was
compiled (thus, the size of the table is N + 2).

All accessor functions in the generated code for tables contain the
offset into this table as a constant. This offset is checked against the
first field (the number of elements), to protect against newer code
reading older data. If this offset is out of range, or the vtable entry
is 0, that means the field is not present in this object, and the
default value is return. Otherwise, the entry is used as offset to the
field to be read.

### Strings and Vectors

Strings are simply a vector of bytes, and are always
null-terminated. Vectors are stored as contiguous aligned scalar
elements prefixed by a 32bit element count (not including any
null termination). Neither is stored inline in their parent, but are referred to
by offset.

### Construction

The current implementation constructs these buffers backwards (starting
at the highest memory address of the buffer), since
that significantly reduces the amount of bookkeeping and simplifies the
construction API.

### Code example

Here's an example of the code that gets generated for the `samples/monster.fbs`.
What follows is the entire file, broken up by comments:

    // automatically generated, do not modify

    #include "flatbuffers/flatbuffers.h"

    namespace MyGame {
    namespace Sample {

Nested namespace support.

    enum {
      Color_Red = 0,
      Color_Green = 1,
      Color_Blue = 2,
    };

    inline const char **EnumNamesColor() {
      static const char *names[] = { "Red", "Green", "Blue", nullptr };
      return names;
    }

    inline const char *EnumNameColor(int e) { return EnumNamesColor()[e]; }

Enums and convenient reverse lookup.

    enum {
      Any_NONE = 0,
      Any_Monster = 1,
    };

    inline const char **EnumNamesAny() {
      static const char *names[] = { "NONE", "Monster", nullptr };
      return names;
    }

    inline const char *EnumNameAny(int e) { return EnumNamesAny()[e]; }

Unions share a lot with enums.

    struct Vec3;
    struct Monster;

Predeclare all data types since circular references between types are allowed
(circular references between object are not, though).

    MANUALLY_ALIGNED_STRUCT(4) Vec3 {
     private:
      float x_;
      float y_;
      float z_;

     public:
      Vec3(float x, float y, float z)
        : x_(flatbuffers::EndianScalar(x)), y_(flatbuffers::EndianScalar(y)), z_(flatbuffers::EndianScalar(z)) {}

      float x() const { return flatbuffers::EndianScalar(x_); }
      float y() const { return flatbuffers::EndianScalar(y_); }
      float z() const { return flatbuffers::EndianScalar(z_); }
    };
    STRUCT_END(Vec3, 12);

These ugly macros do a couple of things: they turn off any padding the compiler
might normally do, since we add padding manually (though none in this example),
and they enforce alignment chosen by FlatBuffers. This ensures the layout of
this struct will look the same regardless of compiler and platform. Note that
the fields are private: this is because these store little endian scalars
regardless of platform (since this is part of the serialized data).
`EndianScalar` then converts back and forth, which is a no-op on all current
mobile and desktop platforms, and a single machine instruction on the few
remaining big endian platforms.

    struct Monster : private flatbuffers::Table {
      const Vec3 *pos() const { return GetStruct<const Vec3 *>(4); }
      int16_t mana() const { return GetField<int16_t>(6, 150); }
      int16_t hp() const { return GetField<int16_t>(8, 100); }
      const flatbuffers::String *name() const { return GetPointer<const flatbuffers::String *>(10); }
      const flatbuffers::Vector<uint8_t> *inventory() const { return GetPointer<const flatbuffers::Vector<uint8_t> *>(14); }
      int8_t color() const { return GetField<int8_t>(16, 2); }
    };

Tables are a bit more complicated. A table accessor struct is used to point at
the serialized data for a table, which always starts with an offset to its
vtable. It derives from `Table`, which contains the `GetField` helper functions.
GetField takes a vtable offset, and a default value. It will look in the vtable
at that offset. If the offset is out of bounds (data from an older version) or
the vtable entry is 0, the field is not present and the default is returned.
Otherwise, it uses the entry as an offset into the table to locate the field.

    struct MonsterBuilder {
      flatbuffers::FlatBufferBuilder &fbb_;
      flatbuffers::uoffset_t start_;
      void add_pos(const Vec3 *pos) { fbb_.AddStruct(4, pos); }
      void add_mana(int16_t mana) { fbb_.AddElement<int16_t>(6, mana, 150); }
      void add_hp(int16_t hp) { fbb_.AddElement<int16_t>(8, hp, 100); }
      void add_name(flatbuffers::Offset<flatbuffers::String> name) { fbb_.AddOffset(10, name); }
      void add_inventory(flatbuffers::Offset<flatbuffers::Vector<uint8_t>> inventory) { fbb_.AddOffset(14, inventory); }
      void add_color(int8_t color) { fbb_.AddElement<int8_t>(16, color, 2); }
      MonsterBuilder(flatbuffers::FlatBufferBuilder &_fbb) : fbb_(_fbb) { start_ = fbb_.StartTable(); }
      flatbuffers::Offset<Monster> Finish() { return flatbuffers::Offset<Monster>(fbb_.EndTable(start_, 7)); }
    };

`MonsterBuilder` is the base helper struct to construct a table using a
`FlatBufferBuilder`. You can add the fields in any order, and the `Finish`
call will ensure the correct vtable gets generated.

    inline flatbuffers::Offset<Monster> CreateMonster(flatbuffers::FlatBufferBuilder &_fbb,
                                                      const Vec3 *pos, int16_t mana,
                                                      int16_t hp,
                                                      flatbuffers::Offset<flatbuffers::String> name,
                                                      flatbuffers::Offset<flatbuffers::Vector<uint8_t>> inventory,
                                                      int8_t color) {
      MonsterBuilder builder_(_fbb);
      builder_.add_inventory(inventory);
      builder_.add_name(name);
      builder_.add_pos(pos);
      builder_.add_hp(hp);
      builder_.add_mana(mana);
      builder_.add_color(color);
      return builder_.Finish();
    }

`CreateMonster` is a convenience function that calls all functions in
`MonsterBuilder` above for you. Note that if you pass values which are
defaults as arguments, it will not actually construct that field, so
you can probably use this function instead of the builder class in
almost all cases.

    inline const Monster *GetMonster(const void *buf) { return flatbuffers::GetRoot<Monster>(buf); }

This function is only generated for the root table type, to be able to
start traversing a FlatBuffer from a raw buffer pointer.

    }; // namespace MyGame
    }; // namespace Sample

### Encoding example.

Below is a sample encoding for the following JSON corresponding to the above
schema:

    { pos: { x: 1, y: 2, z: 3 }, name: "fred", hp: 50 }

Resulting in this binary buffer:

    // Start of the buffer:
    uint32_t 20  // Offset to the root table.

    // Start of the vtable. Not shared in this example, but could be:
    uint16_t 16 // Size of table, starting from here.
    uint16_t 22 // Size of object inline data.
    uint16_t 4, 0, 20, 16, 0, 0  // Offsets to fields from start of (root) table, 0 for not present.

    // Start of the root table:
    int32_t 16     // Offset to vtable used (default negative direction)
    float 1, 2, 3  // the Vec3 struct, inline.
    uint32_t 8     // Offset to the name string.
    int16_t 50     // hp field.
    int16_t 0      // Padding for alignment.

    // Start of name string:
    uint32_t 4  // Length of string.
    int8_t 'f', 'r', 'e', 'd', 0, 0, 0, 0  // Text + 0 termination + padding.

Note that this not the only possible encoding, since the writer has some
flexibility in which of the children of root object to write first (though in
this case there's only one string), and what order to write the fields in.
Different orders may also cause different alignments to happen.

<br>
