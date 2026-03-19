/*
 * Copyright 2017 Google Inc. All rights reserved.
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

#ifndef FLATBUFFERS_MINIREFLECT_H_
#define FLATBUFFERS_MINIREFLECT_H_

#include <limits>

#include "flatbuffers/flatbuffers.h"
#include "flatbuffers/util.h"

namespace flatbuffers {

// Utilities that can be used with the "mini reflection" tables present
// in generated code with --reflect-types (only types) or --reflect-names
// (also names).
// This allows basic reflection functionality such as pretty-printing
// that does not require the use of the schema parser or loading of binary
// schema files at runtime (reflection.h).

// For any of the functions below that take `const TypeTable *`, you pass
// `FooTypeTable()` if the type of the root is `Foo`.

// First, a generic iterator that can be used by multiple algorithms.

struct IterationVisitor {
  // These mark the scope of a table or struct.
  virtual void StartSequence() {}
  virtual void EndSequence() {}
  // Called for each field regardless of whether it is present or not.
  // If not present, val == nullptr. set_idx is the index of all set fields.
  virtual void Field(size_t /*field_idx*/, size_t /*set_idx*/,
                     ElementaryType /*type*/, bool /*is_vector*/,
                     const TypeTable* /*type_table*/, const char* /*name*/,
                     const uint8_t* /*val*/) {}
  // Called for a value that is actually present, after a field, or as part
  // of a vector.
  virtual void UType(uint8_t, const char*) {}
  virtual void Bool(bool) {}
  virtual void Char(int8_t, const char*) {}
  virtual void UChar(uint8_t, const char*) {}
  virtual void Short(int16_t, const char*) {}
  virtual void UShort(uint16_t, const char*) {}
  virtual void Int(int32_t, const char*) {}
  virtual void UInt(uint32_t, const char*) {}
  virtual void Long(int64_t) {}
  virtual void ULong(uint64_t) {}
  virtual void Float(float) {}
  virtual void Double(double) {}
  virtual void String(const String*) {}
  virtual void Unknown(const uint8_t*) {}  // From a future version.
  // These mark the scope of a vector.
  virtual void StartVector() {}
  virtual void EndVector() {}
  virtual void Element(size_t /*i*/, ElementaryType /*type*/,
                       const TypeTable* /*type_table*/,
                       const uint8_t* /*val*/) {}
  virtual ~IterationVisitor() {}
};

inline size_t InlineSize(ElementaryType type, const TypeTable* type_table) {
  switch (type) {
    case ET_UTYPE:
    case ET_BOOL:
    case ET_CHAR:
    case ET_UCHAR:
      return 1;
    case ET_SHORT:
    case ET_USHORT:
      return 2;
    case ET_INT:
    case ET_UINT:
    case ET_FLOAT:
    case ET_STRING:
      return 4;
    case ET_LONG:
    case ET_ULONG:
    case ET_DOUBLE:
      return 8;
    case ET_SEQUENCE:
      switch (type_table->st) {
        case ST_TABLE:
        case ST_UNION:
          return 4;
        case ST_STRUCT:
          return static_cast<size_t>(type_table->values[type_table->num_elems]);
        default:
          FLATBUFFERS_ASSERT(false);
          return 1;
      }
    default:
      FLATBUFFERS_ASSERT(false);
      return 1;
  }
}

inline int64_t LookupEnum(int64_t enum_val, const int64_t* values,
                          size_t num_values) {
  if (!values) return enum_val;
  for (size_t i = 0; i < num_values; i++) {
    if (enum_val == values[i]) return static_cast<int64_t>(i);
  }
  return -1;  // Unknown enum value.
}

template <typename T>
const char* EnumName(T tval, const TypeTable* type_table) {
  if (!type_table || !type_table->names) return nullptr;
  auto i = LookupEnum(static_cast<int64_t>(tval), type_table->values,
                      type_table->num_elems);
  if (i >= 0 && i < static_cast<int64_t>(type_table->num_elems)) {
    return type_table->names[i];
  }
  return nullptr;
}

inline bool MiniReflectInBounds(const uint8_t* p, size_t size,
                                const uint8_t* begin, const uint8_t* end) {
  if (!begin || !end) return true;
  if (!p || p < begin || p > end) return false;
  return size <= static_cast<size_t>(end - p);
}

