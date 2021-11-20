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

#ifndef FLATBUFFERS_BFBS_GEN_H_
#define FLATBUFFERS_BFBS_GEN_H_

#include <cstdint>

#include "flatbuffers/bfbs_generator.h"
#include "flatbuffers/reflection_generated.h"

namespace flatbuffers {

// A concrete base Flatbuffer Generator that specific language generators can
// derive from.
class BaseBfbsGenerator : public BfbsGenerator {
 public:
  virtual ~BaseBfbsGenerator() {}
  BaseBfbsGenerator() : schema_(nullptr) {}

  virtual GeneratorStatus generate_from_schema(
      const reflection::Schema *schema) = 0;

  // Override of the Generator::generate method that does the initial
  // deserialization and verification steps.
  GeneratorStatus generate(const uint8_t *buffer,
                           const int64_t length) FLATBUFFERS_OVERRIDE {
    flatbuffers::Verifier verifier(buffer, static_cast<size_t>(length));
    if (!reflection::VerifySchemaBuffer(verifier)) {
      return FAILED_VERIFICATION;
    }

    // Store the root schema since there are cases where leaf nodes refer to
    // things in the root schema (e.g., indexing the objects).
    schema_ = reflection::GetSchema(buffer);
    GeneratorStatus status = generate_from_schema(schema_);
    schema_ = nullptr;
    return status;
  }

 protected:
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

  std::vector<uint32_t> map_by_field_id(const reflection::Object *object) {
    std::vector<uint32_t> field_index_by_id;
    field_index_by_id.resize(object->fields()->size());

    // Create the mapping of field ID to the index into the vector.
    for (uint32_t i = 0; i < object->fields()->size(); ++i) {
      auto field = object->fields()->Get(i);
      field_index_by_id[field->id()] = i;
    }

    return field_index_by_id;
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
    return base_type == reflection::Obj;
  }

  bool IsScalar(const reflection::BaseType base_type) {
    return base_type >= reflection::UType && base_type <= reflection::Double;
  }

  bool IsFloatingPoint(const reflection::BaseType base_type) {
    return base_type == reflection::Float || base_type == reflection::Double;
  }

  bool IsBool(const reflection::BaseType base_type) {
    return base_type == reflection::Bool;
  }

  bool IsSingleByte(const reflection::BaseType base_type) {
    return base_type >= reflection::UType && base_type <= reflection::UByte;
  }

  bool IsVector(const reflection::BaseType base_type) {
    return base_type == reflection::Vector;
  }

  bool IsArray(const reflection::BaseType base_type) {
    return base_type == reflection::Array;
  }

  std::string make_camel_case(const std::string &in,
                              bool uppercase_first = true) {
    std::string s;
    for (size_t i = 0; i < in.length(); i++) {
      if (!i && uppercase_first)
        s += static_cast<char>(::toupper(static_cast<unsigned char>(in[0])));
      else if (in[i] == '_' && i + 1 < in.length())
        s += static_cast<char>(::toupper(static_cast<unsigned char>(in[++i])));
      else
        s += in[i];
    }
    return s;
  }

  const reflection::Schema *schema_;
};

}  // namespace flatbuffers

#endif  // FLATBUFFERS_BFBS_GEN_H_