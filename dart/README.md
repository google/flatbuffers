# FlatBuffers for Dart

This package is used to read and write [FlatBuffers](https://google.github.io/flatbuffers/).

Most consumers will want to use the [`flatc` - FlatBuffer compiler](https://github.com/google/flatbuffers) binary for your platform.
You can find it in the `generator/{Platform}` directory of the [released package archive](https://pub.dev/packages/flat_buffers/versions/2.0.5.tar.gz).

The FlatBuffer compiler `flatc` reads a FlatBuffers IDL schema and generates Dart code.
The generated classes can be used to read or write binary data/files that are interoperable with
other languages and platforms supported by FlatBuffers, as illustrated in the `example.dart` in the
examples folder.

For more details and documentation, head over to the official site and read the
[Tutorial](https://google.github.io/flatbuffers/flatbuffers_guide_tutorial.html) and how to
[use FlatBuffers in Dart](https://google.github.io/flatbuffers/flatbuffers_guide_use_dart.html).

## Dart 2.0 notes
Version 2.0.5 ships with it's own custom build of `flatc` because this is an extraordinary release to catch-up
with FlatBuffers for other platforms. This generator can only generate dart code (to avoid generating code for other platforms which isn't released yet).
On the other hand, the generated code still produces standard binary FlatBuffers compatible with other languages.
In other words: only `flatc --dart ...` works with this generator, but your app will be able to produce and read standard binary (`Uint8List`) FlatBuffers that are fully compotible with other languages supporting FlatBuffers (e.g. Java, C++, ...).

In the future a common `flatc` binary for all platforms would be shipped through GitHub release page instead.
