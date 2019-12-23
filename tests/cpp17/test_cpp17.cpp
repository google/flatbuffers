/*
 * Copyright 2014 Google Inc. All rights reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

// This is a sandbox for modeling C++17 code generator.
// C++17 code generator: "flatc --cpp_std c++17".
// Warning:
// This is an experimental feature and could change at any time.

#include "flatbuffers/flatbuffers.h"
#include "flatbuffers/flexbuffers.h"
#include "flatbuffers/idl.h"
#include "flatbuffers/minireflect.h"
#include "flatbuffers/registry.h"
#include "flatbuffers/util.h"
#include "test_assert.h"

// Embed generated code into an isolated namespace.
namespace cpp17 {
#include "generated_cpp17/monster_test_generated.h"
}  // namespace cpp17

namespace cpp11 {
#include "../monster_test_generated.h"
}  // namespace cpp11

int FlatBufferCpp17Tests() {
  TEST_ASSERT(true);
  return 0;
}

int main(int /*argc*/, const char * /*argv*/[]) {
  InitTestEngine();

  FlatBufferCpp17Tests();

  if (!testing_fails) {
    TEST_OUTPUT_LINE("C++17: ALL TESTS PASSED");
  } else {
    TEST_OUTPUT_LINE("C++17: %d FAILED TESTS", testing_fails);
  }
  return CloseTestEngine();
}
