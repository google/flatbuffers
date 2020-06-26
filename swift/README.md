FlatBuffers swift can be found in both SPM

`.package(url: "https://github.com/mustiikhalil/flatbuffers.git", from: "X.Y.Z"),`

and Cocoapods

`pod 'FlatBuffers'`

### Notes

1- To report any error please use the main repository.

2- `0.6.0` deprecates `add(condition:bool)` for `add(element:bool)`. You can download the [binary here](https://github.com/google/flatbuffers/actions) and select the latest push to master

### Contribute

1- Always run `swift test --generate-linuxmain` whenever new test functions are added or removed