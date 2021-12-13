#!/usr/bin/env python3
#
# Copyright 2021 Google Inc. All rights reserved.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

import argparse
import filecmp
import glob
import platform
import shutil
import subprocess
import sys
from pathlib import Path

parser = argparse.ArgumentParser()
parser.add_argument(
    "--flatc",
    help="path of the Flat C compiler relative to the root directory",
)
parser.add_argument("--cpp-0x", action="store_true", help="use --cpp-std c++ox")
parser.add_argument(
    "--skip-monster-extra",
    action="store_true",
    help="skip generating tests involving monster_extra.fbs",
)
args = parser.parse_args()

# Get the path where this script is located so we can invoke the script from
# any directory and have the paths work correctly.
script_path = Path(__file__).parent.resolve()

# Get the root path as an absolute path, so all derived paths are absolute.
root_path = script_path.parent.absolute()

# Get the location of the flatc executable, reading from the first command line
# argument or defaulting to default names.
flatc_exe = Path(
    ("flatc" if not platform.system() == "Windows" else "flatc.exe")
    if not args.flatc
    else args.flatc
)

# Find and assert flatc compiler is present.
if root_path in flatc_exe.parents:
    flatc_exe = flatc_exe.relative_to(root_path)
flatc_path = Path(root_path, flatc_exe)
assert flatc_path.exists(), "Cannot find the flatc compiler " + str(flatc_path)

# Specify the other paths that will be referenced
tests_path = Path(root_path, "tests")
samples_path = Path(root_path, "samples")
reflection_path = Path(root_path, "reflection")

# Execute the flatc compiler with the specified parameters
def flatc(
    options, schema, prefix=None, include=None, data=None, cwd=tests_path
):
    cmd = [str(flatc_path)] + options
    if prefix:
        cmd += ["-o"] + [prefix]
    if include:
        cmd += ["-I"] + [include]
    cmd += [schema] if isinstance(schema, str) else schema
    if data:
        cmd += [data] if isinstance(data, str) else data
    result = subprocess.run(cmd, cwd=cwd, check=True)


# Glob a pattern relative to file path
def glob(path, pattern):
    return [str(p) for p in path.glob(pattern)]


# flatc options that are shared
BASE_OPTS = ["--reflect-names", "--gen-mutable", "--gen-object-api"]
NO_INCL_OPTS = BASE_OPTS + ["--no-includes"]

# Language specific options
CS_OPTS = ["--csharp", "--cs-gen-json-serializer"]
CPP_OPTS = [
    "--cpp",
    "--gen-compare",
    "--cpp-ptr-type",
    "flatbuffers::unique_ptr",
] + (["--cpp-std", "c++0x"] if args.cpp_0x else [])

CPP_17_OPTS = NO_INCL_OPTS + [
    "--cpp",
    "--cpp-std",
    "c++17",
    "--cpp-static-reflection",
    "--gen-object-api",
]
RUST_OPTS = BASE_OPTS + ["--rust", "--gen-all", "--gen-name-strings"]
TS_OPTS = ["--ts", "--gen-name-strings"]
LOBSTER_OPTS = ["--lobster"]
SWIFT_OPTS = ["--swift", "--gen-json-emit", "--bfbs-filenames", str(tests_path)]
JAVA_OPTS = ["--java"]
KOTLIN_OPTS = ["--kotlin"]
PHP_OPTS = ["--php"]
DART_OPTS = ["--dart"]
PYTHON_OPTS = ["--python"]
BINARY_OPTS = ["-b", "--schema", "--bfbs-comments", "--bfbs-builtins"]

# Basic Usage

flatc(
    NO_INCL_OPTS
    + CPP_OPTS
    + CS_OPTS
    + TS_OPTS
    + [
        "--binary",
        "--java",
        "--kotlin",
        "--dart",
        "--go",
        "--lobster",
        "--php",
    ],
    schema="monster_test.fbs",
    include="include_test",
    data="monsterdata_test.json",
)

flatc(
    ["--lua", "--bfbs-filenames", str(tests_path)],
    schema="monster_test.fbs",
    include="include_test"    
)

