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
  if (flexbuffers::VerifyBuffer(data, size, &reuse_tracker)) {
    // A buffer that passes verification must be safe to fully traverse. This
    // exercises the "verified => safe to access" contract that the verify-only
    // path above does not (previously untested), and would catch e.g. cyclic
    // buffers that recurse without bound.
    std::string s;
    flexbuffers::GetRoot(data, size).ToString(true, true, s);
  }
  // FIXME: we can't really verify this path, because the fuzzer will
  // construct buffers that time out.
  // Add a simple #define to bound the number of steps just for the fuzzer?
  // flexbuffers::VerifyBuffer(data, size, nullptr);
  return 0;
}
