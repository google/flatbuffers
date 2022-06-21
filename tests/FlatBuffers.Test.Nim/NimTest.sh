nim_dir=`pwd`
cd ..
test_dir=`pwd`
alias flatc='${test_dir}/../build/flatc'
shopt -s expand_aliases

cd ${nim_dir}/tests/
flatc --nim -I ${test_dir}/include_test ${test_dir}/monster_test.fbs ${test_dir}/union_vector/union_vector.fbs
flatc --nim ${test_dir}/optional_scalars.fbs
flatc --nim ${test_dir}/more_defaults.fbs
flatc --nim ${test_dir}/MutatingBool.fbs
flatc --nim ${test_dir}/vector_has_test.fbs
cd ${nim_dir}

# # Goes into the code generation tests
# cd CodeGenerationTests
# flatc --nim test_import.fbs
# flatc --nim --no-includes test_no_include.fbs
# cd ..

# cd ${nim_dir}/Sources/SwiftFlatBuffers
# # create better fuzzing test file
# flatc --nim fuzzer.fbs
# cd ${nim_dir}

# cd ${test_dir}/Flatbuffers.Test.Swift.WASM/Tests/FlatBuffers.Test.Swift.WASMTests
# flatc --nim -I ${test_dir}/include_test ${test_dir}/monster_test.fbs
# cd ${nim_dir}

# swift build --build-tests
# swift test

# if [ $(uname -s) != Darwin ]; then
#   echo fuzzing
#   swift build -c debug -Xswiftc -sanitize=fuzzer,address -Xswiftc -parse-as-library
#   swift build -c release -Xswiftc -sanitize=fuzzer,address -Xswiftc -parse-as-library
# fi

testament cat .
