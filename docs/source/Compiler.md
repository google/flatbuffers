# Using the schema compiler

Usage:

    flatc [ -c ] [ -j ] [ -b ] [ -t ] [ -o PATH ] [ -I PATH ] [ -S ] FILES...
          [ -- FILES...]

The files are read and parsed in order, and can contain either schemas
or data (see below). Later files can make use of definitions in earlier
files.

`--` indicates that the following files are binary files in
FlatBuffer format conforming to the schema(s) indicated before it.
Incompatible binary files currently will give unpredictable results (!)

Depending on the flags passed, additional files may
be generated for each file processed:

-   `-c` : Generate a C++ header for all definitions in this file (as
    `filename_generated.h`). Skipped for data.

-   `-j` : Generate Java classes. Skipped for data.

-   `-n` : Generate C# classes. Skipped for data.

-   `-g` : Generate Go classes. Skipped for data.

-   `-b` : If data is contained in this file, generate a
    `filename.bin` containing the binary flatbuffer.

-   `-t` : If data is contained in this file, generate a
    `filename.json` representing the data in the flatbuffer.

-   `-o PATH` : Output all generated files to PATH (either absolute, or
    relative to the current directory). If omitted, PATH will be the
    current directory. PATH should end in your systems path separator,
    e.g. `/` or `\`.

-   `-I PATH` : when encountering `include` statements, attempt to load the
    files from this path. Paths will be tried in the order given, and if all
    fail (or none are specified) it will try to load relative to the path of
    the schema file being parsed.

-   `--strict-json` : Require & generate strict JSON (field names are enclosed
    in quotes, no trailing commas in tables/vectors). By default, no quotes are
    required/generated, and trailing commas are allowed.

-   `--defaults-json` : Output fields whose value is equal to the default value
    when writing JSON text.

-   `--no-prefix` : Don't prefix enum values in generated C++ by their enum
    type.

-   `--gen-includes` : (deprecated), instead use:
-   `--no-includes` : Don't generate include statements for included schemas the
    generated file depends on (C++).

-   `--gen-mutable` : Generate additional non-const accessors for mutating
    FlatBuffers in-place.

-   `--gen-onefile` :  Generate single output file (useful for C#)

-   `--raw-binary` : Allow binaries without a file_indentifier to be read.
    This may crash flatc given a mismatched schema.

-   `--proto`: Expect input files to be .proto files (protocol buffers).
    Output the corresponding .fbs file.
    Currently supports: `package`, `message`, `enum`.
    Does not support, but will skip without error: `import`, `option`.
    Does not support, will generate error: `service`, `extend`, `extensions`,
    `oneof`, `group`, custom options, nested declarations.

-   `--schema`: Serialize schemas instead of JSON (use with -b). This will
    output a binary version of the specified schema that itself corresponds
    to the reflection/reflection.fbs schema. Loading this binary file is the
    basis for reflection functionality.
