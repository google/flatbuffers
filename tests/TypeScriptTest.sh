#!/usr/bin/env bash
set -eu

script_dir="$(cd "$(dirname "$0")" && pwd)"
repo_dir="$(cd "${script_dir}/.." && pwd)"
flatc="${repo_dir}/flatc"

if ! command -v node >/dev/null 2>&1; then
  echo "Skipping TypeScript tests: node executable not found." >&2
  exit 0
fi

if ! command -v npm >/dev/null 2>&1; then
  echo "Skipping TypeScript tests: npm executable not found." >&2
  exit 0
fi

if [[ ! -x "${flatc}" ]]; then
  echo "Skipping TypeScript tests: flatc executable not found at ${flatc}." >&2
  exit 0
fi

(cd "${script_dir}/ts" && python3 TypeScriptTest.py)
