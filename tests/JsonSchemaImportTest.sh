#!/usr/bin/env bash
set -eu

script_dir="$(cd "$(dirname "$0")" && pwd)"
repo_dir="$(cd "${script_dir}/.." && pwd)"
flatc="${repo_dir}/build/flatc"
if [[ ! -x "${flatc}" ]]; then
  flatc="${repo_dir}/flatc"
fi

if [[ ! -x "${flatc}" ]]; then
  echo "Skipping JSON Schema import tests: flatc executable not found at ${flatc}." >&2
  exit 0
fi

input_dir="${script_dir}/jsonschema_import/inputs"
golden_dir="${script_dir}/jsonschema_import/goldens"

if [[ ! -d "${input_dir}" || ! -d "${golden_dir}" ]]; then
  echo "Missing JSON Schema import fixtures under ${script_dir}/jsonschema_import." >&2
  exit 1
fi

tmp_out="$(mktemp -d)"
tmp_roundtrip="$(mktemp -d)"
cleanup() {
  rm -rf "${tmp_out}" "${tmp_roundtrip}"
}
trap cleanup EXIT

run_case() {
  local filename="$1"
  shift
  local input_path="${input_dir}/${filename}"
  local golden_path="${golden_dir}/${filename}"
  local out_path="${tmp_out}/${filename}"
  local roundtrip_path="${tmp_roundtrip}/${filename}"

  echo "Importing + generating JSON Schema: ${filename}"
  "${flatc}" "$@" --jsonschema -o "${tmp_out}" "${input_path}"
  diff -u "${golden_path}" "${out_path}"

  echo "Round-tripping generated JSON Schema: ${filename}"
  "${flatc}" --jsonschema -o "${tmp_roundtrip}" "${out_path}"
  diff -u "${out_path}" "${roundtrip_path}"
}

run_case "udl_openapi_antenna_subset.schema.json"
run_case "udl_openapi_ais_subset.schema.json"
run_case "udl_openapi_aircraft_mission_tasking_subset.schema.json" \
  --root-type AircraftMissionTasking_Abridged

echo "JSON Schema import tests passed"