flatc(
    NO_INCL_OPTS + CPP_OPTS + ["--grpc"],
    schema="monster_test.fbs",
    include="include_test",
    data="monsterdata_test.json",
)

flatc(
    RUST_OPTS,
    schema="monster_test.fbs",
    include="include_test",
    prefix="monster_test",
    data="monsterdata_test.json",
)

flatc(
    options=BASE_OPTS + ["--python"],
    schema="monster_test.fbs",
    include="include_test",
    data="monsterdata_test.json",
)

flatc(
    options=BASE_OPTS + ["--python", "--gen-onefile"],
    schema="monster_test.fbs",
    include="include_test",
    data="monsterdata_test.json",
)

# For Rust we currently generate two independent schemas, with namespace_test2
# duplicating the types in namespace_test1
flatc(
    RUST_OPTS,
    prefix="namespace_test",
    schema=[
        "namespace_test/namespace_test1.fbs",
        "namespace_test/namespace_test2.fbs",
    ],
)

flatc(
    BASE_OPTS
    + CPP_OPTS
    + CS_OPTS
    + TS_OPTS
    + JAVA_OPTS
    + KOTLIN_OPTS
    + PHP_OPTS,
    prefix="union_vector",
    schema="union_vector/union_vector.fbs",
)

flatc(
    BASE_OPTS + TS_OPTS + ["--gen-name-strings", "--gen-mutable"],
    include="include_test",
    schema="monster_test.fbs",
)

flatc(
    BASE_OPTS + TS_OPTS + ["-b"],
    include="include_test",
    schema="monster_test.fbs",
    data="unicode_test.json",
)

flatc(
    BASE_OPTS + TS_OPTS + ["--gen-name-strings"],
    prefix="union_vector",
    schema="union_vector/union_vector.fbs",
)

flatc(
    RUST_OPTS,
    prefix="include_test1",
    include="include_test",
    schema="include_test/include_test1.fbs",
)

flatc(
    RUST_OPTS,
    prefix="include_test2",
    include="include_test",
    schema="include_test/sub/include_test2.fbs",
)

flatc(
    BINARY_OPTS + ["--bfbs-filenames", str(tests_path)],
    include="include_test",
    schema="monster_test.fbs",
)

flatc(
    CPP_OPTS
    + NO_INCL_OPTS
    + [
        "--bfbs-comments",
        "--bfbs-builtins",
        "--bfbs-gen-embed",
        "--bfbs-filenames",
        str(tests_path),
    ],
    include="include_test",
    schema="monster_test.fbs",
)

flatc(
    BINARY_OPTS + ["--bfbs-filenames", str(tests_path)],
    include="include_test",
    schema="arrays_test.fbs",
)

flatc(
    ["--jsonschema", "--schema"],
    include="include_test",
    schema="monster_test.fbs",
)

if not args.skip_monster_extra:
    flatc(
        CPP_OPTS
        + CS_OPTS
        + NO_INCL_OPTS
        + JAVA_OPTS
        + KOTLIN_OPTS
        + PYTHON_OPTS,
        schema="monster_extra.fbs",
        data="monsterdata_extra.json",
    )

    flatc(
        DART_OPTS + ["--gen-object-api"],
        schema="monster_extra.fbs",
    )

flatc(
    CPP_OPTS
    + CS_OPTS
    + NO_INCL_OPTS
    + JAVA_OPTS
    + ["--jsonschema", "--scoped-enums"],
    schema="arrays_test.fbs",
)

flatc(
    RUST_OPTS,
    prefix="arrays_test",
    schema="arrays_test.fbs",
)

flatc(
    BASE_OPTS + PYTHON_OPTS,
    schema="arrays_test.fbs",
)


# Optional Scalars
optional_scalars_schema = "optional_scalars.fbs"
flatc(
    ["--java", "--kotlin", "--lobster", "--ts"], schema=optional_scalars_schema
)

flatc(["--csharp", "--gen-object-api"], schema=optional_scalars_schema)

