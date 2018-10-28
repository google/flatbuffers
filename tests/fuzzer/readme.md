# Test Flatbuffers library with help of libFuzzer
Test suite of Flatbuffers library has fuzzer section with tests are based on libFuzzer library.

> LibFuzzer is in-process, coverage-guided, evolutionary fuzzing engine.
LibFuzzer is linked with the library under test, and feeds fuzzed inputs to the library via a specific fuzzing entrypoint (aka “target function”);
the fuzzer then tracks which areas of the code are reached, and generates mutations on the corpus of input data in order to maximize the code coverage.
The code coverage information for libFuzzer is provided by LLVM’s SanitizerCoverage instrumentation.

For details about **libFuzzer** see: https://llvm.org/docs/LibFuzzer.html

To build and run these tests LLVM compiler (with clang frontend) and CMake should be installed before.

The fuzzer section include three tests:
- `verifier_fuzzer` checks stability of deserialization engine for `Monster` schema;
- `parser_fuzzer` checks stability of schema and json parser under various inputs;
- `scalar_parser` focused on validation of the parser while parse numeric scalars in schema and/or json files;

## Build tests with locales
Flatbuffers library use only printable-ASCII characters as characters of grammar alphabet for type and data declaration.
This alphabet is fully compatible with JSON specification and make schema declaration fully portable.
Flatbuffers library is independent from global or thread locales used by end-user application.
To run fuzzer tests with selected C-locale under test pass `-DFUZZ_TEST_LOCALE="<locale name>"` to CMake when configuring.
Selected locale must be installed in system before use.
Command line:
```sh
cmake .. -DFUZZ_TEST_LOCALE="ru_RU.CP1251"
```
If use VSCode, use `cmake.configureSettings` section of workspace settings:
```json
"cmake.configureSettings": {
  "FUZZ_TEST_LOCALE" : "ru_RU.CP1251"
}
```

## Run fuzzer
These are examples of fuzzer run.
Flags may vary and depend from version of libFuzzer library.
For detail, run a fuzzer test with help flag: `./parser_fuzzer -help=1`

`./verifier_fuzzer -reduce_depth=1 -use_value_profile=1 -shrink=1 ../.corpus_verifier/`

`./parser_fuzzer -reduce_depth=1 -use_value_profile=1 -shrink=1 ../.corpus_parser/`

`./scalar_fuzzer -reduce_depth=1 -use_value_profile=1 -shrink=1 -max_len=3000 ../.corpus_parser/ ../.seed_parser/`

Flag `-only_ascii=1` is useful for fast number-compatibility checking while run `scalar_fuzzer`:

`./scalar_fuzzer -only_ascii=1 -reduce_depth=1 -use_value_profile=1 -shrink=1 -max_len=3000 -timeout=10 -rss_limit_mb=2048 -jobs=2 ../.corpus_parser/ ../.seed_parser/`

## Merge (minimize) corpus
The **libFuzzer** allow to filter (minimize) corpus with help of `-merge` flag:
> -merge
    If set to 1, any corpus inputs from the 2nd, 3rd etc. corpus directories that trigger new code coverage will be merged into the first corpus directory.
    Defaults to 0. This flag can be used to minimize a corpus.

Merge several seeds to one:
`./scalar_fuzzer -merge=1 ../.corpus/ ../.seed_1/ ../.seed_2/`

## Know limitations
- LLVM 7.0 std::regex library has problem with stack overflow, maximum length of input for `scalar_fuzzer` run should be limited to 3000.
  Example: `./scalar_fuzzer -max_len=3000`
