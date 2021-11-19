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

  const reflection::Object *get_object(const reflection::Type *type) {
    if (type->index() >= 0 && IsStructOrTable(type->base_type())) {
      return get_object_by_index(type->index());
    }
    return nullptr;
  }

  const reflection::Enum *get_enum(const reflection::Type *type) {
    // TODO(derekbailey): it would be better to have a explicit list of allowed
    // base types, instead of negating Obj types.
    if (type->index() >= 0 && !IsStructOrTable(type->base_type())) {
      return get_enum_by_index(type->index());
    }
    return nullptr;
  }

  // Used to get a object that is reference by index. (e.g.
  // reflection::Type::index). Returns nullptr if no object is available.
  const reflection::Object *get_object_by_index(int32_t index) {
    if (!schema_ || index < 0 ||
        index >= static_cast<int32_t>(schema_->objects()->size())) {
      return nullptr;
    }
    return schema_->objects()->Get(index);
  }

  // Used to get a enum that is reference by index. (e.g.
  // reflection::Type::index). Returns nullptr if no enum is available.
  const reflection::Enum *get_enum_by_index(int32_t index) {
    if (!schema_ || index < 0 ||
        index >= static_cast<int32_t>(schema_->enums()->size())) {
      return nullptr;
    }
    return schema_->enums()->Get(index);
  }

  std::vector<size_t> map_by_field_id(const reflection::Object *object) {
    std::vector<size_t> field_index_by_id;
    field_index_by_id.resize(object->fields()->size());

    // Create the mapping of field ID to the index into the vector.
    for (size_t i = 0; i < object->fields()->size(); ++i) {
      auto field = object->fields()->Get(i);
      field_index_by_id[field->id()] = i;
    }

    return field_index_by_id;
  }

  int32_t get_vector_inline_size(const reflection::Type *type) {
    if (IsArray(type->element())) {
      return size_of(type->element()) * type->fixed_length();
    }
    return size_of(type->element());
  }

  int32_t size_of(const reflection::BaseType base_type) {
    switch (base_type) {
      case reflection::BaseType::None:
      case reflection::BaseType::Bool:
      case reflection::BaseType::UType:
      case reflection::BaseType::Byte:
      case reflection::BaseType::UByte: return 1;
      case reflection::BaseType::Short:
      case reflection::BaseType::UShort: return 2;
      case reflection::BaseType::Float:
      case reflection::BaseType::Int:
      case reflection::BaseType::UInt: return 4;
      case reflection::BaseType::Double:
      case reflection::BaseType::Long:
      case reflection::BaseType::ULong: return 8;
      case reflection::BaseType::String:
      case reflection::BaseType::Vector:
      case reflection::BaseType::Obj:
      case reflection::BaseType::Union:
      case reflection::BaseType::Array: return 4;
      default: return 0;
    }
  }

  std::string indentation() const {
    return std::string(characters_per_indent_ * indent_level_, indent_char_);
  }

  bool IsStruct(const reflection::Type *type, bool use_element = false) {
    const reflection::BaseType base_type =
        use_element ? type->element() : type->base_type();
    return IsStructOrTable(base_type) &&
           get_object_by_index(type->index())->is_struct();
  }

  bool IsTable(const reflection::Type *type, bool use_element = false) {
    return !IsStruct(type, use_element);
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

  bool IsArray(const reflection::BaseType base_type) {
    return base_type == reflection::BaseType::Array;
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