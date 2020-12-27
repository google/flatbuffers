FlatBuffers swift can be found in both SPM

`.package(url: "https://github.com/mustiikhalil/flatbuffers.git", from: "X.Y.Z"),`

and Cocoapods

`pod 'FlatBuffers'`

### Notes

1- To report any error please use the main repository.

2- `1.0.0` deprecates `MyGame_Example_Vec3.createVec3(builder: &fbb, x: 10, test2: .blue)` for `MyGame_Example_Vec3(x: 10, test2: .blue)`. This uses Swift native structs instead of workarounds that which leads to a huge performance increase when serializing structs. You can download the [binary here](https://github.com/google/flatbuffers/actions) and select the latest push to master

### Contribute

1- Always run `swift test --generate-linuxmain` whenever new test functions are added or removed