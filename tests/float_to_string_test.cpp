#include "float_to_string_test.h"

#include "flatbuffers/util.h"
#include "test_assert.h"

namespace flatbuffers {
namespace tests {

void FloatToStringTest() {
  // Test various float to string conversions
  struct FloatTestCase {
    float value;
    std::string expected;
  };

  struct DoubleTestCase {
    double value;
    std::string expected;
  };

  // floats
  std::array float_test_cases = {
      FloatTestCase{0.0f, "0.0"},
      FloatTestCase{-0.0f, "-0.0"},
      FloatTestCase{1.0f, "1.0"},
      FloatTestCase{-1.0f, "-1.0"},
      //   FloatTestCase{123.456f, "123.456"},
      //   FloatTestCase{-123.456f, "-123.456"},
      //   FloatTestCase{1.23456e10f, "12345600000.0"},
      //   FloatTestCase{-1.23456e10f, "-12345600000.0"},
      //   FloatTestCase{1.23456e-10f, "0.000000000123456"},
      //   FloatTestCase{-1.23456e-10f, "-0.000000000123456"},
      // infinity
      FloatTestCase{std::numeric_limits<float>::infinity(), "inf"},
      FloatTestCase{-std::numeric_limits<float>::infinity(), "-inf"},
      // nan
      FloatTestCase{std::numeric_limits<float>::quiet_NaN(), "nan"},
  };

  // doubles
  std::array double_test_cases = {
      DoubleTestCase{0.0, "0.0"},
      DoubleTestCase{-0.0, "-0.0"},
      DoubleTestCase{1.0, "1.0"},
      DoubleTestCase{-1.0, "-1.0"},
      //   DoubleTestCase{123.456, "123.456"},
      //   DoubleTestCase{-123.456, "-123.456"},
      //   DoubleTestCase{1.23456e10, "12345600000.0"},
      //   DoubleTestCase{-1.23456e10, "-12345600000.0"},
      //   DoubleTestCase{1.23456e-10, "0.000000000123456"},
      //   DoubleTestCase{-1.23456e-10, "-0.000000000123456"},
      // infinity
      DoubleTestCase{std::numeric_limits<double>::infinity(), "inf"},
      DoubleTestCase{-std::numeric_limits<double>::infinity(), "-inf"},
      // nan
      DoubleTestCase{std::numeric_limits<double>::quiet_NaN(), "nan"},
      // edge case from issue
      DoubleTestCase{6696.133544400395, "6696.133544400395"},
  };

  for (const auto& test : float_test_cases) {
    std::string result = NumToString(test.value);
    TEST_EQ(test.expected, result);
  }

  for (const auto& test : double_test_cases) {
    std::string result = NumToString(test.value);
    TEST_EQ(test.expected, result);
  }
}

}  // namespace tests
}  // namespace flatbuffers
