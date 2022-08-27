#ifndef TESTS_PARSER_TEST_H
#define TESTS_PARSER_TEST_H

namespace flatbuffers {
namespace tests {

void ErrorTest();
void EnumOutOfRangeTest();
void IntegerOutOfRangeTest();
void InvalidFloatTest();
void UnicodeInvalidSurrogatesTest();
void InvalidUTF8Test();

}  // namespace tests
}  // namespace flatbuffers

#endif // TESTS_PARSER_TEST_H