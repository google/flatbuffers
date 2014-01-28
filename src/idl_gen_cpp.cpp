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
namespace cpp {

// Return a C++ type from the table in idl.h
static std::string GenTypeBasic(const Type &type) {
  static const char *ctypename[] = {
    #define FLATBUFFERS_TD(ENUM, IDLTYPE, CTYPE, JTYPE) #CTYPE,
      FLATBUFFERS_GEN_TYPES(FLATBUFFERS_TD)
    #undef FLATBUFFERS_TD
  };
  return ctypename[type.base_type];
}

static std::string GenTypeWire(const Type &type, const char *postfix);

// Return a C++ pointer type, specialized to the actual struct/table types,
// and vector element types.
static std::string GenTypePointer(const Type &type) {
  switch (type.base_type) {
    case BASE_TYPE_STRING:
      return "flatbuffers::String";
    case BASE_TYPE_VECTOR:
      return "flatbuffers::Vector<" + GenTypeWire(type.VectorType(), "") + ">";
    case BASE_TYPE_STRUCT:
      return type.struct_def->name;
    case BASE_TYPE_UNION:
      // fall through
    default:
      return "void";
  }
}

// Return a C++ type for any type (scalar/pointer) specifically for
// building a flatbuffer.
static std::string GenTypeWire(const Type &type, const char *postfix) {
  return IsScalar(type.base_type)
    ? GenTypeBasic(type) + postfix
    : IsStruct(type)
      ? "const " + GenTypePointer(type) + " *"
      : "flatbuffers::Offset<" + GenTypePointer(type) + ">" + postfix;
}

// Return a C++ type for any type (scalar/pointer) specifically for
// using a flatbuffer.
static std::string GenTypeGet(const Type &type, const char *afterbasic,
                              const char *beforeptr, const char *afterptr) {
  return IsScalar(type.base_type)
    ? GenTypeBasic(type) + afterbasic
    : beforeptr + GenTypePointer(type) + afterptr;
}

// Generate a documentation comment, if available.
static void GenComment(const std::string &dc,
                       std::string *code_ptr,
                       const char *prefix = "") {
  std::string &code = *code_ptr;
  if (dc.length()) {
    code += std::string(prefix) + "///" + dc + "\n";
  }
}

// Generate an enum declaration and an enum string lookup table.
static void GenEnum(EnumDef &enum_def, std::string *code_ptr) {
  if (enum_def.generated) return;
  std::string &code = *code_ptr;
  GenComment(enum_def.doc_comment, code_ptr);
  code += "enum {\n";
  for (auto it = enum_def.vals.vec.begin();
       it != enum_def.vals.vec.end();
       ++it) {
    auto &ev = **it;
    GenComment(ev.doc_comment, code_ptr, "  ");
    code += "  " + enum_def.name + "_" + ev.name + " = ";
    code += NumToString(ev.value) + ",\n";
  }
  code += "};\n\n";

  // Generate a generate string table for enum values.
  // Problem is, if values are very sparse that could generate really big
  // tables. Ideally in that case we generate a map lookup instead, but for
  // the moment we simply don't output a table at all.
  int range = enum_def.vals.vec.back()->value -
              enum_def.vals.vec.front()->value + 1;
  // Average distance between values above which we consider a table
  // "too sparse". Change at will.
  static const int kMaxSparseness = 5;
  if (range / static_cast<int>(enum_def.vals.vec.size()) < kMaxSparseness) {
    code += "inline const char **EnumNames" + enum_def.name + "() {\n";
    code += "  static const char *names[] = { ";
    int val = enum_def.vals.vec.front()->value;
    for (auto it = enum_def.vals.vec.begin();
         it != enum_def.vals.vec.end();
         ++it) {
      while (val++ != (*it)->value) code += "\"\", ";
      code += "\"" + (*it)->name + "\", ";
    }
    code += "nullptr };\n  return names;\n}\n\n";
    code += "inline const char *EnumName" + enum_def.name;
    code += "(int e) { return EnumNames" + enum_def.name + "()[e";
    if (enum_def.vals.vec.front()->value)
      code += " - " + enum_def.name + "_" + enum_def.vals.vec.front()->name;
    code += "]; }\n\n";
  }
}

// Generate an accessor struct, builder structs & function for a table.
static void GenTable(StructDef &struct_def, std::string *code_ptr) {
  if (struct_def.generated) return;
  std::string &code = *code_ptr;

  // Generate an accessor struct, with methods of the form:
  // type name() const { return GetField<type>(offset, defaultval); }
  GenComment(struct_def.doc_comment, code_ptr);
  code += "struct " + struct_def.name + " : private flatbuffers::Table";
  code += " {\n";
  for (auto it = struct_def.fields.vec.begin();
       it != struct_def.fields.vec.end();
       ++it) {
    auto &field = **it;
    if (!field.deprecated) {  // Deprecated fields won't be accessible.
      GenComment(field.doc_comment, code_ptr, "  ");
      code += "  " + GenTypeGet(field.value.type, " ", "const ", " *");
      code += field.name + "() const { return ";
      // Call a different accessor for pointers, that indirects.
      code += IsScalar(field.value.type.base_type)
        ? "GetField<"
        : (IsStruct(field.value.type) ? "GetStruct<" : "GetPointer<");
      code += GenTypeGet(field.value.type, "", "const ", " *") + ">(";
      code += NumToString(field.value.offset);
      // Default value as second arg for non-pointer types.
      if (IsScalar(field.value.type.base_type))
        code += ", " + field.value.constant;
      code += "); }\n";
    }
  }
  code += "};\n\n";

  // Generate a builder struct, with methods of the form:
  // void add_name(type name) { fbb_.AddElement<type>(offset, name, default); }
  code += "struct " + struct_def.name;
  code += "Builder {\n  flatbuffers::FlatBufferBuilder &fbb_;\n";
  code += "  flatbuffers::uoffset_t start_;\n";
  for (auto it = struct_def.fields.vec.begin();
       it != struct_def.fields.vec.end();
       ++it) {
    auto &field = **it;
    if (!field.deprecated) {
      code += "  void add_" + field.name + "(";
      code += GenTypeWire(field.value.type, " ") + field.name + ") { fbb_.Add";
      if (IsScalar(field.value.type.base_type))
        code += "Element<" + GenTypeWire(field.value.type, "") + ">";
      else if (IsStruct(field.value.type))
        code += "Struct";
      else
        code += "Offset";
      code += "(" + NumToString(field.value.offset) + ", " + field.name;
      if (IsScalar(field.value.type.base_type))
        code += ", " + field.value.constant;
      code += "); }\n";
    }
  }
  code += "  " + struct_def.name;
  code += "Builder(flatbuffers::FlatBufferBuilder &_fbb) : fbb_(_fbb) ";
  code += "{ start_ = fbb_.StartTable(); }\n";
  code += "  flatbuffers::Offset<" + struct_def.name;
  code += "> Finish() { return flatbuffers::Offset<" + struct_def.name;
  code += ">(fbb_.EndTable(start_, ";
  code += NumToString(struct_def.fields.vec.size()) + ")); }\n};\n\n";

  // Generate a convenient CreateX function that uses the above builder
  // to create a table in one go.
  code += "inline flatbuffers::Offset<" + struct_def.name + "> Create";
  code += struct_def.name;
  code += "(flatbuffers::FlatBufferBuilder &_fbb";
  for (auto it = struct_def.fields.vec.begin();
       it != struct_def.fields.vec.end();
       ++it) {
    auto &field = **it;
    if (!field.deprecated) {
      code += ", " + GenTypeWire(field.value.type, " ") + field.name;
    }
  }
  code += ") {\n  " + struct_def.name + "Builder builder_(_fbb);\n";
  for (size_t size = struct_def.sortbysize ? sizeof(largest_scalar_t) : 1;
       size;
       size /= 2) {
    for (auto it = struct_def.fields.vec.rbegin();
         it != struct_def.fields.vec.rend();
         ++it) {
      auto &field = **it;
      if (!field.deprecated &&
          (!struct_def.sortbysize ||
           size == SizeOf(field.value.type.base_type))) {
        code += "  builder_.add_" + field.name + "(" + field.name + ");\n";
      }
    }
  }
  code += "  return builder_.Finish();\n}\n\n";
}

// Generate an accessor struct with constructor for a flatbuffers struct.
static void GenStruct(StructDef &struct_def, std::string *code_ptr) {
  if (struct_def.generated) return;
  std::string &code = *code_ptr;

  // Generate an accessor struct, with private variables of the form:
  // type name_;
  // Generates manual padding and alignment.
  // Variables are private because they contain little endian data on all
  // platforms.
  GenComment(struct_def.doc_comment, code_ptr);
  code += "MANUALLY_ALIGNED_STRUCT(" + NumToString(struct_def.minalign) + ") ";
  code += struct_def.name + " {\n private:\n";
  int padding_id = 0;
  for (auto it = struct_def.fields.vec.begin();
       it != struct_def.fields.vec.end();
       ++it) {
    auto &field = **it;
    code += "  " + GenTypeGet(field.value.type, " ", "", " ");
    code += field.name + "_;\n";
    if (field.padding) {
      for (int i = 0; i < 4; i++)
        if (static_cast<int>(field.padding) & (1 << i))
          code += "  int" + NumToString((1 << i) * 8) +
                  "_t __padding" + NumToString(padding_id++) + ";\n";
      assert(!(field.padding & ~0xF));
    }
  }

  // Generate a constructor that takes all fields as arguments.
  code += "\n public:\n  " + struct_def.name + "(";
  for (auto it = struct_def.fields.vec.begin();
       it != struct_def.fields.vec.end();
       ++it) {
    auto &field = **it;
    if (it != struct_def.fields.vec.begin()) code += ", ";
    code += GenTypeGet(field.value.type, " ", "const ", " &") + field.name;
  }
  code += ")\n    : ";
  padding_id = 0;
  for (auto it = struct_def.fields.vec.begin();
       it != struct_def.fields.vec.end();
       ++it) {
    auto &field = **it;
    if (it != struct_def.fields.vec.begin()) code += ", ";
    code += field.name + "_(";
    if (IsScalar(field.value.type.base_type))
      code += "flatbuffers::EndianScalar(" + field.name + "))";
    else
      code += field.name + ")";
    if (field.padding)
      code += ", __padding" + NumToString(padding_id++) + "(0)";
  }
  code += " {}\n\n";

  // Generate accessor methods of the form:
  // type name() const { return flatbuffers::EndianScalar(name_); }
  for (auto it = struct_def.fields.vec.begin();
       it != struct_def.fields.vec.end();
       ++it) {
    auto &field = **it;
    GenComment(field.doc_comment, code_ptr, "  ");
    code += "  " + GenTypeGet(field.value.type, " ", "const ", " &");
    code += field.name + "() const { return ";
    if (IsScalar(field.value.type.base_type))
      code += "flatbuffers::EndianScalar(" + field.name + "_)";
    else
      code += field.name + "_";
    code += "; }\n";
  }
  code += "};\nSTRUCT_END(" + struct_def.name + ", ";
  code += NumToString(struct_def.bytesize) + ");\n\n";
}

}  // namespace cpp

