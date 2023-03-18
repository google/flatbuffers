# Flatbuffers Change Log

All major or breaking changes will be documented in this file, as well as any
new features that should be highlighted. Minor fixes or improvements are not
necessarily listed.

## [23.3.3 (Mar 3 2023)](https://github.com/google/flatbuffers/releases/tag/v23.3.3)

* Refactoring of `flatc` generators to use an interface (#7797).

* Removed legacy cmake support and set min to 3.8 (#7801).

## [23.1.21 (Jan 21 2023)](https://github.com/google/flatbuffers/releases/tag/v23.1.20)

* Reworked entry points for Typescript/Javascript and compatibility for single
  file build (#7510)

## [23.1.20 (Jan 20 2023)](https://github.com/google/flatbuffers/releases/tag/v23.1.20)

* Removed go.mod files after some versioning issues were being report (#7780).

## [23.1.4 (Jan 4 2023)](https://github.com/google/flatbuffers/releases/tag/v23.1.4)

* Major release! Just kidding, we are continuing the
  [versioning scheme](https://github.com/google/flatbuffers/wiki/Versioning) of
  using a date to signify releases. This results in the first release of the new
  year to bump the tradition major version field.

* Go minimum version is now 1.19 (#7720) with the addition of Go modules.

* Added CI support for Big Endian regression testing (#7707).

* Fixed `getFullyQualifiedName` in typescript to return name delimited by '.'
  instead of '_' (#7730).

* Fixed the versioning scheme to not include leading zeros which are not
  consistently handled by every package manager. Only the last release
  (12.12.06) should have suffered from this.

## [22.12.06 (Dec 06 2022)](https://github.com/google/flatbuffers/releases/tag/v22.12.06)

* Bug fixing release, no major changes.

## [22.10.25 (Oct 25 2022)](https://github.com/google/flatbuffers/releases/tag/v22.10.25)

* Added Nim language support with generator and runtime libraries (#7534).

## [22.9.29 (Sept 29 2022)](https://github.com/google/flatbuffers/releases/tag/v22.9.29)

* Rust soundness fixes to avoid the crate from bing labelled unsafe (#7518).

## [22.9.24 (Sept 24 2022)](https://github.com/google/flatbuffers/releases/tag/v22.9.24)

* 20 Major releases in a row? Nope, we switched to a new
  [versioning scheme](https://github.com/google/flatbuffers/wiki/Versioning)
  that is based on date.

* Python supports fixed size arrays now (#7529).

* Behavior change in how C++ object API uses `UnPackTo`. The original intent of
  this was to reduce allocations by reusing an existing object to pack data
  into. At some point, this logic started to merge the states of the two objects
  instead of clearing the state of the packee. This change goes back to the
  original intention, the packed object is cleared when getting data packed into
  it (#7527).

* Fixed a bug in C++ alignment that was using `sizeof()` instead of the intended
  `AlignOf()` for structs (#7520).

* C# has an
  [official Nuget package](https://www.nuget.org/packages/Google.FlatBuffers)
  now (#7496).

## 2.0.8 (Aug 29 2022)

* Fix for `--keep-prefix` the was generating the wrong include statements for
  C++ (#7469). The bug was introduced in 2.0.7.

* Added the `Verifier::Options` option struct to allow specifying runtime
  configuration settings for the verifier (#7489). This allows to skip verifying
  nested flatbuffers, a on-by-default change that was introduced in 2.0.7. This
  deprecates the existing `Verifier` constructor, which may be removed in a
  future version.

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
