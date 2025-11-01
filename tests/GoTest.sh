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
go_path_base=${test_dir}/go_gen

function generate_go_code() {
  local preserve_case_flag=$1
  local go_src=$2
  local preserve_case_opt=""

  if [ "${preserve_case_flag}" = "true" ]; then
    preserve_case_opt="--preserve-case"
  fi

  ../flatc -g --gen-object-api ${preserve_case_opt} -I include_test -o ${go_src} monster_test.fbs optional_scalars.fbs
  ../flatc -g --gen-object-api ${preserve_case_opt} -I include_test/sub -o ${go_src} include_test/order.fbs
  ../flatc -g --gen-object-api ${preserve_case_opt} -o ${go_src}/Pizza include_test/sub/no_namespace.fbs
}

function run_go_suite() {
  local variant=$1
  local preserve_case_flag=$2
  local test_source=$3

  local go_path_variant=${go_path_base}_${variant}
  local go_src=${go_path_variant}/src
  local out_data=${test_dir}/monsterdata_go_wire_${variant}.mon

  echo "Running Go tests (${variant})"

  rm -rf "${go_path_variant}"
  mkdir -p "${go_src}"

  generate_go_code "${preserve_case_flag}" "${go_src}"

  mkdir -p ${go_src}/github.com/google/flatbuffers/go
  mkdir -p ${go_src}/flatbuffers_test

  cp -a ../go/* ${go_src}/github.com/google/flatbuffers/go
  cp -a "${test_source}" ${go_src}/flatbuffers_test/go_test.go

  # https://stackoverflow.com/a/63545857/7024978
  # We need to turn off go modules for this script to work.
  go env -w GO111MODULE=off

  GOPATH=${go_path_variant} go test flatbuffers_test \
                       --coverpkg=github.com/google/flatbuffers/go \
                       --cpp_data=${test_dir}/monsterdata_test.mon \
                       --out_data=${out_data} \
                       --bench=. \
                       --benchtime=3s \
                       --fuzz=true \
                       --fuzz_fields=4 \
                       --fuzz_objects=10000

  local go_test_result=$?
  rm -rf ${go_path_variant}/{pkg,src}

  if [[ ${go_test_result} != 0 ]]; then
    echo "KO: Go tests failed for ${variant}."
    # Re-enable go modules before exiting
    go env -w GO111MODULE=on
    exit 1
  fi
}

# Run both default and preserve-case variants.
run_go_suite "default" "false" "./go_test.go"
run_go_suite "preserve_case" "true" "./go_test_preserve_case.go"

echo "OK: Go tests passed for default and preserve-case variants."

NOT_FMT_FILES=$(gofmt -l .)
if [[ ${NOT_FMT_FILES} != "" ]]; then
    echo "These files are not well gofmt'ed:"
    echo
    echo "${NOT_FMT_FILES}"
    # enable this when enums are properly formated
    # exit 1
fi

# Re-enable go modules when done tests
go env -w GO111MODULE=on
