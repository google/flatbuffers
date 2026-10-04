#include "scalar_test.h"

#include <cstring>

#include "flatbuffers/buffer.h"
#include "test_assert.h"

namespace flatbuffers {
namespace tests {
namespace {

template <typename T, typename U>
void TestScalarAccess(T value, U expected_value,
                      const uint8_t* expected_bytes) {
  // The default path is tested at an aligned address. The memcpy path can
  // also access scalar bytes without alignment for U.
  uint64_t storage[4];
  uint8_t* bytes = reinterpret_cast<uint8_t*>(storage);
  size_t end = 9;
#if defined(FLATBUFFERS_STRICT_MEMCPY)
  end = 16;
#endif
  for (size_t offset = 8; offset < end; ++offset) {
    std::memset(storage, 0xA5, sizeof(storage));
    WriteScalar(bytes + offset, value);
    for (size_t i = 0; i < sizeof(storage); ++i) {
      const uint8_t expected = i >= offset && i < offset + sizeof(U)
                                   ? expected_bytes[i - offset]
                                   : 0xA5;
      TEST_EQ(expected, bytes[i]);
    }
    const U result = ReadScalar<U>(bytes + offset);
    // Compare representations so that negative zero and NaN payloads matter.
    TEST_EQ(0, std::memcmp(&expected_value, &result, sizeof(U)));
  }
}

template <typename T>
void TestScalarAccess(T value, const uint8_t* expected_bytes) {
  TestScalarAccess(value, value, expected_bytes);
}

}  // namespace

void ScalarAccessTest() {
  const uint8_t positive[] = {0xEF, 0xCD, 0xAB, 0x89, 0x67, 0x45, 0x23, 0x01};
  TestScalarAccess(static_cast<uint8_t>(0xEF), positive);
  TestScalarAccess(static_cast<uint16_t>(0xCDEF), positive);
  TestScalarAccess(static_cast<uint32_t>(0x89ABCDEF), positive);
  TestScalarAccess(UINT64_C(0x0123456789ABCDEF), positive);

  const uint8_t negative[] = {0xFE, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
  TestScalarAccess(static_cast<int8_t>(-2), negative);
  TestScalarAccess(static_cast<int16_t>(-2), negative);
  TestScalarAccess(static_cast<int32_t>(-2), negative);
  TestScalarAccess(static_cast<int64_t>(-2), negative);

  const uint8_t boolean[] = {1};
  TestScalarAccess(true, boolean);

  const uint8_t float_one[] = {0, 0, 0x80, 0x3F};
  const uint8_t double_one[] = {0, 0, 0, 0, 0, 0, 0xF0, 0x3F};
  TestScalarAccess(1.0f, float_one);
  TestScalarAccess(1.0, double_one);

  const uint8_t float_negative_zero[] = {0, 0, 0, 0x80};
  const uint8_t double_negative_zero[] = {0, 0, 0, 0, 0, 0, 0, 0x80};
  TestScalarAccess(-0.0f, float_negative_zero);
  TestScalarAccess(-0.0, double_negative_zero);

  const uint32_t nan_bits = 0x7FC01234;
  float nan_value;
  std::memcpy(&nan_value, &nan_bits, sizeof(nan_value));
  const uint8_t nan_bytes[] = {0x34, 0x12, 0xC0, 0x7F};
  TestScalarAccess(nan_value, nan_bytes);

  TestScalarAccess(Offset<void>(0x89ABCDEF), static_cast<uoffset_t>(0x89ABCDEF),
                   positive);
}

}  // namespace tests
}  // namespace flatbuffers
