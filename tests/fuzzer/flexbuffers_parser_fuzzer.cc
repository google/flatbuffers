// Copyright 2026 Google LLC. All rights reserved.
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

// Fuzz target for FlexBuffers accessor methods on unverified buffers.
// This complements flexbuffers_verifier_fuzzer.cc (which only tests
// VerifyBuffer) by exercising the GetRoot/Reference code paths that
// callers may reach without prior verification.

#include <cstddef>
#include <cstdint>
#include <string>

#include "flatbuffers/flexbuffers.h"

extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size) {
  auto root = flexbuffers::GetRoot(data, size);

  // Exercise the type-dispatch accessors that read from the buffer.
  (void)root.GetType();
  (void)root.IsNull();
  (void)root.AsBool();
  (void)root.AsInt64();
  (void)root.AsUInt64();
  (void)root.AsDouble();
  (void)root.AsString().c_str();

  if (root.IsVector()) {
    auto vec = root.AsVector();
    for (size_t i = 0; i < vec.size() && i < 8; i++) {
      (void)vec[i].AsInt64();
    }
  }

  if (root.IsMap()) {
    auto map = root.AsMap();
    auto keys = map.Keys();
    for (size_t i = 0; i < keys.size() && i < 8; i++) {
      (void)keys[i].AsKey();
    }
  }

  if (root.IsTypedVector()) {
    auto tv = root.AsTypedVector();
    for (size_t i = 0; i < tv.size() && i < 8; i++) {
      (void)tv[i].AsInt64();
    }
  }

  if (root.IsBlob()) {
    auto blob = root.AsBlob();
    (void)blob.size();
    (void)blob.data();
  }

  (void)root.ToString();

  return 0;
}
