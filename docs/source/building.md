# Building

## Building with CMake

The distribution main build system is configured by
[`cmake`](https://www.cmake.org) which allows you to build the project for any
platform.

### Configuration

Use `cmake` to configure a project based on your environment and platform.

=== "Unix"

    ```sh
    cmake -G "Unix Makefiles" -DCMAKE_BUILD_TYPE=Release
    ```

    !!! note

        To use `clang` instead of `gcc` you may need to set prepend some
        environment variables e.g. `CC=/usr/bin/clang CXX=/usr/bin/clang++ cmake
        -G "Unix MakeFiles"`

=== "Windows"

    ```sh
    cmake -G "Visual Studio 17 2022" -DCMAKE_BUILD_TYPE=Release
    ```

=== "MacOS"

    ```sh
    cmake -G "Xcode" -DCMAKE_BUILD_TYPE=Release
    ```

#### Strict Mode

By default, `cmake` will configure targets to **not** build with strict warnings
on (e.g. `-Werror` or `/WX`). This may cause into issues when submitting code
changes since our CI requires the code to compile in strict mode.

To enable the extra warnings, turn on strict mode with the
`FLATBUFFERS_STRICT_MODE` cmake option.

```cmake
cmake -DFLATBUFFERS_STRICT_MODE=ON
```


### Building

Once the project files are generated, build as normal for your platform.

=== "Unix"

    ```sh
    make -j
    ```

=== "Windows"

    ```sh
    msbuild.exe FlatBuffers.sln
    ```

=== "MacOS"

    ```sh
    xcodebuild -toolchain clang -configuration Release
    ```






## Building with Bazel

## Building with VCPKG
