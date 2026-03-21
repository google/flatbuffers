#!/usr/bin/env bash
# Golden-file codegen test for Go and TS verifier generation.
# Regenerates code from monster_test.fbs and compares against golden files.
# Usage: ./tests/codegen_golden/check_golden.sh [path_to_flatc]

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
REPO_ROOT="$(cd "$SCRIPT_DIR/../.." && pwd)"
FLATC="${1:-$REPO_ROOT/build/flatc}"
TMPDIR="$(mktemp -d)"
trap 'rm -rf "$TMPDIR"' EXIT

FAIL=0

echo "=== Golden-file codegen test ==="
echo "flatc: $FLATC"
echo "$($FLATC --version)"
echo ""

# Generate Go code
echo "--- Generating Go ---"
"$FLATC" --go -o "$TMPDIR/go/" -I "$REPO_ROOT/tests/" -I "$REPO_ROOT/tests/include_test/" \
  "$REPO_ROOT/tests/monster_test.fbs"

# Compare Go golden files
for golden in "$SCRIPT_DIR/go/"*.go; do
  filename="$(basename "$golden")"
  # Determine generated path
  case "$filename" in
    InParentNamespace.go)
      generated="$TMPDIR/go/MyGame/$filename"
      ;;
    *)
      generated="$TMPDIR/go/MyGame/Example/$filename"
      ;;
  esac

  if [ ! -f "$generated" ]; then
    echo "FAIL: $filename not generated"
    FAIL=1
    continue
  fi

  if diff -q "$golden" "$generated" > /dev/null 2>&1; then
    echo "PASS: go/$filename"
  else
    echo "FAIL: go/$filename differs from golden"
    diff -u "$golden" "$generated" | head -40
    FAIL=1
  fi
done

# Generate TS code
echo ""
echo "--- Generating TypeScript ---"
"$FLATC" --ts -o "$TMPDIR/ts/" -I "$REPO_ROOT/tests/" -I "$REPO_ROOT/tests/include_test/" \
  "$REPO_ROOT/tests/monster_test.fbs"

# Compare TS golden files
for golden in "$SCRIPT_DIR/ts/"*.ts; do
  filename="$(basename "$golden")"
  case "$filename" in
    in-parent-namespace.ts)
      generated="$TMPDIR/ts/my-game/$filename"
      ;;
    *)
      generated="$TMPDIR/ts/my-game/example/$filename"
      ;;
  esac

  if [ ! -f "$generated" ]; then
    echo "FAIL: $filename not generated"
    FAIL=1
    continue
  fi

  if diff -q "$golden" "$generated" > /dev/null 2>&1; then
    echo "PASS: ts/$filename"
  else
    echo "FAIL: ts/$filename differs from golden"
    diff -u "$golden" "$generated" | head -40
    FAIL=1
  fi
done

echo ""
if [ $FAIL -eq 0 ]; then
  echo "All golden-file tests PASSED"
else
  echo "Some golden-file tests FAILED"
  exit 1
fi
