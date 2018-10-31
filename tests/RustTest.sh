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

cd ./rust_usage_test
cargo test -- --quiet
TEST_RESULT=$?
if [[ $TEST_RESULT  == 0 ]]; then
    echo "OK: Rust tests passed."
else
    echo "KO: Rust tests failed."
    exit 1
fi

cargo run --bin=alloc_check
TEST_RESULT=$?
if [[ $TEST_RESULT  == 0 ]]; then
    echo "OK: Rust heap alloc test passed."
else
    echo "KO: Rust heap alloc test failed."
    exit 1
fi

cargo bench
