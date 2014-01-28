# Using the schema compiler

Usage:

    flatc [ -c ] [ -j ] [ -b ] [ -t ] file1 file2 ..

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

