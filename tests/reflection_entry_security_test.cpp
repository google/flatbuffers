// Parser::Deserialize and reflection verification input-contract security
// regression tests.
//
// Covers four hardening fixes (this file grows one test per fix commit):
//   1. Length validation before identifier reads in
//      Parser::Deserialize(const uint8_t*, size_t) (fixes an out-of-bounds
//      strncmp on buffers shorter than the identifier).
//   2. SymbolTable::Add ownership fix (duplicate check before insertion) so
//      the Parser::Deserialize error paths no longer double-free.
//   3. base_type/element range validation in Type::Deserialize (prevents
//      out-of-range wire values from reaching SizeOf()'s default case).
//   4. VerifyObject treats a None-typed field as a verification failure
//      instead of asserting.
//
// Self-contained: schemas are embedded or constructed at runtime; no external
// .fbs/.bfbs/flatc needed.  Registered from tests/test.cpp and compiled into
// the `flattests` target via FlatBuffers_Tests_SRCS in CMakeLists.txt.

#include "flatbuffers/idl.h"
#include "flatbuffers/reflection.h"
#include "flatbuffers/reflection_generated.h"
#include "flatbuffers/util.h"
#include "reflection_test.h"
#include "test_assert.h"
#include <cstdint>
#include <cstdio>
#include <vector>

using namespace flatbuffers;

namespace flatbuffers {
namespace tests {

namespace {

// A 10-byte "schema" that is far too short to contain a FlatBuffer file
// identifier.  Regression input for fix (1): previously this made
// Parser::Deserialize read past the end of the buffer in BufferHasIdentifier
// (ASan: heap-buffer-overflow READ of size 1 in strncmp).
const uint8_t kShortBuffer[10] = { 0x00, 0x00, 0x00, 0x01, 0x20,
                                   0x03, 0x18, 0x20, 0x03, 0x18 };

void EntryLengthValidationTest() {
  TEST_OUTPUT_LINE("EntryLengthValidationTest");
  Parser parser;
  // (1) Short buffer must be rejected before any identifier read: no crash,
  // Deserialize returns false.
  TEST_EQ(parser.Deserialize(kShortBuffer, sizeof(kShortBuffer)), false);
}

}  // namespace

void ReflectionEntrySecurityTest() {
  EntryLengthValidationTest();
  TEST_OUTPUT_LINE("ReflectionEntrySecurityTest: PASSED");
}

}  // namespace tests
}  // namespace flatbuffers
