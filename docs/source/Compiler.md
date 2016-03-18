Using the schema compiler    {#flatbuffers_guide_using_schema_compiler}
=========================

Usage:

    flatc [ GENERATOR OPTIONS ] [ -o PATH ] [ -I PATH ] [ -S ] FILES...
          [ -- FILES...]

The files are read and parsed in order, and can contain either schemas
or data (see below). Data files are processed according to the definitions of
the most recent schema specified.

`--` indicates that the following files are binary files in
FlatBuffer format conforming to the schema indicated before it.

Depending on the flags passed, additional files may
be generated for each file processed:

For any schema input files, one or more generators can be specified:

-   `--cpp`, `-c` : Generate a C++ header for all definitions in this file (as
    `filename_generated.h`).

-   `--java`, `-j` : Generate Java code.

-   `--csharp`, `-n` : Generate C# code.

-   `--go`, `-g` : Generate Go code.

-   `--python`, `-p`: Generate Python code.

-   `--javascript`, `-s`: Generate JavaScript code.

-   `--php`: Generate PHP code.

For any data input files:

-   `--binary`, `-b` : If data is contained in this file, generate a
    `filename.bin` containing the binary flatbuffer (or a different extension
    if one is specified in the schema).

-   `--json`, `-t` : If data is contained in this file, generate a
    `filename.json` representing the data in the flatbuffer.

Additional options:

-   `-o PATH` : Output all generated files to PATH (either absolute, or
    relative to the current directory). If omitted, PATH will be the
    current directory. PATH should end in your systems path separator,
    e.g. `/` or `\`.

-   `-I PATH` : when encountering `include` statements, attempt to load the
    files from this path. Paths will be tried in the order given, and if all
    fail (or none are specified) it will try to load relative to the path of
    the schema file being parsed.

-   `-M` : Print make rules for generated files.

-   `--strict-json` : Require & generate strict JSON (field names are enclosed
    in quotes, no trailing commas in tables/vectors). By default, no quotes are
    required/generated, and trailing commas are allowed.

-   `--defaults-json` : Output fields whose value is equal to the default value
    when writing JSON text.

-   `--no-prefix` : Don't prefix enum values in generated C++ by their enum
    type.

-   `--scoped-enums` : Use C++11 style scoped and strongly typed enums in
    generated C++. This also implies `--no-prefix`.

-   `--gen-includes` : (deprecated), this is the default behavior.
                       If the original behavior is required (no include
	                   statements) use `--no-includes.`

-   `--no-includes` : Don't generate include statements for included schemas the
    generated file depends on (C++).

-   `--gen-mutable` : Generate additional non-const accessors for mutating
    FlatBuffers in-place.

-   `--gen-onefile` :  Generate single output file (useful for C#)

-   `--gen-all`: Generate not just code for the current schema files, but
    for all files it includes as well. If the language uses a single file for
    output (by default the case for C++ and JS), all code will end up in
    this one file.

-   `--raw-binary` : Allow binaries without a file_indentifier to be read.
    This may crash flatc given a mismatched schema.

-   `--proto`: Expect input files to be .proto files (protocol buffers).
    Output the corresponding .fbs file.
    Currently supports: `package`, `message`, `enum`, nested declarations,
    `import` (use `-I` for paths), `extend`, `oneof`, `group`.
    Does not support, but will skip without error: `option`, `service`,
    `extensions`, and most everything else.

-   `--schema`: Serialize schemas instead of JSON (use with -b). This will
    output a binary version of the specified schema that itself corresponds
    to the reflection/reflection.fbs schema. Loading this binary file is the
    basis for reflection functionality.

NOTE: short-form options for generators are deprecated, use the long form
whenever possible.
