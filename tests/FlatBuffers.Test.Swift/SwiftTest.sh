swift_dir=`pwd`
cd ..
test_dir=`pwd`
alias fbc='${test_dir}/../flatc'
shopt -s expand_aliases

cd ${swift_dir}/Tests/FlatBuffers.Test.SwiftTests
fbc --swift --gen-mutable --grpc --gen-json-emit --gen-object-api -I ${test_dir}/include_test ${test_dir}/monster_test.fbs ${test_dir}/union_vector/union_vector.fbs
fbc --swift --gen-json-emit ${test_dir}/optional_scalars.fbs
fbc --swift --gen-json-emit --gen-object-api ${test_dir}/more_defaults.fbs
fbc --swift --gen-json-emit --gen-mutable --gen-object-api ${test_dir}/MutatingBool.fbs
fbc --swift ${test_dir}/vector_has_test.fbs
cd ${swift_dir}

# Goes into the code generation tests
cd CodeGenerationTests
fbc --swift --gen-mutable --grpc --gen-json-emit --gen-object-api --swift-implementation-only test_import.fbs
fbc --swift --gen-mutable --grpc --gen-json-emit --gen-object-api --no-includes test_no_include.fbs
cd ..

cd ${swift_dir}/Sources/SwiftFlatBuffers
# create better fuzzing test file
fbc --swift --gen-json-emit fuzzer.fbs
cd ${swift_dir}

cd ${test_dir}/Flatbuffers.Test.Swift.WASM/Tests/FlatBuffers.Test.Swift.WASMTests
fbc --swift --gen-mutable --gen-json-emit --gen-object-api -I ${test_dir}/include_test ${test_dir}/monster_test.fbs
cd ${swift_dir}

swift build --build-tests
swift test

if [ $(uname -s) != Darwin ]; then
  echo fuzzing
  swift build -c debug -Xswiftc -sanitize=fuzzer,address -Xswiftc -parse-as-library
  swift build -c release -Xswiftc -sanitize=fuzzer,address -Xswiftc -parse-as-library
fi
