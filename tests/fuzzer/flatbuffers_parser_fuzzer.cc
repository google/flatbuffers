// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
#include <stddef.h>
#include <stdint.h>
#include <clocale>
#include <string>

#include "flatbuffers/idl.h"

#define flags_strict_json 0x01
#define flags_skip_unexpected_fields_in_json 0x02
#define flags_allow_non_utf8 0x04
#define flags_flag_4 0x08
#define flags_flag_5 0x10
#define flags_flag_6 0x20
#define flags_flag_7 0x40
#define flags_clocale 0x80  // change default C-locale

extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size) {
  // Reserve one byte for Parser flags.
  if (size < 1) return 0;
  // REMEMBER: the first character in crash dump is not part of input!
  // Extract single byte for fuzzing flags value.
  const uint8_t flags = data[0];
  data += 1;  // move to next
  size -= 1;

  const std::string original(reinterpret_cast<const char *>(data), size);
  auto input = std::string(original.c_str());  // until '\0'
  if (input.empty()) return 0;

// Change default ASCII locale (affects to isalpha, isalnum, decimal
// delimiters, other). https://en.cppreference.com/w/cpp/locale/setlocale
#if defined(FLATBUFFERS_FORCE_LOCALE_INDEPENDENT)
  if (flags & flags_clocale) {
    // The ru_RU.CP1251 use ',' as decimal point delimiter instead of '.'.
    // std::to_string(12.0) will return "12,0000".
    // Ubuntu:>sudo locale-gen ru_RU.CP1251
    assert(std::setlocale(LC_ALL, FLATBUFFERS_FORCE_LOCALE_INDEPENDENT));
  }
#endif

  flatbuffers::IDLOptions opts;
  opts.strict_json = (flags & flags_strict_json);
  opts.skip_unexpected_fields_in_json =
      (flags & flags_skip_unexpected_fields_in_json);
  opts.allow_non_utf8 = (flags & flags_allow_non_utf8);

  flatbuffers::Parser parser(opts);
  // Guarantee 0-termination.
  parser.Parse(input.c_str());
  return 0;
}
