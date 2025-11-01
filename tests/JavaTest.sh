#!/usr/bin/env bash
set -eu

script_dir="$(cd "$(dirname "$0")" && pwd)"
repo_dir="$(cd "${script_dir}/.." && pwd)"
java_dir="${repo_dir}/java"
flatc="${repo_dir}/flatc"

if ! command -v java >/dev/null 2>&1; then
  echo "Skipping Java tests: java executable not found." >&2
  exit 0
fi
if ! command -v mvn >/dev/null 2>&1; then
  echo "Skipping Java tests: mvn executable not found." >&2
  exit 0
fi
if [[ ! -x "${flatc}" ]]; then
  echo "Skipping Java tests: flatc executable not found at ${flatc}." >&2
  exit 0
fi

java_test_src="${java_dir}/src/test/java"
classpath_file="${java_dir}/target/test-classpath.txt"

compile_tests() {
  (cd "${java_dir}" && mvn -q -DskipTests clean test-compile)
}

build_test_classpath() {
  (cd "${java_dir}" && mvn -q -DincludeScope=test -DskipTests dependency:build-classpath -Dmdep.outputFile="target/test-classpath.txt" >/dev/null)
}

run_junit_test() {
  local test_name="$1"
  local cp="$(cat "${classpath_file}"):${java_dir}/target/test-classes:${java_dir}/target/classes"
  (cd "${java_dir}" && java -cp "${cp}" org.junit.runner.JUnitCore "${test_name}")
}

generate_preserve_case_code() {
  local out_dir="$1"
  local opts=("--java" "--reflect-names" "--gen-mutable" "--preserve-case" "--java-package-prefix" "preservecase")
  mkdir -p "${out_dir}"

  "${flatc}" "${opts[@]}" \
    -I "${script_dir}/include_test" \
    -I "${script_dir}/include_test/sub" \
    -o "${out_dir}" \
    "${script_dir}/monster_test.fbs"

  "${flatc}" "${opts[@]}" \
    -o "${out_dir}" \
    "${script_dir}/optional_scalars.fbs" \
    "${script_dir}/dictionary_lookup.fbs" \
    "${script_dir}/union_vector/union_vector.fbs" \
    "${script_dir}/namespace_test/namespace_test1.fbs" \
    "${script_dir}/namespace_test/namespace_test2.fbs" \
    "${script_dir}/arrays_test.fbs"
}

cleanup_preserve_case() {
  if [[ -n "${preserve_case_dir:-}" && -d "${preserve_case_dir}" ]]; then
    rm -rf "${preserve_case_dir}"
  fi
  if [[ -f "${java_test_src}/JavaPreserveCaseTest.java" ]]; then
    rm -f "${java_test_src}/JavaPreserveCaseTest.java"
  fi
  if [[ -d "${java_test_src}/preservecase" ]]; then
    rm -rf "${java_test_src}/preservecase"
  fi
  rm -f "${java_dir}/target/test-classpath.txt"
}

trap cleanup_preserve_case EXIT

echo "Running Java tests (default naming)"
compile_tests
build_test_classpath
run_junit_test "JavaTest"

echo "Running Java tests (preserve-case naming)"
preserve_case_dir="$(mktemp -d "${script_dir}/java_preserve_case_XXXX")"

generate_preserve_case_code "${preserve_case_dir}"

if [[ -d "${preserve_case_dir}/preservecase" ]]; then
  cp -R "${preserve_case_dir}/preservecase" "${java_test_src}/"
fi

cp "${script_dir}/java_preserve_case/JavaPreserveCaseTest.java" "${java_test_src}/JavaPreserveCaseTest.java"

compile_tests
build_test_classpath
run_junit_test "JavaPreserveCaseTest"

echo "Java tests (default + preserve-case) passed"