template <typename T>
inline bool MiniReflectReadScalar(const uint8_t* p, const uint8_t* begin,
                                  const uint8_t* end, T* val) {
  if (!MiniReflectInBounds(p, sizeof(T), begin, end)) return false;
  *val = ReadScalar<T>(p);
  return true;
}

inline bool MiniReflectAdvance(const uint8_t* p, size_t offset,
                               const uint8_t* begin, const uint8_t* end,
                               const uint8_t** out) {
  if (!p || !out) return false;
  if (!begin || !end) {
    *out = p + offset;
    return true;
  }
  if (p > end || offset > static_cast<size_t>(end - p)) return false;
  *out = p + offset;
  return true;
}

inline const uint8_t* MiniReflectGetTableFieldAddress(const uint8_t* obj,
                                                      voffset_t field,
                                                      const uint8_t* begin,
                                                      const uint8_t* end) {
  if (!begin || !end) {
    return reinterpret_cast<const Table*>(obj)->GetAddressOf(field);
  }
  soffset_t voff = 0;
  if (!MiniReflectReadScalar(obj, begin, end, &voff)) return nullptr;
  if (voff < 0 || static_cast<size_t>(voff) > static_cast<size_t>(obj - begin)) {
    return nullptr;
  }
  const auto* vtable = obj - voff;
  voffset_t vsize = 0;
  voffset_t tsize = 0;
  if (!MiniReflectReadScalar(vtable, begin, end, &vsize) ||
      !MiniReflectReadScalar(vtable + sizeof(voffset_t), begin, end, &tsize) ||
      field >= vsize) {
    return nullptr;
  }
  voffset_t foff = 0;
  if (!MiniReflectReadScalar(vtable + field, begin, end, &foff) ||
      !foff || foff >= tsize) {
    return nullptr;
  }
  const uint8_t* p = nullptr;
  return MiniReflectAdvance(obj, foff, begin, end, &p) ? p : nullptr;
}

void IterateObject(const uint8_t* obj, const TypeTable* type_table,
                   IterationVisitor* visitor, const uint8_t* begin = nullptr,
                   const uint8_t* end = nullptr);

