# Flatbuffers Change Log

All major or breaking changes will be documented in this file, as well as any
new features that should be highlighted. Minor fixes or improvements are not
necessarily listed.

## 2.0.7 (Aug 22 2022)

* This is the first version with an explicit change log, so all the previous
features will not be listed.

* Verifier now checks that buffers are at least the minimum size required to be
a flatbuffers (12 bytes). This includes nested flatbuffers, which previously
could be declared valid at size 0.

* Annotated binaries. Given a flatbuffer binary and a schema (or binary schema)
one can generate an annotated flatbuffer (.afb) to describe each byte in the
binary with schema metadata and value.

* First binary schema generator (Lua) to generate Lua code via a .bfbs file. 
This is mostly an implementation detail of flatc internals, but will be slowly
applied to the other language generators.