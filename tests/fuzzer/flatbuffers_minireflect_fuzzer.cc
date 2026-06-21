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

// Fuzz target for the MiniReflect API (FlatBufferToString, IterateFlatBuffer).
// This exercises minireflect.h which was previously completely unfuzzed.
//
// The fuzzer feeds arbitrary bytes as a FlatBuffer to the MiniReflect
// iteration/traversal functions, exercising:
//   - IterateObject / IterateValue (union dispatch, vector iteration)
//   - FlatBufferToString (ToStringVisitor)
//   - TypeTable-driven traversal of all field types including union vectors
//
// Uses the Movie schema (union_vector.fbs) which contains:
//   - Union vectors: characters:[Character]
//   - Single unions: main_character:Character
//   - Structs in unions: Rapunzel, BookReader
//   - Strings in unions: Other, Unused
//   - Nested tables: Attacker, HandFan

#include <stddef.h>
#include <stdint.h>

#include <string>

#include "cpp17/generated_cpp17/union_vector_generated.h"
#include "flatbuffers/minireflect.h"

extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size) {
  // Exercise FlatBufferToString with the Movie schema.
  // This traverses all fields including union vectors via IterateObject
  // and IterateValue, covering the union type dispatch path in minireflect.h.
  flatbuffers::FlatBufferToString(data, MovieTypeTable());

  // Also exercise with the Monster schema for broader coverage of
  // table/struct/enum field types without union vectors.
  flatbuffers::FlatBufferToString(data, AttackerTypeTable());

  return 0;
}
