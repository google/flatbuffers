// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
#include <stddef.h>
#include <stdint.h>
#include <string>

#include "flatbuffers/flexbuffers.h"

extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size) {
  std::vector<uint8_t> reuse_tracker;
  // Check both with and without reuse tracker paths.
  flexbuffers::VerifyBuffer(data, size, &reuse_tracker);
  flexbuffers::VerifyBuffer(data, size, nullptr);
  return 0;
}
