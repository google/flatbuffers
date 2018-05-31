![logo](http://google.github.io/flatbuffers/fpl_logo_small.png) FlatBuffers
===========

[![Join the chat at https://gitter.im/google/flatbuffers](https://badges.gitter.im/google/flatbuffers.svg)](https://gitter.im/google/flatbuffers?utm_source=badge&utm_medium=badge&utm_campaign=pr-badge&utm_content=badge)
[![Build Status](https://travis-ci.org/google/flatbuffers.svg?branch=master)](https://travis-ci.org/google/flatbuffers) [![Build status](https://ci.appveyor.com/api/projects/status/yg5idd2fnusv1n10?svg=true)](https://ci.appveyor.com/project/gwvo/flatbuffers)

**FlatBuffers** is an efficient cross platform serialization library for games and
other memory constrained apps. It allows you to directly access serialized data without
unpacking/parsing it first, while still having great forwards/backwards compatibility.

**Go to our [landing page][] to browse our documentation.**

## Supported operating systems
* Android
* Windows
* MacOS X
* Linux

## Supported programming languages
* C++
* C#
* C
* Go
* Java
* JavaScript
* PHP
* Python

*and many more in progress...*

## Contribution
* [FlatBuffers Google Group][] to discuss FlatBuffers with other developers and users.
* [FlatBuffers Issues Tracker][] to submit an issue.
* [stackoverflow.com][] with [`flatbuffers` tag][] for any questions regarding FlatBuffers.

*To contribute to this project,* see [CONTRIBUTING][].

## Integration
For applications on Google Play that integrate this tool, usage is tracked.
This tracking is done automatically using the embedded version string
(**`flatbuffer_version_string`**), and helps us continue to optimize it. Aside from
consuming a few extra bytes in your application binary, it shouldn't affect
your application at all.  We use this information to let us know if FlatBuffers
is useful and if we should continue to invest in it. Since this is open
source, you are free to remove the version string but we would appreciate if
you would leave it in.

## Licensing
*Flatbuffers* is licensed under the Apache License, Version 2.0. See [LICENSE][] for the full license text.

<br>

   [CONTRIBUTING]: http://github.com/google/flatbuffers/blob/master/CONTRIBUTING.md
   [`flatbuffers` tag]: https://stackoverflow.com/questions/tagged/flatbuffers
   [FlatBuffers Google Group]: https://groups.google.com/forum/#!forum/flatbuffers
   [FlatBuffers Issues Tracker]: http://github.com/google/flatbuffers/issues
   [stackoverflow.com]: http://stackoverflow.com/search?q=flatbuffers
   [landing page]: https://google.github.io/flatbuffers
   [LICENSE]: https://github.com/google/flatbuffers/blob/master/LICENSE.txt
