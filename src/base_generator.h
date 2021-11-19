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

#include <time.h>

#include <cstdint>

#include "flatbuffers/generator.h"
#include "flatbuffers/reflection_generated.h"

namespace flatbuffers {

class BaseGenerator : public Generator {
 public:
  virtual ~BaseGenerator() {}
  BaseGenerator(int8_t characters_per_indent, char indent_char)
      : indent_level_(0),
        characters_per_indent_(characters_per_indent),
        indent_char_(indent_char),
        schema_(nullptr) {}

  virtual GeneratorStatus generate(const reflection::Schema *schema) = 0;

  GeneratorStatus generate(const uint8_t *buffer,
                           const int64_t length) override {
    flatbuffers::Verifier verifier(buffer, length);
    if (!reflection::VerifySchemaBuffer(verifier)) {
      return GeneratorStatus::FAILED_VERIFICATION;
    }

    // Store the root schema since there are cases where leaf nodes refer to
    // things in the root schema (e.g., indexing the objects).
    schema_ = reflection::GetSchema(buffer);
    GeneratorStatus status = generate(schema_);
    schema_ = nullptr;
    return status;
  }

 protected:
  void indent() { indent_level_++; };
  void dedent() { indent_level_--; };

  // Used to get a object that is reference by index. (e.g.
  // reflection::Type::index). Returns nullptr if no object is available.
  const reflection::Object *get_object_by_index(int32_t index) {
    if (!schema_ || index >= static_cast<int32_t>(schema_->objects()->size())) {
      return nullptr;
    }
    return schema_->objects()->Get(index);
  }

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

  std::string make_camel_case(const std::string &in,
                              bool uppercase_first = true) {
    std::string s;
    for (size_t i = 0; i < in.length(); i++) {
      if (!i && uppercase_first)
        s += std::toupper(in[0]);
      else if (in[i] == '_' && i + 1 < in.length())
        s += std::toupper(in[++i]);
      else
        s += in[i];
    }
    return s;
  }

  std::string get_current_time() {
    time_t now;
    time(&now);
    char buf[sizeof("2021-11-18T17:40:05-0800")];
    strftime(buf, sizeof(buf), "%Y-%m-%dT%H:%M:%S%z", localtime(&now));
    return buf;
  }

  int8_t indent_level_;
  const int8_t characters_per_indent_;
  const char indent_char_;

  const reflection::Schema *schema_;
};

}  // namespace flatbuffers

#endif  // FLATBUFFERS_BASE_GENERATOR_H_