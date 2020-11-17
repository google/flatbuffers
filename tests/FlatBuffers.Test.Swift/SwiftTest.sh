swift_dir=`pwd`
cd ..
test_dir=`pwd`
alias fbc='${test_dir}/../debug/flatc'

cd FlatBuffers.GRPC.Swift/Sources/Model
fbc --swift --grpc greeter.fbs
cd ${test_dir}

cd ${swift_dir}/Tests/FlatBuffers.Test.SwiftTests
fbc --swift --gen-mutable --grpc --gen-object-api -I ${test_dir}/include_test ${test_dir}/monster_test.fbs ${test_dir}/union_vector/union_vector.fbs
fbc --swift ${test_dir}/optional_scalars.fbs
cd ${swift_dir}
swift build --build-tests
swift test
