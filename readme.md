FlatBuffers in .NET Version 0.1 (alpha)

This project is a .NET (C#) port of FlatBuffers by Google Inc. 

With this project you can read and write FlatBuffers from .NET applications.

See the original project at [FlatBuffers GitHub Page][] for more information about FlatBuffers.

The port is close to a straight port of the Java version of FlatBuffers, and as such the 
generated code will look and behave mostly the same.

The **flatc** code generator has been adapted to generate C# code, using the -n
command line switch, which I've added specifically for the .NET version.

To see how to use FlatBuffers for .NET, I've included a simple port of the JavaTest code as a couple
of MSVC Unit Tests.

Future intentions for this project will be to build a simple Protobuf-net style serialization
library on top, to allow serialization of POCOs to FlatBuffer format.



Original FlatBuffers readme text is as follows:

# Welcome to FlatBuffers!

FlatBuffers is a serialization library for games and other memory constrained
apps. Go to our [landing page][] to browse our documentation.

FlatBuffers allows you to directly access serialized data without
unpacking/parsing it first, while still having great forwards/backwards
compatibility. FlatBuffers can be built for many different systems (Android,
Windows, OS X, Linux), see `docs/html/index.html`

Discuss FlatBuffers with other developers and users on the
[FlatBuffers Google Group][]. File issues on the [FlatBuffers Issues Tracker][]
or post your questions to [stackoverflow.com][] with a mention of
**flatbuffers**.

For applications on Google Play that integrate this tool, usage is tracked.
This tracking is done automatically using the embedded version string
(flatbuffer_version_string), and helps us continue to optimize it. Aside from
consuming a few extra bytes in your application binary, it shouldn't affect
your application at all.  We use this information to let us know if FlatBuffers
is useful and if we should continue to invest in it. Since this is open
source, you are free to remove the version string but we would appreciate if
you would leave it in.

  [FlatBuffers Google Group]: http://group.google.com/group/flatbuffers
  [FlatBuffers Issues Tracker]: http://github.com/google/flatbuffers/issues
  [stackoverflow.com]: http://www.stackoverflow.com
  [landing page]: http://google.github.io/flatbuffers
  [FlatBuffers GitHub Page]: https://github.com/google/flatbuffers