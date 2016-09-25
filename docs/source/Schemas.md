Writing a schema    {#flatbuffers_guide_writing_schema}
================

The syntax of the schema language (aka IDL, [Interface Definition Language][])
should look quite familiar to users of any of the C family of
languages, and also to users of other IDLs. Let's look at an example
first:

    // example IDL file

    namespace MyGame;

    attribute "priority";

    enum Color : byte { Red = 1, Green, Blue }

    union Any { Monster, Weapon, Pickup }

    struct Vec3 {
      x:float;
      y:float;
      z:float;
    }

    table Monster {
      pos:Vec3;
      mana:short = 150;
      hp:short = 100;
      name:string;
      friendly:bool = false (deprecated, priority: 1);
      inventory:[ubyte];
      color:Color = Blue;
      test:Any;
    }

    root_type Monster;

(`Weapon` & `Pickup` not defined as part of this example).

### Tables

Tables are the main way of defining objects in FlatBuffers, and consist
of a name (here `Monster`) and a list of fields. Each field has a name,
a type, and optionally a default value (if omitted, it defaults to `0` /
`NULL`).

Each field is optional: It does not have to appear in the wire
representation, and you can choose to omit fields for each individual
object. As a result, you have the flexibility to add fields without fear of
bloating your data. This design is also FlatBuffer's mechanism for forward
and backwards compatibility. Note that:

-   You can add new fields in the schema ONLY at the end of a table
    definition. Older data will still
    read correctly, and give you the default value when read. Older code
    will simply ignore the new field.
    If you want to have flexibility to use any order for fields in your
    schema, you can manually assign ids (much like Protocol Buffers),
    see the `id` attribute below.

-   You cannot delete fields you don't use anymore from the schema,
    but you can simply
    stop writing them into your data for almost the same effect.
    Additionally you can mark them as `deprecated` as in the example
    above, which will prevent the generation of accessors in the
    generated C++, as a way to enforce the field not being used any more.
    (careful: this may break code!).

-   You may change field names and table names, if you're ok with your
    code breaking until you've renamed them there too.

See "Schema evolution examples" below for more on this
topic.

### Structs

Similar to a table, only now none of the fields are optional (so no defaults
either), and fields may not be added or be deprecated. Structs may only contain
scalars or other structs. Use this for
simple objects where you are very sure no changes will ever be made
(as quite clear in the example `Vec3`). Structs use less memory than
tables and are even faster to access (they are always stored in-line in their
parent object, and use no virtual table).

### Types

Built-in scalar types are:

-   8 bit: `byte`, `ubyte`, `bool`

-   16 bit: `short`, `ushort`

-   32 bit: `int`, `uint`, `float`

-   64 bit: `long`, `ulong`, `double`

Built-in non-scalar types:

-   Vector of any other type (denoted with `[type]`). Nesting vectors
    is not supported, instead you can wrap the inner vector in a table.

-   `string`, which may only hold UTF-8 or 7-bit ASCII. For other text encodings
    or general binary data use vectors (`[byte]` or `[ubyte]`) instead.

-   References to other tables or structs, enums or unions (see
    below).

You can't change types of fields once they're used, with the exception
of same-size data where a `reinterpret_cast` would give you a desirable result,
e.g. you could change a `uint` to an `int` if no values in current data use the
high bit yet.

### (Default) Values

Values are a sequence of digits. Values may be optionally followed by a decimal
point (`.`) and more digits, for float constants, or optionally prefixed by
a `-`. Floats may also be in scientific notation; optionally ending with an `e`
or `E`, followed by a `+` or `-` and more digits.

Only scalar values can have defaults, non-scalar (string/vector/table) fields
default to `NULL` when not present.

You generally do not want to change default values after they're initially
defined. Fields that have the default value are not actually stored in the
serialized data (see also Gotchas below) but are generated in code,
so when you change the default, you'd
now get a different value than from code generated from an older version of
the schema. There are situations, however, where this may be
desirable, especially if you can ensure a simultaneous rebuild of
all code.

### Enums

Define a sequence of named constants, each with a given value, or
increasing by one from the previous one. The default first value
is `0`. As you can see in the enum declaration, you specify the underlying
integral type of the enum with `:` (in this case `byte`), which then determines
the type of any fields declared with this enum type.

Typically, enum values should only ever be added, never removed (there is no
deprecation for enums). This requires code to handle forwards compatibility
itself, by handling unknown enum values.

### Unions

Unions share a lot of properties with enums, but instead of new names
for constants, you use names of tables. You can then declare
a union field, which can hold a reference to any of those types, and
additionally a hidden field with the suffix `_type` is generated that
holds the corresponding enum value, allowing you to know which type to
cast to at runtime.

Unions are a good way to be able to send multiple message types as a FlatBuffer.
Note that because a union field is really two fields, it must always be
part of a table, it cannot be the root of a FlatBuffer by itself.

If you have a need to distinguish between different FlatBuffers in a more
open-ended way, for example for use as files, see the file identification
feature below.

### Namespaces

These will generate the corresponding namespace in C++ for all helper
code, and packages in Java. You can use `.` to specify nested namespaces /
packages.

### Includes

You can include other schemas files in your current one, e.g.:

    include "mydefinitions.fbs";

This makes it easier to refer to types defined elsewhere. `include`
automatically ensures each file is parsed just once, even when referred to
more than once.

When using the `flatc` compiler to generate code for schema definitions,
only definitions in the current file will be generated, not those from the
included files (those you still generate separately).

### Root type

This declares what you consider to be the root table (or struct) of the
serialized data. This is particularly important for parsing JSON data,
which doesn't include object type information.

### File identification and extension

Typically, a FlatBuffer binary buffer is not self-describing, i.e. it
needs you to know its schema to parse it correctly. But if you
want to use a FlatBuffer as a file format, it would be convenient
to be able to have a "magic number" in there, like most file formats
have, to be able to do a sanity check to see if you're reading the
kind of file you're expecting.

Now, you can always prefix a FlatBuffer with your own file header,
but FlatBuffers has a built-in way to add an identifier to a
FlatBuffer that takes up minimal space, and keeps the buffer
compatible with buffers that don't have such an identifier.

You can specify in a schema, similar to `root_type`, that you intend
for this type of FlatBuffer to be used as a file format:

    file_identifier "MYFI";

Identifiers must always be exactly 4 characters long. These 4 characters
will end up as bytes at offsets 4-7 (inclusive) in the buffer.

For any schema that has such an identifier, `flatc` will automatically
add the identifier to any binaries it generates (with `-b`),
and generated calls like `FinishMonsterBuffer` also add the identifier.
If you have specified an identifier and wish to generate a buffer
without one, you can always still do so by calling
`FlatBufferBuilder::Finish` explicitly.

After loading a buffer, you can use a call like
`MonsterBufferHasIdentifier` to check if the identifier is present.

Note that this is best for open-ended uses such as files. If you simply wanted
to send one of a set of possible messages over a network for example, you'd
be better off with a union.

Additionally, by default `flatc` will output binary files as `.bin`.
This declaration in the schema will change that to whatever you want:

    file_extension "ext";

### RPC interface declarations

You can declare RPC calls in a schema, that define a set of functions
that take a FlatBuffer as an argument (the request) and return a FlatBuffer
as the response (both of which must be table types):

    rpc_service MonsterStorage {
      Store(Monster):StoreResponse;
      Retrieve(MonsterId):Monster;
    }

What code this produces and how it is used depends on language and RPC system
used, FlatBuffers itself does not offer this functionality.

### Comments & documentation

May be written as in most C-based languages. Additionally, a triple
comment (`///`) on a line by itself signals that a comment is documentation
for whatever is declared on the line after it
(table/struct/field/enum/union/element), and the comment is output
in the corresponding C++ code. Multiple such lines per item are allowed.

### Attributes

Attributes may be attached to a declaration, behind a field, or after
the name of a table/struct/enum/union. These may either have a value or
not. Some attributes like `deprecated` are understood by the compiler;
user defined ones need to be declared with the attribute declaration
(like `priority` in the example above), and are
available to query if you parse the schema at runtime.
This is useful if you write your own code generators/editors etc., and
you wish to add additional information specific to your tool (such as a
help text).

Current understood attributes:

-   `id: n` (on a table field): manually set the field identifier to `n`.
    If you use this attribute, you must use it on ALL fields of this table,
    and the numbers must be a contiguous range from 0 onwards.
    Additionally, since a union type effectively adds two fields, its
    id must be that of the second field (the first field is the type
    field and not explicitly declared in the schema).
    For example, if the last field before the union field had id 6,
    the union field should have id 8, and the unions type field will
    implicitly be 7.
    IDs allow the fields to be placed in any order in the schema.
    When a new field is added to the schema is must use the next available ID.
-   `deprecated` (on a field): do not generate accessors for this field
    anymore, code should stop using this data.
-   `required` (on a non-scalar table field): this field must always be set.
    By default, all fields are optional, i.e. may be left out. This is
    desirable, as it helps with forwards/backwards compatibility, and
    flexibility of data structures. It is also a burden on the reading code,
    since for non-scalar fields it requires you to check against NULL and
    take appropriate action. By specifying this field, you force code that
    constructs FlatBuffers to ensure this field is initialized, so the reading
    code may access it directly, without checking for NULL. If the constructing
    code does not initialize this field, they will get an assert, and also
    the verifier will fail on buffers that have missing required fields.
-   `original_order` (on a table): since elements in a table do not need
    to be stored in any particular order, they are often optimized for
    space by sorting them to size. This attribute stops that from happening.
-   `force_align: size` (on a struct): force the alignment of this struct
    to be something higher than what it is naturally aligned to. Causes
    these structs to be aligned to that amount inside a buffer, IF that
    buffer is allocated with that alignment (which is not necessarily
    the case for buffers accessed directly inside a `FlatBufferBuilder`).
-   `bit_flags` (on an enum): the values of this field indicate bits,
    meaning that any value N specified in the schema will end up
    representing 1<<N, or if you don't specify values at all, you'll get
    the sequence 1, 2, 4, 8, ...
-   `nested_flatbuffer: "table_name"` (on a field): this indicates that the field
    (which must be a vector of ubyte) contains flatbuffer data, for which the
    root type is given by `table_name`. The generated code will then produce
    a convenient accessor for the nested FlatBuffer.
-   `key` (on a field): this field is meant to be used as a key when sorting
    a vector of the type of table it sits in. Can be used for in-place
    binary search.

## JSON Parsing

The same parser that parses the schema declarations above is also able
to parse JSON objects that conform to this schema. So, unlike other JSON
parsers, this parser is strongly typed, and parses directly into a FlatBuffer
(see the compiler documentation on how to do this from the command line, or
the C++ documentation on how to do this at runtime).

Besides needing a schema, there are a few other changes to how it parses
JSON:

-   It accepts field names with and without quotes, like many JSON parsers
    already do. It outputs them without quotes as well, though can be made
    to output them using the `strict_json` flag.
-   If a field has an enum type, the parser will recognize symbolic enum
    values (with or without quotes) instead of numbers, e.g.
    `field: EnumVal`. If a field is of integral type, you can still use
    symbolic names, but values need to be prefixed with their type and
    need to be quoted, e.g. `field: "Enum.EnumVal"`. For enums
    representing flags, you may place multiple inside a string
    separated by spaces to OR them, e.g.
    `field: "EnumVal1 EnumVal2"` or `field: "Enum.EnumVal1 Enum.EnumVal2"`.
-   Similarly, for unions, these need to specified with two fields much like
    you do when serializing from code. E.g. for a field `foo`, you must
    add a field `foo_type: FooOne` right before the `foo` field, where
    `FooOne` would be the table out of the union you want to use.
-   A field that has the value `null` (e.g. `field: null`) is intended to
    have the default value for that field (thus has the same effect as if
    that field wasn't specified at all).

When parsing JSON, it recognizes the following escape codes in strings:

-   `\n` - linefeed.
-   `\t` - tab.
-   `\r` - carriage return.
-   `\b` - backspace.
-   `\f` - form feed.
-   `\"` - double quote.
-   `\\` - backslash.
-   `\/` - forward slash.
-   `\uXXXX` - 16-bit unicode code point, converted to the equivalent UTF-8
    representation.
-   `\xXX` - 8-bit binary hexadecimal number XX. This is the only one that is
     not in the JSON spec (see http://json.org/), but is needed to be able to
     encode arbitrary binary in strings to text and back without losing
     information (e.g. the byte 0xFF can't be represented in standard JSON).

It also generates these escape codes back again when generating JSON from a
binary representation.

## Gotchas

### Schemas and version control

FlatBuffers relies on new field declarations being added at the end, and earlier
declarations to not be removed, but be marked deprecated when needed. We think
this is an improvement over the manual number assignment that happens in
Protocol Buffers (and which is still an option using the `id` attribute
mentioned above).

One place where this is possibly problematic however is source control. If user
A adds a field, generates new binary data with this new schema, then tries to
commit both to source control after user B already committed a new field also,
and just auto-merges the schema, the binary files are now invalid compared to
the new schema.

The solution of course is that you should not be generating binary data before
your schema changes have been committed, ensuring consistency with the rest of
the world. If this is not practical for you, use explicit field ids, which
should always generate a merge conflict if two people try to allocate the same
id.

### Schema evolution examples

Some examples to clarify what happens as you change a schema:

If we have the following original schema:

    table { a:int; b:int; }

And we extend it:

    table { a:int; b:int; c:int; }

This is ok. Code compiled with the old schema reading data generated with the
new one will simply ignore the presence of the new field. Code compiled with the
new schema reading old data will get the default value for `c` (which is 0
in this case, since it is not specified).

    table { a:int (deprecated); b:int; }

This is also ok. Code compiled with the old schema reading newer data will now
always get the default value for `a` since it is not present. Code compiled
with the new schema now cannot read nor write `a` anymore (any existing code
that tries to do so will result in compile errors), but can still read
old data (they will ignore the field).

    table { c:int a:int; b:int; }

This is NOT ok, as this makes the schemas incompatible. Old code reading newer
data will interpret `c` as if it was `a`, and new code reading old data
accessing `a` will instead receive `b`.

    table { c:int (id: 2); a:int (id: 0); b:int (id: 1); }

This is ok. If your intent was to order/group fields in a way that makes sense
semantically, you can do so using explicit id assignment. Now we are compatible
with the original schema, and the fields can be ordered in any way, as long as
we keep the sequence of ids.

    table { b:int; }

NOT ok. We can only remove a field by deprecation, regardless of wether we use
explicit ids or not.

    table { a:uint; b:uint; }

This is MAYBE ok, and only in the case where the type change is the same size,
like here. If old data never contained any negative numbers, this will be
safe to do.

    table { a:int = 1; b:int = 2; }

Generally NOT ok. Any older data written that had 0 values were not written to
the buffer, and rely on the default value to be recreated. These will now have
those values appear to `1` and `2` instead. There may be cases in which this
is ok, but care must be taken.

    table { aa:int; bb:int; }

Occasionally ok. You've renamed fields, which will break all code (and JSON
files!) that use this schema, but as long as the change is obvious, this is not
incompatible with the actual binary buffers, since those only ever address
fields by id/offset.
<br>

### Testing whether a field is present in a table

Most serialization formats (e.g. JSON or Protocol Buffers) make it very
explicit in the format whether a field is present in an object or not,
allowing you to use this as "extra" information.

In FlatBuffers, this also holds for everything except scalar values.

FlatBuffers by default will not write fields that are equal to the default
value (for scalars), sometimes resulting in a significant space savings.

However, this also means testing whether a field is "present" is somewhat
meaningless, since it does not tell you if the field was actually written by
calling `add_field` style calls, unless you're only interested in this
information for non-default values.

Some `FlatBufferBuilder` implementations have an option called `force_defaults`
that circumvents this behavior, and writes fields even if they are equal to
the default. You can then use `IsFieldPresent` to query this.

Another option that works in all languages is to wrap a scalar field in a
struct. This way it will return null if it is not present. The cool thing
is that structs don't take up any more space than the scalar they represent.

   [Interface Definition Language]: https://en.wikipedia.org/wiki/Interface_description_language
