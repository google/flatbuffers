#!/bin/bash
set -eu

script_dir="$(cd "$(dirname "$0")" && pwd)"
repo_dir="$(cd "${script_dir}/.." && pwd)"
dart_dir="${repo_dir}/dart"
test_dir="${dart_dir}/test"
preserve_dir="${dart_dir}/test_preserve_case"

command -v dart >/dev/null 2>&1 || {
  echo >&2 "Dart tests require dart to be in path but it's not installed. Aborting."
  exit 1
}

pushd "${script_dir}" >/dev/null

generate_preserve_case() {
  local flatc="${repo_dir}/flatc"
  local opts=(--dart --gen-object-api --preserve-case)
  "${flatc}" "${opts[@]}" -I "${script_dir}/include_test" -o "${preserve_dir}" "${script_dir}/monster_test.fbs"
  "${flatc}" "${opts[@]}" -I "${script_dir}/include_test/sub" -o "${preserve_dir}" "${script_dir}/include_test/include_test1.fbs"
  "${flatc}" "${opts[@]}" -I "${script_dir}/include_test" -o "${preserve_dir}" "${script_dir}/include_test/sub/include_test2.fbs"
  "${flatc}" "${opts[@]}" -o "${preserve_dir}" "${test_dir}/enums.fbs"
  "${flatc}" "${opts[@]}" -o "${preserve_dir}" "${test_dir}/bool_structs.fbs"
}

run_default_tests() {
  echo "Running Dart tests (default naming)"
  ( cd "${dart_dir}" && dart pub get && dart test "test/" )
}

run_preserve_case_tests() {
  echo "Running Dart tests (preserve-case naming)"
  if [[ ! -d "${preserve_dir}" ]]; then
    echo >&2 "Missing ${preserve_dir}; create preserve-case test copies first."
    exit 1
  fi
  generate_preserve_case
  ( cd "${dart_dir}" && dart test "test_preserve_case/" )
}

run_default_tests
run_preserve_case_tests

popd >/dev/null
