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

#include <stddef.h>
#include <stdint.h>

#include <clocale>
#include <string>

#include "cpp17/generated_cpp17/monster_test_generated.h"
#include "flatbuffers/idl.h"
#include "test_init.h"

namespace {

static constexpr size_t kMinInputLength = 1;
static constexpr size_t kMaxInputLength = 99000;

static constexpr uint8_t flags_strict_json = 0x80;
static constexpr uint8_t flags_skip_unexpected_fields_in_json = 0x40;
static constexpr uint8_t flags_allow_non_utf8 = 0x20;

std::string LoadBinarySchema(const char *file_name) {
    std::string schemafile;
    TEST_EQ(true,
            flatbuffers::LoadFile(file_name, true, &schemafile));

    flatbuffers::Verifier verifier(
        reinterpret_cast<const uint8_t *>(schemafile.c_str()),
        schemafile.size());
    TEST_EQ(true, reflection::VerifySchemaBuffer(verifier));
    return schemafile;
}

flatbuffers::Parser make_parser(const flatbuffers::IDLOptions opts) {
  // once loaded from disk
  static const std::string schemafile = LoadBinarySchema("./monster_test.bfbs");
  // parse schema first, so we can use it to parse the data after
  flatbuffers::Parser parser;
  TEST_EQ(true, parser.Deserialize(
                    reinterpret_cast<const uint8_t *>(schemafile.c_str()),
                    schemafile.size()));
  // (re)define parser options
  parser.opts = opts;
  return parser;
}

std::string do_test(const flatbuffers::IDLOptions &opts,
                    const std::string input_json) {
  auto parser = make_parser(opts);
  std::string jsongen;
  if (parser.ParseJson(input_json.c_str())) {
    flatbuffers::Verifier verifier(parser.builder_.GetBufferPointer(),
                                   parser.builder_.GetSize());
    TEST_EQ(true, MyGame::Example::VerifyMonsterBuffer(verifier));
    TEST_ASSERT(
        GenerateText(parser, parser.builder_.GetBufferPointer(), &jsongen));
  }
  return jsongen;
};
}  // namespace

// Utility for test run.
OneTimeTestInit OneTimeTestInit::one_time_init_;

extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size) {
  // Reserve one byte for Parser flags and one byte for repetition counter.
  if (size < 3) return 0;
  const uint8_t flags = data[0];
  (void)data[1];  //  reserved
  data += 2;
  size -= 2;  // bypass

  const std::string original(reinterpret_cast<const char *>(data), size);
  auto input = std::string(original.c_str());  // until '\0'
  if (input.size() < kMinInputLength || input.size() > kMaxInputLength)
    return 0;

  flatbuffers::IDLOptions opts;
  opts.strict_json = (flags & flags_strict_json);
  opts.skip_unexpected_fields_in_json =
      (flags & flags_skip_unexpected_fields_in_json);
  opts.allow_non_utf8 = (flags & flags_allow_non_utf8);

  const std::string jsongen_1 = do_test(opts, input);
  if (!jsongen_1.empty()) {
    const std::string jsongen_2 = do_test(opts, jsongen_1);
    TEST_EQ(jsongen_1, jsongen_2);
  }
  return 0;
}