// Iterate through all definitions we haven't generate code for (enums, structs,
// and tables) and output them to a single file.
static std::string GenerateCPP(const Parser &parser) {
  using namespace cpp;

  // Generate code for all the enum declarations.
  std::string enum_code;
  for (auto it = parser.enums_.vec.begin();
       it != parser.enums_.vec.end(); ++it) {
    GenEnum(**it, &enum_code);
  }

  // Generate forward declarations for all structs/tables, since they may
  // have circular references.
  std::string forward_decl_code;
  for (auto it = parser.structs_.vec.begin();
       it != parser.structs_.vec.end(); ++it) {
    if (!(*it)->generated)
      forward_decl_code += "struct " + (*it)->name + ";\n";
  }

  // Generate code for all structs, then all tables.
  std::string decl_code;
  for (auto it = parser.structs_.vec.begin();
       it != parser.structs_.vec.end(); ++it) {
    if ((**it).fixed) GenStruct(**it, &decl_code);
  }
  for (auto it = parser.structs_.vec.begin();
       it != parser.structs_.vec.end(); ++it) {
    if (!(**it).fixed) GenTable(**it, &decl_code);
  }

  // Only output file-level code if there were any declarations.
  if (enum_code.length() || forward_decl_code.length() || decl_code.length()) {
    std::string code;
    code = "// automatically generated, do not modify\n\n";
    code += "#include \"flatbuffers/flatbuffers.h\"\n\n";
    for (auto it = parser.name_space_.begin();
         it != parser.name_space_.end(); ++it) {
      code += "namespace " + *it + " {\n";
    }
    code += "\n";
    code += enum_code;
    code += forward_decl_code;
    code += "\n";
    code += decl_code;
    if (parser.root_struct_def) {
      code += "inline const " + parser.root_struct_def->name + " *Get";
      code += parser.root_struct_def->name;
      code += "(const void *buf) { return flatbuffers::GetRoot<";
      code += parser.root_struct_def->name + ">(buf); }\n\n";
    }
    for (auto it = parser.name_space_.begin();
         it != parser.name_space_.end(); ++it) {
      code += "}; // namespace " + *it + "\n";
    }

    return code;
  }

  return std::string();
}

bool GenerateCPP(const Parser &parser,
                 const std::string &path,
                 const std::string &file_name) {
    auto code = GenerateCPP(parser);
    return !code.length() ||
           SaveFile((path + file_name + "_generated.h").c_str(), code, false);
}

}  // namespace flatbuffers

