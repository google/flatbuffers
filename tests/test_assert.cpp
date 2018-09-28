#include "test_assert.h"

int testing_fails = 0;

void TestFail(const char *expval, const char *val, const char *exp,
              const char *file, int line, const char *func) {
  TEST_OUTPUT_LINE("VALUE: \"%s\"", expval);
  TEST_OUTPUT_LINE("EXPECTED: \"%s\"", val);
  TEST_OUTPUT_LINE("TEST FAILED: %s:%d, %s in %s", file, line, exp, func? func : "");
  assert(0);
  testing_fails++;
}

void TestEqStr(const char *expval, const char *val, const char *exp,
               const char *file, int line) {
  if (strcmp(expval, val) != 0) { TestFail(expval, val, exp, file, line); }
}

