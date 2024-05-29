This directory is not intended to be used independently of the flatbuffers
repository. Instead, this whole directory serves as a unit test for the
`rules_js` integration in the flatbuffers repo.

Run this test from the top-level of the flatbuffers repo.
```console
$ bazel test //tests/ts:bazel_repository_test
```
