// Copyright 2024 Google Inc. All rights reserved.
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

// libFuzzer harness for FlatBuffers mini-reflection path.
// Exercises FlatBufferToString() with union vectors to detect
// OOB reads from desynchronized vector sizes.

#include <stddef.h>
#include <stdint.h>
#include <string>

#include "flatbuffers/minireflect.h"
#include "tests/union_vector/union_vector_generated.h"

extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size) {
  // Movie schema — characters union vector (primary crash path)
  {
    std::string s = flatbuffers::FlatBufferToString(data, MovieTypeTable());
    (void)s;
  }
  return 0;
}
