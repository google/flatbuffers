#!/bin/sh
# Copyright 2014 Google Inc. All rights reserved.
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

set -e

testdir=$(dirname $0)

# Ensure that this test is run from the tests directory:
if [[ "$testdir" != "." ]]; then
  echo error: must be run from inside the ./tests directory
  echo you ran it from `pwd`
  exit 1
fi

# Emit Go code for the example schema in the test dir:
./../flatc -g monster_test.fbs

# Go requires a particular layout of files in order to link multiple packages.
# Copy flatbuffer Go files to their own package directories to compile the
# test binary:
mkdir -p ./go_gen/src/MyGame/Example
mkdir -p ./go_gen/src/github.com/google/flatbuffers/go
mkdir -p ./go_gen/src/flatbuffers_test

cp MyGame/Example/*.go ./go_gen/src/MyGame/Example/
cp ../go/* ./go_gen/src/github.com/google/flatbuffers/go
cp ./go_test.go ./go_gen/src/flatbuffers_test/

# Declare gopath to be the generated code directory:
gopath=`pwd`/go_gen

# Run tests with necessary flags.
# Developers may wish to see more detail by appending the verbosity flag
# -test.v to arguments for this command, as in:
#   go -test -test.v ...
# Developers may also wish to run benchmarks, which may be achieved with the
# flag -test.bench and the wildcard regexp ".":
#   go -test -test.bench=. ...
GOPATH=${gopath} go test flatbuffers_test \
                    --test.coverpkg=github.com/google/flatbuffers/go \
                    --cpp_data=`pwd`/monsterdata_test.bin \
                    --java_data=`pwd`/monsterdata_java_wire.bin \
                    --out_data=`pwd`/monsterdata_go_wire.bin \
                    --fuzz=true \
                    --super_fuzz=false \
                    --fuzz_fields=4 \
                    --fuzz_objects=10000

# Remove compiled Go artifacts:
if [ -d ./go_gen/pkg ] ; then
  rm -r ./go_gen/pkg
fi
# Remove copied source code:
if [ -d ./go_gen/src ] ; then
  rm -r ./go_gen/src
fi

echo OK: Go tests passed.
