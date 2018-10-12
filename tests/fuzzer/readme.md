## libFuzzer
https://llvm.org/docs/LibFuzzer.html

## Run fuzzer

Flag `-only_ascii=1` is useful for fast number-compatibility checking.

Additional flags: `-reduce_depth=1 -use_value_profile=1 -shrink=1`

Help: `-help=1`

Example:
`./scalar_fuzzer -only_ascii=0 -reduce_depth=1 -use_value_profile=1 -shrink=1 -max_len=3000 -timeout=10 -rss_limit_mb=2048 -jobs=2 ../.corpus/ ../.seed/`

Merge several seeds:
`./scalar_fuzzer -merge=1 ../.corpus/ ../.seed_1/ ../.seed_2/`

## Know limitations
- LLVM 7.0 std::regex have problem with stack overflow, maximum length should be limited to 3000.
  Example: `./scalar_fuzzer -max_len=3000 -timeout=10 ../.corpus/ ../.seed/`

## Test with locales
To run fuzzer test with selected C-locale pass `-DFUZZ_TEST_LOCALE="<locale name>"` to CMake when configuring.

Command line:
```sh
cmake .. -DFUZZ_TEST_LOCALE="ru_RU.CP1251"
```

Use `cmake.configureSettings` section of workspace settings:
```json
"cmake.configureSettings": {
  "FUZZ_TEST_LOCALE" : "ru_RU.CP1251"
}
```
