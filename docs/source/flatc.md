# FlatBuffers Compiler (`flatc`)

The main compiler for FlatBuffers is called `flatc` and is used to convert
schema definitions into generated code files for a variety of languages. 

After [building](building.md) `flatc`, it is used as follows:

```sh
flatc [ GENERATOR_OPTIONS ] [ -o PATH ] [- I PATH ] 
  FILES... 
  [ -- BINARY_FILES... ]
```

* The `GENERATOR_OPTIONS` specify the language(s) to compile code for as well as
various features to enable/disable.

* The `-o PATH` specifies the path where the generated files are placed. It
defaults to the current path if not specified.

* The `-I PATH` specifies the paths where included schema files are located. It
  defaults to the current path if not specified.

## Input Files

`FILES...` specifies one or more schema or data files to process. They are
processed in the order provided.

### Schema Files

For schema files, language specifiers indicate what languages to generate code
for.

  * `--cpp`: C++
  * `--java`: Java
  * `--kotlin`: Kotlin
  * `--csharp`: C#
  * `--go`: Golang
  * `--python`: Python
  * `--js`: JavaScript
  * `--ts`: TypeScript
  * `--php`: PHP
  * `--dart`: Dart
  * `--lua`: Lua
  * `--lobster`: Lobster
  * `--rust`: Rust
  * `--swift`: Swift
  * `--nim`: Nim

Additionally, adding:

  * `--grpc` Will generate RPC stub code for gRPC (not available in all
    languages)

### Data Files

If `FILES...` contain data files, they can be exported to either a binary or
JSON representation.

* `--binary`, `-b`: Generate a binary file containing a serialized flatbuffer.
* `--json`, `-j`: Generate JSON file from a serialized flatbuffer.

Both options require the corresponding schema file to be included first in the
list of `FILES...`.

=== "To Binary"

    To serialize the JSON data in `mydata.json` using the schema `myschema.fbs`:
   
    ```sh
    flatc --binary myschema.fbs mydata.json
    ```

    This will generate a `mydata_wire.bin` file containing the serialized 
    flatbuffer data.

=== "To JSON"

    To convert the serialized binary flatbuffer `mydata.bin` using the schema 
    `myschema.fbs` to JSON:

    ```sh
    flatc --json myschema.fbs mydata.bin
    ```

    This will generate a `mydata.json` file.


### Additional options

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

-   `--allow-non-utf8` : Pass non-UTF-8 input through parser and emit nonstandard
    \x escapes in JSON. (Default is to raise parse error on non-UTF-8 input.)

-   `--natural-utf8` : Output strings with UTF-8 as human-readable strings.
     By default, UTF-8 characters are printed as \uXXXX escapes."

-   `--defaults-json` : Output fields whose value is equal to the default value
    when writing JSON text.

-   `--no-prefix` : Don't prefix enum values in generated C++ by their enum
    type.

-   `--scoped-enums` : Use C++11 style scoped and strongly typed enums in
    generated C++. This also implies `--no-prefix`.

-   `--no-emit-min-max-enum-values` : Disable generation of MIN and MAX
    enumerated values for scoped enums and prefixed enums.

-   `--gen-includes` : (deprecated), this is the default behavior.
                       If the original behavior is required (no include
	                   statements) use `--no-includes.`

-   `--no-includes` : Don't generate include statements for included schemas the
    generated file depends on (C++ / Python).

-   `--gen-mutable` : Generate additional non-const accessors for mutating
    FlatBuffers in-place.

-   `--gen-onefile` : Generate single output file for C#, Go, and Python.

-   `--gen-name-strings` : Generate type name functions for C++.

-   `--gen-object-api` : Generate an additional object-based API. This API is
    more convenient for object construction and mutation than the base API,
    at the cost of efficiency (object allocation). Recommended only to be used
    if other options are insufficient.

-   `--gen-compare`  :  Generate operator== for object-based API types.

-   `--gen-nullable` : Add Clang \_Nullable for C++ pointer. or @Nullable for Java.

-   `--gen-generated` : Add @Generated annotation for Java.

-   `--gen-jvmstatic` : Add @JvmStatic annotation for Kotlin methods
    in companion object for interop from Java to Kotlin.

-   `--gen-all` : Generate not just code for the current schema files, but
    for all files it includes as well. If the language uses a single file for
    output (by default the case for C++ and JS), all code will end up in
    this one file.

-   `--cpp-include` : Adds an #include in generated file

-   `--cpp-ptr-type T` : Set object API pointer type (default std::unique_ptr)

-   `--cpp-str-type T` : Set object API string type (default std::string)
    T::c_str(), T::length() and T::empty() must be supported.
    The custom type also needs to be constructible from std::string (see the
	--cpp-str-flex-ctor option to change this behavior).

-   `--cpp-str-flex-ctor` : Don't construct custom string types by passing
    std::string from Flatbuffers, but (char* + length). This allows efficient
	construction of custom string types, including zero-copy construction.

-   `--no-cpp-direct-copy` : Don't generate direct copy methods for C++
    object-based API.

