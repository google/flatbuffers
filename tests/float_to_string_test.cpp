#include "float_to_string_test.h"

#include "flatbuffers/util.h"
#include "test_assert.h"

namespace flatbuffers {
namespace tests {

void FloatToStringTest() {
  // convert an array of floats to strings and back, and check they are the same
  std::array floats_to_test = {
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
  std::array doubles_to_test = {
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

}  // namespace tests
}  // namespace flatbuffers
