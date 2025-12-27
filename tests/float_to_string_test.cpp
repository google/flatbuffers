#include "float_to_string_test.h"

#include "flatbuffers/util.h"
#include "test_assert.h"

namespace flatbuffers {
namespace tests {

void FloatToStringRoundTripTest() {
  // convert an array of floats to strings and back, and check they are the same
  constexpr std::array<float, 13> floats_to_test = {
      0.0f,
      -0.0f,
      1.0f,
      -1.0f,
      123.456f,
      -123.456f,
      1.23456e10f,
      -1.23456e10f,
      std::numeric_limits<float>::infinity(),
      -std::numeric_limits<float>::infinity(),
      std::numeric_limits<float>::quiet_NaN(),
      // these don't pass the old method
      1.23456e-10f,
      -1.23456e-10f,
  };

  for (auto original_float : floats_to_test) {
    std::string float_str = NumToString(original_float);
    float parsed_float = 0.0f;
    bool success = StringToNumber(float_str.c_str(), &parsed_float);

    if (std::isnan(original_float)) {
      TEST_ASSERT(std::isnan(parsed_float));
    } else {
      TEST_ASSERT(success);
      TEST_EQ(original_float, parsed_float);
    }
  }

  // doubles
  constexpr std::array<double, 13> doubles_to_test = {
      0.0,
      -0.0,
      1.0,
      -1.0,
      123.456,
      -123.456,
      1.234567890123e100,
      -1.234567890123e100,
      std::numeric_limits<double>::infinity(),
      -std::numeric_limits<double>::infinity(),
      std::numeric_limits<double>::quiet_NaN(),
      // these don't pass the old method
      1.234567890123e-100,
      -1.234567890123e-100,
  };

  for (auto original_double : doubles_to_test) {
    std::string double_str = NumToString(original_double);
    double parsed_double = 0.0;
    bool success = StringToNumber(double_str.c_str(), &parsed_double);

    if (std::isnan(original_double)) {
      TEST_ASSERT(std::isnan(parsed_double));
    } else {
      TEST_ASSERT(success);
      TEST_EQ(original_double, parsed_double);
    }
  }
}

namespace {

template <typename T>
struct TestCase {
  T input;
  std::string_view expected_output;
};

}  // namespace

void FloatToStringOutputTest() {
  // Test that FloatToString produces expected string outputs for given inputs

  constexpr std::array<TestCase<float>, 13> test_cases{
      TestCase<float>{0.0f, "0.0"},
      TestCase<float>{-0.0f, "-0.0"},
      TestCase<float>{1.0f, "1.0"},
      TestCase<float>{-1.0f, "-1.0"},
      TestCase<float>{123.456f, "123.456"},
      TestCase<float>{-123.456f, "-123.456"},
      TestCase<float>{1.234567890123e10f, "12345679000.0"},
      TestCase<float>{-1.234567890123e10f, "-12345679000.0"},
      TestCase<float>{1.234567890123e-10f, "0.00000000012345679"},
      TestCase<float>{-1.234567890123e-10f, "-0.00000000012345679"},
      TestCase<float>{std::numeric_limits<float>::infinity(), "inf"},
      TestCase<float>{-std::numeric_limits<float>::infinity(), "-inf"},
      TestCase<float>{std::numeric_limits<float>::quiet_NaN(), "nan"},
  };

  for (const auto& test_case : test_cases) {
    std::string output = NumToString(test_case.input);
    TEST_EQ(output, std::string{test_case.expected_output});
  }

  // doubles
  constexpr std::array<TestCase<double>, 13> double_test_cases = {
      TestCase<double>{0.0, "0.0"},
      TestCase<double>{-0.0, "-0.0"},
      TestCase<double>{1.0, "1.0"},
      TestCase<double>{-1.0, "-1.0"},
      TestCase<double>{123.456789012345, "123.456789012345"},
      TestCase<double>{-123.456789012345, "-123.456789012345"},
      TestCase<double>{1.234567890123456e100,
                       "1234567890123456000000000000000000000000000000000000000"
                       "0000000000000000"
                       "000000000000000000000000000000.0"},
      TestCase<double>{std::numeric_limits<double>::infinity(), "inf"},
      TestCase<double>{-std::numeric_limits<double>::infinity(), "-inf"},
      TestCase<double>{std::numeric_limits<double>::quiet_NaN(), "nan"},
  };

  for (const auto& test_case : double_test_cases) {
    std::string output = NumToString(test_case.input);
    TEST_EQ(output, std::string{test_case.expected_output});
  }
}

}  // namespace tests
}  // namespace flatbuffers
