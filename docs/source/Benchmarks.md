# Benchmarks

Comparing against other serialization solutions, running on Windows 7
64bit. We use the LITE runtime for Protocol Buffers (less code / lower
overhead), and Rapid JSON, one of the fastest C++ JSON parsers around.

We compare against Flatbuffers with the binary wire format (as
intended), and also with JSON as the wire format with the optional JSON
parser (which, using a schema, parses JSON into a binary buffer that can
then be accessed as before).

The benchmark object is a set of about 10 objects containing an array, 4
strings, and a large variety of int/float scalar values of all sizes,
meant to be representative of game data, e.g. a scene format.

|                                                        | FlatBuffers (binary)  | Protocol Buffers LITE | Rapid JSON            | FlatBuffers (JSON)    |
|--------------------------------------------------------|-----------------------|-----------------------|-----------------------|-----------------------|
| Decode + Traverse + Dealloc (1 million times, seconds) | 0.08                  | 305                   | 583                   | 105                   |
| Decode / Traverse / Dealloc (breakdown)                | 0 / 0.08 / 0          | 220 / 3.6 / 81        | 294 / 0.9 / 287       | 70 / 0.08 / 35        |
| Encode (1 million times, seconds)                      | 3.2                   | 185                   | 650                   | 169                   |
| Wire format size (normal / zlib, bytes)                | 344 / 220             | 228 / 174             | 1475 / 322            | 1029 / 298            |
| Memory needed to store decoded wire (bytes / blocks)   | 0 / 0                 | 760 / 20              | 65689 / 40            | 328 / 1               |
| Transient memory allocated during decode (KB)          | 0                     | 1                     | 131                   | 4                     |
| Generated source code size (KB)                        | 4                     | 61                    | 0                     | 4                     |
| Field access in handwritten traversal code             | accessors             | accessors             | manual error checking | accessors             |
| Library source code (KB)                               | 15                    | some subset of 3800   | 87                    | 43                    |

### Some other serialization systems we compared against but did not benchmark (yet), in rough order of applicability:

-   Cap'n'Proto promises to reduce Protocol Buffers much like FlatBuffers does,
    though with a more complicated binary encoding and less flexibility (no
    optional fields to allow deprecating fields or serializing with missing
    fields for which defaults exist).
    It currently also isn't fully cross-platform portable (lack of VS support).
-   msgpack: has very minimal forwards/backwards compatability support when used
    with the typed C++ interface. Also lacks VS2010 support.
-   Thrift: very similar to Protocol Buffers, but appears to be less efficient,
    and have more dependencies.
-   XML: typically even slower than JSON, but has the advantage that it can be
    parsed with a schema to reduce error-checking boilerplate code.
-   YAML: a superset of JSON and otherwise very similar. Used by e.g. Unity.
-   C# comes with built-in serialization functionality, as used by Unity also.
    Being tied to the language, and having no automatic versioning support
    limits its applicability.
-   Project Anarchy (the free mobile engine by Havok) comes with a serialization
    system, that however does no automatic versioning (have to code around new
    fields manually), is very much tied to the rest of the engine, and works
    without a schema to generate code (tied to your C++ class definition).

