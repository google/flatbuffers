#ifndef TESTS_PROTO_TEST_H
#define TESTS_PROTO_TEST_H

#include <string>

namespace flatbuffers {
namespace tests {

void ParseProtoTest(const std::string& tests_data_path);
void ParseProtoTestWithSuffix(const std::string& tests_data_path);
void ParseProtoTestWithIncludes(const std::string& tests_data_path);
void ParseProtoBufAsciiTest();


}  // namespace tests
}  // namespace flatbuffers

#endif