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
import generate_grpc_examples
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
parser.add_argument(
    "--skip-gen-reflection",
    action="store_true",
    help="skip generating the reflection.fbs files",
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
swift_code_gen = Path(root_path, "tests/swift/tests/CodeGenerationTests")
samples_path = Path(root_path, "samples")
reflection_path = Path(root_path, "reflection")

# Execute the flatc compiler with the specified parameters
def flatc(options, schema, prefix=None, include=None, data=None, cwd=tests_path):
    cmd = [str(flatc_path)] + options
    if prefix:
        cmd += ["-o"] + [prefix]
    if include:
        cmd += ["-I"] + [include]
    cmd += [schema] if isinstance(schema, str) else schema
    if data:
        cmd += [data] if isinstance(data, str) else data
    result = subprocess.run(cmd, cwd=str(cwd), check=True)


# Generate the code for flatbuffers reflection schema
def flatc_reflection(options, location, target):
    full_options = ["--no-prefix"] + options
    temp_dir = ".tmp"
    flatc(
        full_options,
        prefix=temp_dir,
        schema="reflection.fbs",
        cwd=reflection_path,
    )
    new_reflection_path = Path(reflection_path, temp_dir, target)
    original_reflection_path = Path(root_path, location, target)
    if not filecmp.cmp(str(new_reflection_path), str(original_reflection_path)):
        shutil.rmtree(str(original_reflection_path), ignore_errors=True)
        shutil.move(str(new_reflection_path), str(original_reflection_path))
    shutil.rmtree(str(Path(reflection_path, temp_dir)))

def flatc_annotate(schema, file, include=None, cwd=tests_path):
    cmd = [str(flatc_path)]
    if include:
        cmd += ["-I"] + [include]
    cmd += ["--annotate", schema, file]
    result = subprocess.run(cmd, cwd=str(cwd), check=True)

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
RUST_OPTS = BASE_OPTS + [
    "--rust",
    "--gen-all",
    "--gen-name-strings",
    "--rust-module-root-file",
]
RUST_SERIALIZE_OPTS = BASE_OPTS + [
    "--rust",
    "--gen-all",
    "--gen-name-strings",
    "--rust-serialize",
    "--rust-module-root-file",
]
TS_OPTS = ["--ts", "--gen-name-strings"]
LOBSTER_OPTS = ["--lobster"]
SWIFT_OPTS = ["--swift", "--gen-json-emit", "--bfbs-filenames", str(tests_path)]
SWIFT_OPTS_CODE_GEN = [
    "--swift",
    "--gen-json-emit",
    "--bfbs-filenames",
    swift_code_gen
]
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
    NO_INCL_OPTS
    + DART_OPTS,
    schema="include_test/include_test1.fbs",
    include="include_test/sub",
)

flatc(
    NO_INCL_OPTS
    + DART_OPTS,
    schema="include_test/sub/include_test2.fbs",
    include="include_test",
)

flatc(
    NO_INCL_OPTS
    + TS_OPTS,
    schema="monster_test.fbs",
    prefix="ts",
    include="include_test",
    data="monsterdata_test.json",
)

flatc(
    ["--lua", "--bfbs-filenames", str(tests_path)],
    schema="monster_test.fbs",
    include="include_test",
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
    RUST_SERIALIZE_OPTS,
    schema="monster_test.fbs",
    include="include_test",
    prefix="monster_test_serialize",
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
    RUST_OPTS + CS_OPTS,
    prefix="namespace_test",
    schema=[
        "namespace_test/namespace_test1.fbs",
        "namespace_test/namespace_test2.fbs",
    ],
)

flatc(
    BASE_OPTS + CPP_OPTS + CS_OPTS + JAVA_OPTS + KOTLIN_OPTS + PHP_OPTS,
    prefix="union_vector",
    schema="union_vector/union_vector.fbs",
)

flatc(
    BASE_OPTS + TS_OPTS,
    prefix="ts/union_vector",
    schema="union_vector/union_vector.fbs",
)

flatc(
    BASE_OPTS + TS_OPTS + ["--gen-name-strings", "--gen-mutable"],
    include="include_test",
    prefix="ts",
    schema="monster_test.fbs",
)

# Generate the complete flat file TS of monster.
flatc(
    ["--ts", "--gen-all", "--ts-flat-files"],
    include="include_test",
    schema="monster_test.fbs",
    prefix="ts/ts-flat-files"
)

flatc(
    BASE_OPTS + TS_OPTS + ["-b"],
    include="include_test",
    prefix="ts",
    schema="monster_test.fbs",
    data="unicode_test.json",
)

