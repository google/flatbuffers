#!/bin/bash
# This is a script used by the typescript flatbuffer bazel rules to compile
# a flatbuffer schema (.fbs file) to typescript and then use esbuild to
# generate a single output.
# Note: This uses flatc --file-names-only to determine the generated main entry
# point file to pass to esbuild.
# --- begin runfiles.bash initialization v2 ---
# Copy-pasted from the Bazel Bash runfiles library v2.
set -uo pipefail; set +e; f=bazel_tools/tools/bash/runfiles/runfiles.bash
source "${RUNFILES_DIR:-/dev/null}/$f" 2>/dev/null || \
  source "$(grep -sm1 "^$f " "${RUNFILES_MANIFEST_FILE:-/dev/null}" | cut -f2- -d' ')" 2>/dev/null || \
  source "$0.runfiles/$f" 2>/dev/null || \
  source "$(grep -sm1 "^$f " "$0.runfiles_manifest" | cut -f2- -d' ')" 2>/dev/null || \
  source "$(grep -sm1 "^$f " "$0.exe.runfiles_manifest" | cut -f2- -d' ')" 2>/dev/null || \
  { echo>&2 "ERROR: cannot find $f"; exit 1; }; f=; set -e
# --- end runfiles.bash initialization v2 ---
set -eu
runfiles_export_envvars
FLATC=$(rlocation com_github_google_flatbuffers/flatc)
# The schema file (.fbs) is the last argument passed to flatc ($@).
FBS_FILE="${!#}"
SCHEMA_BASE=$(basename "${FBS_FILE}" .fbs)
${FLATC} "$@"
# flatc no longer prints generated entry point paths to console output when
# compiling. Use --file-names-only to discover the generated .ts entry point.
TS_FILE=$(${FLATC} --file-names-only "$@" 2>/dev/null | grep "/${SCHEMA_BASE}\(_[a-zA-Z0-9_-]*\)\?\.ts$" | head -n 1)
export PATH="$(rlocation nodejs_linux_amd64/bin/nodejs/bin):${PATH}"
${ESBUILD_BIN} "${TS_FILE}" --format=cjs --bundle --outfile="${OUTPUT_FILE}"  --external:flatbuffers --log-level=warning
