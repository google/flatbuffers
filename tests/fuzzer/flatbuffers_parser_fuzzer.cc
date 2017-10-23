// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
#include <stddef.h>
#include <stdint.h>
#include <string>

#include "flatbuffers/idl.h"

extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size) {
  flatbuffers::Parser parser;
  // Guarantee 0-termination.
  std::string s(reinterpret_cast<const char *>(data), size);
  parser.Parse(s.c_str());
  return 0;
}
