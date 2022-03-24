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

#ifndef FLATBUFFERS_BINARY_ANNOTATOR_H_
#define FLATBUFFERS_BINARY_ANNOTATOR_H_

#include <map>
#include <string>
#include <vector>

#include "flatbuffers/base.h"
#include "flatbuffers/reflection.h"
#include "flatbuffers/stl_emulation.h"

namespace flatbuffers {

enum class BinaryRegionType {
  Unknown = 0,
  UOffset = 1,
  SOffset = 2,
  VOffset = 3,
  Bool = 4,
  Byte = 5,
  Char = 6,
  Uint8 = 7,
  Int8 = 8,
  Uint16 = 9,
  Int16 = 10,
  Uint32 = 11,
  Int32 = 12,
  Uint64 = 13,
  Int64 = 14,
  Float = 15,
  Double = 16,
  UType = 17,
};

template<typename T> static inline T GetScalar(const uint8_t *binary) {
  return *reinterpret_cast<const T *>(binary);
}

template<typename T>
static inline std::string ToHex(T i, size_t width = sizeof(T)) {
  std::stringstream stream;
  stream << std::hex << std::uppercase << std::setfill('0') << std::setw(width)
         << i;
  return stream.str();
}

// Specialized version for uint8_t that don't work well with std::hex.
static inline std::string ToHex(uint8_t i) {
  return ToHex(static_cast<int>(i), 2);
}

struct BinaryRegion {
  // Offset into the binary where this region begins.
  uint64_t offset = 0;

  // The length of this region in bytes.
  uint64_t length = 0;

  // The underlying datatype of this region
  BinaryRegionType type = BinaryRegionType::Unknown;

  // If `type` is an array/vector, this is the number of those types this region
  // encompasses.
  uint64_t array_length = 0;

  // If the is an offset to some other region, this is what it points to. The
  // offset is relative to overall binary, not to this region.
  uint64_t points_to_offset = 0;

  // The comment on the region.
  // TODO(dbaileychess): Consider moving this to a more structure comment field
  // so that other generators can parse it easier.
  std::string comment;
};

enum class BinarySectionType {
  Unknown = 0,
  Header = 1,
  Table = 2,
  RootTable = 3,
  VTable = 4,
  Struct = 5,
  String = 6,
  Vector = 7,
  Union = 8,
  Padding = 9,
};

// A section of the binary that is grouped together in some logical manner, and
// often is pointed too by some other offset BinaryRegion. Sections include
// `tables`, `vtables`, `strings`, `vectors`, etc..
struct BinarySection {
  // User-specified name of the section, if applicable.
  std::string name;

  // The type of this section.
  BinarySectionType type = BinarySectionType::Unknown;

  // The binary regions that make up this section, in order of their offsets.
  std::vector<BinaryRegion> regions;
};

inline static BinaryRegionType GetRegionType(reflection::BaseType base_type) {
  switch (base_type) {
    case reflection::UType: return BinaryRegionType::UType;
    case reflection::Bool: return BinaryRegionType::Uint8;
    case reflection::Byte: return BinaryRegionType::Uint8;
    case reflection::UByte: return BinaryRegionType::Uint8;
    case reflection::Short: return BinaryRegionType::Int16;
    case reflection::UShort: return BinaryRegionType::Uint16;
    case reflection::Int: return BinaryRegionType::Uint32;
    case reflection::UInt: return BinaryRegionType::Uint32;
    case reflection::Long: return BinaryRegionType::Int64;
    case reflection::ULong: return BinaryRegionType::Uint64;
    case reflection::Float: return BinaryRegionType::Float;
    case reflection::Double: return BinaryRegionType::Double;
    default: return BinaryRegionType::Unknown;
  }
}

inline static std::string ToString(const BinaryRegionType type) {
  switch (type) {
    case BinaryRegionType::UOffset: return "UOffset32";
    case BinaryRegionType::SOffset: return "SOffset32";
    case BinaryRegionType::VOffset: return "VOffset16";
    case BinaryRegionType::Bool: return "bool";
    case BinaryRegionType::Char: return "char";
    case BinaryRegionType::Byte: return "int8_t";
    case BinaryRegionType::Uint8: return "uint8_t";
    case BinaryRegionType::Uint16: return "uint16_t";
    case BinaryRegionType::Uint32: return "uint32_t";
    case BinaryRegionType::Uint64: return "uint64_t"; ;
    case BinaryRegionType::Int8: return "int8_t";
    case BinaryRegionType::Int16: return "int16_t";
    case BinaryRegionType::Int32: return "int32_t";
    case BinaryRegionType::Int64: return "int64_t";
    case BinaryRegionType::Double: return "double";
    case BinaryRegionType::Float: return "float";
    case BinaryRegionType::UType: return "UType8";
    case BinaryRegionType::Unknown: return "?uint8_t";
    default: return "todo";
  }
}

class BinaryAnnotator {
 public:
  explicit BinaryAnnotator(const uint8_t *const bfbs,
                           const uint64_t bfbs_length,
                           const uint8_t *const binary,
                           const uint64_t binary_length)
      : bfbs_(bfbs),
        bfbs_length_(bfbs_length),
        schema_(reflection::GetSchema(bfbs)),
        binary_(binary),
        binary_length_(binary_length) {}

