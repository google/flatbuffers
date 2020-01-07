swift_dir=`pwd`
cd ..
test_dir=`pwd`

${test_dir}/../flatc --swift --gen-mutable -I ${test_dir}/include_test ${test_dir}/monster_test.fbs ${test_dir}/union_vector/union_vector.fbs
cd ${test_dir}
mv *_generated.swift ${swift_dir}/Tests/FlatBuffers.Test.SwiftTests
cd ${swift_dir}
swift build --build-tests
swift test