flatc(RUST_OPTS, prefix="optional_scalars", schema=optional_scalars_schema)

flatc(NO_INCL_OPTS + CPP_OPTS, schema=optional_scalars_schema)

# Generate string/vector default code for tests
flatc(RUST_OPTS, prefix="more_defaults", schema="more_defaults.fbs")

# Generate the schema evolution tests
flatc(
    CPP_OPTS + ["--scoped-enums"],
    prefix="evolution_test",
    schema=glob(tests_path, "evolution_test/evolution_v*.fbs"),
)

# Generate the keywords tests
flatc(BASE_OPTS + CS_OPTS, schema="keyword_test.fbs")
flatc(RUST_OPTS, prefix="keyword_test", schema="keyword_test.fbs")
flatc(
    BASE_OPTS + CS_OPTS + ["--cs-global-alias", "--gen-onefile"],
    prefix="nested_namespace_test",
    schema=glob(tests_path, "nested_namespace_test/nested_namespace_test*.fbs"),
)

# Swift Tests
swift_prefix = "FlatBuffers.Test.Swift/Tests/FlatBuffers.Test.SwiftTests"
flatc(
    SWIFT_OPTS + NO_INCL_OPTS + ["--grpc"],
    schema="monster_test.fbs",
    include="include_test",
    prefix=swift_prefix,
)
flatc(
    SWIFT_OPTS + BASE_OPTS,
    schema="union_vector/union_vector.fbs",
    prefix=swift_prefix,
)
flatc(SWIFT_OPTS, schema="optional_scalars.fbs", prefix=swift_prefix)
flatc(
    SWIFT_OPTS + ["--gen-object-api"],
    schema="more_defaults.fbs",
    prefix=swift_prefix,
)

# --filename-suffix and --filename-ext tests
flatc(
    CPP_OPTS
    + NO_INCL_OPTS
    + ["--filename-suffix", "_suffix", "--filename-ext", "hpp"],
    include="include_test",
    schema="monster_test.fbs",
)
orig_monster_file = Path(tests_path, "monster_test_generated.h")
new_monster_file = Path(tests_path, "monster_test_suffix.hpp")
assert (
    new_monster_file.exists()
), "filename suffix option did not produce a file"
assert filecmp.cmp(
    orig_monster_file, new_monster_file
), "filename suffix option did not produce identical results"
new_monster_file.unlink()

# Flag c++17 requires Clang6, GCC7, MSVC2017 (_MSC_VER >= 1914) or higher.
cpp_17_prefix = "cpp17/generated_cpp17"
flatc(
    CPP_17_OPTS,
    schema="monster_test.fbs",
    include="include_test",
    prefix=cpp_17_prefix,
)
flatc(
    CPP_17_OPTS,
    schema="optional_scalars.fbs",
    prefix=cpp_17_prefix,
)
flatc(
    CPP_17_OPTS,
    schema="union_vector/union_vector.fbs",
    prefix=cpp_17_prefix,
)

# Sample files
samples_schema = "monster.fbs"
flatc(
    BASE_OPTS + CPP_OPTS + LOBSTER_OPTS, schema=samples_schema, cwd=samples_path
)
flatc(
    RUST_OPTS, prefix="rust_generated", schema=samples_schema, cwd=samples_path
)
flatc(
    BINARY_OPTS + ["--bfbs-filenames", str(samples_path)],
    schema=samples_schema,
    cwd=samples_path,
)

# Reflection
temp_dir = ".tmp"
flatc(
    ["-c", "--cpp-std", "c++0x", "--no-prefix"],
    prefix=temp_dir,
    schema="reflection.fbs",
    cwd=reflection_path,
)
new_reflection_file = Path(reflection_path, temp_dir, "reflection_generated.h")
original_reflection_file = Path(
    root_path, "include/flatbuffers/reflection_generated.h"
)
if not filecmp.cmp(new_reflection_file, original_reflection_file):
    shutil.move(new_reflection_file, original_reflection_file)
shutil.rmtree(Path(reflection_path, temp_dir))
