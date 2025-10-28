#!/bin/bash
set -e
#
# Copyright 2018 Google Inc. All rights reserved.
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

if [[ "$1" == "mips-unknown-linux-gnu" ]]; then
    TARGET_FLAG="--target mips-unknown-linux-gnu"
    export CARGO_TARGET_MIPS_UNKNOWN_LINUX_GNU_LINKER=mips-linux-gnu-gcc
    export CARGO_TARGET_MIPS_UNKNOWN_LINUX_GNU_RUNNER="qemu-mips -L /usr/mips-linux-gnu"
fi

export PATH="${HOME}/.cargo/bin:${PATH}"
CARGO_BIN="${HOME}/.cargo/bin/cargo"
if [[ ! -x "${CARGO_BIN}" ]]; then
    CARGO_BIN="cargo"
fi

function generate_preserve_case_rust() {
    local out_root="preserve_case_rust"
    local base_opts=(
        --rust
        --reflect-names
        --gen-mutable
        --gen-object-api
        --gen-all
        --gen-name-strings
        --rust-module-root-file
        --preserve-case
    )

    rm -rf "${out_root}/monster_test"
    ../flatc "${base_opts[@]}" \
        -o "${out_root}/monster_test" -I include_test monster_test.fbs

    rm -rf "${out_root}/include_test1"
    ../flatc "${base_opts[@]}" \
        -o "${out_root}/include_test1" -I include_test include_test/include_test1.fbs

    rm -rf "${out_root}/include_test2"
    ../flatc "${base_opts[@]}" \
        -o "${out_root}/include_test2" -I include_test include_test/sub/include_test2.fbs

    rm -rf "${out_root}/optional_scalars"
    ../flatc "${base_opts[@]}" \
        -o "${out_root}/optional_scalars" optional_scalars.fbs

    rm -rf "${out_root}/arrays_test"
    ../flatc "${base_opts[@]}" \
        -o "${out_root}/arrays_test" arrays_test.fbs

    rm -rf "${out_root}/namespace_test"
    ../flatc "${base_opts[@]}" \
        -o "${out_root}/namespace_test" namespace_test/namespace_test1.fbs namespace_test/namespace_test2.fbs

    rm -rf "${out_root}/keyword_test"
    ../flatc "${base_opts[@]}" \
        -o "${out_root}/keyword_test" keyword_test.fbs

    rm -rf "${out_root}/rust_namer_test"
    ../flatc "${base_opts[@]}" \
        -o "${out_root}/rust_namer_test" rust_namer_test.fbs
}

generate_preserve_case_rust


function check_test_result() {
    if [[ $? == 0 ]]; then
        echo OK: $1 passed.
    else
        echo KO: $1 failed.
        exit 1
    fi
}

cd ./rust_serialize_test
"${CARGO_BIN}" run $TARGET_FLAG -- --quiet
check_test_result "Rust serde tests"

cd ../rust_no_std_compilation_test
rustup install nightly
rustup component add rust-src --toolchain nightly
rustup target add thumbv7m-none-eabi
"${CARGO_BIN}" +nightly build
check_test_result "Rust flatbuffers test no_std compilation"

cd ../rust_usage_test
"${CARGO_BIN}" test $TARGET_FLAG -- --quiet
check_test_result "Rust tests"

"${CARGO_BIN}" test $TARGET_FLAG --no-default-features -- --quiet
check_test_result "Rust tests (no_std)"

"${CARGO_BIN}" run $TARGET_FLAG --bin=flatbuffers_alloc_check
check_test_result "Rust flatbuffers heap alloc test"

"${CARGO_BIN}" run $TARGET_FLAG --bin=flexbuffers_alloc_check
check_test_result "Rust flexbuffers heap alloc test"

rustup component add clippy
"${CARGO_BIN}" clippy $TARGET_FLAG
check_test_result "No Cargo clippy lints test"

"${CARGO_BIN}" bench $TARGET_FLAG

# This test is dependent on flatc.
if [[ -f ../../flatc ]]; then
    cd outdir
    "${CARGO_BIN}" test
    check_test_result "Rust generated file in \$OUT_DIR"
    cd ..
fi

# RUST_NIGHTLY environment variable set in dockerfile.
if [[ $RUST_NIGHTLY == 1 ]]; then
  rustup +nightly component add miri
  MIRIFLAGS="-Zmiri-disable-isolation" "${CARGO_BIN}" +nightly miri test
fi
