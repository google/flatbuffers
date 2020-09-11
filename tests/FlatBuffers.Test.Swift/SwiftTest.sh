swift_dir=`pwd`
cd ..
test_dir=`pwd`

cd ${swift_dir}/Tests/FlatBuffers.Test.SwiftTests
${test_dir}/../flatc --swift --gen-mutable --grpc --gen-object-api -I ${test_dir}/include_test ${test_dir}/monster_test.fbs ${test_dir}/union_vector/union_vector.fbs
${test_dir}/../flatc --swift ${test_dir}/optional_scalars.fbs
cd ${swift_dir}
swift build --build-tests
swift test
