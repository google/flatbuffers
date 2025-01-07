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