inline void IterateValue(ElementaryType type, const uint8_t* val,
                         const TypeTable* type_table, const uint8_t* prev_val,
                         soffset_t vector_index, IterationVisitor* visitor,
                         const uint8_t* begin = nullptr,
                         const uint8_t* end = nullptr) {
  switch (type) {
    case ET_UTYPE: {
      uint8_t tval = 0;
      if (!MiniReflectReadScalar(val, begin, end, &tval)) {
        visitor->Unknown(val);
        return;
      }
      visitor->UType(tval, EnumName(tval, type_table));
      break;
    }
    case ET_BOOL: {
      uint8_t tval = 0;
      if (!MiniReflectReadScalar(val, begin, end, &tval)) {
        visitor->Unknown(val);
        return;
      }
      visitor->Bool(tval != 0);
      break;
    }
    case ET_CHAR: {
      int8_t tval = 0;
      if (!MiniReflectReadScalar(val, begin, end, &tval)) {
        visitor->Unknown(val);
        return;
      }
      visitor->Char(tval, EnumName(tval, type_table));
      break;
    }
    case ET_UCHAR: {
      uint8_t tval = 0;
      if (!MiniReflectReadScalar(val, begin, end, &tval)) {
        visitor->Unknown(val);
        return;
      }
      visitor->UChar(tval, EnumName(tval, type_table));
      break;
    }
    case ET_SHORT: {
      int16_t tval = 0;
      if (!MiniReflectReadScalar(val, begin, end, &tval)) {
        visitor->Unknown(val);
        return;
      }
      visitor->Short(tval, EnumName(tval, type_table));
      break;
    }
    case ET_USHORT: {
      uint16_t tval = 0;
      if (!MiniReflectReadScalar(val, begin, end, &tval)) {
        visitor->Unknown(val);
        return;
      }
      visitor->UShort(tval, EnumName(tval, type_table));
      break;
    }
    case ET_INT: {
      int32_t tval = 0;
      if (!MiniReflectReadScalar(val, begin, end, &tval)) {
        visitor->Unknown(val);
        return;
      }
      visitor->Int(tval, EnumName(tval, type_table));
      break;
    }
    case ET_UINT: {
      uint32_t tval = 0;
      if (!MiniReflectReadScalar(val, begin, end, &tval)) {
        visitor->Unknown(val);
        return;
      }
      visitor->UInt(tval, EnumName(tval, type_table));
      break;
    }
    case ET_LONG: {
      int64_t tval = 0;
      if (!MiniReflectReadScalar(val, begin, end, &tval)) {
        visitor->Unknown(val);
        return;
      }
      visitor->Long(tval);
      break;
    }
    case ET_ULONG: {
      uint64_t tval = 0;
      if (!MiniReflectReadScalar(val, begin, end, &tval)) {
        visitor->Unknown(val);
        return;
      }
      visitor->ULong(tval);
      break;
    }
    case ET_FLOAT: {
      float tval = 0.0f;
      if (!MiniReflectReadScalar(val, begin, end, &tval)) {
        visitor->Unknown(val);
        return;
      }
      visitor->Float(tval);
      break;
    }
    case ET_DOUBLE: {
      double tval = 0.0;
      if (!MiniReflectReadScalar(val, begin, end, &tval)) {
        visitor->Unknown(val);
        return;
      }
      visitor->Double(tval);
      break;
    }
    case ET_STRING: {
      uoffset_t off = 0;
      const uint8_t* str = nullptr;
      uoffset_t len = 0;
      if (!MiniReflectReadScalar(val, begin, end, &off) ||
          !MiniReflectAdvance(val, off, begin, end, &str) ||
          !MiniReflectReadScalar(str, begin, end, &len) ||
          !MiniReflectInBounds(str, sizeof(uoffset_t) + len + 1, begin, end)) {
        visitor->Unknown(val);
        return;
      }
      visitor->String(reinterpret_cast<const String*>(str));
      break;
    }
    case ET_SEQUENCE: {
      switch (type_table->st) {
        case ST_TABLE: {
          uoffset_t off = 0;
          const uint8_t* ptr = nullptr;
          if (!MiniReflectReadScalar(val, begin, end, &off) ||
              !MiniReflectAdvance(val, off, begin, end, &ptr)) {
            visitor->Unknown(val);
            return;
          }
          IterateObject(ptr, type_table, visitor, begin, end);
          break;
        }
        case ST_STRUCT:
          IterateObject(val, type_table, visitor, begin, end);
          break;
        case ST_UNION: {
          uoffset_t off = 0;
          const uint8_t* ptr = nullptr;
          if (!MiniReflectReadScalar(val, begin, end, &off) ||
              !MiniReflectAdvance(val, off, begin, end, &ptr)) {
            visitor->Unknown(val);
            return;
          }
          FLATBUFFERS_ASSERT(prev_val);
          uint8_t union_type = 0;
          if (!MiniReflectReadScalar(prev_val, begin, end, &union_type)) {
            visitor->Unknown(val);
            return;
          }
          if (vector_index >= 0) {
            uoffset_t type_vec_size = 0;
            const uint8_t* type_vec_data = nullptr;
            if (!MiniReflectReadScalar(prev_val, begin, end, &type_vec_size) ||
                !MiniReflectAdvance(prev_val, sizeof(uoffset_t), begin, end,
                                   &type_vec_data) ||
                !MiniReflectInBounds(type_vec_data, type_vec_size, begin, end) ||
                static_cast<uoffset_t>(vector_index) >= type_vec_size ||
                !MiniReflectReadScalar(
                  type_vec_data + static_cast<size_t>(vector_index), begin,
                  end,
                                       &union_type)) {
              visitor->Unknown(val);
              return;
            }
          }
          auto type_code_idx =
              LookupEnum(union_type, type_table->values, type_table->num_elems);
          if (type_code_idx >= 0 &&
              type_code_idx < static_cast<int32_t>(type_table->num_elems)) {
            auto type_code = type_table->type_codes[type_code_idx];
            switch (type_code.base_type) {
              case ET_SEQUENCE: {
                auto ref = type_table->type_refs[type_code.sequence_ref]();
                IterateObject(ptr, ref, visitor, begin, end);
                break;
              }
              case ET_STRING:
                {
                  uoffset_t len = 0;
                  if (!MiniReflectReadScalar(ptr, begin, end, &len) ||
                      !MiniReflectInBounds(ptr,
                                           sizeof(uoffset_t) + len + 1,
                                           begin, end)) {
                    visitor->Unknown(ptr);
                    return;
                  }
                  visitor->String(reinterpret_cast<const String*>(ptr));
                }
                break;
              default:
                visitor->Unknown(ptr);
            }
          } else {
            visitor->Unknown(ptr);
          }
          break;
        }
        case ST_ENUM:
          FLATBUFFERS_ASSERT(false);
          break;
      }
      break;
    }
    default: {
      visitor->Unknown(val);
      break;
    }
  }
}

