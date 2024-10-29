# Flatbuffers Change Log

All major or breaking changes will be documented in this file, as well as any
new features that should be highlighted. Minor fixes or improvements are not
necessarily listed.

## 2.0.8 (Aug 29 2022)

* Fix for `--keep-prefix` the was generating the wrong include statements for
C++ (#7469). The bug was introduced in 2.0.7.

* Added the `Verifier::Options` option struct to allow specifying runtime
configuration settings for the verifier (#7489). This allows to skip verifying
nested flatbuffers, a on-by-default change that was introduced in 2.0.7. This
deprecates the existing `Verifier` constructor, which may be removed in a future
version.

* Refactor of `tests/test.cpp` that lead to ~10% speedup in compilation of the
entire project (#7487).

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