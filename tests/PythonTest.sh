#!/bin/bash
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

set -eu

pushd "$(dirname $0)" >/dev/null
test_dir="$(pwd)"
gen_code_path=${test_dir}
runtime_library_dir=${test_dir}/../python

# Clean up any previous generated files
rm -rf ${gen_code_path}/MyGame
mkdir -p ${gen_code_path}

# Function to generate code (with or without preserve-case)
function generate_code() {
  preserve_case_flag=$1

  echo "Generating code (preserve-case: ${preserve_case_flag})..."

  if [ "${preserve_case_flag}" = "true" ]; then
    preserve_case_opt="--preserve-case"
  else
    preserve_case_opt=""
  fi

  ${test_dir}/../flatc -p -o ${gen_code_path} -I include_test monster_test.fbs --gen-object-api ${preserve_case_opt}
  ${test_dir}/../flatc -p -o ${gen_code_path} -I include_test monster_test.fbs --gen-object-api --gen-onefile ${preserve_case_opt}
  ${test_dir}/../flatc -p -o ${gen_code_path} -I include_test monster_extra.fbs --gen-object-api --python-typing --gen-compare ${preserve_case_opt}
  ${test_dir}/../flatc -p -o ${gen_code_path} -I include_test arrays_test.fbs --gen-object-api --python-typing ${preserve_case_opt}
  ${test_dir}/../flatc -p -o ${gen_code_path} -I include_test nested_union_test.fbs --gen-object-api --python-typing --python-decode-obj-api-strings ${preserve_case_opt}
  ${test_dir}/../flatc -p -o ${gen_code_path} -I include_test service_test.fbs --grpc --grpc-python-typed-handlers --python-typing --no-python-gen-numpy --gen-onefile ${preserve_case_opt}
}

# Syntax: run_tests <interpreter> <benchmark vtable dedupes>
#                   <benchmark read count> <benchmark build count>
interpreters_tested=()
function run_tests() {
  if $(which ${1} >/dev/null); then
    echo "Testing with interpreter: ${1}"

    # First run without preserve-case
    generate_code false
    PYTHONDONTWRITEBYTECODE=1 \
    JYTHONDONTWRITEBYTECODE=1 \
    PYTHONPATH=${runtime_library_dir}:${gen_code_path} \
    JYTHONPATH=${runtime_library_dir}:${gen_code_path} \
    COMPARE_GENERATED_TO_GO=0 \
    COMPARE_GENERATED_TO_JAVA=0 \
    $1 py_test.py $2 $3 $4 $5 $6

    # Then run with preserve-case
    generate_code true
    PYTHONDONTWRITEBYTECODE=1 \
    PYTHONPATH=${runtime_library_dir}:${gen_code_path} \
    $1 py_test_preserve_case.py 0 0 0 0 false

    # Flexbuffers test for python3 only
    if [ $1 = python3 ]; then
      generate_code false
      PYTHONDONTWRITEBYTECODE=1 \
      PYTHONPATH=${runtime_library_dir}:${gen_code_path} \
      $1 py_flexbuffers_test.py
    fi
    interpreters_tested+=(${1})
    echo
  fi
}

# Run test suite with these interpreters. The arguments are benchmark counts.
run_tests python2.6 100 100 100 100 false
run_tests python2.7 100 100 100 100 false
run_tests python2.7 100 100 100 100 true
run_tests python3 100 100 100 100 false
run_tests python3 100 100 100 100 true
run_tests pypy 100 100 100 100 false

if [ ${#interpreters_tested[@]} -eq 0 ]; then
  echo "No Python interpeters found on this system, could not run tests."
  exit 1
fi

# Run test suite with default python interpreter.
if $(which coverage >/dev/null); then
  echo 'Found coverage utility, running coverage with default Python:'

  generate_code false
  PYTHONDONTWRITEBYTECODE=1 \
  PYTHONPATH=${runtime_library_dir}:${gen_code_path} \
  coverage run --source=flatbuffers,MyGame py_test.py 0 0 0 0 false > /dev/null

  echo
  cov_result=`coverage report --omit="*flatbuffers/vendor*,*py_test*"` \
              `| tail -n 1 | awk ' { print $4 } '`
  echo "Code coverage: ${cov_result}"
else
  echo -n "Did not find coverage utility for default Python, skipping. "
  echo "Install with 'pip install coverage'."
fi

echo
echo "OK: all tests passed for ${#interpreters_tested[@]} interpreters: ${interpreters_tested[@]}."