flatc(
    BASE_OPTS + TS_OPTS + ["--gen-name-strings"],
    prefix="ts/union_vector",
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

# Generate the annotated binary of the monster_test binary schema.
flatc_annotate(
    schema="../reflection/reflection.fbs",
    file="monster_test.bfbs",
    include="include_test"
)

flatc_annotate(
    schema="monster_test.fbs",
    file="monsterdata_test.mon",
    include="include_test"
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
        CPP_OPTS + CS_OPTS + NO_INCL_OPTS + JAVA_OPTS + KOTLIN_OPTS + PYTHON_OPTS,
        schema="monster_extra.fbs",
        data="monsterdata_extra.json",
    )

    flatc(
        DART_OPTS + ["--gen-object-api"],
        schema="monster_extra.fbs",
    )

flatc(
    CPP_OPTS + CS_OPTS + NO_INCL_OPTS + JAVA_OPTS + ["--jsonschema", "--scoped-enums"],
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


flatc(
    BASE_OPTS + PYTHON_OPTS,
    schema="nested_union_test.fbs",
)


# Optional Scalars
optional_scalars_schema = "optional_scalars.fbs"
flatc(["--java", "--kotlin", "--lobster"], schema=optional_scalars_schema)
flatc(TS_OPTS, schema=optional_scalars_schema, prefix="ts")

flatc(["--csharp", "--python", "--gen-object-api"], schema=optional_scalars_schema)

flatc(RUST_OPTS, prefix="optional_scalars", schema=optional_scalars_schema)

flatc(NO_INCL_OPTS + CPP_OPTS, schema=optional_scalars_schema)

# Type / field collsion
type_field_collsion_schema = "type_field_collsion.fbs"

flatc(["--csharp", "--gen-object-api"], schema=type_field_collsion_schema)

# Union / value collision
flatc(
    CS_OPTS + ["--gen-object-api", "--gen-onefile"],
    prefix="union_value_collsion",
    schema="union_value_collision.fbs"
)

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
flatc(BASE_OPTS + DART_OPTS, prefix="../dart/test/", schema="keyword_test.fbs")

# Field key lookup with default value test
dictionary_lookup_schema = "dictionary_lookup.fbs"
flatc(["--java", "--kotlin"], schema=dictionary_lookup_schema)

# Swift Tests
swift_prefix = "swift/tests/Tests/FlatBuffers.Test.SwiftTests"
flatc(
    SWIFT_OPTS + BASE_OPTS + ["--grpc"],
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
flatc(SWIFT_OPTS, schema="vector_has_test.fbs", prefix=swift_prefix)
flatc(SWIFT_OPTS, schema="nan_inf_test.fbs", prefix=swift_prefix)
flatc(
    SWIFT_OPTS + ["--gen-object-api"],
    schema="more_defaults.fbs",
    prefix=swift_prefix,
)
flatc(
    SWIFT_OPTS + BASE_OPTS,
    schema="MutatingBool.fbs",
    prefix=swift_prefix,
)

flatc(
    SWIFT_OPTS_CODE_GEN + BASE_OPTS + ["--grpc", "--swift-implementation-only"],
    schema="test_import.fbs",
    cwd=swift_code_gen
)

flatc(
    SWIFT_OPTS_CODE_GEN + NO_INCL_OPTS + ["--grpc"],
    schema="test_no_include.fbs",
    cwd=swift_code_gen
)

# Swift Wasm Tests
swift_Wasm_prefix = "swift/Wasm.tests/Tests/FlatBuffers.Test.Swift.WasmTests"
flatc(
    SWIFT_OPTS + BASE_OPTS,
    schema="monster_test.fbs",
    include="include_test",
    prefix=swift_Wasm_prefix,
)

# Nim Tests
NIM_OPTS = BASE_OPTS + ["--nim"]
flatc(NIM_OPTS, schema="monster_test.fbs", include="include_test")
flatc(NIM_OPTS, schema="optional_scalars.fbs")
flatc(NIM_OPTS, schema="more_defaults.fbs")
flatc(NIM_OPTS, schema="MutatingBool.fbs")

# --filename-suffix and --filename-ext tests
flatc(
    CPP_OPTS + NO_INCL_OPTS + ["--grpc", "--filename-ext", "hpp"],
    include="include_test",
    prefix="monster_test_suffix/ext_only",
    schema="monster_test.fbs",
)
flatc(
    CPP_OPTS + NO_INCL_OPTS + ["--grpc", "--filename-suffix", "_suffix"],
    include="include_test",
    prefix="monster_test_suffix/filesuffix_only",
    schema="monster_test.fbs",
)
flatc(
    CPP_OPTS + NO_INCL_OPTS + ["--grpc", "--filename-suffix", "_suffix", "--filename-ext", "hpp"],
    include="include_test",
    prefix="monster_test_suffix",
    schema="monster_test.fbs",
)

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

# Private annotations
annotations_test_schema = "private_annotation_test.fbs"

flatc(RUST_OPTS + ["--no-leak-private-annotation", "--gen-object-api"], prefix="private_annotation_test", schema=annotations_test_schema)

# Sample files
samples_schema = "monster.fbs"
flatc(BASE_OPTS + CPP_OPTS + LOBSTER_OPTS, schema=samples_schema, cwd=samples_path)
flatc(RUST_OPTS, prefix="rust_generated", schema=samples_schema, cwd=samples_path)
flatc(
    BINARY_OPTS + ["--bfbs-filenames", str(samples_path)],
    schema=samples_schema,
    cwd=samples_path,
)

# Reflection

# Skip generating the reflection if told too, as we run this script after
# building flatc which uses the reflection_generated.h itself.
if not args.skip_gen_reflection:
    # C++ Reflection
    flatc_reflection(
        ["-c", "--cpp-std", "c++0x"], "include/flatbuffers", "reflection_generated.h"
    )

# Python Reflection
flatc_reflection(["-p"], "python/flatbuffers", "reflection")

# Annotation


def flatc_annotate(schema, include=None, data=None, cwd=tests_path):
    cmd = [str(flatc_path)]
    if include:
        cmd += ["-I"] + [include]
    cmd += ["--annotate", schema]
    if data:
        cmd += [data] if isinstance(data, str) else data
    subprocess.run(cmd, cwd=str(cwd), check=True)


flatc_annotate(
    schema="monster_test.fbs", include="include_test", data="monsterdata_test.mon"
)

# Run the generate_grpc_examples script
generate_grpc_examples.GenerateGRPCExamples()
