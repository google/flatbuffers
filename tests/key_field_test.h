#ifndef TESTS_KEY_FIELD_TEST_H
#define TESTS_KEY_FIELD_TEST_H

namespace flatbuffers {
namespace tests {

void CallLookupByKeyOnString();
void FixedSizedScalarKeyInStructTest();
void StructKeyInStructTest();
void NestedStructKeyInStructTest();
void FixedSizedStructArrayKeyInStructTest();

}  // namespace tests
}  // namespace flatbuffers

#endif
