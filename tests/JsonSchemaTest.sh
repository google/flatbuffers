#!/usr/bin/env bash
set -eu

script_dir="$(cd "$(dirname "$0")" && pwd)"
repo_dir="$(cd "${script_dir}/.." && pwd)"
flatc="${repo_dir}/flatc"

if [[ ! -x "${flatc}" ]]; then
  echo "Skipping JSON Schema tests: flatc executable not found at ${flatc}." >&2
  exit 0
fi

schemas=(
  "monster_test.fbs"
  "arrays_test.fbs"
)
include_flags=(
  "-I" "include_test"
  "-I" "include_test/sub"
)

golden_files=(
  "monster_test.schema.json"
  "arrays_test.schema.json"
)

compare_output() {
  local out_dir="$1"
  for golden in "${golden_files[@]}"; do
    local generated="${out_dir}/${golden}"
    if ! diff -u "${script_dir}/${golden}" "${generated}"; then
      echo "JSON Schema mismatch for ${golden}" >&2
      exit 1
    fi
  done
}

run_case() {
  local label="$1"
  local out_dir="$2"
  shift 2

  echo "Generating JSON Schemas (${label})"
  rm -rf "${out_dir}"
  mkdir -p "${out_dir}"
  ( cd "${script_dir}" && "${flatc}" "$@" "${include_flags[@]}" -o "${out_dir}" "${schemas[@]}" )
  compare_output "${out_dir}"
}

tmp_default="$(mktemp -d)"
tmp_preserve="$(mktemp -d)"
cleanup() {
  rm -rf "${tmp_default}" "${tmp_preserve}"
}
trap cleanup EXIT

run_case "default naming" "${tmp_default}" --jsonschema
run_case "preserve-case naming" "${tmp_preserve}" --jsonschema --preserve-case

echo "JSON Schema tests (default + preserve-case) passed"
