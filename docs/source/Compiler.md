# Using the schema compiler

Usage:

    flatc [ -c ] [ -j ] [ -b ] [ -t ] [ -o PATH ] [ -S ] file1 file2 ..

The files are read and parsed in order, and can contain either schemas
or data (see below). Later files can make use of definitions in earlier
files. Depending on the flags passed, additional files may
be generated for each file processed:

-   `-c` : Generate a C++ header for all definitions in this file (as
    `filename_generated.h`). Skips data.

-   `-j` : Generate Java classes.

-   `-b` : If data is contained in this file, generate a
    `filename_wire.bin` containing the binary flatbuffer.

-   `-t` : If data is contained in this file, generate a
    `filename_wire.txt` (for debugging).

-   `-o PATH` : Output all generated files to PATH (either absolute, or
    relative to the current directory). If omitted, PATH will be the
    current directory. PATH should end in your systems path separator,
    e.g. `/` or `\`.

-   `-S` : Generate strict JSON (field names are enclosed in quotes).
    By default, no quotes are generated.
