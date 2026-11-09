/*
 * Copyright 2015 Google Inc. All rights reserved.
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

#include "flatbuffers/reflection.h"

#include <algorithm>

#include "flatbuffers/util.h"

// Helper functionality for reflection.

namespace flatbuffers {

namespace {

static void CopyInline(FlatBufferBuilder& fbb,
                       const reflection::Field& fielddef, const Table& table,
                       size_t align, size_t size) {
  fbb.Align(align);
  fbb.PushBytes(table.GetStruct<const uint8_t*>(fielddef.offset()), size);
  fbb.TrackField(fielddef.offset(), fbb.GetSize());
}

static bool VerifyStruct(flatbuffers::Verifier& v,
                         const flatbuffers::Table& parent_table,
                         voffset_t field_offset, const reflection::Object& obj,
                         bool required) {
  auto offset = parent_table.GetOptionalFieldOffset(field_offset);
  if (required && !offset) {
    return false;
  }

  return !offset ||
         v.VerifyFieldStruct(reinterpret_cast<const uint8_t*>(&parent_table),
                             offset, obj.bytesize(), obj.minalign());
}

static bool VerifyVectorOfStructs(flatbuffers::Verifier& v,
                                  const flatbuffers::Table& parent_table,
                                  voffset_t field_offset,
                                  const reflection::Object& obj,
                                  bool required) {
  auto p = parent_table.GetPointer<const uint8_t*>(field_offset);
  if (required && !p) {
    return false;
  }

  return !p || v.VerifyVectorOrString(p, obj.bytesize());
}

// forward declare to resolve cyclic deps between VerifyObject and VerifyVector
static bool VerifyObject(flatbuffers::Verifier& v,
                         const reflection::Schema& schema,
                         const reflection::Object& obj,
                         const flatbuffers::Table* table, bool required);

static bool VerifyUnion(flatbuffers::Verifier& v,
                        const reflection::Schema& schema, uint8_t utype,
                        const uint8_t* elem,
                        const reflection::Field& union_field) {
  if (!utype) return true;  // Not present.
  auto fb_enum = schema.enums()->Get(union_field.type()->index());
  if (utype >= fb_enum->values()->size()) return false;
  auto elem_type = fb_enum->values()->Get(utype)->union_type();
  switch (elem_type->base_type()) {
    case reflection::Obj: {
      auto elem_obj = schema.objects()->Get(elem_type->index());
      if (elem_obj->is_struct()) {
        return v.VerifyFromPointer(elem, elem_obj->bytesize());
      } else {
        return VerifyObject(v, schema, *elem_obj,
                            reinterpret_cast<const flatbuffers::Table*>(elem),
                            true);
      }
    }
    case reflection::String:
      return v.VerifyString(reinterpret_cast<const flatbuffers::String*>(elem));
    default:
      return false;
  }
}

static bool VerifyVector(flatbuffers::Verifier& v,
                         const reflection::Schema& schema,
                         const flatbuffers::Table& table,
                         const reflection::Field& vec_field) {
  FLATBUFFERS_ASSERT(vec_field.type()->base_type() == reflection::Vector);
  if (!table.VerifyField<uoffset_t>(v, vec_field.offset(), sizeof(uoffset_t)))
    return false;

  switch (vec_field.type()->element()) {
    case reflection::UType:
      return v.VerifyVector(flatbuffers::GetFieldV<uint8_t>(table, vec_field));
    case reflection::Bool:
    case reflection::Byte:
    case reflection::UByte:
      return v.VerifyVector(flatbuffers::GetFieldV<int8_t>(table, vec_field));
    case reflection::Short:
    case reflection::UShort:
      return v.VerifyVector(flatbuffers::GetFieldV<int16_t>(table, vec_field));
    case reflection::Int:
    case reflection::UInt:
      return v.VerifyVector(flatbuffers::GetFieldV<int32_t>(table, vec_field));
    case reflection::Long:
    case reflection::ULong:
      return v.VerifyVector(flatbuffers::GetFieldV<int64_t>(table, vec_field));
    case reflection::Float:
      return v.VerifyVector(flatbuffers::GetFieldV<float>(table, vec_field));
    case reflection::Double:
      return v.VerifyVector(flatbuffers::GetFieldV<double>(table, vec_field));
    case reflection::String: {
      auto vec_string =
          flatbuffers::GetFieldV<flatbuffers::Offset<flatbuffers::String>>(
              table, vec_field);
      if (v.VerifyVector(vec_string) && v.VerifyVectorOfStrings(vec_string)) {
        return true;
      } else {
        return false;
      }
    }
    case reflection::Obj: {
      auto obj = schema.objects()->Get(vec_field.type()->index());
      if (obj->is_struct()) {
        return VerifyVectorOfStructs(v, table, vec_field.offset(), *obj,
                                     vec_field.required());
      } else {
        auto vec =
            flatbuffers::GetFieldV<flatbuffers::Offset<flatbuffers::Table>>(
                table, vec_field);
        if (!v.VerifyVector(vec)) return false;
        if (!vec) return true;
        for (uoffset_t j = 0; j < vec->size(); j++) {
          if (!VerifyObject(v, schema, *obj, vec->Get(j), true)) {
            return false;
          }
        }
        return true;
      }
    }
    case reflection::Union: {
      auto vec = flatbuffers::GetFieldV<flatbuffers::Offset<uint8_t>>(
          table, vec_field);
      if (!v.VerifyVector(vec)) return false;
      if (!vec) return true;
      auto type_vec = table.GetPointer<Vector<uint8_t>*>(vec_field.offset() -
                                                         sizeof(voffset_t));
      if (!v.VerifyVector(type_vec)) return false;
      if (type_vec->size() != vec->size()) return false;
      for (uoffset_t j = 0; j < vec->size(); j++) {
        //  get union type from the prev field
        auto utype = type_vec->Get(j);
        auto elem = vec->Get(j);
        if (!VerifyUnion(v, schema, utype, elem, vec_field)) return false;
      }
      return true;
    }
    case reflection::Vector:
    case reflection::None:
    default:
      FLATBUFFERS_ASSERT(false);
      return false;
  }
}

static bool VerifyObject(flatbuffers::Verifier& v,
                         const reflection::Schema& schema,
                         const reflection::Object& obj,
                         const flatbuffers::Table* table, bool required) {
  if (!table) return !required;
  if (!table->VerifyTableStart(v)) return false;
  for (uoffset_t i = 0; i < obj.fields()->size(); i++) {
    auto field_def = obj.fields()->Get(i);
    switch (field_def->type()->base_type()) {
      case reflection::None:
        FLATBUFFERS_ASSERT(false);
        break;
      case reflection::UType:
        if (!table->VerifyField<uint8_t>(v, field_def->offset(),
                                         sizeof(uint8_t)))
          return false;
        break;
      case reflection::Bool:
      case reflection::Byte:
      case reflection::UByte:
        if (!table->VerifyField<int8_t>(v, field_def->offset(), sizeof(int8_t)))
          return false;
        break;
      case reflection::Short:
      case reflection::UShort:
        if (!table->VerifyField<int16_t>(v, field_def->offset(),
                                         sizeof(int16_t)))
          return false;
        break;
      case reflection::Int:
      case reflection::UInt:
        if (!table->VerifyField<int32_t>(v, field_def->offset(),
                                         sizeof(int32_t)))
          return false;
        break;
      case reflection::Long:
      case reflection::ULong:
        if (!table->VerifyField<int64_t>(v, field_def->offset(),
                                         sizeof(int64_t)))
          return false;
        break;
      case reflection::Float:
        if (!table->VerifyField<float>(v, field_def->offset(), sizeof(float)))
          return false;
        break;
      case reflection::Double:
        if (!table->VerifyField<double>(v, field_def->offset(), sizeof(double)))
          return false;
        break;
      case reflection::String:
        if (!table->VerifyField<uoffset_t>(v, field_def->offset(),
                                           sizeof(uoffset_t)) ||
            !v.VerifyString(flatbuffers::GetFieldS(*table, *field_def))) {
          return false;
        }
        break;
      case reflection::Vector:
        if (!VerifyVector(v, schema, *table, *field_def)) return false;
        break;
      case reflection::Obj: {
        auto child_obj = schema.objects()->Get(field_def->type()->index());
        if (child_obj->is_struct()) {
          if (!VerifyStruct(v, *table, field_def->offset(), *child_obj,
                            field_def->required())) {
            return false;
          }
        } else {
          if (!VerifyObject(v, schema, *child_obj,
                            flatbuffers::GetFieldT(*table, *field_def),
                            field_def->required())) {
            return false;
          }
        }
        break;
      }
      case reflection::Union: {
        //  get union type from the prev field
        voffset_t utype_offset = field_def->offset() - sizeof(voffset_t);
        auto utype = table->GetField<uint8_t>(utype_offset, 0);
        auto uval = reinterpret_cast<const uint8_t*>(
            flatbuffers::GetFieldT(*table, *field_def));
        if (!VerifyUnion(v, schema, utype, uval, *field_def)) {
          return false;
        }
        break;
      }
      default:
        FLATBUFFERS_ASSERT(false);
        break;
    }
  }

  if (!v.EndTable()) return false;

  return true;
}

}  // namespace

int64_t GetAnyValueI(reflection::BaseType type, const uint8_t* data) {
  // clang-format off
  #define FLATBUFFERS_GET(T) static_cast<int64_t>(ReadScalar<T>(data))
  switch (type) {
    case reflection::UType:
    case reflection::Bool:
    case reflection::UByte:  return FLATBUFFERS_GET(uint8_t);
    case reflection::Byte:   return FLATBUFFERS_GET(int8_t);
    case reflection::Short:  return FLATBUFFERS_GET(int16_t);
    case reflection::UShort: return FLATBUFFERS_GET(uint16_t);
    case reflection::Int:    return FLATBUFFERS_GET(int32_t);
    case reflection::UInt:   return FLATBUFFERS_GET(uint32_t);
    case reflection::Long:   return FLATBUFFERS_GET(int64_t);
    case reflection::ULong:  return FLATBUFFERS_GET(uint64_t);
    case reflection::Float:  return FLATBUFFERS_GET(float);
    case reflection::Double: return FLATBUFFERS_GET(double);
    case reflection::String: {
      auto s = reinterpret_cast<const String *>(ReadScalar<uoffset_t>(data) +
                                                data);
      return s ? StringToInt(s->c_str()) : 0;
    }
    default: return 0;  // Tables & vectors do not make sense.
  }
  #undef FLATBUFFERS_GET
  // clang-format on
}

double GetAnyValueF(reflection::BaseType type, const uint8_t* data) {
  switch (type) {
    case reflection::Float:
      return static_cast<double>(ReadScalar<float>(data));
    case reflection::Double:
      return ReadScalar<double>(data);
    case reflection::String: {
      auto s =
          reinterpret_cast<const String*>(ReadScalar<uoffset_t>(data) + data);
      if (s) {
        double d;
        StringToNumber(s->c_str(), &d);
        return d;
      } else {
        return 0.0;
      }
    }
    default:
      return static_cast<double>(GetAnyValueI(type, data));
  }
}

std::string GetAnyValueS(reflection::BaseType type, const uint8_t* data,
                         const reflection::Schema* schema, int type_index) {
  switch (type) {
    case reflection::Float:
    case reflection::Double:
      return NumToString(GetAnyValueF(type, data));
    case reflection::String: {
      auto s =
          reinterpret_cast<const String*>(ReadScalar<uoffset_t>(data) + data);
      return s ? s->c_str() : "";
    }
    case reflection::Obj:
      if (schema) {
        // Convert the table to a string. This is mostly for debugging purposes,
        // and does NOT promise to be JSON compliant.
        // Also prefixes the type.
        auto& objectdef = *schema->objects()->Get(type_index);
        auto s = objectdef.name()->str();
        if (objectdef.is_struct()) {
          s += "(struct)";  // TODO: implement this as well.
        } else {
          auto table_field = reinterpret_cast<const Table*>(
              ReadScalar<uoffset_t>(data) + data);
          s += " { ";
          auto fielddefs = objectdef.fields();
          for (auto it = fielddefs->begin(); it != fielddefs->end(); ++it) {
            auto& fielddef = **it;
            if (!table_field->CheckField(fielddef.offset())) continue;
            auto val = GetAnyFieldS(*table_field, fielddef, schema);
            if (fielddef.type()->base_type() == reflection::String) {
              std::string esc;
              flatbuffers::EscapeString(val.c_str(), val.length(), &esc, true,
                                        false);
              val = esc;
            }
            s += fielddef.name()->str();
            s += ": ";
            s += val;
            s += ", ";
          }
          s += "}";
        }
        return s;
      } else {
        return "(table)";
      }
    case reflection::Vector:
      return "[(elements)]";  // TODO: implement this as well.
    case reflection::Union:
      return "(union)";  // TODO: implement this as well.
    default:
      return NumToString(GetAnyValueI(type, data));
  }
}

void ForAllFields(const reflection::Object* object, bool reverse,
                  std::function<void(const reflection::Field*)> func) {
  std::vector<uint32_t> field_to_id_map;
  field_to_id_map.resize(object->fields()->size());

  // Create the mapping of field ID to the index into the vector.
  for (uint32_t i = 0; i < object->fields()->size(); ++i) {
    auto field = object->fields()->Get(i);
    field_to_id_map[field->id()] = i;
  }

  for (size_t i = 0; i < field_to_id_map.size(); ++i) {
    func(object->fields()->Get(
        field_to_id_map[reverse ? field_to_id_map.size() - (i + 1) : i]));
  }
}

void SetAnyValueI(reflection::BaseType type, uint8_t* data, int64_t val) {
  // clang-format off
  #define FLATBUFFERS_SET(T) WriteScalar(data, static_cast<T>(val))
  switch (type) {
    case reflection::UType:
    case reflection::Bool:
    case reflection::UByte:  FLATBUFFERS_SET(uint8_t ); break;
    case reflection::Byte:   FLATBUFFERS_SET(int8_t  ); break;
    case reflection::Short:  FLATBUFFERS_SET(int16_t ); break;
    case reflection::UShort: FLATBUFFERS_SET(uint16_t); break;
    case reflection::Int:    FLATBUFFERS_SET(int32_t ); break;
    case reflection::UInt:   FLATBUFFERS_SET(uint32_t); break;
    case reflection::Long:   FLATBUFFERS_SET(int64_t ); break;
    case reflection::ULong:  FLATBUFFERS_SET(uint64_t); break;
    case reflection::Float:  FLATBUFFERS_SET(float   ); break;
    case reflection::Double: FLATBUFFERS_SET(double  ); break;
    // TODO: support strings
    default: break;
  }
  #undef FLATBUFFERS_SET
  // clang-format on
}

void SetAnyValueF(reflection::BaseType type, uint8_t* data, double val) {
  switch (type) {
    case reflection::Float:
      WriteScalar(data, static_cast<float>(val));
      break;
    case reflection::Double:
      WriteScalar(data, val);
      break;
    // TODO: support strings.
    default:
      SetAnyValueI(type, data, static_cast<int64_t>(val));
      break;
  }
}

void SetAnyValueS(reflection::BaseType type, uint8_t* data, const char* val) {
  switch (type) {
    case reflection::Float:
    case reflection::Double: {
      double d;
      StringToNumber(val, &d);
      SetAnyValueF(type, data, d);
      break;
    }
    // TODO: support strings.
    default:
      SetAnyValueI(type, data, StringToInt(val));
      break;
  }
}

// The "<name>_type" field holding the type of union field "unionfield".
static const reflection::Field* GetUnionTypeField(
    const reflection::Object& parent, const reflection::Field& unionfield) {
  return parent.fields()->LookupByKey(
      (unionfield.name()->str() + UnionTypeFieldSuffix()).c_str());
}

// The type of the value (a table, struct or string) stored in union field
// "unionfield" with type tag "utype", or null for NONE and unknown tags.
static const reflection::Type* GetUnionMemberType(
    const reflection::Schema& schema, const reflection::Field& unionfield,
    uint8_t utype) {
  auto enumval = schema.enums()
                     ->Get(unionfield.type()->index())
                     ->values()
                     ->LookupByKey(utype);
  if (!enumval || !enumval->union_type()) return nullptr;
  const auto base_type = enumval->union_type()->base_type();
  if (base_type != reflection::Obj && base_type != reflection::String)
    return nullptr;
  return enumval->union_type();
}

// The force_align attribute of a field, or 0. Attributes are only present in
// schemas serialized with builtin attributes (flatc --bfbs-builtins).
static size_t GetForceAlign(const reflection::Field& field) {
  if (!field.attributes()) return 0;
  auto attr = field.attributes()->LookupByKey("force_align");
  if (!attr || !attr->value()) return 0;
  return static_cast<size_t>(StringToUInt(attr->value()->c_str()));
}

// For a "<name>_type" field (a union type tag, or a vector of them), the union
// field it holds the type of, if any.
static const reflection::Field* GetUnionFieldOfType(
    const reflection::Object& parent, const reflection::Field& typefield) {
  const auto& name = typefield.name()->str();
  const std::string suffix = UnionTypeFieldSuffix();
  if (name.size() <= suffix.size() ||
      name.compare(name.size() - suffix.size(), suffix.size(), suffix) != 0)
    return nullptr;
  auto unionfield = parent.fields()->LookupByKey(
      name.substr(0, name.size() - suffix.size()).c_str());
  if (!unionfield) return nullptr;
  auto type = unionfield->type();
  const bool is_union = type->base_type() == reflection::Union ||
                        (type->base_type() == reflection::Vector &&
                         type->element() == reflection::Union);
  return is_union ? unionfield : nullptr;
}

static bool HasAttribute(const reflection::Field& field, const char* name) {
  return field.attributes() && field.attributes()->LookupByKey(name);
}

// The largest alignment data described by "schema" may need: that of the
// largest scalar, or more for structs and vectors with force_align.
static size_t GetMaxAlignment(const reflection::Schema& schema) {
  size_t align = sizeof(largest_scalar_t);
  auto consider = [&align](size_t a) {
    if (a > align && VerifyAlignmentRequirements(a)) align = a;
  };
  if (!schema.objects()) return align;
  for (auto obj : *schema.objects()) {
    if (obj->is_struct()) consider(static_cast<size_t>(obj->minalign()));
    if (!obj->fields()) continue;
    for (auto field : *obj->fields()) consider(GetForceAlign(*field));
  }
  return align;
}

// The alignment a vector field of "size" elements is built with beyond what
// its elements need: its force_align, or that of the buffer nested in it.
// 0 if there is none.
static size_t GetExtraVectorAlignment(const reflection::Schema& schema,
                                      const reflection::Field& field,
                                      size_t size) {
  size_t align = 0;
  const auto force_align = GetForceAlign(field);
  if (VerifyAlignmentRequirements(force_align)) align = force_align;
  if (HasAttribute(field, "nested_flatbuffer")) {
    // A finished buffer is padded to a multiple of its alignment, so the
    // largest power of two dividing its size covers that alignment, without
    // padding after it when it is the first thing in the new buffer.
    align =
        std::max(align, std::min(GetMaxAlignment(schema), size & (~size + 1)));
  }
  return VerifyAlignmentRequirements(align) ? align : 0;
}

// Resize a FlatBuffer in-place by iterating through all offsets in the buffer
// and adjusting them by "delta" if they straddle the start offset.
// Once that is done, bytes can now be inserted/deleted safely.
// "delta" may be negative (shrinking).
// Unless "delta" is a multiple of the largest alignment, you'll create a small
// amount of garbage space in the buffer (usually 0..7 bytes, more if the
// schema uses force_align).
// If your FlatBuffer's root table is not the schema's root table, you should
// pass in your root_table type as well.
class ResizeContext {
 public:
  ResizeContext(const reflection::Schema& schema, uoffset_t start, int delta,
                std::vector<uint8_t>* flatbuf,
                const reflection::Object* root_table = nullptr)
      : schema_(schema),
        startptr_(flatbuf->data() + start),
        delta_(delta),
        buf_(*flatbuf),
        dag_check_(flatbuf->size() / sizeof(uoffset_t), false) {
    // Everything after start moves by delta_, so round it to the largest
    // alignment the data may need.
    auto mask = static_cast<int>(GetMaxAlignment(schema) - 1);
    delta_ = (delta_ + mask) & ~mask;
    if (!delta_) return;  // We can't shrink by less than that alignment.
    // Now change all the offsets by delta_.
    auto root = GetAnyRoot(buf_.data());
    Straddle<uoffset_t, 1>(buf_.data(), root, buf_.data());
    ResizeTable(root_table ? *root_table : *schema.root_table(), root);
    // We can now add or remove bytes at start.
    if (delta_ > 0)
      buf_.insert(buf_.begin() + start, delta_, 0);
    else
      buf_.erase(buf_.begin() + start + delta_, buf_.begin() + start);
  }

  // Check if the range between first (lower address) and second straddles
  // the insertion point. If it does, change the offset at offsetloc (of
  // type T, with direction D). Something that starts exactly at the insertion
  // point moves along with everything after it, so it doesn't straddle.
  template <typename T, int D>
  void Straddle(const void* first, const void* second, void* offsetloc) {
    if (first < startptr_ && second >= startptr_) {
      WriteScalar<T>(offsetloc, ReadScalar<T>(offsetloc) + delta_ * D);
      DagCheck(offsetloc) = true;
    }
  }

  // This returns a boolean that records if the corresponding offset location
  // has been modified already. If so, we can't even read the corresponding
  // offset, since it is pointing to a location that is illegal until the
  // resize actually happens.
  // This must be checked for every offset, since we can't know which offsets
  // will straddle and which won't.
  uint8_t& DagCheck(const void* offsetloc) {
    auto dag_idx = reinterpret_cast<const uoffset_t*>(offsetloc) -
                   reinterpret_cast<const uoffset_t*>(buf_.data());
    return dag_check_[dag_idx];
  }

  void ResizeTable(const reflection::Object& objectdef, Table* table) {
    if (DagCheck(table)) return;  // Table already visited.
    auto vtable = table->GetVTable();
    // Early out: since all fields inside the table must point forwards in
    // memory, if the insertion point is before the table we can stop here.
    auto tableloc = reinterpret_cast<uint8_t*>(table);
    if (startptr_ <= tableloc) {
      // Check if insertion point is between the table and a vtable that
      // precedes it. This can't happen in current construction code, but check
      // just in case we ever change the way flatbuffers are built. The table
      // moves away from its vtable, so its soffset (table - vtable) grows.
      Straddle<soffset_t, 1>(vtable, table, table);
    } else {
      // Check each field.
      auto fielddefs = objectdef.fields();
      for (auto it = fielddefs->begin(); it != fielddefs->end(); ++it) {
        auto& fielddef = **it;
        auto base_type = fielddef.type()->base_type();
        // Ignore scalars.
        if (base_type <= reflection::Double) continue;
        // Ignore fields that are not stored.
        auto offset = table->GetOptionalFieldOffset(fielddef.offset());
        if (!offset) continue;
        // Ignore structs.
        auto subobjectdef =
            base_type == reflection::Obj
                ? schema_.objects()->Get(fielddef.type()->index())
                : nullptr;
        if (subobjectdef && subobjectdef->is_struct()) continue;
        // Get this fields' offset, and read it if safe.
        auto offsetloc = tableloc + offset;
        if (DagCheck(offsetloc)) continue;  // This offset already visited.
        auto ref = offsetloc + ReadScalar<uoffset_t>(offsetloc);
        Straddle<uoffset_t, 1>(offsetloc, ref, offsetloc);
        // Recurse.
        switch (base_type) {
          case reflection::Obj: {
            if (subobjectdef) {
              ResizeTable(*subobjectdef, reinterpret_cast<Table*>(ref));
            }
            break;
          }
          case reflection::Vector: {
            auto elem_type = fielddef.type()->element();
            if (elem_type == reflection::Union) {
              ResizeUnionVector(objectdef, fielddef, *table,
                                reinterpret_cast<Vector<uoffset_t>*>(ref));
              break;
            }
            if (elem_type != reflection::Obj && elem_type != reflection::String)
              break;
            auto vec = reinterpret_cast<Vector<uoffset_t>*>(ref);
            auto elemobjectdef =
                elem_type == reflection::Obj
                    ? schema_.objects()->Get(fielddef.type()->index())
                    : nullptr;
            if (elemobjectdef && elemobjectdef->is_struct()) break;
            for (uoffset_t i = 0; i < vec->size(); i++) {
              auto loc = vec->Data() + i * sizeof(uoffset_t);
              if (DagCheck(loc)) continue;  // This offset already visited.
              auto dest = loc + vec->Get(i);
              Straddle<uoffset_t, 1>(loc, dest, loc);
              if (elemobjectdef)
                ResizeTable(*elemobjectdef, reinterpret_cast<Table*>(dest));
            }
            break;
          }
          case reflection::Union: {
            auto type_field = GetUnionTypeField(objectdef, fielddef);
            if (!type_field) break;
            ResizeUnionMember(fielddef, GetFieldI<uint8_t>(*table, *type_field),
                              ref);
            break;
          }
          case reflection::String:
            break;
          default:
            FLATBUFFERS_ASSERT(false);
        }
      }
      // Check if the vtable offset points beyond the insertion point.
      // Must do this last, since GetOptionalFieldOffset above still reads
      // this value.
      Straddle<soffset_t, -1>(table, vtable, table);
    }
  }

  // Unions can also hold structs and strings, which contain no offsets.
  void ResizeUnionMember(const reflection::Field& unionfield, uint8_t utype,
                         uint8_t* value) {
    auto member = GetUnionMemberType(schema_, unionfield, utype);
    if (!member || member->base_type() != reflection::Obj) return;
    auto& memberdef = *schema_.objects()->Get(member->index());
    if (!memberdef.is_struct())
      ResizeTable(memberdef, reinterpret_cast<Table*>(value));
  }

  void ResizeUnionVector(const reflection::Object& objectdef,
                         const reflection::Field& fielddef, const Table& table,
                         Vector<uoffset_t>* vec) {
    // The element types are in the "<name>_type" vector.
    auto type_field = GetUnionTypeField(objectdef, fielddef);
    if (!type_field) return;
    auto type_offset = table.GetOptionalFieldOffset(type_field->offset());
    if (!type_offset) return;
    auto typesloc = reinterpret_cast<const uint8_t*>(&table) + type_offset;
    auto types_rel = ReadScalar<uoffset_t>(typesloc);
    // If that offset was adjusted already, undo that to find the vector now.
    if (DagCheck(typesloc)) types_rel -= static_cast<uoffset_t>(delta_);
    auto types = reinterpret_cast<const Vector<uint8_t>*>(typesloc + types_rel);
    for (uoffset_t i = 0; i < vec->size() && i < types->size(); i++) {
      if (!types->Get(i)) continue;  // NONE.
      auto loc = vec->Data() + i * sizeof(uoffset_t);
      if (DagCheck(loc)) continue;  // This offset already visited.
      auto dest = loc + vec->Get(i);
      Straddle<uoffset_t, 1>(loc, dest, loc);
      ResizeUnionMember(fielddef, types->Get(i), dest);
    }
  }

 private:
  const reflection::Schema& schema_;
  uint8_t* startptr_;
  int delta_;
  std::vector<uint8_t>& buf_;
  std::vector<uint8_t> dag_check_;
};

void SetString(const reflection::Schema& schema, const std::string& val,
               const String* str, std::vector<uint8_t>* flatbuf,
               const reflection::Object* root_table) {
  const auto old_size = str->size();
  auto delta = static_cast<int>(val.size()) - static_cast<int>(old_size);
  auto str_start = static_cast<uoffset_t>(
      reinterpret_cast<const uint8_t*>(str) - flatbuf->data());
  auto start = str_start + static_cast<uoffset_t>(sizeof(uoffset_t));
  if (delta) {
    // Clear the old string, since we don't want parts of it remaining.
    memset(flatbuf->data() + start, 0, old_size);
    // Different size, we must expand (or contract). Grow at the first
    // character, but shrink from the end of the old characters, so that only
    // bytes of the old string are removed, never its length or what precedes
    // it.
    ResizeContext ctx(schema, delta < 0 ? start + old_size : start, delta,
                      flatbuf, root_table);
    // Set the new length.
    WriteScalar(flatbuf->data() + str_start,
                static_cast<uoffset_t>(val.size()));
  }
  // Copy new data. Safe because we created the right amount of space.
  memcpy(flatbuf->data() + start, val.c_str(), val.size() + 1);
}

uint8_t* ResizeAnyVector(const reflection::Schema& schema, uoffset_t newsize,
                         const VectorOfAny* vec, uoffset_t num_elems,
                         uoffset_t elem_size, std::vector<uint8_t>* flatbuf,
                         const reflection::Object* root_table) {
  auto delta_elem = static_cast<int>(newsize) - static_cast<int>(num_elems);
  auto delta_bytes = delta_elem * static_cast<int>(elem_size);
  auto vec_start = reinterpret_cast<const uint8_t*>(vec) - flatbuf->data();
  auto start = static_cast<uoffset_t>(vec_start) +
               static_cast<uoffset_t>(sizeof(uoffset_t)) +
               elem_size * num_elems;
  if (delta_bytes) {
    if (delta_elem < 0) {
      // Clear elements we're throwing away, since some might remain in the
      // buffer.
      auto size_clear = -delta_elem * elem_size;
      memset(flatbuf->data() + start - size_clear, 0, size_clear);
    }
    ResizeContext ctx(schema, start, delta_bytes, flatbuf, root_table);
    WriteScalar(flatbuf->data() + vec_start, newsize);  // Length field.
    // Set new elements to 0.. this can be overwritten by the caller.
    if (delta_elem > 0) {
      memset(flatbuf->data() + start, 0,
             static_cast<size_t>(delta_elem) * elem_size);
    }
  }
  return flatbuf->data() + start;
}

const uint8_t* AddFlatBuffer(std::vector<uint8_t>& flatbuf,
                             const uint8_t* newbuf, size_t newlen) {
  if (!newbuf || newlen < sizeof(uoffset_t)) return nullptr;
  FLATBUFFERS_ASSERT(newlen >= sizeof(uoffset_t));
  auto root = ReadScalar<uoffset_t>(newbuf);
  if (root < sizeof(uoffset_t) || root >= newlen) return nullptr;
  // We're going to chop off the root offset, so align to sizeof(uoffset_t)
  // past the largest alignment newbuf may contain (structs with force_align).
  const size_t align =
      std::max<size_t>(FLATBUFFERS_MAX_ALIGNMENT, sizeof(largest_scalar_t));
  while ((flatbuf.size() & (align - 1)) != sizeof(uoffset_t)) {
    flatbuf.push_back(0);
  }
  auto insertion_point = static_cast<uoffset_t>(flatbuf.size());
  // Insert the entire FlatBuffer minus the root pointer.
  flatbuf.insert(flatbuf.end(), newbuf + sizeof(uoffset_t), newbuf + newlen);
  auto root_offset = root - sizeof(uoffset_t);
  return flatbuf.data() + insertion_point + root_offset;
}

// Copies a union value (a table, struct or string) with type tag "utype".
// Returns 0 for NONE and unknown tags.
static uoffset_t CopyUnionMember(FlatBufferBuilder& fbb,
                                 const reflection::Schema& schema,
                                 const reflection::Field& unionfield,
                                 uint8_t utype, const uint8_t* value,
                                 bool use_string_pooling) {
  auto member = GetUnionMemberType(schema, unionfield, utype);
  if (!member) return 0;
  if (member->base_type() == reflection::String) {
    auto str = reinterpret_cast<const String*>(value);
    return use_string_pooling ? fbb.CreateSharedString(str).o
                              : fbb.CreateString(str).o;
  }
  auto& memberdef = *schema.objects()->Get(member->index());
  if (!memberdef.is_struct()) {
    return CopyTable(fbb, schema, memberdef,
                     *reinterpret_cast<const Table*>(value), use_string_pooling)
        .o;
  }
  // Structs in unions are stored out of line, like FlatBufferBuilder's
  // CreateStruct does.
  const auto align = static_cast<size_t>(memberdef.minalign());
  if (VerifyAlignmentRequirements(align)) fbb.Align(align);
  fbb.PushBytes(value, static_cast<size_t>(memberdef.bytesize()));
  return static_cast<uoffset_t>(fbb.GetSize());
}

Offset<const Table*> CopyTable(FlatBufferBuilder& fbb,
                               const reflection::Schema& schema,
                               const reflection::Object& objectdef,
                               const Table& table, bool use_string_pooling) {
  // Before we can construct the table, we have to first generate any
  // subobjects, and collect their offsets.
  std::vector<uoffset_t> offsets;
  auto fielddefs = objectdef.fields();
  for (auto it = fielddefs->begin(); it != fielddefs->end(); ++it) {
    auto& fielddef = **it;
    // Skip if field is not present in the source.
    if (!table.CheckField(fielddef.offset())) continue;
    uoffset_t offset = 0;
    switch (fielddef.type()->base_type()) {
      case reflection::String: {
        offset = use_string_pooling
                     ? fbb.CreateSharedString(GetFieldS(table, fielddef)).o
                     : fbb.CreateString(GetFieldS(table, fielddef)).o;
        break;
      }
      case reflection::Obj: {
        auto& subobjectdef = *schema.objects()->Get(fielddef.type()->index());
        if (!subobjectdef.is_struct()) {
          offset = CopyTable(fbb, schema, subobjectdef,
                             *GetFieldT(table, fielddef), use_string_pooling)
                       .o;
        }
        break;
      }
      case reflection::Union: {
        auto type_field = GetUnionTypeField(objectdef, fielddef);
        if (!type_field) break;
        offset = CopyUnionMember(
            fbb, schema, fielddef, GetFieldI<uint8_t>(table, *type_field),
            table.GetPointer<const uint8_t*>(fielddef.offset()),
            use_string_pooling);
        break;
      }
      case reflection::Vector: {
        auto vec =
            table.GetPointer<const Vector<Offset<Table>>*>(fielddef.offset());
        auto element_base_type = fielddef.type()->element();
        auto elemobjectdef =
            element_base_type == reflection::Obj
                ? schema.objects()->Get(fielddef.type()->index())
                : nullptr;
        // Keep any extra alignment the source vector was built with.
        const auto extra_alignment =
            GetExtraVectorAlignment(schema, fielddef, vec->size());
        switch (element_base_type) {
          case reflection::String: {
            std::vector<Offset<const String*>> elements(vec->size());
            auto vec_s = reinterpret_cast<const Vector<Offset<String>>*>(vec);
            for (uoffset_t i = 0; i < vec_s->size(); i++) {
              elements[i] = use_string_pooling
                                ? fbb.CreateSharedString(vec_s->Get(i)).o
                                : fbb.CreateString(vec_s->Get(i)).o;
            }
            if (extra_alignment > sizeof(uoffset_t)) {
              fbb.ForceVectorAlignment(elements.size(), sizeof(uoffset_t),
                                       extra_alignment);
            }
            offset = fbb.CreateVector(elements).o;
            break;
          }
          case reflection::Union: {
            // Copy each value according to its tag in the "<name>_type"
            // vector, which itself is copied as a vector of scalars.
            auto type_field = GetUnionTypeField(objectdef, fielddef);
            auto types = type_field ? table.GetPointer<const Vector<uint8_t>*>(
                                          type_field->offset())
                                    : nullptr;
            auto values = reinterpret_cast<const Vector<uoffset_t>*>(vec);
            std::vector<uoffset_t> elements(values->size(), 0);
            for (uoffset_t i = 0; i < values->size(); i++) {
              if (!types || i >= types->size()) break;
              auto loc = values->Data() + i * sizeof(uoffset_t);
              elements[i] =
                  CopyUnionMember(fbb, schema, fielddef, types->Get(i),
                                  loc + values->Get(i), use_string_pooling);
            }
            fbb.StartVector(elements.size(), sizeof(uoffset_t),
                            std::max(sizeof(uoffset_t), extra_alignment));
            for (auto i = elements.size(); i-- > 0;) {
              // NONE elements have no value to point to.
              if (elements[i])
                fbb.PushElement(Offset<void>(elements[i]));
              else
                fbb.PushElement<uoffset_t>(0);
            }
            offset = fbb.EndVector(elements.size());
            break;
          }
          case reflection::Obj: {
            if (!elemobjectdef->is_struct()) {
              std::vector<Offset<const Table*>> elements(vec->size());
              for (uoffset_t i = 0; i < vec->size(); i++) {
                elements[i] = CopyTable(fbb, schema, *elemobjectdef,
                                        *vec->Get(i), use_string_pooling);
              }
              if (extra_alignment > sizeof(uoffset_t)) {
                fbb.ForceVectorAlignment(elements.size(), sizeof(uoffset_t),
                                         extra_alignment);
              }
              offset = fbb.CreateVector(elements).o;
              break;
            }
          }
            FLATBUFFERS_FALLTHROUGH();  // fall thru
          default: {                    // Scalars and structs.
            auto element_size = GetTypeSize(element_base_type);
            auto element_alignment = element_size;  // For primitive elements
            if (elemobjectdef && elemobjectdef->is_struct()) {
              element_size = elemobjectdef->bytesize();
              const auto struct_alignment =
                  static_cast<size_t>(elemobjectdef->minalign());
              if (VerifyAlignmentRequirements(struct_alignment))
                element_alignment = struct_alignment;
            }
            element_alignment = std::max(element_alignment, extra_alignment);
            const uint8_t* data = vec->Data();
            std::vector<uint8_t> tags;
            if (element_base_type == reflection::UType) {
              if (auto unionfield = GetUnionFieldOfType(objectdef, fielddef)) {
                // Tags of union values that can't be copied (see above)
                // become NONE.
                tags.assign(data, data + vec->size());
                for (auto& tag : tags) {
                  if (!GetUnionMemberType(schema, *unionfield, tag)) tag = 0;
                }
                data = tags.data();
              }
            }
            fbb.StartVector(vec->size(), element_size, element_alignment);
            fbb.PushBytes(data, element_size * vec->size());
            offset = fbb.EndVector(vec->size());
            break;
          }
        }
        break;
      }
      default:  // Scalars.
        break;
    }
    // The loop below takes one offset for each such field (0 for none, such as
    // a NONE union), so the two stay in step.
    const auto base_type = fielddef.type()->base_type();
    if (base_type == reflection::String || base_type == reflection::Union ||
        base_type == reflection::Vector ||
        (base_type == reflection::Obj &&
         !schema.objects()->Get(fielddef.type()->index())->is_struct())) {
      offsets.push_back(offset);
    }
  }
  // Now we can build the actual table from either offsets or scalar data.
  auto start = objectdef.is_struct() ? fbb.StartStruct(objectdef.minalign())
                                     : fbb.StartTable();
  size_t offset_idx = 0;
  for (auto it = fielddefs->begin(); it != fielddefs->end(); ++it) {
    auto& fielddef = **it;
    if (!table.CheckField(fielddef.offset())) continue;
    auto base_type = fielddef.type()->base_type();
    switch (base_type) {
      case reflection::Obj: {
        auto& subobjectdef = *schema.objects()->Get(fielddef.type()->index());
        if (subobjectdef.is_struct()) {
          CopyInline(fbb, fielddef, table, subobjectdef.minalign(),
                     subobjectdef.bytesize());
          break;
        }
      }
        FLATBUFFERS_FALLTHROUGH();  // fall thru
      case reflection::Union:
      case reflection::String:
      case reflection::Vector:
        fbb.AddOffset(fielddef.offset(), Offset<void>(offsets[offset_idx++]));
        break;
      default: {  // Scalars.
        if (base_type == reflection::UType) {
          // Leave out the tag of a union value that couldn't be copied (see
          // above), so the union becomes NONE instead of referring to nothing.
          auto unionfield = GetUnionFieldOfType(objectdef, fielddef);
          if (unionfield &&
              !GetUnionMemberType(schema, *unionfield,
                                  GetFieldI<uint8_t>(table, fielddef)))
            break;
        }
        auto size = GetTypeSize(base_type);
        CopyInline(fbb, fielddef, table, size, size);
        break;
      }
    }
  }
  FLATBUFFERS_ASSERT(offset_idx == offsets.size());
  if (objectdef.is_struct()) {
    fbb.ClearOffsets();
    return fbb.EndStruct();
  } else {
    return fbb.EndTable(start);
  }
}

bool Verify(const reflection::Schema& schema, const reflection::Object& root,
            const uint8_t* const buf, const size_t length,
            const uoffset_t max_depth, const uoffset_t max_tables) {
  Verifier v(buf, length, max_depth, max_tables);
  return VerifyObject(v, schema, root, flatbuffers::GetAnyRoot(buf),
                      /*required=*/true);
}

bool VerifySizePrefixed(const reflection::Schema& schema,
                        const reflection::Object& root,
                        const uint8_t* const buf, const size_t length,
                        const uoffset_t max_depth, const uoffset_t max_tables) {
  Verifier v(buf, length, max_depth, max_tables);
  return VerifyObject(v, schema, root, flatbuffers::GetAnySizePrefixedRoot(buf),
                      /*required=*/true);
}

}  // namespace flatbuffers
