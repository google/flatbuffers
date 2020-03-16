Use in C#    {#flatbuffers_guide_use_c-sharp}
==============

## Before you get started

Before diving into the FlatBuffers usage in C#, it should be noted that
the [Tutorial](@ref flatbuffers_guide_tutorial) page has a complete guide to
general FlatBuffers usage in all of the supported languages (including C#).
This page is designed to cover the nuances of FlatBuffers usage,
specific to C#.

You should also have read the [Building](@ref flatbuffers_guide_building)
documentation to build `flatc` and should be familiar with
[Using the schema compiler](@ref flatbuffers_guide_using_schema_compiler) and
[Writing a schema](@ref flatbuffers_guide_writing_schema).

## FlatBuffers C-sharp code location

The code for the FlatBuffers C# library can be found at
`flatbuffers/net/FlatBuffers`. You can browse the library on the
[FlatBuffers GitHub page](https://github.com/google/flatbuffers/tree/master/net/
FlatBuffers).

## Testing the FlatBuffers C-sharp libraries

The code to test the libraries can be found at `flatbuffers/tests`.

The test code for C# is located in the [FlatBuffers.Test](https://github.com/
google/flatbuffers/tree/master/tests/FlatBuffers.Test) subfolder. To run the
tests, open `FlatBuffers.Test.csproj` in [Visual Studio](
https://www.visualstudio.com), and compile/run the project.

Optionally, you can run this using [Mono](http://www.mono-project.com/) instead.
Once you have installed `Mono`, you can run the tests from the command line
by running the following commands from inside the `FlatBuffers.Test` folder:

~~~{.sh}
  mcs *.cs ../MyGame/Example/*.cs ../../net/FlatBuffers/*.cs
  mono Assert.exe
~~~

## Using the FlatBuffers C# library

*Note: See [Tutorial](@ref flatbuffers_guide_tutorial) for a more in-depth
example of how to use FlatBuffers in C#.*

FlatBuffers supports reading and writing binary FlatBuffers in C#.

To use FlatBuffers in your own code, first generate C# classes from your
schema with the `--csharp` option to `flatc`.
Then you can include both FlatBuffers and the generated code to read
or write a FlatBuffer.

For example, here is how you would read a FlatBuffer binary file in C#:
First, import the library and generated code. Then, you read a FlatBuffer binary
file into a `byte[]`.  You then turn the `byte[]` into a `ByteBuffer`, which you
pass to the `GetRootAsMyRootType` function:

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~{.cs}
    using MyGame.Example;
    using FlatBuffers;

    // This snippet ignores exceptions for brevity.
    byte[] data = File.ReadAllBytes("monsterdata_test.mon");

    ByteBuffer bb = new ByteBuffer(data);
    Monster monster = Monster.GetRootAsMonster(bb);
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Now you can access the data from the `Monster monster`:

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~{.cs}
    short hp = monster.Hp;
    Vec3 pos = monster.Pos;
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

C# code naming follows standard C# style with `PascalCasing` identifiers,
e.g. `GetRootAsMyRootType`. Also, values (except vectors and unions) are
available as properties instead of parameterless accessor methods.
The performance-enhancing methods to which you can pass an already created
object are prefixed with `Get`, e.g.:

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~{.cs}
    // property
    var pos = monster.Pos;

    // method filling a preconstructed object
    var preconstructedPos = new Vec3();
    monster.GetPos(preconstructedPos);
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

## Storing dictionaries in a FlatBuffer

FlatBuffers doesn't support dictionaries natively, but there is support to
emulate their behavior with vectors and binary search, which means you
can have fast lookups directly from a FlatBuffer without having to unpack
your data into a `Dictionary` or similar.

To use it:
-   Designate one of the fields in a table as the "key" field. You do this
    by setting the `key` attribute on this field, e.g.
    `name:string (key)`.
    You may only have one key field, and it must be of string or scalar type.
-   Write out tables of this type as usual, collect their offsets in an
    array.
-   Instead of calling standard generated method,
    e.g.: `Monster.createTestarrayoftablesVector`,
    call `CreateSortedVectorOfMonster` in C#
    which will first sort all offsets such that the tables they refer to
    are sorted by the key field, then serialize it.
-   Now when you're accessing the FlatBuffer, you can use
    the `ByKey` accessor to access elements of the vector, e.g.:
    `monster.TestarrayoftablesByKey("Frodo")` in C#,
    which returns an object of the corresponding table type,
    or `null` if not found.
    `ByKey` performs a binary search, so should have a similar
    speed to `Dictionary`, though may be faster because of better caching.
    `ByKey` only works if the vector has been sorted, it will
    likely not find elements if it hasn't been sorted.

## Text parsing

There currently is no support for parsing text (Schema's and JSON) directly
from C#, though you could use the C++ parser through native call
interfaces available to each language. Please see the
C++ documentation for more on text parsing.

## Object based API

FlatBuffers is all about memory efficiency, which is why its base API is written
around using as little as possible of it. This does make the API clumsier
(requiring pre-order construction of all data, and making mutation harder).

For times when efficiency is less important a more convenient object based API
can be used (through `--gen-object-api`) that is able to unpack & pack a
FlatBuffer into objects and standard System.Collections.Generic containers, allowing for convenient
construction, access and mutation.

To use:

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~{.cs}
    // Deserialize from buffer into object.
    MonsterT monsterobj = GetMonster(flatbuffer).UnPack();

    // Update object directly like a C# class instance.
    Console.WriteLine(monsterobj.Name);
    monsterobj.Name = "Bob";  // Change the name.

    // Serialize into new flatbuffer.
    FlatBufferBuilder fbb = new FlatBufferBuilder(1);
    fbb.Finish(Monster.Pack(fbb, monsterobj).Value);
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

### Json Serialization

An additional feature of the object API is the ability to allow you to
serialize & deserialize a JSON text. 
To use Json Serialization, add `--gen-json-serializer` option to `flatc` and
add `Newtonsoft.Json` nuget package to csproj.

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~{.cs}
    // Deserialize MonsterT from json
    string jsonText = File.ReadAllText(@"Resources/monsterdata_test.json");
    MonsterT mon = MonsterT.DeserializeFromJson(jsonText);

    // Serialize MonsterT to json
    string jsonText2 = mon.SerializeToJson();
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

* Limitation
  * `hash` attribute currentry not supported.
* NuGet package Dependency
  * [Newtonsoft.Json](https://github.com/JamesNK/Newtonsoft.Json)

<br>
