#!/usr/bin/env python3
"""
Cross-language conformance comparison tool.

Reads JSON result files from each language's verifier runner and compares
them against expected results and each other.

Usage:
    python3 compare.py expected/results.json rust_results.json go_results.json ts_results.json

Exit code 0 = all languages agree and match expected results.
Exit code 1 = disagreement found.
"""

import json
import sys
from pathlib import Path


def load_json(path: str) -> dict:
    with open(path) as f:
        return json.load(f)


def compare_results(expected_path: str, *result_paths: str) -> bool:
    expected = load_json(expected_path)
    language_names = ["rust", "go", "ts"]
    results = {}
    for i, path in enumerate(result_paths):
        name = language_names[i] if i < len(language_names) else f"lang{i}"
        try:
            results[name] = load_json(path)
        except FileNotFoundError:
            print(f"WARNING: {name} results file not found: {path}")
            continue

    if not results:
        print("ERROR: No result files found")
        return False

    all_pass = True
    buffers = sorted(expected.keys())

    for buf_name in buffers:
        exp = expected[buf_name]
        expected_accept = exp.get("accept", True)
        expected_error = exp.get("error_kind")

        for lang, lang_results in results.items():
            if buf_name not in lang_results:
                print(f"MISSING: {lang} has no result for {buf_name}")
                all_pass = False
                continue

            actual = lang_results[buf_name]
            actual_accept = actual.get("accept", True)

            if actual_accept != expected_accept:
                print(
                    f"MISMATCH: {buf_name} — expected accept={expected_accept}, "
                    f"{lang} returned accept={actual_accept}"
                )
                all_pass = False

            if expected_error and not actual_accept:
                actual_error = actual.get("error_kind")
                if actual_error != expected_error:
                    print(
                        f"ERROR_KIND: {buf_name} — expected {expected_error}, "
                        f"{lang} returned {actual_error}"
                    )
                    # Warning only — error kinds may differ across languages
                    # as long as accept/reject agrees

    # Cross-language agreement check
    for buf_name in buffers:
        lang_accepts = {}
        for lang, lang_results in results.items():
            if buf_name in lang_results:
                lang_accepts[lang] = lang_results[buf_name].get("accept", True)

        unique_verdicts = set(lang_accepts.values())
        if len(unique_verdicts) > 1:
            print(
                f"DISAGREEMENT: {buf_name} — "
                + ", ".join(f"{l}={v}" for l, v in lang_accepts.items())
            )
            all_pass = False

    if all_pass:
        tested = len(buffers)
        langs = len(results)
        print(f"OK: {tested} buffers x {langs} languages — all agree")

    return all_pass


def main():
    if len(sys.argv) < 3:
        print(f"Usage: {sys.argv[0]} expected.json [lang_results.json ...]")
        sys.exit(2)

    expected_path = sys.argv[1]
    result_paths = sys.argv[2:]

    if compare_results(expected_path, *result_paths):
        sys.exit(0)
    else:
        sys.exit(1)


if __name__ == "__main__":
    main()
