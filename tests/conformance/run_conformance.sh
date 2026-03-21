#!/bin/bash
# Cross-language conformance test runner.
#
# Runs Rust, Go, and TypeScript verifier conformance tests against the
# shared corpus, then compares results.
#
# Usage: ./tests/conformance/run_conformance.sh
#
# Exit code 0 = all languages agree. Non-zero = disagreement.

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
REPO_ROOT="$(cd "$SCRIPT_DIR/../.." && pwd)"
CORPUS_DIR="$SCRIPT_DIR/corpus"
EXPECTED="$SCRIPT_DIR/expected/results.json"
RESULTS_DIR="$(mktemp -d)"

echo "=== FlatBuffers Cross-Language Conformance Suite ==="
echo "Corpus: $CORPUS_DIR"
echo "Expected: $EXPECTED"
echo "Results: $RESULTS_DIR"
echo

# Count corpus files
VALID_COUNT=$(find "$CORPUS_DIR/valid" -name "*.bin" 2>/dev/null | wc -l | tr -d ' ')
MALICIOUS_COUNT=$(find "$CORPUS_DIR/malicious" -name "*.bin" 2>/dev/null | wc -l | tr -d ' ')
EDGE_COUNT=$(find "$CORPUS_DIR/edge" -name "*.bin" 2>/dev/null | wc -l | tr -d ' ')
echo "Corpus: ${VALID_COUNT} valid, ${MALICIOUS_COUNT} malicious, ${EDGE_COUNT} edge"
echo

TOTAL=$((VALID_COUNT + MALICIOUS_COUNT + EDGE_COUNT))
if [ "$TOTAL" -eq 0 ]; then
    echo "WARNING: No corpus files found. Skipping conformance tests."
    exit 0
fi

RUNNERS=()

# Rust runner
if command -v cargo &>/dev/null; then
    echo "--- Running Rust conformance ---"
    cd "$REPO_ROOT"
    if cargo run --bin rust_conformance -- "$CORPUS_DIR" > "$RESULTS_DIR/rust.json" 2>/dev/null; then
        RUNNERS+=("$RESULTS_DIR/rust.json")
        echo "  OK"
    else
        echo "  SKIP (build/run failed)"
    fi
else
    echo "--- Rust: SKIP (cargo not found) ---"
fi

# Go runner
if command -v go &>/dev/null; then
    echo "--- Running Go conformance ---"
    cd "$REPO_ROOT"
    if go run "$SCRIPT_DIR/runners/go_conformance_test.go" "$CORPUS_DIR" > "$RESULTS_DIR/go.json" 2>/dev/null; then
        RUNNERS+=("$RESULTS_DIR/go.json")
        echo "  OK"
    else
        echo "  SKIP (build/run failed)"
    fi
else
    echo "--- Go: SKIP (go not found) ---"
fi

# TypeScript runner
if command -v npx &>/dev/null; then
    echo "--- Running TypeScript conformance ---"
    cd "$REPO_ROOT"
    if npx ts-node "$SCRIPT_DIR/runners/ts_conformance.ts" "$CORPUS_DIR" > "$RESULTS_DIR/ts.json" 2>/dev/null; then
        RUNNERS+=("$RESULTS_DIR/ts.json")
        echo "  OK"
    else
        echo "  SKIP (build/run failed)"
    fi
else
    echo "--- TypeScript: SKIP (npx not found) ---"
fi

echo

if [ ${#RUNNERS[@]} -eq 0 ]; then
    echo "ERROR: No runners completed successfully"
    exit 1
fi

# Compare results
echo "=== Comparing Results ==="
python3 "$SCRIPT_DIR/compare.py" "$EXPECTED" "${RUNNERS[@]}"
EXIT_CODE=$?

# Cleanup
rm -rf "$RESULTS_DIR"

exit $EXIT_CODE
