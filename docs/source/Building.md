# Building

There are project files for Visual Studio and Xcode that should allow you
to build the compiler `flatc`, the samples and the tests out of the box.

Alternatively, the distribution comes with a `cmake` file that should allow
you to build project/make files for any platform. For details on `cmake`, see
<http://www.cmake.org>. In brief, depending on your platform, use one of
e.g.:

    cmake -G "Unix Makefiles"
    cmake -G "Visual Studio 10"
    cmake -G "Xcode"

Then, build as normal for your platform. This should result in a `flatc`
executable, essential for the next steps.
Note that to use clang instead of gcc, you may need to set up your environment
variables, e.g.
`CC=/usr/bin/clang CXX=/usr/bin/clang++ cmake -G "Unix Makefiles"`.

Optionally, run the `flattests` executable.
to ensure everything is working correctly on your system. If this fails,
please contact us!

Building should also produce two sample executables, `sample_binary` and
`sample_text`, see the corresponding `.cpp` file in the samples directory.

There is an `android` directory that contains all you need to build the test
executable on android (use the included `build_apk.sh` script, or use
`ndk_build` / `adb` etc. as usual). Upon running, it will output to the log
if tests succeeded or not.

There is usually no runtime to compile, as the code consists of a single
header, `include/flatbuffers/flatbuffers.h`. You should add the
`include` folder to your include paths. If you wish to be
able to load schemas and/or parse text into binary buffers at runtime,
you additionally need the other headers in `include/flatbuffers`. You must
also compile/link `src/idl_parser.cpp` (and `src/idl_gen_text.cpp` if you
also want to be able convert binary to text).

For applications on Google Play that integrate this library, usage is tracked.
This tracking is done automatically using the embedded version string
(flatbuffer_version_string), and helps us continue to optimize it.
Aside from consuming a few extra bytes in your application binary, it shouldn't
affect your application at all. We use this information to let us know if
FlatBuffers is useful and if we should continue to invest in it. Since this is
open source, you are free to remove the version string but we would appreciate
if you would leave it in.