  std::map<uint64_t, BinarySection> Annotate();

 private:
  struct VTable {
    struct Entry {
      const reflection::Field *field = nullptr;
      uint16_t offset_from_table = 0;
    };

    // Field ID -> {field def, offset from table}
    std::map<uint16_t, Entry> fields;

    uint16_t vtable_size = 0;
    uint16_t table_size = 0;
  };

  uint64_t BuildHeader(uint64_t offset);

  void BuildVTable(uint64_t offset, const reflection::Object *table,
                   uint64_t offset_of_referring_table);

  void BuildTable(uint64_t offset, const BinarySectionType type,
                  const reflection::Object *table);

  uint64_t BuildStruct(uint64_t offset, std::vector<BinaryRegion> &regions,
                       const reflection::Object *structure);

  void BuildString(uint64_t offset, const reflection::Object *table,
                   const reflection::Field *field);

  void BuildVector(uint64_t offset, const reflection::Object *table,
                   const reflection::Field *field, uint64_t parent_table_offset,
                   const VTable &vtable);

  std::string BuildUnion(uint64_t offset, uint8_t realized_type,
                         const reflection::Field *field);

  void FixMissingRegions();
  void FixMissingSections();

  inline bool IsValidOffset(const uint64_t offset) const {
    return offset < binary_length_;
  }

  // Determines if performing a GetScalar request for `T` at `offset` would read
  // passed the end of the binary.
  template<typename T> inline bool IsValidRead(const uint64_t offset) const {
    return IsValidRead(offset, sizeof(T));
  }

  inline bool IsValidRead(const uint64_t offset, const uint64_t length) const {
    return IsValidOffset(offset + length - 1);
  }

  // Calculate the number of bytes remaining from the given offset. If offset is
  // > binary_length, 0 is returned.
  uint64_t RemainingBytes(const uint64_t offset) const {
    return IsValidOffset(offset) ? binary_length_ - offset : 0;
  }

  template<typename T>
  flatbuffers::Optional<T> ReadScalar(const uint64_t offset) const {
    if (!IsValidRead<T>(offset)) { return flatbuffers::nullopt; }

    return flatbuffers::ReadScalar<T>(binary_ + offset);
  }

  // Adds the provided `section` keyed by the `offset` it occurs at. If a
  // section is already added at that offset, it doesn't replace the exisiting
  // one.
  void AddSection(const uint64_t offset, const BinarySection &section) {
    sections_.insert(std::make_pair(offset, section));
  }

  bool IsInlineField(const reflection::Field *const field) {
    if (field->type()->base_type() == reflection::BaseType::Obj) {
      return schema_->objects()->Get(field->type()->index())->is_struct();
    }
    return IsScalar(field->type()->base_type());
  }

  bool IsUnionType(const reflection::BaseType type) {
    return (type == reflection::BaseType::UType ||
            type == reflection::BaseType::Union);
  }

  bool IsUnionType(const reflection::Field *const field) {
    return IsUnionType(field->type()->base_type()) &&
           field->type()->index() >= 0;
  }

  bool IsValidUnionValue(const reflection::Field *const field,
                         const uint8_t value) {
    return IsUnionType(field) &&
           IsValidUnionValue(field->type()->index(), value);
  }

  bool IsValidUnionValue(const uint32_t enum_id, const uint8_t value) {
    if (enum_id >= schema_->enums()->size()) { return false; }

    const reflection::Enum *enum_def = schema_->enums()->Get(enum_id);

    if (enum_def == nullptr) { return false; }

    return value < enum_def->values()->size();
  }

  // The schema for the binary file
  const uint8_t *bfbs_;
  const uint64_t bfbs_length_;
  const reflection::Schema *schema_;

  // The binary data itself.
  const uint8_t *binary_;
  const uint64_t binary_length_;

  // Map of binary offset to vtables, to dedupe vtables.
  std::map<uint64_t, VTable> vtables_;

  // A set of binary offset to string sections, to dedupe shared strings.
  std::set<uint64_t> strings_;

  // The annotated binary sections, index by their absolute offset.
  std::map<uint64_t, BinarySection> sections_;
};

}  // namespace flatbuffers

#endif  // FLATBUFFERS_BINARY_ANNOTATOR_H_