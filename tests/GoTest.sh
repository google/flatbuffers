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

# Emit Go code for the example schemas in the test dir:
# A go.mod file in tests directory declares modules "tests" so use it as import path reference in generated code
../flatc -g --gen-object-api --go-module-name tests -I include_test monster_test.fbs optional_scalars.fbs
../flatc -g --gen-object-api -I include_test/sub --go-module-name tests -o . include_test/order.fbs
../flatc -g --gen-object-api --go-module-name tests -o Pizza include_test/sub/no_namespace.fbs

# do not compile the gRPC generated files, which are not tested by go_test.go
# below, but have their own test.
rm -f ./MyGame/Example/*_grpc.go

# Run tests with necessary flags.
# Developers may wish to see more detail by appending the verbosity flag
# -test.v to arguments for this command, as in:
#   go -test -test.v ...
# Developers may also wish to run benchmarks, which may be achieved with the
# flag -test.bench and the wildcard regexp ".":
#   go -test -test.bench=. ...
GO111MODULE=on; go test go_test.go -cpp_data=./monsterdata_test.mon --out_data=./monsterdata_go_wire.mon --bench=. --benchtime=3s --fuzz=true --fuzz_fields=4 --fuzz_objects=10000
GO_TEST_RESULT=$?

if [[ $GO_TEST_RESULT  == 0 ]]; then
    echo "OK: Go tests passed."
else
    echo "KO: Go tests failed."
    exit 1
fi

NOT_FMT_FILES=$(gofmt -l .)
if [[ ${NOT_FMT_FILES} != "" ]]; then
    echo "These files are not well gofmt'ed:"
    echo
    echo "${NOT_FMT_FILES}"
    # enable this when enums are properly formated
    # exit 1
fi

popd
