#!/bin/bash -eu
#
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

pushd "$(dirname $0)" >/dev/null
test_dir="$(pwd)"
go_path=${test_dir}/go_gen
go_src=${go_path}/src
go_package=${go_src}

# Emit Go code for the example schema in the test dir:
../flatc -g --gen-object-api --gen-mutable -I include_test monster_test.fbs
../flatc -g --gen-object-api --gen-mutable  ./union_vector/union_vector.fbs

# Go requires a particular layout of files in order to link multiple packages.
# Copy flatbuffer Go files to their own package directories to compile the
# test binary:
mkdir -p ${go_src}/MyGame/Example
mkdir -p ${go_src}/MyGame/Example2
mkdir -p ${go_src}/go
mkdir -p ${go_src}/testdata
mkdir -p ${go_src}/Movie
# mkdir -p ${go_src}/flatbuffers_test
# mkdir -p ${go_src}/union_vector_test

cp -a MyGame/*.go ./go_gen/src/MyGame/
cp -a Movie/*.go ./go_gen/src/Movie/
cp ../go.mod  ./go_gen/src/
cp ./monsterdata_test.mon ./go_gen/src/testdata
#cp ./monsterdata_java_wire.mon ./go_gen/src/testdata
#TODO: tsingson: go can't pass monsterdata_java_wire.com test, i will identify it out after fixed_array feature done.
cp -a MyGame/Example/*.go ./go_gen/src/MyGame/Example/
cp -a MyGame/Example2/*.go ./go_gen/src/MyGame/Example2/
# do not compile the gRPC generated files, which are not tested by go_test.go
# below, but have their own test.
rm ./go_gen/src/MyGame/Example/*_grpc.go
cp -a ../go/* ./go_gen/src/go
cp -a ./go_test.go ./go_gen/src/
# cp -a ./union_vector_test.go ./go_gen/src/union_vector_test

# Run tests with necessary flags.
# Developers may wish to see more detail by appending the verbosity flag
# -test.v to arguments for this command, as in:
#   go -test -test.v ...
# Developers may also wish to run benchmarks, which may be achieved with the
# flag -test.bench and the wildcard regexp ".":
#   go -test -test.bench=. ...
cd ${go_src}
go test . \
                     --test.coverpkg=github.com/google/flatbuffers/go \
                     --test.bench=. \
                     --test.benchtime 3s \
                     --fuzz=true \
                     --fuzz_fields=4 \
                     --fuzz_objects=10000 \
                    --test.timeout=3s

GO_TEST_RESULT=$?
cd ${test_dir}
cp ${go_src}/testdata/monsterdata_go_wire.mon  ${test_dir}/
rm -rf ${go_path}/{pkg,src}
if [[ $GO_TEST_RESULT  == 0 ]]; then
    echo "OK: Go tests passed."
else
    echo "KO: Go tests failed."
    exit 1
fi

NOT_FMT_FILES=$(gofmt -l MyGame)
if [[ ${NOT_FMT_FILES} != "" ]]; then
    echo "These files are not well gofmt'ed:"
    echo
    echo "${NOT_FMT_FILES}"
    # enable this when enums are properly formated
    # exit 1
fi
