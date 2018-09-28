#ifdef _MSC_VER
#include <assert.h>
#include <crtdbg.h>
#endif

#include "test_assert.h"

int testing_fails = 0;

void TestFail(const char *expval, const char *val, const char *exp,
              const char *file, int line, const char *func) {
  TEST_OUTPUT_LINE("VALUE: \"%s\"", expval);
  TEST_OUTPUT_LINE("EXPECTED: \"%s\"", val);
  TEST_OUTPUT_LINE("TEST FAILED: %s:%d, %s in %s", file, line, exp, func? func : "");
  testing_fails++;
  assert(0); // assert on first failure under debug
}

void TestEqStr(const char *expval, const char *val, const char *exp,
               const char *file, int line) {
  if (strcmp(expval, val) != 0) { TestFail(expval, val, exp, file, line); }
}

// Without this hook function the message box not suppressed.
int msvc_no_dialog_box_on_assert(int nRptType, char *szMsg,
                                 int * /* retVal */) {
  // Generate the debug report using the current settings for the report type,
  // mode, and file. In addition, by specifying the _CrtDbgReport return value
  // in returnValue, the application can also control whether a debug break
  // occurs.
  TEST_OUTPUT_LINE("TEST ASSERTED: %d: %s", nRptType, szMsg);
  return 1;
}

void InitTestEngine() {
  testing_fails = 0;
  // Disable stdout buffering to prevent information lost on assertion or core
  // dump.
  setvbuf(stdout, NULL, _IONBF, 0);
  setvbuf(stderr, NULL, _IONBF, 0);

#ifdef _MSC_VER
  // Suppress pop-up message box on assertion (MSVC2010, MSVC2012).
  // This message box hangs CI-test on the hour until timeout expired.
  // Default mode is file, file is stderr.
  _CrtSetReportMode(_CRT_ASSERT, _CRTDBG_MODE_FILE | _CRTDBG_MODE_DEBUG);
  _CrtSetReportFile(_CRT_ASSERT, _CRTDBG_FILE_STDERR);
  _CrtSetReportHook(msvc_no_dialog_box_on_assert);
#endif
}
