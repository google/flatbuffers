/*
 * Copyright 2026 Google Inc. All rights reserved.
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

#include <stddef.h>
#include <stdint.h>

#include "flatbuffers/idl.h"
#include "flatbuffers/reflection.h"
#include "test_init.h"

// Utility for test run.
OneTimeTestInit OneTimeTestInit::one_time_init_;

static flatbuffers::Parser parser_;

namespace {

// Verify() reads the root offset before bounds-checking it.
static constexpr size_t kMinInputLength = sizeof(flatbuffers::uoffset_t);
static constexpr size_t kMaxInputLength = 16384;

// Bound the verifier below its defaults: it does not memoize shared subtables,
// so a small buffer encoding a wide DAG can drive ~1M re-verifications.
static constexpr flatbuffers::uoffset_t kVerifyMaxDepth = 32;
static constexpr flatbuffers::uoffset_t kVerifyMaxTables = 1 << 14;

// A fixed schema exercising scalars, structs, strings, vectors, a nested table
// and a union, so the fuzzer spends its budget on the reflected data buffer.
static const char kSchema[] =
    "namespace repro;\n"
    "struct Vec3 { x:float; y:float; z:float; }\n"
    "table Stat { id:string; val:long; count:ushort; }\n"
    "union Any { Stat }\n"
    "table Monster {\n"
    "  pos:Vec3;\n"
    "  mana:short = 150;\n"
    "  hp:short = 100;\n"
    "  name:string;\n"
    "  inventory:[ubyte];\n"
    "  friends:[Monster];\n"
    "  stats:Stat;\n"
    "  scores:[long];\n"
    "  test:Any;\n"
    "}\n"
    "root_type Monster;\n";

}  // namespace

extern "C" int LLVMFuzzerInitialize(int* argc, char*** argv) {
  (void)argc;
  (void)argv;

  // Parse and serialize the schema once; reused to verify and read every input.
  TEST_EQ(true, parser_.Parse(kSchema));
  parser_.Serialize();
  return 0;
}

extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size) {
  if (size < kMinInputLength || size > kMaxInputLength) return 0;

  const reflection::Schema& schema =
      *reflection::GetSchema(parser_.builder_.GetBufferPointer());
  const reflection::Object& root = *schema.root_table();

  // Verify the untrusted buffer against the schema using reflection.
  if (!flatbuffers::Verify(schema, root, data, size, kVerifyMaxDepth,
                           kVerifyMaxTables))
    return 0;

  // Read every field reflectively (nullptr schema: no recursive pretty-print).
  const flatbuffers::Table* table = flatbuffers::GetAnyRoot(data);
  const auto* fields = root.fields();
  for (flatbuffers::uoffset_t i = 0; fields && i < fields->size(); i++) {
    const reflection::Field* field = fields->Get(i);
    if (field == nullptr) continue;
    flatbuffers::GetAnyFieldI(*table, *field);
    flatbuffers::GetAnyFieldF(*table, *field);
    flatbuffers::GetAnyFieldS(*table, *field, nullptr);
  }

  return 0;
}
