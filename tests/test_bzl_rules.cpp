#include "flatbuffers/flexbuffers.h"

#ifdef __ANDROID__
  #include <android/log.h>
  #define TEST_OUTPUT_LINE(...) \
    __android_log_print(ANDROID_LOG_INFO, "FlatBuffers", __VA_ARGS__)
  #define FLATBUFFERS_NO_FILE_TESTS
#else
  #define TEST_OUTPUT_LINE(...) \
    { printf(__VA_ARGS__); printf("\n"); }
#endif

int testing_fails = 0;

void TestFail(const char *expval, const char *val, const char *exp,
              const char *file, int line) {
  TEST_OUTPUT_LINE("VALUE: \"%s\"", expval);
  TEST_OUTPUT_LINE("EXPECTED: \"%s\"", val);
  TEST_OUTPUT_LINE("TEST FAILED: %s:%d, %s", file, line, exp);
  assert(0);
  testing_fails++;
}

void TestEqStr(const char *expval, const char *val, const char *exp,
               const char *file, int line) {
  if (strcmp(expval, val) != 0) { TestFail(expval, val, exp, file, line); }
}

template<typename T, typename U>
void TestEq(T expval, U val, const char *exp, const char *file, int line) {
  if (U(expval) != val) {
    TestFail(flatbuffers::NumToString(expval).c_str(),
             flatbuffers::NumToString(val).c_str(), exp, file, line);
  }
}

#define TEST_EQ(exp, val) TestEq(exp, val, #exp, __FILE__, __LINE__)
#define TEST_NOTNULL(exp) TestEq(exp == NULL, false, #exp, __FILE__, __LINE__)
#define TEST_EQ_STR(exp, val) TestEqStr(exp, val, #exp, __FILE__, __LINE__)


void FlexBuffersTest() {
  TEST_EQ_STR("test", "test");
}

int main(int /*argc*/, const char * /*argv*/ []) {
  // clang-format off
  #if defined(FLATBUFFERS_MEMORY_LEAK_TRACKING) && \
      defined(_MSC_VER) && defined(_DEBUG)
    _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF
      // For more thorough checking:
      //| _CRTDBG_CHECK_ALWAYS_DF | _CRTDBG_DELAY_FREE_MEM_DF
    );
  #endif

  FlexBuffersTest();

  if (!testing_fails) {
    TEST_OUTPUT_LINE("ALL TESTS PASSED");
    return 0;
  } else {
    TEST_OUTPUT_LINE("%d FAILED TESTS", testing_fails);
    return 1;
  }
}
