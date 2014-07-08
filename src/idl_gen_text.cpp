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

// independent from idl_parser, since this code is not needed for most clients

#include "flatbuffers/flatbuffers.h"
#include "flatbuffers/idl.h"
#include "flatbuffers/util.h"

namespace flatbuffers {

static void GenStruct(const StructDef &struct_def, const Table *table,
                      int indent, const GeneratorOptions &opts,
                      std::string *_text);

// If indentation is less than 0, that indicates we don't want any newlines
// either.
const char *NewLine(int indent_step) {
  return indent_step >= 0 ? "\n" : "";
}

// Print (and its template specialization below for pointers) generate text
// for a single FlatBuffer value into JSON format.
// The general case for scalars:
template<typename T> void Print(T val, Type /*type*/, int /*indent*/,
                                StructDef * /*union_sd*/,
                                const GeneratorOptions & /*opts*/,
                                std::string *_text) {
  std::string &text = *_text;
  text += NumToString(val);
}

// Print a vector a sequence of JSON values, comma separated, wrapped in "[]".
template<typename T> void PrintVector(const Vector<T> &v, Type type,
                                      int indent, const GeneratorOptions &opts,
                                      std::string *_text) {
  std::string &text = *_text;
  text += "[";
  text += NewLine(opts.indent_step);
  for (uoffset_t i = 0; i < v.Length(); i++) {
    if (i) {
      text += ",";
      text += NewLine(opts.indent_step);
    }
    text.append(indent + opts.indent_step, ' ');
    if (IsStruct(type))
      Print(v.GetStructFromOffset(i * type.struct_def->bytesize), type,
            indent + opts.indent_step, nullptr, opts, _text);
    else
      Print(v.Get(i), type, indent + opts.indent_step, nullptr,
            opts, _text);
  }
  text += NewLine(opts.indent_step);
  text.append(indent, ' ');
  text += "]";
}

static void EscapeString(const String &s, std::string *_text) {
  std::string &text = *_text;
  text += "\"";
  for (uoffset_t i = 0; i < s.Length(); i++) {
    char c = s.Get(i);
    switch (c) {
      case '\n': text += "\\n"; break;
      case '\t': text += "\\t"; break;
      case '\r': text += "\\r"; break;
      case '\"': text += "\\\""; break;
      case '\\': text += "\\\\"; break;
      default:
        if (c >= ' ' && c <= '~') {
          text += c;
        } else {
          auto u = static_cast<unsigned char>(c);
          text += "\\x" + IntToStringHex(u);
        }
        break;
    }
  }
  text += "\"";
}

// Specialization of Print above for pointer types.
template<> void Print<const void *>(const void *val,
                                    Type type, int indent,
                                    StructDef *union_sd,
                                    const GeneratorOptions &opts,
                                    std::string *_text) {
  switch (type.base_type) {
    case BASE_TYPE_UNION:
      // If this assert hits, you have an corrupt buffer, a union type field
      // was not present or was out of range.
      assert(union_sd);
      GenStruct(*union_sd,
                reinterpret_cast<const Table *>(val),
                indent,
                opts,
                _text);
      break;
    case BASE_TYPE_STRUCT:
      GenStruct(*type.struct_def,
                reinterpret_cast<const Table *>(val),
                indent,
                opts,
                _text);
      break;
    case BASE_TYPE_STRING: {
      EscapeString(*reinterpret_cast<const String *>(val), _text);
      break;
    }
    case BASE_TYPE_VECTOR:
      type = type.VectorType();
      // Call PrintVector above specifically for each element type:
      switch (type.base_type) {
        #define FLATBUFFERS_TD(ENUM, IDLTYPE, CTYPE, JTYPE) \
          case BASE_TYPE_ ## ENUM: \
            PrintVector<CTYPE>( \
              *reinterpret_cast<const Vector<CTYPE> *>(val), \
              type, indent, opts, _text); break;
          FLATBUFFERS_GEN_TYPES(FLATBUFFERS_TD)
        #undef FLATBUFFERS_TD
      }
      break;
    default: assert(0);
  }
}

// Generate text for a scalar field.
template<typename T> static void GenField(const FieldDef &fd,
                                          const Table *table, bool fixed,
                                          const GeneratorOptions &opts,
                                          int indent,
                                          std::string *_text) {
  Print(fixed ?
    reinterpret_cast<const Struct *>(table)->GetField<T>(fd.value.offset) :
    table->GetField<T>(fd.value.offset, 0), fd.value.type, indent, nullptr,
                                            opts, _text);
}

// Generate text for non-scalar field.
static void GenFieldOffset(const FieldDef &fd, const Table *table, bool fixed,
                           int indent, StructDef *union_sd,
                           const GeneratorOptions &opts, std::string *_text) {
  const void *val = nullptr;
  if (fixed) {
    // The only non-scalar fields in structs are structs.
    assert(IsStruct(fd.value.type));
    val = reinterpret_cast<const Struct *>(table)->
            GetStruct<const void *>(fd.value.offset);
  } else {
    val = IsStruct(fd.value.type)
      ? table->GetStruct<const void *>(fd.value.offset)
      : table->GetPointer<const void *>(fd.value.offset);
  }
  Print(val, fd.value.type, indent, union_sd, opts, _text);
}

// Generate text for a struct or table, values separated by commas, indented,
// and bracketed by "{}"
static void GenStruct(const StructDef &struct_def, const Table *table,
                      int indent, const GeneratorOptions &opts,
                      std::string *_text) {
  std::string &text = *_text;
  text += "{";
  text += NewLine(opts.indent_step);
  int fieldout = 0;
  StructDef *union_sd = nullptr;
  for (auto it = struct_def.fields.vec.begin();
       it != struct_def.fields.vec.end();
       ++it) {
    FieldDef &fd = **it;
    if (struct_def.fixed || table->CheckField(fd.value.offset)) {
      // The field is present.
      if (fieldout++) {
        text += ",";
        text += NewLine(opts.indent_step);
      }
      text.append(indent + opts.indent_step, ' ');
      if (opts.strict_json) text += "\"";
      text += fd.name;
      if (opts.strict_json) text += "\"";
      text += ": ";
      switch (fd.value.type.base_type) {
         #define FLATBUFFERS_TD(ENUM, IDLTYPE, CTYPE, JTYPE) \
           case BASE_TYPE_ ## ENUM: \
              GenField<CTYPE>(fd, table, struct_def.fixed, \
                              opts, indent + opts.indent_step, _text); \
              break;
          FLATBUFFERS_GEN_TYPES_SCALAR(FLATBUFFERS_TD)
        #undef FLATBUFFERS_TD
        // Generate drop-thru case statements for all pointer types:
        #define FLATBUFFERS_TD(ENUM, IDLTYPE, CTYPE, JTYPE) \
          case BASE_TYPE_ ## ENUM:
          FLATBUFFERS_GEN_TYPES_POINTER(FLATBUFFERS_TD)
        #undef FLATBUFFERS_TD
            GenFieldOffset(fd, table, struct_def.fixed, indent + opts.indent_step,
                           union_sd, opts, _text);
            break;
      }
      if (fd.value.type.base_type == BASE_TYPE_UTYPE) {
        union_sd = fd.value.type.enum_def->ReverseLookup(
                                 table->GetField<uint8_t>(fd.value.offset, 0));
      }
    }
  }
  text += NewLine(opts.indent_step);
  text.append(indent, ' ');
  text += "}";
}

// Generate a text representation of a flatbuffer in JSON format.
void GenerateText(const Parser &parser, const void *flatbuffer,
                  const GeneratorOptions &opts, std::string *_text) {
  std::string &text = *_text;
  assert(parser.root_struct_def);  // call SetRootType()
  text.reserve(1024);   // Reduce amount of inevitable reallocs.
  GenStruct(*parser.root_struct_def,
            GetRoot<Table>(flatbuffer),
            0,
            opts,
            _text);
  text += NewLine(opts.indent_step);
}

}  // namespace flatbuffers

