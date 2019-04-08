#include "test_assert.h"

#include <assert.h>

#ifdef _MSC_VER
#  include <crtdbg.h>
#  include <windows.h>
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

#if defined(FLATBUFFERS_MEMORY_LEAK_TRACKING) && defined(_MSC_VER) && \
    defined(_DEBUG)
#define FLATBUFFERS_MEMORY_LEAK_TRACKING_MSVC
#endif

void InitTestEngine(TestFailEventListener listener) {
  testing_fails = 0;
  // Disable stdout buffering to prevent information lost on assertion or core
  // dump.
  setvbuf(stdout, NULL, _IONBF, 0);
  setvbuf(stderr, NULL, _IONBF, 0);

  // clang-format off

  #ifdef _MSC_VER
    // By default, send all reports to STDOUT to prevent CI hangs.
    // Enable assert report box [Abort|Retry|Ignore] if a debugger is present.
    const int dbg_mode = (_CRTDBG_MODE_FILE | _CRTDBG_MODE_DEBUG) |
                         (IsDebuggerPresent() ? _CRTDBG_MODE_WNDW : 0);
    (void)dbg_mode; // release mode fix
    // CrtDebug reports to _CRT_WARN channel.
    _CrtSetReportMode(_CRT_WARN, dbg_mode);
    _CrtSetReportFile(_CRT_WARN, _CRTDBG_FILE_STDOUT);
    // The assert from <assert.h> reports to _CRT_ERROR channel
    _CrtSetReportMode(_CRT_ERROR, dbg_mode);
    _CrtSetReportFile(_CRT_ERROR, _CRTDBG_FILE_STDOUT);
    // Internal CRT assert channel?
    _CrtSetReportMode(_CRT_ASSERT, dbg_mode);
    _CrtSetReportFile(_CRT_ASSERT, _CRTDBG_FILE_STDOUT);
  #endif

  #if defined(FLATBUFFERS_MEMORY_LEAK_TRACKING_MSVC)
    // For more thorough checking:
    // _CRTDBG_DELAY_FREE_MEM_DF | _CRTDBG_CHECK_ALWAYS_DF
    auto flags = _CrtSetDbgFlag(_CRTDBG_REPORT_FLAG);
    _CrtSetDbgFlag(flags | _CRTDBG_ALLOC_MEM_DF);
  #endif
  // clang-format on

  fail_listener_ = listener;
}

int CloseTestEngine(bool force_report) {
  if (!testing_fails || force_report) {
  #if defined(FLATBUFFERS_MEMORY_LEAK_TRACKING_MSVC)
      auto flags = _CrtSetDbgFlag(_CRTDBG_REPORT_FLAG);
      flags &= ~_CRTDBG_DELAY_FREE_MEM_DF;
      flags |= _CRTDBG_LEAK_CHECK_DF;
      _CrtSetDbgFlag(flags);
  #endif
  }
  return (0 != testing_fails);
}
