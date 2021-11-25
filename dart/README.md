# FlatBuffers for Dart

This package is used to read and write [FlatBuffers](https://google.github.io/flatbuffers/).

Most consumers will want to use the [`flatc` - FlatBuffer compiler](https://github.com/google/flatbuffers) binary for your platform:
* [Linux](https://github.com/google/flatbuffers/suites/4363603985/artifacts/114682272)
* [macOS](https://github.com/google/flatbuffers/suites/4363603985/artifacts/114682273)
* [Windows](https://github.com/google/flatbuffers/suites/4363603985/artifacts/114682274)

The FlatBuffer compiler `flatc` reads a FlatBuffers IDL schema and generates Dart code.
The generated classes can be used to read or write binary data/files that are interoperable with
other languages and platforms supported by FlatBuffers, as illustrated in the `example.dart` in the
examples folder.

For more details and documentation, head over to the official site and read the
[Tutorial](https://google.github.io/flatbuffers/flatbuffers_guide_tutorial.html) and how to
[use FlatBuffers in Dart](https://google.github.io/flatbuffers/flatbuffers_guide_use_dart.html).