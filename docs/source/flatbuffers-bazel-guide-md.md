# FlatBuffers Bazel Integration Guide

This guide will walk you through the process of integrating FlatBuffers Bazel rules into your project. We'll create a simple example project and provide step-by-step instructions for novice Bazel users.

## Prerequisites

- Bazel installed on your system
- Basic understanding of FlatBuffers
- FlatBuffers repository cloned or added as a dependency

## Example Project Structure

```
my_project/
├── WORKSPACE
├── BUILD
└── src/
    ├── BUILD
    └── monster.fbs
```

## Step-by-Step Instructions

### 1. Set up your WORKSPACE file

In your project's root directory, create or modify the `WORKSPACE` file:

```python
load("@bazel_tools//tools/build_defs/repo:http.bzl", "http_archive")

http_archive(
    name = "com_github_google_flatbuffers",
    strip_prefix = "flatbuffers-23.5.26",
    urls = ["https://github.com/google/flatbuffers/archive/v23.5.26.tar.gz"],
)
```

Replace the version number with the latest FlatBuffers release.

### 2. Create your FlatBuffers schema

In `src/monster.fbs`:

```fbs
namespace MyGame;

table Monster {
  name:string;
  hp:short = 100;
  mana:short = 150;
}

root_type Monster;
```

### 3. Set up your BUILD files

In the root `BUILD` file:

```python
package(default_visibility = ["//visibility:public"])
```

In `src/BUILD`:

```python
load("@com_github_google_flatbuffers//:build_defs.bzl", "flatbuffer_library_public")

flatbuffer_library_public(
    name = "monster_fbs",
    srcs = ["monster.fbs"],
    language_flag = "--cpp",
)

cc_binary(
    name = "monster_example",
    srcs = ["monster_example.cc"],
    deps = [":monster_fbs"],
)
```

### 4. Use the generated code

Create `src/monster_example.cc`:

```cpp
#include "src/monster_generated.h"
#include <iostream>

int main() {
    flatbuffers::FlatBufferBuilder builder(1024);
    auto name = builder.CreateString("Orc");
    auto monster = MyGame::CreateMonster(builder, name, 80, 100);
    builder.Finish(monster);

    auto monster_buffer = builder.GetBufferPointer();
    auto monster_size = builder.GetSize();

    auto verified_monster = flatbuffers::GetRoot<MyGame::Monster>(monster_buffer);
    std::cout << "Monster name: " << verified_monster->name()->str() << std::endl;
    std::cout << "Monster HP: " << verified_monster->hp() << std::endl;
    std::cout << "Monster Mana: " << verified_monster->mana() << std::endl;

    return 0;
}
```

### 5. Build and run

From your project root, run:

```bash
bazel build //src:monster_example
bazel run //src:monster_example
```

## Common Use Cases

1. **Multiple schemas**: For projects with multiple `.fbs` files, create separate `flatbuffer_library_public` targets for each schema.

2. **Cross-language support**: Use different language flags in the `flatbuffer_library_public` rule, e.g., `--python` for Python, `--java` for Java, etc.

3. **Custom include paths**: Use the `include_paths` attribute in the `flatbuffer_library_public` rule to specify additional include directories.

## Potential Pitfalls

1. **Version mismatches**: Ensure the FlatBuffers version in your `WORKSPACE` file matches the version you're using elsewhere in your project.

2. **Missing dependencies**: If you're using FlatBuffers with other libraries, make sure to declare all necessary dependencies in your `BUILD` files.

3. **Namespace conflicts**: Be careful with namespace naming to avoid conflicts, especially in large projects.

4. **Language-specific setup**: Different languages might require additional setup. Refer to the FlatBuffers documentation for language-specific details.

## Advanced Usage

For more advanced usage, including custom build flags and multi-language support, refer to the `build_defs.bzl` file in the FlatBuffers repository. This file contains the definitions for the Bazel rules and provides more options for customization.

Remember to regenerate your FlatBuffers code whenever you modify your `.fbs` schema files by rebuilding your targets.
