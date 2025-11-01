#!/usr/bin/env bash
set -eu

script_dir="$(cd "$(dirname "$0")" && pwd)"
repo_dir="$(cd "${script_dir}/.." && pwd)"
preserve_dir="${repo_dir}/php/test_preserve_case"
flatc="${repo_dir}/flatc"

if ! command -v php >/dev/null 2>&1; then
  echo "Skipping PHP tests: php binary not found." >&2
  exit 0
fi
if [[ ! -x "${flatc}" ]]; then
  echo "Skipping PHP tests: flatc executable not found at ${flatc}." >&2
  exit 0
fi

run_php() {
  PHP_GENERATED_ROOT="$1" PHP_UNION_GENERATED_DIR="$2" php "$3"
}

generate_preserve_case() {
  rm -rf "${preserve_dir}"
  mkdir -p "${preserve_dir}"
  local schemas=(
    "${script_dir}/monster_test.fbs"
    "${script_dir}/nested_union_test.fbs"
    "${script_dir}/service_test.fbs"
    "${script_dir}/union_vector/union_vector.fbs"
    "${script_dir}/include_test/order.fbs"
    "${script_dir}/include_test/sub/no_namespace.fbs"
  )
  "${flatc}" --php --preserve-case --gen-object-api \
    -I "${script_dir}/include_test" \
    -I "${script_dir}/include_test/sub" \
    -o "${preserve_dir}" \
    "${schemas[@]}"
}

echo "Running PHP tests (default naming)"
run_php "${script_dir}" "${script_dir}/union_vector" "${script_dir}/phpTest.php"
run_php "${script_dir}" "${script_dir}/union_vector" "${script_dir}/phpUnionVectorTest.php"

echo "Running PHP tests (preserve-case naming)"
generate_preserve_case
run_php "${preserve_dir}" "${preserve_dir}" "${script_dir}/phpTestPreserveCase.php"
run_php "${preserve_dir}" "${preserve_dir}" "${script_dir}/phpUnionVectorTestPreserveCase.php"

echo "PHP tests (default + preserve-case) passed"
