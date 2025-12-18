# DigitalArsenal FlatBuffers
![Build status](https://github.com/digitalarsenal/flatbuffers/actions/workflows/build.yml/badge.svg?branch=master)


**This is a fork of the Google Flatbuffers Library with the following features added:**

- A `--preserve-case` flag to prevent IDL name mangling
- JSON Schema schema import/export (`--jsonschema`, `*.schema.json`)
- Optional lossless JSON Schema round-tripping via `--jsonschema-xflatbuffers` metadata

## Fork Features

This fork adds a few features to the `flatc` compiler intended to treat JSON Schema as a first-class schema format, while still flowing through the same FlatBuffers schema IR (see [`reflection/reflection.fbs`](reflection/reflection.fbs)).

### Preserve-case naming (`--preserve-case`)

By default, many language generators apply case conversions to schema identifiers (for example converting `snake_case` field names into `camelCase` accessors). The `--preserve-case` flag disables this name mangling for identifiers coming from the schema, and emits names “as written” instead.

Example:

```sh
flatc --cpp --preserve-case schema.fbs
```

Notes:

- This is currently supported for these generators: C++, Go, Java, Rust, Dart, Python, TypeScript, PHP, and JSON Schema (see [`src/flatc.cpp`](src/flatc.cpp)).
- Implementation: [`src/flatc.cpp`](src/flatc.cpp), [`src/util.cpp`](src/util.cpp).
- Tests: [`tests/GoTest.sh`](tests/GoTest.sh), [`tests/PHPTest.sh`](tests/PHPTest.sh), [`tests/PythonTest.sh`](tests/PythonTest.sh), [`tests/JsonSchemaTest.sh`](tests/JsonSchemaTest.sh).

### JSON Schema schema import/export

- Export a FlatBuffers schema (`.fbs`) to JSON Schema (`.schema.json`).
- Import a JSON Schema (`.schema.json`) as a schema input (as if it were an IDL), map it into FlatBuffers’ schema IR, and run the normal FlatBuffers code generators.

Implementation:

- JSON Schema generator: [`src/idl_gen_json_schema.cpp`](src/idl_gen_json_schema.cpp)
- JSON Schema importer/parser: [`src/idl_parser.cpp`](src/idl_parser.cpp) (`Parser::DoParseJsonSchema`)
- CLI wiring + `.schema.json` input detection: [`src/flatc.cpp`](src/flatc.cpp)
- Additional docs: [`docs/source/json_schema.md`](docs/source/json_schema.md), [`docs/source/flatc.md`](docs/source/flatc.md)

#### Export: FlatBuffers → JSON Schema (`--jsonschema`)

Generate `*.schema.json` from `*.fbs`:

```sh
flatc --jsonschema -o out_dir schema.fbs
```

This produces `out_dir/schema.schema.json`.

#### Import: JSON Schema → FlatBuffers IR (`*.schema.json` input)

Any file ending in `.schema.json` can be used anywhere `flatc` expects a schema file:

```sh
flatc --cpp -o out_dir schema.schema.json
```

Root selection:

- If the schema root contains a `$ref` to a definition, that definition becomes the FlatBuffers root type.
- Otherwise you can specify/override the root with `--root-type` (see [`src/flatc.cpp`](src/flatc.cpp) and [`docs/source/flatc.md`](docs/source/flatc.md)).

#### Best-effort mapping for “wild” JSON Schema + OpenAPI

The importer is intentionally permissive and ignores unknown JSON Schema/OpenAPI keywords while making “sane defaults” to treat the input as an IDL.

Supported input shapes:

- Schema definitions under `definitions`, `$defs`, or OpenAPI `components.schemas` (see fixtures under [`tests/jsonschema_import/inputs`](tests/jsonschema_import/inputs)).
- `$ref` resolution for `#/definitions/...`, `#/$defs/...`, and `#/components/schemas/...`.

Type/shape mapping (when `x-flatbuffers` is not present):

- `type: "object"` → FlatBuffers table by default; may infer a struct if the definition contains fixed-length arrays and is otherwise “struct-safe” (see [`src/idl_parser.cpp`](src/idl_parser.cpp)).
- `type: "array"` → FlatBuffers vector; if `minItems == maxItems` it may become a fixed-length array (and will fall back to a vector with `minItems`/`maxItems` preserved if a fixed array would be illegal in FlatBuffers).
- `type: "integer"` → a concrete FlatBuffers integer scalar inferred from numeric range (`minimum`/`maximum`) when provided; `format` of `int32`/`int64`/`uint32`/`uint64` overrides inference.
- `type: "number"` → `float` by default; `format` of `float`/`double` overrides.
- `type: "string"` → FlatBuffers `string`.
- String `enum: ["A", "B", ...]` on a field → generates a FlatBuffers enum for that field.
- `anyOf: [{ "$ref": ... }, ...]` on a field → FlatBuffers union. If the input follows the FlatBuffers JSON/JSON-Schema union convention (a value field plus a sibling `<name>_type` field), the importer will link them (see [`src/idl_parser.cpp`](src/idl_parser.cpp)).

JSON Schema/OpenAPI keyword preservation:

To keep the generated JSON Schema close to the original, the importer preserves a subset of JSON Schema/OpenAPI keywords (either as FlatBuffers doc flags, or as `jsonschema_*` attributes so they survive through the schema IR), and the JSON Schema generator re-emits them:

- Definitions + fields: `description`
- Fields: `deprecated`
- Objects: `required`, `additionalProperties`
- Arrays: `minItems`, `maxItems`, `uniqueItems`
- Strings: `format`, `minLength`, `maxLength`, `readOnly`
- Numbers/integers: `minimum`, `maximum`, `exclusiveMinimum`, `exclusiveMaximum`

#### Optional lossless semantics: `x-flatbuffers` (`--jsonschema-xflatbuffers`)

JSON Schema cannot represent some FlatBuffers semantics (for example: struct vs table, exact scalar widths, union details, field ids, and presence rules). To enable lossless round-trips, the JSON Schema generator can emit an optional vendor extension:

```sh
flatc --jsonschema --jsonschema-xflatbuffers -o out_dir schema.fbs
```

This emits `x-flatbuffers` metadata objects at the schema root, at each definition, and at each field. Because this uses the standard vendor extension mechanism (`x-...`), most JSON Schema tooling ignores it and continues to work normally (for example QuickType and similar code generators).

At a high level:

- Root metadata: `root_type`, plus optional `file_identifier` and `file_extension`.
- Definition metadata: enum/union kind + values; struct/table kind + (struct-only) `minalign`/`bytesize`.
- Field metadata: exact FlatBuffers type (including union/enum refs), plus presence and selected field attributes (for example `id`, `deprecated`, `key`).

The allowed keys/values for the `x-flatbuffers` vendor extension are described by the meta-schema:

- [`docs/source/schemas/x-flatbuffers.schema.json`](docs/source/schemas/x-flatbuffers.schema.json)

#### Tests (goldens + round-trip stability)

This fork uses golden JSON Schemas and round-trip tests to ensure stability:

- Generation + round-trip for FlatBuffers-emitted JSON Schema (with and without `x-flatbuffers`): [`tests/JsonSchemaTest.sh`](tests/JsonSchemaTest.sh)
  - Goldens: [`tests/monster_test.schema.json`](tests/monster_test.schema.json), [`tests/arrays_test.schema.json`](tests/arrays_test.schema.json)
- Import “wild” JSON Schema / OpenAPI fixtures and ensure stable regeneration: [`tests/JsonSchemaImportTest.sh`](tests/JsonSchemaImportTest.sh)
  - Inputs: [`tests/jsonschema_import/inputs`](tests/jsonschema_import/inputs)
  - Goldens: [`tests/jsonschema_import/goldens`](tests/jsonschema_import/goldens)

Typical local run:

```sh
cmake -B build -S .
cmake --build build --target flatc -j
tests/JsonSchemaTest.sh
tests/JsonSchemaImportTest.sh
```


![logo](https://flatbuffers.dev/assets/flatbuffers_logo.svg) FlatBuffers
===========

![Build status](https://github.com/google/flatbuffers/actions/workflows/build.yml/badge.svg?branch=master)
[![BuildKite status](https://badge.buildkite.com/7979d93bc6279aa539971f271253c65d5e8fe2fe43c90bbb25.svg)](https://buildkite.com/bazel/flatbuffers)
[![Fuzzing Status](https://oss-fuzz-build-logs.storage.googleapis.com/badges/flatbuffers.svg)](https://bugs.chromium.org/p/oss-fuzz/issues/list?sort=-opened&can=1&q=proj:flatbuffers)
[![Discord Chat](https://img.shields.io/discord/656202785926152206.svg)](https:///discord.gg/6qgKs3R)
[![Twitter Follow](https://img.shields.io/twitter/follow/wvo.svg?style=social)](https://twitter.com/wvo)
[![Twitter Follow](https://img.shields.io/twitter/follow/dbaileychess.svg?style=social)](https://twitter.com/dbaileychess)


**FlatBuffers** is a cross platform serialization library architected for
maximum memory efficiency. It allows you to directly access serialized data without parsing/unpacking it first, while still having great forwards/backwards compatibility.

## Quick Start

1. Build the compiler for flatbuffers (`flatc`)

    Use `cmake` to create the build files for your platform and then perform the compilation (Linux example).

    ```
    cmake -G "Unix Makefiles"
    make -j
    ```

2. Define your flatbuffer schema (`.fbs`)

    Write the [schema](https://flatbuffers.dev/flatbuffers_guide_writing_schema.html) to define the data you want to serialize. See [monster.fbs](https://github.com/google/flatbuffers/blob/master/samples/monster.fbs) for an example.

3. Generate code for your language(s)

    Use the `flatc` compiler to take your schema and generate language-specific code:

    ```
    ./flatc --cpp --rust monster.fbs
    ```

    Which generates `monster_generated.h` and `monster_generated.rs` files.

4. Serialize data

    Use the generated code, as well as the `FlatBufferBuilder` to construct your serialized buffer. ([`C++` example](https://github.com/google/flatbuffers/blob/master/samples/sample_binary.cpp#L24-L56))

5. Transmit/store/save Buffer

    Use your serialized buffer however you want. Send it to someone, save it for later, etc...

6. Read the data

    Use the generated accessors to read the data from the serialized buffer.

    It doesn't need to be the same language/schema version, FlatBuffers ensures the data is readable across languages and schema versions. See the [`Rust` example](https://github.com/google/flatbuffers/blob/master/samples/sample_binary.rs#L92-L106) reading the data written by `C++`.

## Documentation

**Go to our [landing page][] to browse our documentation.**

## Supported operating systems
- Windows
- macOS
- Linux
- Android
- And any others with a recent C++ compiler (C++ 11 and newer)

## Supported programming languages

Code generation and runtime libraries for many popular languages.

1. C
1. C++ - [snapcraft.io](https://snapcraft.io/flatbuffers)
1. C# - [nuget.org](https://www.nuget.org/packages/Google.FlatBuffers)
1. Dart - [pub.dev](https://pub.dev/packages/flat_buffers)
1. Go - [go.dev](https://pkg.go.dev/github.com/google/flatbuffers)
1. Java - [Maven](https://search.maven.org/artifact/com.google.flatbuffers/flatbuffers-java)
1. JavaScript - [NPM](https://www.npmjs.com/package/flatbuffers)
1. Kotlin
1. Lobster
1. Lua
1. PHP
1. Python - [PyPI](https://pypi.org/project/flatbuffers/)
1. Rust - [crates.io](https://crates.io/crates/flatbuffers)
1. Swift - [swiftpackageindex](https://swiftpackageindex.com/google/flatbuffers)
1. TypeScript - [NPM](https://www.npmjs.com/package/flatbuffers)
1. Nim

## Versioning

FlatBuffers does not follow traditional SemVer versioning (see [rationale](https://github.com/google/flatbuffers/wiki/Versioning)) but rather uses a format of the date of the release.

## Contribution

* [FlatBuffers Issues Tracker][] to submit an issue.
* [stackoverflow.com][] with [`flatbuffers` tag][] for any questions regarding FlatBuffers.

*To contribute to this project,* see [CONTRIBUTING][].

## Community

* [Discord Server](https:///discord.gg/6qgKs3R)

## Security

Please see our [Security Policy](SECURITY.md) for reporting vulnerabilities.

## Licensing
*Flatbuffers* is licensed under the Apache License, Version 2.0. See [LICENSE][] for the full license text.

<br>

   [CONTRIBUTING]: http://github.com/google/flatbuffers/blob/master/CONTRIBUTING.md
   [`flatbuffers` tag]: https://stackoverflow.com/questions/tagged/flatbuffers
   [FlatBuffers Google Group]: https://groups.google.com/forum/#!forum/flatbuffers
   [FlatBuffers Issues Tracker]: http://github.com/google/flatbuffers/issues
   [stackoverflow.com]: http://stackoverflow.com/search?q=flatbuffers
   [landing page]: https://google.github.io/flatbuffers
   [LICENSE]: https://github.com/google/flatbuffers/blob/master/LICENSE