inline void IterateObject(const uint8_t* obj, const TypeTable* type_table,
                          IterationVisitor* visitor, const uint8_t* begin,
                          const uint8_t* end) {
  visitor->StartSequence();
  const uint8_t* prev_val = nullptr;
  size_t set_idx = 0;
  size_t array_idx = 0;
  for (size_t i = 0; i < type_table->num_elems; i++) {
    auto type_code = type_table->type_codes[i];
    auto type = static_cast<ElementaryType>(type_code.base_type);
    auto is_repeating = type_code.is_repeating != 0;
    auto ref_idx = type_code.sequence_ref;
    const TypeTable* ref = nullptr;
    if (ref_idx >= 0) {
      ref = type_table->type_refs[ref_idx]();
    }
    auto name = type_table->names ? type_table->names[i] : nullptr;
    const uint8_t* val = nullptr;
    if (type_table->st == ST_TABLE) {
      val = MiniReflectGetTableFieldAddress(
          obj, FieldIndexToOffset(static_cast<voffset_t>(i)), begin, end);
    } else {
      if (!MiniReflectAdvance(obj, type_table->values[i], begin, end, &val)) {
        visitor->Unknown(obj);
        return;
      }
    }
    visitor->Field(i, set_idx, type, is_repeating, ref, name, val);
    if (val) {
      set_idx++;
      if (is_repeating) {
        auto elem_ptr = val;
        size_t size = 0;
        if (type_table->st == ST_TABLE) {
          // variable length vector
          uoffset_t off = 0;
          uoffset_t vec_size = 0;
          if (!MiniReflectReadScalar(val, begin, end, &off) ||
              !MiniReflectAdvance(val, off, begin, end, &val) ||
              !MiniReflectReadScalar(val, begin, end, &vec_size)) {
            visitor->Unknown(val);
            return;
          }
          if (!MiniReflectAdvance(val, sizeof(uoffset_t), begin, end,
                                  &elem_ptr)) {
            visitor->Unknown(val);
            return;
          }
          size = vec_size;
          const auto elem_size = InlineSize(type, ref);
          if (size > (std::numeric_limits<size_t>::max)() / elem_size) {
            visitor->Unknown(elem_ptr);
            return;
          }
          const auto bytes = size * elem_size;
          if (!MiniReflectInBounds(elem_ptr, bytes, begin, end)) {
            visitor->Unknown(elem_ptr);
            return;
          }
        } else {
          // otherwise fixed size array
          size = type_table->array_sizes[array_idx];
          ++array_idx;
          const auto elem_size = InlineSize(type, ref);
          if (size > (std::numeric_limits<size_t>::max)() / elem_size ||
              !MiniReflectInBounds(elem_ptr, size * elem_size, begin, end)) {
            visitor->Unknown(elem_ptr);
            return;
          }
        }
        visitor->StartVector();
        for (size_t j = 0; j < size; j++) {
          visitor->Element(j, type, ref, elem_ptr);
          IterateValue(type, elem_ptr, ref, prev_val, static_cast<soffset_t>(j),
                       visitor, begin, end);
          elem_ptr += InlineSize(type, ref);
        }
        visitor->EndVector();
      } else {
        IterateValue(type, val, ref, prev_val, -1, visitor, begin, end);
      }
    }
    prev_val = val;
  }
  visitor->EndSequence();
}

inline void IterateFlatBuffer(const uint8_t* buffer,
                              const TypeTable* type_table,
                              IterationVisitor* callback) {
  IterateObject(GetRoot<uint8_t>(buffer), type_table, callback, nullptr,
                nullptr);
}

inline void IterateFlatBuffer(const uint8_t* buffer, size_t buffer_size,
                              const TypeTable* type_table,
                              IterationVisitor* callback) {
  if (buffer_size < sizeof(uoffset_t)) return;
  const auto* begin = buffer;
  const auto* end = buffer + buffer_size;
  uoffset_t root = 0;
  const uint8_t* obj = nullptr;
  if (!MiniReflectReadScalar(buffer, begin, end, &root) ||
      !MiniReflectAdvance(buffer, root, begin, end, &obj)) {
    return;
  }
  IterateObject(obj, type_table, callback, begin, end);
}

