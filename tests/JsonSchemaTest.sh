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

xfb_metaschema="${repo_dir}/docs/source/schemas/x-flatbuffers.schema.json"
if [[ ! -f "${xfb_metaschema}" ]]; then
  echo "Missing x-flatbuffers meta-schema at ${xfb_metaschema}" >&2
  exit 1
fi

validate_xflatbuffers_metadata_file() {
  local generated_path="$1"
  python3 - "${generated_path}" "${xfb_metaschema}" <<'PY'
import json
import sys


schema_path, metaschema_path = sys.argv[1], sys.argv[2]

# Ensure the meta-schema is parseable JSON (tooling can consume this file).
with open(metaschema_path, "r", encoding="utf-8") as f:
    json.load(f)

with open(schema_path, "r", encoding="utf-8") as f:
    schema = json.load(f)


SCALAR_NAMES = {
    "bool",
    "byte",
    "ubyte",
    "short",
    "ushort",
    "int",
    "uint",
    "long",
    "ulong",
    "float",
    "double",
    "string",
    "utype",
}

TYPE_BASE_NAMES = SCALAR_NAMES | {"struct", "table", "union", "enum", "vector", "array"}

PRESENCE = {"required", "optional", "default"}

ENUM_UNDERLYING_TYPES = {"byte", "ubyte", "short", "ushort", "int", "uint", "long", "ulong", "utype"}


def path_str(path) -> str:
    parts = []
    for p in path:
        if isinstance(p, int):
            parts.append(f"[{p}]")
        else:
            parts.append(str(p))
    return ".".join(parts)


def fail(path, msg):
    raise AssertionError(f"{schema_path}:{path_str(path)}: {msg}")


def ensure(cond, path, msg):
    if not cond:
        fail(path, msg)


def ensure_type(obj, t, path, msg):
    if not isinstance(obj, t):
        fail(path, msg)


def validate_type_meta(obj, path):
    ensure_type(obj, dict, path, "x-flatbuffers.type must be an object")
    allowed = {"base", "ref", "scalar", "vector64", "fixed_length", "element"}
    unknown = set(obj.keys()) - allowed
    ensure(not unknown, path, f"unknown keys in x-flatbuffers type: {sorted(unknown)}")

    base = obj.get("base")
    ensure(isinstance(base, str), path, "x-flatbuffers.type.base must be a string")
    ensure(base in TYPE_BASE_NAMES, path, f"invalid x-flatbuffers.type.base: {base!r}")

    if base in {"bool", "byte", "ubyte", "short", "ushort", "int", "uint", "long", "ulong", "float", "double", "string"}:
        ensure(set(obj.keys()) == {"base"}, path, f"scalar x-flatbuffers.type must only contain 'base' (got {sorted(obj.keys())})")
        return

    if base in {"struct", "table", "union", "utype"}:
        ensure("ref" in obj, path, f"x-flatbuffers.type for base={base!r} requires 'ref'")
        ensure_type(obj["ref"], str, path + ["ref"], "x-flatbuffers.type.ref must be a string")
        ensure(set(obj.keys()) == {"base", "ref"}, path, f"x-flatbuffers.type for base={base!r} must only contain 'base' and 'ref'")
        return

    if base == "enum":
        ensure("ref" in obj, path, "x-flatbuffers.type for base='enum' requires 'ref'")
        ensure("scalar" in obj, path, "x-flatbuffers.type for base='enum' requires 'scalar'")
        ensure_type(obj["ref"], str, path + ["ref"], "x-flatbuffers.type.ref must be a string")
        ensure_type(obj["scalar"], str, path + ["scalar"], "x-flatbuffers.type.scalar must be a string")
        ensure(obj["scalar"] in SCALAR_NAMES, path + ["scalar"], f"invalid x-flatbuffers.type.scalar: {obj['scalar']!r}")
        ensure(set(obj.keys()) == {"base", "ref", "scalar"}, path, "x-flatbuffers.type for base='enum' must only contain 'base', 'ref', and 'scalar'")
        return

    if base == "vector":
        ensure("element" in obj, path, "x-flatbuffers.type for base='vector' requires 'element'")
        validate_type_meta(obj["element"], path + ["element"])
        if "vector64" in obj:
            ensure_type(obj["vector64"], bool, path + ["vector64"], "x-flatbuffers.type.vector64 must be boolean")
        allowed_keys = {"base", "element", "vector64"} if "vector64" in obj else {"base", "element"}
        ensure(set(obj.keys()) == allowed_keys, path, f"x-flatbuffers.type for base='vector' has unexpected keys: {sorted(set(obj.keys()) - allowed_keys)}")
        return

    if base == "array":
        ensure("element" in obj, path, "x-flatbuffers.type for base='array' requires 'element'")
        ensure("fixed_length" in obj, path, "x-flatbuffers.type for base='array' requires 'fixed_length'")
        validate_type_meta(obj["element"], path + ["element"])
        ensure_type(obj["fixed_length"], int, path + ["fixed_length"], "x-flatbuffers.type.fixed_length must be integer")
        ensure(obj["fixed_length"] >= 0, path + ["fixed_length"], "x-flatbuffers.type.fixed_length must be >= 0")
        ensure(set(obj.keys()) == {"base", "element", "fixed_length"}, path, "x-flatbuffers.type for base='array' must only contain 'base', 'element', and 'fixed_length'")
        return

    fail(path, f"unhandled x-flatbuffers.type.base: {base!r}")


def validate_field_meta(obj, path):
    ensure_type(obj, dict, path, "x-flatbuffers field metadata must be an object")
    allowed = {
        "type",
        "id",
        "presence",
        "deprecated",
        "key",
        "shared",
        "native_inline",
        "flexbuffer",
        "offset64",
        "nested_flatbuffer",
        "union_type_field",
        "union_value_field",
    }
    unknown = set(obj.keys()) - allowed
    ensure(not unknown, path, f"unknown keys in x-flatbuffers field metadata: {sorted(unknown)}")

    ensure("type" in obj, path, "x-flatbuffers field metadata missing 'type'")
    ensure("presence" in obj, path, "x-flatbuffers field metadata missing 'presence'")
    validate_type_meta(obj["type"], path + ["type"])
    ensure_type(obj["presence"], str, path + ["presence"], "x-flatbuffers.presence must be a string")
    ensure(obj["presence"] in PRESENCE, path + ["presence"], f"invalid x-flatbuffers.presence: {obj['presence']!r}")

    if "id" in obj:
        ensure(isinstance(obj["id"], (int, str)), path + ["id"], "x-flatbuffers.id must be integer or string")
    for key in ("deprecated", "key", "shared", "native_inline", "flexbuffer", "offset64"):
        if key in obj:
            ensure_type(obj[key], bool, path + [key], f"x-flatbuffers.{key} must be boolean")
    for key in ("nested_flatbuffer", "union_type_field", "union_value_field"):
        if key in obj:
            ensure_type(obj[key], str, path + [key], f"x-flatbuffers.{key} must be a string")


def validate_enum_def_meta(obj, path):
    ensure_type(obj, dict, path, "x-flatbuffers enum metadata must be an object")
    allowed = {"kind", "name", "namespace", "underlying_type", "values"}
    unknown = set(obj.keys()) - allowed
    ensure(not unknown, path, f"unknown keys in x-flatbuffers enum metadata: {sorted(unknown)}")

    for req in ("kind", "name", "namespace", "underlying_type", "values"):
        ensure(req in obj, path, f"x-flatbuffers enum metadata missing {req!r}")

    ensure_type(obj["kind"], str, path + ["kind"], "x-flatbuffers.kind must be a string")
    ensure(obj["kind"] in {"enum", "union"}, path + ["kind"], f"invalid x-flatbuffers.kind for enum: {obj['kind']!r}")
    ensure_type(obj["name"], str, path + ["name"], "x-flatbuffers.name must be a string")
    ensure_type(obj["namespace"], list, path + ["namespace"], "x-flatbuffers.namespace must be an array")
    for i, v in enumerate(obj["namespace"]):
        ensure_type(v, str, path + ["namespace", i], "x-flatbuffers.namespace items must be strings")
    ensure_type(obj["underlying_type"], str, path + ["underlying_type"], "x-flatbuffers.underlying_type must be a string")
    ensure(obj["underlying_type"] in ENUM_UNDERLYING_TYPES, path + ["underlying_type"], f"invalid x-flatbuffers.underlying_type: {obj['underlying_type']!r}")
    if obj["kind"] == "union":
        ensure(obj["underlying_type"] == "utype", path + ["underlying_type"], "union underlying_type must be 'utype'")

    ensure_type(obj["values"], list, path + ["values"], "x-flatbuffers.values must be an array")
    for i, val in enumerate(obj["values"]):
        ensure_type(val, dict, path + ["values", i], "x-flatbuffers.values items must be objects")
        allowed_val = {"name", "value", "union_type"}
        unknown_val = set(val.keys()) - allowed_val
        ensure(not unknown_val, path + ["values", i], f"unknown keys in enum value metadata: {sorted(unknown_val)}")
        ensure("name" in val, path + ["values", i], "enum value metadata missing 'name'")
        ensure("value" in val, path + ["values", i], "enum value metadata missing 'value'")
        ensure_type(val["name"], str, path + ["values", i, "name"], "enum value name must be a string")
        ensure_type(val["value"], str, path + ["values", i, "value"], "enum value must be a string")
        if "union_type" in val:
            ensure_type(val["union_type"], str, path + ["values", i, "union_type"], "enum value union_type must be a string")


def validate_struct_def_meta(obj, path):
    ensure_type(obj, dict, path, "x-flatbuffers struct/table metadata must be an object")
    allowed = {"kind", "name", "namespace", "has_key", "minalign", "bytesize"}
    unknown = set(obj.keys()) - allowed
    ensure(not unknown, path, f"unknown keys in x-flatbuffers struct/table metadata: {sorted(unknown)}")

    for req in ("kind", "name", "namespace"):
        ensure(req in obj, path, f"x-flatbuffers struct/table metadata missing {req!r}")

    ensure_type(obj["kind"], str, path + ["kind"], "x-flatbuffers.kind must be a string")
    ensure(obj["kind"] in {"struct", "table"}, path + ["kind"], f"invalid x-flatbuffers.kind for struct/table: {obj['kind']!r}")
    ensure_type(obj["name"], str, path + ["name"], "x-flatbuffers.name must be a string")
    ensure_type(obj["namespace"], list, path + ["namespace"], "x-flatbuffers.namespace must be an array")
    for i, v in enumerate(obj["namespace"]):
        ensure_type(v, str, path + ["namespace", i], "x-flatbuffers.namespace items must be strings")
    if "has_key" in obj:
        ensure_type(obj["has_key"], bool, path + ["has_key"], "x-flatbuffers.has_key must be boolean")
    if obj["kind"] == "struct":
        ensure("minalign" in obj and "bytesize" in obj, path, "struct metadata must include 'minalign' and 'bytesize'")
        ensure_type(obj["minalign"], int, path + ["minalign"], "x-flatbuffers.minalign must be integer")
        ensure_type(obj["bytesize"], int, path + ["bytesize"], "x-flatbuffers.bytesize must be integer")
    else:
        ensure("minalign" not in obj and "bytesize" not in obj, path, "table metadata must not include 'minalign' or 'bytesize'")


def validate_root_meta(obj, path):
    ensure_type(obj, dict, path, "x-flatbuffers root metadata must be an object")
    allowed = {"root_type", "file_identifier", "file_extension"}
    unknown = set(obj.keys()) - allowed
    ensure(not unknown, path, f"unknown keys in x-flatbuffers root metadata: {sorted(unknown)}")

    ensure("root_type" in obj, path, "x-flatbuffers root metadata missing 'root_type'")
    ensure_type(obj["root_type"], str, path + ["root_type"], "x-flatbuffers.root_type must be a string")
    if "file_identifier" in obj:
        ensure_type(obj["file_identifier"], str, path + ["file_identifier"], "x-flatbuffers.file_identifier must be a string")
        ensure(len(obj["file_identifier"]) == 4, path + ["file_identifier"], "x-flatbuffers.file_identifier must be 4 characters")
    if "file_extension" in obj:
        ensure_type(obj["file_extension"], str, path + ["file_extension"], "x-flatbuffers.file_extension must be a string")


def validate_xflatbuffers(obj, path):
    # Prefer structural classification (so this works even if metadata moves).
    if isinstance(obj, dict) and "root_type" in obj:
        validate_root_meta(obj, path)
        return
    if isinstance(obj, dict) and "type" in obj and "presence" in obj:
        validate_field_meta(obj, path)
        return
    if isinstance(obj, dict) and obj.get("kind") in {"enum", "union"}:
        validate_enum_def_meta(obj, path)
        return
    if isinstance(obj, dict) and obj.get("kind") in {"struct", "table"}:
        validate_struct_def_meta(obj, path)
        return
    if isinstance(obj, dict) and "base" in obj:
        validate_type_meta(obj, path)
        return
    fail(path, "unrecognized x-flatbuffers metadata object")


def walk(obj, path):
    if isinstance(obj, dict):
        if "x-flatbuffers" in obj:
            validate_xflatbuffers(obj["x-flatbuffers"], path + ["x-flatbuffers"])
        for k, v in obj.items():
            walk(v, path + [k])
    elif isinstance(obj, list):
        for i, v in enumerate(obj):
            walk(v, path + [i])


walk(schema, [])
PY
}

