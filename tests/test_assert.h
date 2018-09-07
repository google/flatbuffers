#ifndef TEST_ASSERT_H
#define TEST_ASSERT_H

#include "flatbuffers/flatbuffers.h"
#include "flatbuffers/util.h"

#ifdef __ANDROID__
#include <android/log.h>
  #define TEST_OUTPUT_LINE(...) \
    __android_log_print(ANDROID_LOG_INFO, "FlatBuffers", __VA_ARGS__)
  #define FLATBUFFERS_NO_FILE_TESTS
#else
#define TEST_OUTPUT_LINE(...) \
    { printf(__VA_ARGS__); printf("\n"); }
#endif
// clang-format on

extern int testing_fails;

void TestFail(const char *expval, const char *val, const char *exp,
              const char *file, int line);

void TestEqStr(const char *expval, const char *val, const char *exp,
               const char *file, int line);

template<typename T, typename U>
void TestEq(T expval, U val, const char *exp, const char *file, int line) {
  if (U(expval) != val) {
    TestFail(flatbuffers::NumToString(expval).c_str(),
             flatbuffers::NumToString(val).c_str(), exp, file, line);
  }
}

#define TEST_EQ(exp, val) TestEq(exp, val, #exp, __FILE__, __LINE__)
#define TEST_ASSERT(exp) TestEq(exp, true, #exp, __FILE__, __LINE__)
#define TEST_NOTNULL(exp) TestEq(exp == NULL, false, #exp, __FILE__, __LINE__)
#define TEST_EQ_STR(exp, val) TestEqStr(exp, val, #exp, __FILE__, __LINE__)

#endif // TEST_ASSERT_H
