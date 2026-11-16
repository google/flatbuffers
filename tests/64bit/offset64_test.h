#ifndef TESTS_64BIT_OFFSET64_TEST_H
#define TESTS_64BIT_OFFSET64_TEST_H

#include <string>

namespace flatbuffers {
namespace tests {

void Offset64Test();
void Offset64SerializedFirst();
void Offset64NestedFlatBuffer();
void Offset64CreateDirect();
void Offset64Evolution();
void Offset64VectorOfStructs();
void Offset64SizePrefix();
void Offset64ManyVectors();
void Offset64ForceAlign();
void Offset64GenTextTest(const std::string& tests_data_path);

}  // namespace tests
}  // namespace flatbuffers

#endif  // TESTS_64BIT_OFFSET64_TEST_H
