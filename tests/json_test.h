#ifndef TESTS_JSON_TEST_H
#define TESTS_JSON_TEST_H

#include <string>

namespace flatbuffers {
namespace tests {

void JsonDefaultTest(const std::string& tests_data_path);
void JsonEnumsTest(const std::string& tests_data_path);
void JsonOptionalTest(const std::string& tests_data_path, bool default_scalars);

}  // namespace tests
}  // namespace flatbuffers

#endif