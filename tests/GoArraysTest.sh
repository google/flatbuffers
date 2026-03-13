#!/bin/bash -eu
#
# Copyright 2025 Google Inc. All rights reserved.
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

# Test script for Go fixed-size array support.
# Generates Go code from arrays_test.fbs, compiles, and runs tests.

pushd "$(dirname $0)" >/dev/null
test_dir="$(pwd)"
go_path=${test_dir}/go_arrays_gen
go_src=${go_path}/src

# Generate Go code from the arrays test schema.
../build/flatc -g --gen-object-api -o ${go_src} arrays_test.fbs

# Set up the GOPATH layout that Go expects.
mkdir -p ${go_src}/github.com/google/flatbuffers/go
mkdir -p ${go_src}/flatbuffers_arrays_test

cp -a ../go/* ${go_src}/github.com/google/flatbuffers/go/
cp -a go_arrays_test_test.go ${go_src}/flatbuffers_arrays_test/

GO111MODULE=off GOPATH=${go_path} go test flatbuffers_arrays_test -v

GO_TEST_RESULT=$?
rm -rf ${go_path}
if [[ $GO_TEST_RESULT == 0 ]]; then
    echo "OK: Go fixed-size array tests passed."
else
    echo "KO: Go fixed-size array tests failed."
    exit 1
fi

popd >/dev/null
