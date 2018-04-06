Use in Dart    {#flatbuffers_guide_use_dart}
===========

## Before you get started

Before diving into the FlatBuffers usage in Dart, it should be noted that
the [Tutorial](@ref flatbuffers_guide_tutorial) page has a complete guide
to general FlatBuffers usage in all of the supported languages (including Dart).
This page is designed to cover the nuances of FlatBuffers usage, specific to
Dart.

You should also have read the [Building](@ref flatbuffers_guide_building)
documentation to build `flatc` and should be familiar with
[Using the schema compiler](@ref flatbuffers_guide_using_schema_compiler) and
[Writing a schema](@ref flatbuffers_guide_writing_schema).

## FlatBuffers Dart library code location

The code for the FlatBuffers Go library can be found at
`flatbuffers/dart`. You can browse the library code on the [FlatBuffers
GitHub page](https://github.com/google/flatbuffers/tree/master/dart).

## Testing the FlatBuffers Dart library

The code to test the Dart library can be found at `flatbuffers/tests`.
The test code itself is located in [dart_test.dart](https://github.com/google/
flatbuffers/blob/master/tests/dart_test.dart).

To run the tests, use the [DartTest.sh](https://github.com/google/flatbuffers/
blob/master/tests/DartTest.sh) shell script.

*Note: The shell script requires the [Dart SD](https://www.dartlang.org/tools/sdk)
to be installed.*

## Using the FlatBuffers Dart library

*Note: See [Tutorial](@ref flatbuffers_guide_tutorial) for a more in-depth
example of how to use FlatBuffers in Dart.*

FlatBuffers supports reading and writing binary FlatBuffers in Dart.

To use FlatBuffers in your own code, first generate Dart classes from your
schema with the `--dart` option to `flatc`. Then you can include both FlatBuffers
and the generated code to read or write a FlatBuffer.

For example, here is how you would read a FlatBuffer binary file in Dart: First,
include the library and generated code. Then read a FlatBuffer binary file into
a `List<int>`, which you pass to the factory constructor for `Monster`:

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~{.dart}
import 'dart:io' as io;

import 'package:flat_buffers/flat_buffers.dart' as fb;
import './monster_my_game.sample_generated.dart' as myGame;

List<int> data = await new io.File('monster.dat').readAsBytes();
var monster = new myGame.Monster(data);
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Now you can access values like this:

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~{.dart}
var hp = monster.hp;
var pos = monster.pos;
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

## Text Parsing

There currently is no support for parsing text (Schema's and JSON) directly
from Dart, though you could use the C++ parser through Dart Native Extensions.
Please see the C++ documentation for more on text parsing (note that this is
not currently an option in Flutter - follow [this issue](https://github.com/flutter/flutter/issues/7053)
for the latest).

<br>