validate_xflatbuffers_metadata_dir() {
  local out_dir="$1"
  for golden in "${golden_files[@]}"; do
    validate_xflatbuffers_metadata_file "${out_dir}/${golden}"
  done
}

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

compare_output_against_dir() {
  local expected_dir="$1"
  local out_dir="$2"
  for golden in "${golden_files[@]}"; do
    local expected="${expected_dir}/${golden}"
    local generated="${out_dir}/${golden}"
    if ! diff -u "${expected}" "${generated}"; then
      echo "JSON Schema mismatch for ${golden}" >&2
      exit 1
    fi
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
  validate_xflatbuffers_metadata_dir "${out_dir}"
}

run_case_roundtrip_golden() {
  local label="$1"
  local out_dir="$2"
  shift 2

  echo "Round-tripping JSON Schemas (${label})"
  rm -rf "${out_dir}"
  mkdir -p "${out_dir}"
  ( cd "${script_dir}" && "${flatc}" "$@" -o "${out_dir}" "${golden_files[@]}" )
  compare_output "${out_dir}"
}

run_case_roundtrip_xflatbuffers() {
  local label="$1"
  local gen_dir="$2"
  local out_dir="$3"
  shift 3

  echo "Round-tripping JSON Schemas (${label})"
  rm -rf "${gen_dir}" "${out_dir}"
  mkdir -p "${gen_dir}" "${out_dir}"

  ( cd "${script_dir}" && "${flatc}" "$@" "${include_flags[@]}" -o "${gen_dir}" "${schemas[@]}" )
  ( cd "${gen_dir}" && "${flatc}" "$@" -o "${out_dir}" "${golden_files[@]}" )

  compare_output_against_dir "${gen_dir}" "${out_dir}"
  validate_xflatbuffers_metadata_dir "${gen_dir}"
  validate_xflatbuffers_metadata_dir "${out_dir}"
}

