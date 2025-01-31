# Annotating FlatBuffers

This provides a way to annotate flatbuffer binary data, byte-by-byte, with a
schema. It is useful for development purposes and understanding the details of
the internal format.

## Annotating

Given a `schema`, as either a plain-text (`.fbs`) or a binary schema (`.bfbs`),
and `binary` file(s) that were created by the `schema`. You can annotate them
using:

```sh
flatc --annotate SCHEMA -- BINARY_FILES...
```

This will produce a set of annotated files (`.afb` Annotated FlatBuffer)
corresponding to the input binary files.

### Example

Taken from the [tests/annotated_binary](https://github.com/google/flatbuffers/tree/master/tests/annotated_binary).

```sh
cd tests/annotated_binary
../../flatc --annotate annotated_binary.fbs -- annotated_binary.bin
```

Which will produce a `annotated_binary.afb` file in the current directory.

The `annotated_binary.bin` is the flatbufer binary of the data contained within
`annotated_binary.json`, which was made by the following command:

```sh
..\..\flatc -b annotated_binary.fbs annotated_binary.json
```

## .afb Text Format

Currently there is a built-in text-based format for outputting the annotations.
A full example is shown here:
[`annotated_binary.afb`](https://github.com/google/flatbuffers/blob/master/tests/annotated_binary/annotated_binary.afb)

The data is organized as a table with fixed [columns](#columns) grouped into
Binary [sections](#binary-sections) and [regions](#binary-regions), starting
from the beginning of the binary (offset `0`).

### Columns

The columns are as follows:

1. The offset from the start of the binary, expressed in hexadecimal format
   (e.g. `+0x003c`).

    The prefix `+` is added to make searching for the offset (compared to some
    random value) a bit easier.

2. The raw binary data, expressed in hexadecimal format. 
   
    This is in the little endian format the buffer uses internally and what you
    would see with a normal binary text viewer.

3. The type of the data.

    This may be the type specified in the schema or some internally defined
    types:


    | Internal Type | Purpose                                            |
    |---------------|----------------------------------------------------|
    | `VOffset16`   | Virtual table offset, relative to the table offset |
    | `UOffset32`   | Unsigned offset, relative to the current offset    |
    | `SOffset32`   | Signed offset, relative to the current offset      |


4. The value of the data.

    This is shown in big endian format that is generally written for humans to
    consume (e.g. `0x0013`). As well as the "casted" value (e.g. `0x0013 `is
    `19` in decimal) in parentheses.

5. Notes about the particular data.

    This describes what the data is about, either some internal usage, or tied
    to the schema.

### Binary Sections

The file is broken up into Binary Sections, which are comprised of contiguous
[binary regions](#binary-regions) that are logically grouped together. For
example, a binary section may be a single instance of a flatbuffer `Table` or
its `vtable`. The sections may be labelled with the name of the associated type,
as defined in the input schema.

An example of a `vtable` Binary Section that is associated with the user-defined
`AnnotateBinary.Bar` table.

```
vtable (AnnotatedBinary.Bar):
  +0x00A0 | 08 00      | uint16_t   | 0x0008 (8)   | size of this vtable
  +0x00A2 | 13 00      | uint16_t   | 0x0013 (19)  | size of referring table
  +0x00A4 | 08 00      | VOffset16  | 0x0008 (8)   | offset to field `a` (id: 0)
  +0x00A6 | 04 00      | VOffset16  | 0x0004 (4)   | offset to field `b` (id: 1)
```

These are purely annotative, there is no embedded information about these
regions in the flatbuffer itself.

### Binary Regions

Binary regions are contiguous bytes regions that are grouped together to form 
some sort of value, e.g. a `scalar` or an array of scalars. A binary region may
be split up over multiple text lines, if the size of the region is large.

#### Annotation Example

Looking at an example binary region:

```
vtable (AnnotatedBinary.Bar):
  +0x00A0 | 08 00      | uint16_t   | 0x0008 (8)   | size of this vtable
```

The first column (`+0x00A0`) is the offset to this region from the beginning of
the buffer. 

The second column are the raw bytes (hexadecimal) that make up this region.
These are expressed in the little-endian format that flatbuffers uses for the
wire format.

The third column is the type to interpret the bytes as. For the above example,
the type is `uint16_t` which is a 16-bit unsigned integer type.

The fourth column shows the raw bytes as a compacted, big-endian value. The raw
bytes are duplicated in this fashion since it is more intuitive to read the data
in the big-endian format (e.g., `0x0008`). This value is followed by the decimal
representation of the value (e.g., `(8)`). For strings, the raw string value is
shown instead. 

The fifth column is a textual comment on what the value is. As much metadata as
known is provided.

### Offsets

If the type in the 3rd column is of an absolute offset (`SOffet32` or
`Offset32`), the fourth column also shows an `Loc: +0x025A` value which shows
where in the binary this region is pointing to. These values are absolute from
the beginning of the file, their calculation from the raw value in the 4th
column depends on the context.