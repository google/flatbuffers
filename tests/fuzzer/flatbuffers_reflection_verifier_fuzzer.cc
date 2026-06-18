// OSS-Fuzz target for the reflection-based binary verifier
// (flatbuffers::Verify(const reflection::Schema&, const reflection::Object&,
//  const uint8_t*, size_t) defined in src/reflection.cpp).
//
// This entry point is part of flatbuffers' untrusted-input security model: it
// is the function callers use to make an arbitrary buffer safe to traverse when
// the type is known only at runtime via a reflection (.bfbs) schema. It is
// currently exercised only by unit tests (tests/reflection_test.cpp); the
// existing verifier/monster fuzzers drive the generated-code VerifyMonsterBuffer
// path instead, and the annotator fuzzer drives BinaryAnnotator. Neither reaches
// this function.
//
// The schema is fixed and trusted; only the buffer (data) is fuzzed, matching
// the real threat model (trusted schema, untrusted serialized data).

#include <cstdint>
#include <cstddef>

#include "flatbuffers/idl.h"
#include "flatbuffers/reflection.h"

static flatbuffers::Parser *parser_ = nullptr;
static const reflection::Schema *schema_ = nullptr;

extern "C" int LLVMFuzzerInitialize(int * /*argc*/, char *** /*argv*/) {
  // A schema containing a vector of unions, which is the case that exercises
  // the union type-vector lookup in the reflection verifier.
  static const char *fbs =
      "table Foo { x:int; }\n"
      "table Bar { y:int; }\n"
      "union MyUnion { Foo, Bar }\n"
      "table Root { things:[MyUnion]; }\n"
      "root_type Root;\n";
  parser_ = new flatbuffers::Parser();
  parser_->Parse(fbs);
  parser_->Serialize();
  schema_ = reflection::GetSchema(parser_->builder_.GetBufferPointer());
  return 0;
}

extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size) {
  if (schema_ == nullptr || schema_->root_table() == nullptr) return 0;
  // Must not crash on any input; it should return true/false safely.
  flatbuffers::Verify(*schema_, *schema_->root_table(), data,
                      static_cast<size_t>(size));
  return 0;
}
