Building    {#flatbuffers_guide_building}
========

## Building with CMake

The distribution comes with a `cmake` file that should allow
you to build project/make files for any platform. For details on `cmake`, see
<https://www.cmake.org>. In brief, depending on your platform, use one of
e.g.:

    cmake -G "Unix Makefiles" -DCMAKE_BUILD_TYPE=Release
    cmake -G "Visual Studio 10" -DCMAKE_BUILD_TYPE=Release
    cmake -G "Xcode" -DCMAKE_BUILD_TYPE=Release

Then, build as normal for your platform. This should result in a `flatc`
executable, essential for the next steps.
Note that to use clang instead of gcc, you may need to set up your environment
variables, e.g.
`CC=/usr/bin/clang CXX=/usr/bin/clang++ cmake -G "Unix Makefiles"`.

Optionally, run the `flattests` executable from the root `flatbuffers/`
directory to ensure everything is working correctly on your system. If this
fails, please contact us!

Building should also produce two sample executables, `flatsamplebinary` and
`flatsampletext`, see the corresponding `.cpp` files in the
`flatbuffers/samples` directory.

*Note that you MUST be in the root of the FlatBuffers distribution when you
run 'flattests' or `flatsampletext`, or it will fail to load its files.*

## Building for Android

There is a `flatbuffers/android` directory that contains all you need to build
the test executable on android (use the included `build_apk.sh` script, or use
`ndk_build` / `adb` etc. as usual). Upon running, it will output to the log
if tests succeeded or not.

You may also run an android sample from inside the `flatbuffers/samples`, by
running the `android_sample.sh` script. Optionally, you may go to the
`flatbuffers/samples/android` folder and build the sample with the
`build_apk.sh` script or `ndk_build` / `adb` etc.

## Using FlatBuffers in your own projects

For C++, there is usually no runtime to compile, as the code consists of a
single header, `include/flatbuffers/flatbuffers.h`. You should add the
`include` folder to your include paths. If you wish to be
able to load schemas and/or parse text into binary buffers at runtime,
you additionally need the other headers in `include/flatbuffers`. You must
also compile/link `src/idl_parser.cpp` (and `src/idl_gen_text.cpp` if you
also want to be able convert binary to text).

To see how to include FlatBuffers in any of our supported languages, please
view the [Tutorial](@ref flatbuffers_guide_tutorial) and select your appropriate
language using the radio buttons.

### Using in CMake-based projects
If you want to use FlatBuffers in a project which already uses CMake then a more
robust and flexible approach is to build FlatBuffers as part of that project directly.
This is done by making the FlatBuffers source code available to the main build
and adding it using CMake's `add_subdirectory()` command. This has the
significant advantage that the same compiler and linker settings are used
between FlatBuffers and the rest of your project, so issues associated with using
incompatible libraries (eg debug/release), etc. are avoided. This is
particularly useful on Windows.

Suppose you put FlatBuffers source code in the directory `${FLATBUFFERS_SRC_DIR}`.
To build it as part of your project, add the following code to your
`CMakeLists.txt` file:
```cmake
# Add FlatBuffers directly to our build. This defines the `flatbuffers` target.
add_subdirectory(${FLATBUFFERS_SRC_DIR}
                 ${CMAKE_CURRENT_BINARY_DIR}/flatbuffers-build
                 EXCLUDE_FROM_ALL)

# Now simply link against flatbuffers as needed to your already declared target.
# The flatbuffers target carries header search path information automatically
# when using CMake version > 2.8.11.
target_link_libraries(your_project_target PRIVATE flatbuffers)
```
When building your project, the `flatbuffers` library will then be compiled and linked 
to the target as part of your project.

#### Automatically Running `flatc`

In addition to the above you will also likely want to add custom CMake targets
to automatically compile your `*.fbs` files when you build your project.  This
has the advantage that you will not need to manually run the `flatc` compiler
(or risk forgetting to do so) when you update your Flatbuffers schema files.

If you've followed the recipe in this section to incorporate Flatbuffers into
your project's build (recommended) then there will also be a new target called
`flatc` visible to you; this is the target representing the `flatc` executable
(compiler) that you will need to run to compile the schemas.  Ideally, whenever
you build your project, this compiler will be run (and built first if necessary)
to compile any `*.fbs` files that have changed since the last build.

To set this up you will essentially need to add a series of CMake Custom
Commands to your build and then make your target depend on the files generated
by that target.  There are two examples of this in the Flatbuffers package:

1. There is a function provided by the Flatbuffers package called `build_flatbuffers`
in the file `${FLATBUFFERS_SRC_DIR}/CMake/BuildFlatBuffers.cmake` that does exactly this.
See the comments at the top of that file for details.  Essentially, you call the function
with a list of schema files and it will create all of the custom commands for you.
2. There is also an example of directly adding such custom commands in Flatbuffers'
own `CMakeLists.txt` file at the root of the source tree (see the function called
`compile_flatbuffers_schema_to_cpp`).

Since each project has slightly different build requirements, you may want to
take those two examples and adapt them for your own project's needs, adding
or removing as necessary.  The first option, if it satifies your project's needs,
will likely be simpler; the second option will be more flexible.

The following is an example of a simple starting point for the second approach:

1. Create a folder called `flatbuffers` somewhere in your project's source tree.
2. Place all of your `*.fbs` files in there, along with a `CMakeLists.txt` file.
3. In the parent folder, add the `flatbuffers` folder via `add_subdirectory`.
4. In the `flatbuffers/CMakeLists.txt` file put something like this:

```cmake
file( GLOB fbs "*.fbs" )

# Agglomeration of generated include files (*_generated.h).
set( generated_files "" )

foreach( fb ${fbs} )
  get_filename_component( stem ${fb} NAME_WE )
  get_filename_component( name ${fb} NAME )
  set( generated_include ${CMAKE_CURRENT_BINARY_DIR}/${stem}_generated.h )
  add_custom_command(
    OUTPUT ${generated_include}
    COMMENT "Compiling flatbuffer for ${name}"
    COMMAND flatc # This will be a target recognized by CMake.
    --cpp
    --scoped-enums          # (optional)
    --reflect-types         # (optional)
    --reflect-names         # (optional)
    -o ${CMAKE_CURRENT_BINARY_DIR}
    -I ${CMAKE_CURRENT_SOURCE_DIR}
    ${fb}
    DEPENDS flatc ${fb}
    WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
  )
  list( APPEND generated_files ${generated_include} )
endforeach()

# Create a custom target that depends on all the generated files.
# Depending on this will trigger all schemas to be compiled.
add_custom_target( your_flatbuf_target DEPENDS ${generated_files} )

set_property(
  TARGET your_flatbuf_target
  PROPERTY INCLUDE_DIR
  ${CMAKE_CURRENT_BINARY_DIR}
)
```
Then, in the parent folder's `CMakeLists.txt` file you can add a `flatbuffers`
dependency as described at the start of this section, as well as:
```cmake
add_dependencies( your_project_target your_flatbuf_target )
get_target_property( FLATBUFF_INCLUDE_DIR your_flatbuf_target INCLUDE_DIR )

target_include_directories(
    your_project_target
    PRIVATE
    ...
    ${FLATBUFF_INCLUDE_DIR}
    ...)
```
These will trigger the schema generation and allow your source files to
include the generated files.  Finally, as mentioned above, the `build_flatbuffers`
function in the `CMake/BuildFlatBuffers.cmake` file can also set this up
for you; see the comments at the top of that file for an example.  Many users,
however, will prefer to add the Custom Commands directly since it is ultimately
more flexible.

However, this is an important shortcoming of the above methodology: specifically,
the custom targets are not aware of dependencies among schema files via include
statements.  In other words, if schema file `A.fbs` includes `B.fbs` then the
correct rebuilding process is to recompile both files when `B.fbs` changes.  The
above method will not do this, and thus can lead to incorrect rebuilding.  One
solution would be to hardcode the dependencies in the CMake file `DEPENDS` part
of the custom command, though this is not ideal for reasons of maintenance.

#### Override default depth limit of nested objects
To override [the depth limit of recursion](@ref flatbuffers_guide_use_cpp), 
add this directive:
```cmake
set(FLATBUFFERS_MAX_PARSING_DEPTH 16)
```
to `CMakeLists.txt` file before `add_subdirectory(${FLATBUFFERS_SRC_DIR})` line.

#### For Google Play apps

For applications on Google Play that integrate this library, usage is tracked.
This tracking is done automatically using the embedded version string
(flatbuffer_version_string), and helps us continue to optimize it.
Aside from consuming a few extra bytes in your application binary, it shouldn't
affect your application at all. We use this information to let us know if
FlatBuffers is useful and if we should continue to invest in it. Since this is
open source, you are free to remove the version string but we would appreciate
if you would leave it in.
