// Copyright 2026 Google Inc. All rights reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

// Regression test for GitHub issue #9144:
// SEGV in StructDef::Deserialize via IsStruct on a bfbs schema that
// passes VerifySchemaBuffer but has unresolved struct cross-references.

#include <cstdint>
#include <cstdio>
#include <vector>

#include "flatbuffers/idl.h"
#include "flatbuffers/reflection.h"
#include "flatbuffers/verifier.h"

// PoC binary from issue #9144 (base64-decoded, 134 bytes).
// This schema passes VerifySchemaBuffer but declares a BASE_TYPE_STRUCT
// field whose struct_def cross-reference is never resolved, causing a
// null-pointer dereference in IsStruct() -> StructDef::Deserialize().
static const uint8_t kPocData[] = {
    0x00, 0x00, 0x10, 0x00, 0x00, 0x00, 0x42, 0x46, 0x42, 0x53, 0x08, 0x00,
    0x0c, 0x00, 0x04, 0x00, 0x08, 0x00, 0x08, 0x00, 0x00, 0x00, 0x0c, 0x00,
    0x00, 0x00, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00,
    0x00, 0x00, 0x10, 0x00, 0x00, 0x00, 0x0c, 0x00, 0x14, 0x00, 0x08, 0x00,
    0x0c, 0x00, 0x07, 0x00, 0x10, 0x00, 0x0c, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x01, 0x40, 0x00, 0x00, 0x00, 0x08, 0x00, 0x00, 0x00, 0x01, 0x00,
    0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x0c, 0x00, 0x00, 0x00, 0x08, 0x00,
    0x0e, 0x00, 0x04, 0x00, 0x08, 0x00, 0x08, 0x00, 0x00, 0x00, 0x18, 0x00,
    0x00, 0x00, 0x0c, 0x00, 0x00, 0x00, 0x00, 0x00, 0x06, 0x00, 0x08, 0x00,
    0x07, 0x00, 0x06, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0f, 0x01, 0x00,
    0x00, 0x00, 0x66, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x53,
    0x00, 0x00, 0x00,
};

static const size_t kPocSize = sizeof(kPocData);

int main() {
  printf("=== Regression test for #9144 ===\n");

  // Step 1: Verify the PoC passes VerifySchemaBuffer (as reported).
  flatbuffers::Verifier verifier(kPocData, kPocSize);
  bool verify_ok = reflection::VerifySchemaBuffer(verifier);
  printf("VerifySchemaBuffer: %s\n", verify_ok ? "PASSED" : "FAILED");

  if (!verify_ok) {
    printf("PoC did not pass verifier — test inconclusive.\n");
    return 0;
  }

  // Step 2: Deserialize should return false (not crash with SEGV).
  // Before the fix, this would crash with a null-pointer dereference
  // in IsStruct() at idl.h:533.
  flatbuffers::Parser parser;
  bool deserialize_ok = parser.Deserialize(kPocData, kPocSize);

  printf("Parser::Deserialize: %s\n", deserialize_ok ? "OK" : "FAILED (expected)");

  if (deserialize_ok) {
    printf("UNEXPECTED: Deserialize succeeded on malformed schema.\n");
    return 1;
  }

  printf("PASS: Deserialize correctly rejected malformed schema without crashing.\n");
  return 0;
}