// Outputting a Flatbuffer to a string. Tries to conform as close to JSON /
// the output generated by idl_gen_text.cpp.

struct ToStringVisitor : public IterationVisitor {
  std::string s;
  std::string d;
  bool q;
  std::string in;
  size_t indent_level;
  bool vector_delimited;
  ToStringVisitor(std::string delimiter, bool quotes, std::string indent,
                  bool vdelimited = true)
      : d(delimiter),
        q(quotes),
        in(indent),
        indent_level(0),
        vector_delimited(vdelimited) {}
  ToStringVisitor(std::string delimiter)
      : d(delimiter),
        q(false),
        in(""),
        indent_level(0),
        vector_delimited(true) {}

  void append_indent() {
    for (size_t i = 0; i < indent_level; i++) {
      s += in;
    }
  }

  void StartSequence() {
    s += "{";
    s += d;
    indent_level++;
  }
  void EndSequence() {
    s += d;
    indent_level--;
    append_indent();
    s += "}";
  }
  void Field(size_t /*field_idx*/, size_t set_idx, ElementaryType /*type*/,
             bool /*is_vector*/, const TypeTable* /*type_table*/,
             const char* name, const uint8_t* val) {
    if (!val) return;
    if (set_idx) {
      s += ",";
      s += d;
    }
    append_indent();
    if (name) {
      if (q) s += "\"";
      s += name;
      if (q) s += "\"";
      s += ": ";
    }
  }
  template <typename T>
  void Named(T x, const char* name) {
    if (name) {
      if (q) s += "\"";
      s += name;
      if (q) s += "\"";
    } else {
      s += NumToString(x);
    }
  }
  void UType(uint8_t x, const char* name) { Named(x, name); }
  void Bool(bool x) { s += x ? "true" : "false"; }
  void Char(int8_t x, const char* name) { Named(x, name); }
  void UChar(uint8_t x, const char* name) { Named(x, name); }
  void Short(int16_t x, const char* name) { Named(x, name); }
  void UShort(uint16_t x, const char* name) { Named(x, name); }
  void Int(int32_t x, const char* name) { Named(x, name); }
  void UInt(uint32_t x, const char* name) { Named(x, name); }
  void Long(int64_t x) { s += NumToString(x); }
  void ULong(uint64_t x) { s += NumToString(x); }
  void Float(float x) { s += NumToString(x); }
  void Double(double x) { s += NumToString(x); }
  void String(const struct String* str) {
    EscapeString(str->c_str(), str->size(), &s, true, false);
  }
  void Unknown(const uint8_t*) { s += "(?)"; }
  void StartVector() {
    s += "[";
    if (vector_delimited) {
      s += d;
      indent_level++;
      append_indent();
    } else {
      s += " ";
    }
  }
  void EndVector() {
    if (vector_delimited) {
      s += d;
      indent_level--;
      append_indent();
    } else {
      s += " ";
    }
    s += "]";
  }
  void Element(size_t i, ElementaryType /*type*/,
               const TypeTable* /*type_table*/, const uint8_t* /*val*/) {
    if (i) {
      s += ",";
      if (vector_delimited) {
        s += d;
        append_indent();
      } else {
        s += " ";
      }
    }
  }
};

inline std::string FlatBufferToString(const uint8_t* buffer,
                                      size_t buffer_size,
                                      const TypeTable* type_table,
                                      bool multi_line = false,
                                      bool vector_delimited = true,
                                      const std::string& indent = "",
                                      bool quotes = false) {
  ToStringVisitor tostring_visitor(multi_line ? "\n" : " ", quotes, indent,
                                   vector_delimited);
  IterateFlatBuffer(buffer, buffer_size, type_table, &tostring_visitor);
  return tostring_visitor.s;
}

inline std::string FlatBufferToString(const uint8_t* buffer,
                                      const TypeTable* type_table,
                                      bool multi_line = false,
                                      bool vector_delimited = true,
                                      const std::string& indent = "",
                                      bool quotes = false) {
  ToStringVisitor tostring_visitor(multi_line ? "\n" : " ", quotes, indent,
                                   vector_delimited);
  IterateFlatBuffer(buffer, type_table, &tostring_visitor);
  return tostring_visitor.s;
}

}  // namespace flatbuffers

#endif  // FLATBUFFERS_MINIREFLECT_H_
