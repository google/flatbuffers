#!/usr/bin/env bash
set -eu

script_dir="$(cd "$(dirname "$0")" && pwd)"
repo_dir="$(cd "${script_dir}/.." && pwd)"
flatc="${repo_dir}/build/flatc"
if [[ ! -x "${flatc}" ]]; then
  flatc="${repo_dir}/flatc"
fi

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

compare_output_stripping_xflatbuffers() {
  local out_dir="$1"
  for golden in "${golden_files[@]}"; do
    local golden_path="${script_dir}/${golden}"
    local generated_path="${out_dir}/${golden}"
    python3 - "${golden_path}" "${generated_path}" <<'PY'
import json
import sys


def count_xflatbuffers(obj) -> int:
    if isinstance(obj, dict):
        count = 1 if "x-flatbuffers" in obj else 0
        return count + sum(count_xflatbuffers(v) for v in obj.values())
    if isinstance(obj, list):
        return sum(count_xflatbuffers(v) for v in obj)
    return 0


def strip_xflatbuffers(obj):
    if isinstance(obj, dict):
        return {k: strip_xflatbuffers(v) for k, v in obj.items() if k != "x-flatbuffers"}
    if isinstance(obj, list):
        return [strip_xflatbuffers(v) for v in obj]
    return obj


golden_path, generated_path = sys.argv[1], sys.argv[2]
with open(golden_path, "r", encoding="utf-8") as f:
    golden = json.load(f)
with open(generated_path, "r", encoding="utf-8") as f:
    generated = json.load(f)

xfb_count = count_xflatbuffers(generated)
if xfb_count == 0:
    print(f"Expected x-flatbuffers metadata in {generated_path}", file=sys.stderr)
    sys.exit(1)

if strip_xflatbuffers(golden) != strip_xflatbuffers(generated):
    print(f"JSON Schema mismatch (ignoring x-flatbuffers) for {generated_path}", file=sys.stderr)
    sys.exit(1)
PY
  done
}

run_case_diff() {
  local label="$1"
  local out_dir="$2"
  shift 2

  echo "Generating JSON Schemas (${label})"
  rm -rf "${out_dir}"
  mkdir -p "${out_dir}"
  ( cd "${script_dir}" && "${flatc}" "$@" "${include_flags[@]}" -o "${out_dir}" "${schemas[@]}" )
  compare_output "${out_dir}"
}

run_case_xflatbuffers() {
  local label="$1"
  local out_dir="$2"
  shift 2

  echo "Generating JSON Schemas (${label})"
  rm -rf "${out_dir}"
  mkdir -p "${out_dir}"
  ( cd "${script_dir}" && "${flatc}" "$@" "${include_flags[@]}" -o "${out_dir}" "${schemas[@]}" )
  compare_output_stripping_xflatbuffers "${out_dir}"
}

tmp_default="$(mktemp -d)"
tmp_preserve="$(mktemp -d)"
tmp_xflatbuffers_default="$(mktemp -d)"
tmp_xflatbuffers_preserve="$(mktemp -d)"
cleanup() {
  rm -rf "${tmp_default}" "${tmp_preserve}" "${tmp_xflatbuffers_default}" "${tmp_xflatbuffers_preserve}"
}
trap cleanup EXIT

run_case_diff "default naming" "${tmp_default}" --jsonschema
run_case_diff "preserve-case naming" "${tmp_preserve}" --jsonschema --preserve-case
run_case_xflatbuffers "x-flatbuffers metadata" "${tmp_xflatbuffers_default}" --jsonschema --jsonschema-xflatbuffers
run_case_xflatbuffers "x-flatbuffers metadata + preserve-case" "${tmp_xflatbuffers_preserve}" --jsonschema --jsonschema-xflatbuffers --preserve-case

echo "JSON Schema tests (default + preserve-case + x-flatbuffers) passed"