-   `--cpp-std CPP_STD` : Generate a C++ code using features of selected C++ standard.
     Supported `CPP_STD` values:
    * `c++0x` - generate code compatible with old compilers (VS2010),
    * `c++11` - use C++11 code generator (default),
    * `c++17` - use C++17 features in generated code (experimental).

-   `--object-prefix` : Customise class prefix for C++ object-based API.

-   `--object-suffix` : Customise class suffix for C++ object-based API.

-   `--go-namespace` : Generate the overrided namespace in Golang.

-   `--go-import` : Generate the overrided import for flatbuffers in Golang.
     (default is "github.com/google/flatbuffers/go").

-   `--raw-binary` : Allow binaries without a file_indentifier to be read.
    This may crash flatc given a mismatched schema.

-   `--size-prefixed` : Input binaries are size prefixed buffers.

-   `--proto`: Expect input files to be .proto files (protocol buffers).
    Output the corresponding .fbs file.
    Currently supports: `package`, `message`, `enum`, nested declarations,
    `import` (use `-I` for paths), `extend`, `oneof`, `group`.
    Does not support, but will skip without error: `option`, `service`,
    `extensions`, and most everything else.

-   `--oneof-union` : Translate .proto oneofs to flatbuffer unions.

-   `--grpc` : Generate GRPC interfaces for the specified languages.

-   `--schema`: Serialize schemas instead of JSON (use with -b). This will
    output a binary version of the specified schema that itself corresponds
    to the reflection/reflection.fbs schema. Loading this binary file is the
    basis for reflection functionality.

-   `--bfbs-comments`: Add doc comments to the binary schema files.

-   `--conform FILE` : Specify a schema the following schemas should be
    an evolution of. Gives errors if not. Useful to check if schema
    modifications don't break schema evolution rules.

-   `--conform-includes PATH` : Include path for the schema given with
    `--conform PATH`.

-   `--filename-suffix SUFFIX` : The suffix appended to the generated
    file names. Default is '\_generated'.

-   `--filename-ext EXTENSION` : The extension appended to the generated
    file names. Default is language-specific (e.g. "h" for C++). This
    should not be used when multiple languages are specified.

-   `--include-prefix PATH` : Prefix this path to any generated include
    statements.

-   `--keep-prefix` : Keep original prefix of schema include statement.

-   `--reflect-types` : Add minimal type reflection to code generation.

-   `--reflect-names` : Add minimal type/name reflection.

-   `--root-type T` : Select or override the default root_type.

-   `--require-explicit-ids` : When parsing schemas, require explicit ids (id: x).

-   `--force-defaults` : Emit default values in binary output from JSON.

-   `--force-empty` : When serializing from object API representation, force
     strings and vectors to empty rather than null.

-   `--force-empty-vectors` : When serializing from object API representation, force
     vectors to empty rather than null.

-   `--flexbuffers` : Used with "binary" and "json" options, it generates
     data using schema-less FlexBuffers.

-   `--no-warnings` : Inhibit all warning messages.

-   `--cs-global-alias` : Prepend `global::` to all user generated csharp classes and structs.

-   `--json-nested-bytes` : Allow a nested_flatbuffer field to be parsed as a
    vector of bytes in JSON, which is unsafe unless checked by a verifier
    afterwards.

-   `--python-no-type-prefix-suffix` : Skip emission of Python functions that are prefixed
    with typenames

-   `--python-typing` : Generate Python type annotations

Additional gRPC options:

-   `--grpc-filename-suffix`: `[C++]` An optional suffix for the generated
    files' names. For example, compiling gRPC for C++ with
    `--grpc-filename-suffix=.fbs` will generate `{name}.fbs.h` and
    `{name}.fbs.cc` files.

-   `--grpc-additional-header`: `[C++]` Additional headers to include in the
    generated files.

-   `--grpc-search-path`: `[C++]` An optional prefix for the gRPC runtime path.
    For example, compiling gRPC for C++ with `--grpc-search-path=some/path` will
    generate the following includes:

    ```cpp
      #include "some/path/grpcpp/impl/codegen/async_stream.h"
      #include "some/path/grpcpp/impl/codegen/async_unary_call.h"
      #include "some/path/grpcpp/impl/codegen/method_handler.h"
      ...
    ```

-   `--grpc-use-system-headers`: `[C++]` Whether to generate `#include <header>`
    instead of `#include "header.h"` for all headers when compiling gRPC for
    C++. For example, compiling gRPC for C++ with `--grpc-use-system-headers`
    will generate the following includes:

    ```cpp
      #include <some/path/grpcpp/impl/codegen/async_stream.h>
      #include <some/path/grpcpp/impl/codegen/async_unary_call.h>
      #include <some/path/grpcpp/impl/codegen/method_handler.h>
      ...
    ```

    NOTE: This option can be negated with `--no-grpc-use-system-headers`.

-   `--grpc-python-typed-handlers`: `[Python]` Whether to generate the typed
    handlers that use the generated Python classes instead of raw bytes for
    requests/responses.

NOTE: short-form options for generators are deprecated, use the long form
whenever possible.

