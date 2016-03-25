Use in C    {#flatbuffers_guide_use_c}
==========

The C language binding exists in a separate project named [FlatCC](https://github.com/dvidelabs/flatcc).

The `flatcc` C schema compiler can generate code offline as well as
online via a C library. It can also generate buffer verifiers and fast
JSON parsers, printers.

Great effort has been made to ensure compatibily with the main `flatc`
project.


## General Documention

- [Tutorial](@ref flatbuffers_guide_tutorial) - select C as language
  when scrolling down
- General Use in C (the README) <https://github.com/dvidelabs/flatcc/blob/master/README.md>
- The C Builder Interface, advanced <https://github.com/dvidelabs/flatcc/blob/master/doc/builder.md>


## Basic Reflection

The C-API does support reading binary schema (.bfbs)
files via code generated from the `reflection.fbs` schema, and an
[example usage](https://github.com/dvidelabs/flatcc/tree/master/samples/reflection)
shows how to use this. The schema files are pre-generated
in the [runtime distribution](https://github.com/dvidelabs/flatcc/tree/master/include/flatcc/reflection). Extended reflection


## Mutating Reflection

The C-API does not support mutating reflection like C++ does.

Although the following isn't reflection, it is possible to create new
buffers using complex objects from existing buffers as source. This can
be very efficient due to direct copy semantics without endian conversion or
temporary stack allocation.

It is currently not possible to use an existing table or vector of table
as source, but it would be possible to add support for this at some
point.


## Why not integrate with the `flatc` tool?

[It was considered how the C code generator could be integrated into the
`flatc` tool](https://github.com/dvidelabs/flatcc/issues/1), but it
would either require that the standalone C implementation of the schema
compiler was dropped, or it would lead to excessive code duplication, or
a complicated intermediate representation would have to be invented.
Neither of these alternatives are very attractive, and it isn't a big
deal to use the `flatcc` tool instead of `flatc` given that the
FlatBuffers C runtime library needs to be made available regardless.
