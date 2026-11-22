#!/usr/bin/env bash
#
# Exercises the flatbuffers CMake package's static/shared packaging story:
# a single canonical flatbuffers::flatbuffers target whose type is resolved
# at find_package() time via COMPONENTS, FlatBuffers_SHARED_LIBS,
# BUILD_SHARED_LIBS, and (failing all of those) whichever variant is
# actually installed. See flatbuffers-config.cmake.
set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
CONSUMER_DIR="${ROOT}/tests/CMakeConsumerTest"
WORK="$(mktemp -d)"
trap 'rm -rf "${WORK}"' EXIT

NPROC="$(getconf _NPROCESSORS_ONLN 2>/dev/null || echo 2)"

build_and_install() {
  local name="$1" install_prefix="$2"
  shift 2
  local build_dir="${WORK}/${name}-build"
  cmake -S "${ROOT}" -B "${build_dir}" \
    -DCMAKE_INSTALL_PREFIX="${install_prefix}" \
    -DFLATBUFFERS_BUILD_TESTS=OFF \
    -DFLATBUFFERS_BUILD_FLATC=OFF \
    -DFLATBUFFERS_BUILD_FLATHASH=OFF \
    "$@" \
    > "${build_dir}.configure.log" 2>&1
  cmake --build "${build_dir}" --target flatbuffers -j"${NPROC}" \
    > "${build_dir}.build.log" 2>&1
  cmake --install "${build_dir}" > "${build_dir}.install.log" 2>&1
}

# configure_consumer PREFIX [extra cmake args...]
# Leaves the consumer build directory in $CONSUMER_BUILD_DIR on success.
configure_consumer() {
  local prefix="$1"
  shift
  CONSUMER_BUILD_DIR="${WORK}/consumer-$$-${RANDOM}"
  cmake -S "${CONSUMER_DIR}" -B "${CONSUMER_BUILD_DIR}" \
    -DCMAKE_PREFIX_PATH="${prefix}" \
    "$@" \
    > "${CONSUMER_BUILD_DIR}.log" 2>&1
}

fail() {
  echo "FAIL: $1" >&2
  echo "--- log: ${2:-} ---" >&2
  [ -n "${2:-}" ] && cat "$2" >&2
  exit 1
}

# expect_type PREFIX WANT_TYPE [extra cmake args...]
# Configures and builds the consumer; asserts flatbuffers::flatbuffers
# resolved to WANT_TYPE (STATIC_LIBRARY or SHARED_LIBRARY) and that the
# resulting binaries run.
expect_type() {
  local prefix="$1" want_type="$2"
  shift 2
  echo "-- expect ${want_type}: prefix=${prefix} $* --"
  if ! configure_consumer "${prefix}" "$@"; then
    fail "expected successful configure" "${CONSUMER_BUILD_DIR}.log"
  fi
  if ! grep -q "TYPE = ${want_type}" "${CONSUMER_BUILD_DIR}.log"; then
    fail "expected TYPE = ${want_type}" "${CONSUMER_BUILD_DIR}.log"
  fi
  if ! cmake --build "${CONSUMER_BUILD_DIR}" >> "${CONSUMER_BUILD_DIR}.log" 2>&1; then
    fail "expected successful build" "${CONSUMER_BUILD_DIR}.log"
  fi
  "${CONSUMER_BUILD_DIR}/consumer_canonical" \
    || fail "consumer_canonical did not run" "${CONSUMER_BUILD_DIR}.log"
  if [ -x "${CONSUMER_BUILD_DIR}/consumer_legacy_shared" ]; then
    "${CONSUMER_BUILD_DIR}/consumer_legacy_shared" \
      || fail "consumer_legacy_shared did not run" "${CONSUMER_BUILD_DIR}.log"
  fi
}

# expect_not_found PREFIX [extra cmake args...]
# Asserts find_package(FlatBuffers) fails (not just any configure error).
expect_not_found() {
  local prefix="$1"
  shift
  echo "-- expect NOT_FOUND: prefix=${prefix} $* --"
  if configure_consumer "${prefix}" "$@"; then
    fail "expected configure to fail, but it succeeded" "${CONSUMER_BUILD_DIR}.log"
  fi
  if ! grep -q "considered to be NOT FOUND" "${CONSUMER_BUILD_DIR}.log"; then
    fail "expected a FlatBuffers NOT_FOUND message" "${CONSUMER_BUILD_DIR}.log"
  fi
}

echo "=== Building static-only package ==="
STATIC_INSTALL="${WORK}/static-install"
build_and_install static "${STATIC_INSTALL}"

echo "=== Building shared-only package (-DBUILD_SHARED_LIBS=ON) ==="
SHARED_INSTALL="${WORK}/shared-install"
build_and_install shared "${SHARED_INSTALL}" -DBUILD_SHARED_LIBS=ON

echo "=== Building shared-only package via deprecated Fedora-style flags ==="
# Matches Fedora's flatbuffers.spec exactly: -DFLATBUFFERS_BUILD_SHAREDLIB=ON
# -DFLATBUFFERS_BUILD_FLATLIB=OFF. The deprecation shim must still produce a
# working flatbuffers::flatbuffers (shared) with no spec changes required.
LEGACY_SHARED_INSTALL="${WORK}/legacy-shared-install"
build_and_install legacy-shared "${LEGACY_SHARED_INSTALL}" \
  -DFLATBUFFERS_BUILD_SHAREDLIB=ON -DFLATBUFFERS_BUILD_FLATLIB=OFF

# --- Precedence chain: static-only install ---
expect_type "${STATIC_INSTALL}" STATIC_LIBRARY
expect_type "${STATIC_INSTALL}" STATIC_LIBRARY -DFlatBuffers_SHARED_LIBS=OFF
expect_type "${STATIC_INSTALL}" STATIC_LIBRARY -DFLATBUFFERS_CONSUMER_TEST_COMPONENTS=static
expect_type "${STATIC_INSTALL}" STATIC_LIBRARY -DBUILD_SHARED_LIBS=ON  # falls back: no shared installed
expect_not_found "${STATIC_INSTALL}" -DFlatBuffers_SHARED_LIBS=ON
expect_not_found "${STATIC_INSTALL}" -DFLATBUFFERS_CONSUMER_TEST_COMPONENTS=shared
expect_not_found "${STATIC_INSTALL}" -DFLATBUFFERS_CONSUMER_TEST_COMPONENTS="static;shared"
expect_not_found "${STATIC_INSTALL}" -DFLATBUFFERS_CONSUMER_TEST_COMPONENTS=bogus

# --- Precedence chain: shared-only install ---
expect_type "${SHARED_INSTALL}" SHARED_LIBRARY
expect_type "${SHARED_INSTALL}" SHARED_LIBRARY -DFlatBuffers_SHARED_LIBS=ON
expect_type "${SHARED_INSTALL}" SHARED_LIBRARY -DFLATBUFFERS_CONSUMER_TEST_COMPONENTS=shared
expect_type "${SHARED_INSTALL}" SHARED_LIBRARY -DBUILD_SHARED_LIBS=OFF  # falls back: no static installed
expect_not_found "${SHARED_INSTALL}" -DFlatBuffers_SHARED_LIBS=OFF
expect_not_found "${SHARED_INSTALL}" -DFLATBUFFERS_CONSUMER_TEST_COMPONENTS=static

# --- Deprecation shim produces a working shared install ---
expect_type "${LEGACY_SHARED_INSTALL}" SHARED_LIBRARY

echo "=== All CMake packaging scenarios passed ==="
