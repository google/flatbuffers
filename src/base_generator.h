/*
 * Copyright 2021 Google Inc. All rights reserved.
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

#ifndef FLATBUFFERS_BASE_GENERATOR_H_
#define FLATBUFFERS_BASE_GENERATOR_H_

#include <bits/stdint-intn.h>

#include "flatbuffers/generator.h"
#include "flatbuffers/reflection_generated.h"

namespace flatbuffers {

class BaseGenerator : public Generator {
 public:
  virtual ~BaseGenerator() {}
  BaseGenerator(int8_t characters_per_indent, char indent_char)
      : indent_level_(0),
        characters_per_indent_(characters_per_indent),
        indent_char_(indent_char) {}

  virtual GeneratorStatus generate(const reflection::Schema *schema) = 0;

  GeneratorStatus generate(const uint8_t *buffer,
                           const int64_t length) override {
    flatbuffers::Verifier verifier(buffer, length);
    if (!reflection::VerifySchemaBuffer(verifier)) {
      return GeneratorStatus::FAILED_VERIFICATION;
    }

    return generate(reflection::GetSchema(buffer));
  }

 protected:
  void indent() { indent_level_++; };
  void dedent() { indent_level_--; };

  std::string indentation() const {
    return std::string(characters_per_indent_ * indent_level_, indent_char_);
  }

  bool IsStructOrTable(const reflection::BaseType base_type) {
    return base_type == reflection::BaseType::Obj;
  }

  bool IsScalar(const reflection::BaseType base_type) {
    return base_type >= reflection::BaseType::UType &&
           base_type <= reflection::BaseType::Double;
  }

  bool IsFloatingPoint(const reflection::BaseType base_type) {
    return base_type == reflection::BaseType::Float ||
           base_type == reflection::BaseType::Double;
  }

  bool IsBool(const reflection::BaseType base_type) {
    return base_type == reflection::BaseType::Bool;
  }

  bool IsSingleByte(const reflection::BaseType base_type) {
    return base_type >= reflection::BaseType::UType &&
           base_type <= reflection::BaseType::UByte;
  }

  std::string GetFileName(const reflection::Schema *schema) {
    std::string filename =
        schema->file_ident()->str() + schema->file_ext()->str();
    return filename;
  }

  int8_t indent_level_;
  const int8_t characters_per_indent_;
  const char indent_char_;
};

}  // namespace flatbuffers

#endif  // FLATBUFFERS_BASE_GENERATOR_H_