tmp_default="$(mktemp -d)"
tmp_preserve="$(mktemp -d)"
tmp_xflatbuffers_default="$(mktemp -d)"
tmp_xflatbuffers_preserve="$(mktemp -d)"
tmp_roundtrip_default="$(mktemp -d)"
tmp_roundtrip_xfb_gen="$(mktemp -d)"
tmp_roundtrip_xfb="$(mktemp -d)"
cleanup() {
  rm -rf "${tmp_default}" "${tmp_preserve}" \
    "${tmp_xflatbuffers_default}" "${tmp_xflatbuffers_preserve}" \
    "${tmp_roundtrip_default}" "${tmp_roundtrip_xfb_gen}" "${tmp_roundtrip_xfb}"
}
trap cleanup EXIT

run_case_diff "default naming" "${tmp_default}" --jsonschema
run_case_diff "preserve-case naming" "${tmp_preserve}" --jsonschema --preserve-case
run_case_xflatbuffers "x-flatbuffers metadata" "${tmp_xflatbuffers_default}" --jsonschema --jsonschema-xflatbuffers
run_case_xflatbuffers "x-flatbuffers metadata + preserve-case" "${tmp_xflatbuffers_preserve}" --jsonschema --jsonschema-xflatbuffers --preserve-case
run_case_roundtrip_golden "goldens" "${tmp_roundtrip_default}" --jsonschema
run_case_roundtrip_xflatbuffers "x-flatbuffers metadata" "${tmp_roundtrip_xfb_gen}" "${tmp_roundtrip_xfb}" --jsonschema --jsonschema-xflatbuffers

echo "JSON Schema tests (generation + roundtrip) passed"
