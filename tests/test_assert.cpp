#include <assert.h>
#include "test_assert.h"

#ifdef _MSC_VER
#  include <crtdbg.h>
#endif

int testing_fails = 0;
static TestFailEventListener fail_listener_ = nullptr;

void TestFail(const char *expval, const char *val, const char *exp,
              const char *file, int line, const char *func) {
  TEST_OUTPUT_LINE("VALUE: \"%s\"", expval);
  TEST_OUTPUT_LINE("EXPECTED: \"%s\"", val);
  TEST_OUTPUT_LINE("TEST FAILED: %s:%d, %s in %s", file, line, exp,
                   func ? func : "");
  testing_fails++;

  // Notify, emulate 'gtest::OnTestPartResult' event handler.
  if (fail_listener_) (*fail_listener_)(expval, val, exp, file, line, func);

  assert(0);  // ignored in Release if NDEBUG defined
}

void TestEqStr(const char *expval, const char *val, const char *exp,
               const char *file, int line) {
  if (strcmp(expval, val) != 0) { TestFail(expval, val, exp, file, line); }
}

#ifdef _MSC_VER
// Without this hook function the message box not suppressed.
int msvc_no_dialog_box_on_assert(int rpt_type, char *msg, int *ret_val) {
  (void)ret_val;
  TEST_OUTPUT_LINE("TEST ASSERTED: %d: %s", rpt_type, msg);
  return 1;
}
#endif

void InitTestEngine(TestFailEventListener listener) {
  testing_fails = 0;
  // Disable stdout buffering to prevent information lost on assertion or core
  // dump.
  setvbuf(stdout, NULL, _IONBF, 0);
  setvbuf(stderr, NULL, _IONBF, 0);

  // clang-format off

  #ifdef _MSC_VER
    // Suppress pop-up message box on assertion (MSVC2010, MSVC2012).
    // This message box hangs CI-test on the hour until timeout expired.
    // Default mode is file, file is stderr.
    _CrtSetReportMode(_CRT_ASSERT, _CRTDBG_MODE_FILE | _CRTDBG_MODE_DEBUG);
    _CrtSetReportFile(_CRT_ASSERT, _CRTDBG_FILE_STDERR);
    _CrtSetReportHook(msvc_no_dialog_box_on_assert);
  #endif
  // clang-format on

  fail_listener_ = listener;
}
