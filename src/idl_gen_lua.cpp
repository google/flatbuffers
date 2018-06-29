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

#include <string>

#include "flatbuffers/code_generators.h"
#include "flatbuffers/flatbuffers.h"
#include "flatbuffers/idl.h"
#include "flatbuffers/util.h"

#include <unordered_set>

namespace flatbuffers {
namespace lua {

// Hardcode spaces per indentation.
const std::string Indent = "    ";
const std::string Comment = "-- ";
const std::string EndFunc = "end\n\n";
const std::string SelfData = "self._tab";

class LuaGenerator : public BaseGenerator {
 public:
	 LuaGenerator(const Parser &parser, const std::string &path,
                  const std::string &file_name)
      : BaseGenerator(parser, path, file_name, "" /* not used */,
                      "" /* not used */){
    static const char * const keywords[] = {
      "and",
      "break",
      "do",
      "else",
      "elseif",
      "end",
      "false",
      "for",
      "function",
      "goto",
      "if",
      "in",
      "local",
      "nil",
      "not",
      "or",
      "repeat",
      "return",
      "then",
      "true",
      "until",
      "while"
		// todo: do i need to include the standard lib names (os, math, etc..?)
    };
    for (auto kw = keywords; *kw; kw++) keywords_.insert(*kw);
  }

  // Most field accessors need to retrieve and test the field offset first,
  // this is the prefix code for that.
  std::string OffsetPrefix(const FieldDef &field) {
    return "\n" + Indent + 
          "local o = " + SelfData + ":Offset(" + NumToString(field.value.offset) + ")\n" +
          Indent + "if o ~= 0 then\n";
  }

  // Begin a class declaration.
  void BeginClass(const StructDef &struct_def, std::string *code_ptr) {
    std::string &code = *code_ptr;
	code += "local " + NormalizedName(struct_def) + " = {} -- the module\n";
	code += "local " + NormalizedMetaName(struct_def) + " = {} -- the class metatable\n";
    code += "\n";
  }

  // Begin enum code with a class declaration.
  void BeginEnum(const std::string class_name, std::string *code_ptr) {
    std::string &code = *code_ptr;
	code += "local " + class_name + " = {\n";    
  }

  std::string EscapeKeyword(const std::string &name) const {
    return keywords_.find(name) == keywords_.end() ? name : "_" + name;
  }

  std::string NormalizedName(const Definition &definition) const {
    return EscapeKeyword(definition.name);
  }

  std::string NormalizedName(const EnumVal &ev) const {
    return EscapeKeyword(ev.name);
  }

  std::string NormalizedMetaName(const Definition &definition) const {
	  return EscapeKeyword(definition.name) + "_mt";
  }

  // A single enum member.
  void EnumMember(const EnumVal ev, std::string *code_ptr) {
    std::string &code = *code_ptr;
    code += Indent + NormalizedName(ev) + " = " + NumToString(ev.value) + ",\n";
  }

  // End enum code.
  void EndEnum(std::string *code_ptr) {
    std::string &code = *code_ptr;
    code += "}\n";
  }

  // Initialize a new struct or table from existing data.
  void NewRootTypeFromBuffer(const StructDef &struct_def,
                             std::string *code_ptr) {
    std::string &code = *code_ptr;
  
    code += "function m.GetRootAs" + NormalizedName(struct_def) + "(cls, buf, offset)\n";
	code += Indent + "local n = flatbuffers.encode.Get(N.UOffset, buf, offset)\n";
    code += Indent + "local x = " + NormalizedName(struct_def) + ".New(buf, n + offset)\n";
    code += Indent + "return x\n";
    code += EndFunc;
  }

  // Initialize an existing object with other data, to avoid an allocation.
  void InitializeExisting(const StructDef &struct_def,
                          std::string *code_ptr) {
    std::string &code = *code_ptr;

    GenReceiver(struct_def, code_ptr);
    code += "Init(buf, pos):\n";
    code += Indent + SelfData + " = flatbuffers.table.Table(buf, pos)\n";
    code += EndFunc;
  }

  // Get the length of a vector.
  void GetVectorLen(const StructDef &struct_def, const FieldDef &field,
                    std::string *code_ptr) {
    std::string &code = *code_ptr;

    GenReceiver(struct_def, code_ptr);
    code += MakeCamel(NormalizedName(field)) + "Length()";
    code += OffsetPrefix(field);
    code += Indent + Indent + "return "+SelfData+":VectorLen(o)\n";
	code += Indent + "end\n";
    code += Indent + "return 0\n";
	code += EndFunc;
  }

  // Get the value of a struct's scalar.
  void GetScalarFieldOfStruct(const StructDef &struct_def,
                              const FieldDef &field,
                              std::string *code_ptr) {
    std::string &code = *code_ptr;
    std::string getter = GenGetter(field.value.type);
    GenReceiver(struct_def, code_ptr);
    code += MakeCamel(NormalizedName(field));
    code += "(self): return " + getter;
    code += "self._tab.Pos + flatbuffers.number_types.UOffsetTFlags.py_type(";
    code += NumToString(field.value.offset) + "))\n";
  }

  // Get the value of a table's scalar.
  void GetScalarFieldOfTable(const StructDef &struct_def,
                             const FieldDef &field,
                             std::string *code_ptr) {
    std::string &code = *code_ptr;
    std::string getter = GenGetter(field.value.type);
    GenReceiver(struct_def, code_ptr);
    code += MakeCamel(NormalizedName(field));
    code += "():";
    code += OffsetPrefix(field);
    getter += "o + self._tab.Pos)";
    auto is_bool = field.value.type.base_type == BASE_TYPE_BOOL;
    if (is_bool) {
      getter = "bool(" + getter + ")";
    }
    code += Indent + Indent + Indent + "return " + getter + "\n";
    std::string default_value;
    if (is_bool) {
      default_value = field.value.constant == "0" ? "False" : "True";
    } else {
      default_value = field.value.constant;
    }
    code += Indent + Indent + "return " + default_value + "\n\n";
  }

  // Get a struct by initializing an existing struct.
  // Specific to Struct.
  void GetStructFieldOfStruct(const StructDef &struct_def,
                              const FieldDef &field,
                              std::string *code_ptr) {
    std::string &code = *code_ptr;
    GenReceiver(struct_def, code_ptr);
    code += MakeCamel(NormalizedName(field));
    code += "(self, obj):\n";
    code += Indent + Indent + "obj.Init(self._tab.Bytes, self._tab.Pos + ";
    code += NumToString(field.value.offset) + ")";
    code += "\n" + Indent + Indent + "return obj\n\n";
  }

  // Get a struct by initializing an existing struct.
  // Specific to Table.
  void GetStructFieldOfTable(const StructDef &struct_def,
                             const FieldDef &field,
                             std::string *code_ptr) {
    std::string &code = *code_ptr;
    GenReceiver(struct_def, code_ptr);
    code += MakeCamel(NormalizedName(field));
    code += "(self):";
    code += OffsetPrefix(field);
    if (field.value.type.struct_def->fixed) {
      code += Indent + Indent + Indent + "x = o + self._tab.Pos\n";
    } else {
      code += Indent + Indent + Indent;
      code += "x = self._tab.Indirect(o + self._tab.Pos)\n";
    }
    code += Indent + Indent + Indent;
    code += "from ." + TypeName(field) + " import " + TypeName(field) + "\n";
    code += Indent + Indent + Indent + "obj = " + TypeName(field) + "()\n";
    code += Indent + Indent + Indent + "obj.Init(self._tab.Bytes, x)\n";
    code += Indent + Indent + Indent + "return obj\n";
    code += Indent + Indent + "return None\n\n";
  }

  // Get the value of a string.
  void GetStringField(const StructDef &struct_def, const FieldDef &field,
                      std::string *code_ptr) {
    std::string &code = *code_ptr;
    GenReceiver(struct_def, code_ptr);
    code += MakeCamel(NormalizedName(field));
    code += "(self):";
    code += OffsetPrefix(field);
    code += Indent + Indent + Indent + "return " + GenGetter(field.value.type);
    code += "o + self._tab.Pos)\n";
    code += Indent + Indent + "return None\n\n";
  }

  // Get the value of a union from an object.
  void GetUnionField(const StructDef &struct_def, const FieldDef &field,
                     std::string *code_ptr) {
    std::string &code = *code_ptr;
    GenReceiver(struct_def, code_ptr);
    code += MakeCamel(NormalizedName(field)) + "(self):";
    code += OffsetPrefix(field);

    // TODO(rw): this works and is not the good way to it:
    bool is_native_table = TypeName(field) == "*flatbuffers.Table";
    if (is_native_table) {
      code += Indent + Indent + Indent + "from flatbuffers.table import Table\n";
    } else {
      code += Indent + Indent + Indent;
      code += "from ." + TypeName(field) + " import " + TypeName(field) + "\n";
    }
    code += Indent + Indent + Indent + "obj = Table(bytearray(), 0)\n";
    code += Indent + Indent + Indent + GenGetter(field.value.type);
    code += "obj, o)\n" + Indent + Indent + Indent + "return obj\n";
    code += Indent + Indent + "return None\n\n";
  }

  // Get the value of a vector's struct member.
  void GetMemberOfVectorOfStruct(const StructDef &struct_def,
                                 const FieldDef &field,
                                 std::string *code_ptr) {
    std::string &code = *code_ptr;
    auto vectortype = field.value.type.VectorType();

    GenReceiver(struct_def, code_ptr);
    code += MakeCamel(NormalizedName(field));
    code += "(self, j):" + OffsetPrefix(field);
    code += Indent + Indent + Indent + "x = self._tab.Vector(o)\n";
    code += Indent + Indent + Indent;
    code += "x += flatbuffers.number_types.UOffsetTFlags.py_type(j) * ";
    code += NumToString(InlineSize(vectortype)) + "\n";
    if (!(vectortype.struct_def->fixed)) {
      code += Indent + Indent + Indent + "x = self._tab.Indirect(x)\n";
    }
    code += Indent + Indent + Indent;
    code += "from ." + TypeName(field) + " import " + TypeName(field) + "\n";
    code += Indent + Indent + Indent + "obj = " + TypeName(field) + "()\n";
    code += Indent + Indent + Indent + "obj.Init(self._tab.Bytes, x)\n";
    code += Indent + Indent + Indent + "return obj\n";
    code += Indent + Indent + "return None\n\n";
  }

  // Get the value of a vector's non-struct member. Uses a named return
  // argument to conveniently set the zero value for the result.
  void GetMemberOfVectorOfNonStruct(const StructDef &struct_def,
                                    const FieldDef &field,
                                    std::string *code_ptr) {
    std::string &code = *code_ptr;
    auto vectortype = field.value.type.VectorType();

    GenReceiver(struct_def, code_ptr);
    code += MakeCamel(NormalizedName(field));
    code += "(self, j):";
    code += OffsetPrefix(field);
    code += Indent + Indent + Indent + "a = self._tab.Vector(o)\n";
    code += Indent + Indent + Indent;
    code += "return " + GenGetter(field.value.type);
    code += "a + flatbuffers.number_types.UOffsetTFlags.py_type(j * ";
    code += NumToString(InlineSize(vectortype)) + "))\n";
    if (vectortype.base_type == BASE_TYPE_STRING) {
      code += Indent + Indent + "return \"\"\n";
    } else {
      code += Indent + Indent + "return 0\n";
    }
    code += "\n";
  }

  // Returns a non-struct vector as a numpy array. Much faster
  // than iterating over the vector element by element.
  void GetVectorOfNonStructAsNumpy(const StructDef &struct_def,
                                   const FieldDef &field,
                                   std::string *code_ptr) {
    std::string &code = *code_ptr;
    auto vectortype = field.value.type.VectorType();

    // Currently, we only support accessing as numpy array if
    // the vector type is a scalar.
    if (!(IsScalar(vectortype.base_type))) { return; }

    GenReceiver(struct_def, code_ptr);
    code += MakeCamel(NormalizedName(field)) + "AsNumpy(self):";
    code += OffsetPrefix(field);

    code += Indent + Indent + Indent;
    code += "return ";
    code += "self._tab.GetVectorAsNumpy(flatbuffers.number_types.";
    code += MakeCamel(GenTypeGet(field.value.type));
    code += "Flags, o)\n";

    if (vectortype.base_type == BASE_TYPE_STRING) {
      code += Indent + Indent + "return \"\"\n";
    } else {
      code += Indent + Indent + "return 0\n";
    }
    code += "\n";
  }

  // Begin the creator function signature.
  void BeginBuilderArgs(const StructDef &struct_def,
                        std::string *code_ptr) {
    std::string &code = *code_ptr;

    code += "\n";
    code += "def Create" + NormalizedName(struct_def);
    code += "(builder";
  }

  // Recursively generate arguments for a constructor, to deal with nested
  // structs.
  void StructBuilderArgs(const StructDef &struct_def,
                         const char *nameprefix, std::string *code_ptr) {
    for (auto it = struct_def.fields.vec.begin();
        it != struct_def.fields.vec.end(); ++it) {
      auto &field = **it;
      if (IsStruct(field.value.type)) {
        // Generate arguments for a struct inside a struct. To ensure names
        // don't clash, and to make it obvious these arguments are constructing
        // a nested struct, prefix the name with the field name.
        StructBuilderArgs(*field.value.type.struct_def,
                          (nameprefix + (NormalizedName(field) + "_")).c_str(), code_ptr);
      } else {
        std::string &code = *code_ptr;
        code += (std::string) ", " + nameprefix;
        code += MakeCamel(NormalizedName(field), false);
      }
    }
  }

  // End the creator function signature.
  void EndBuilderArgs(std::string *code_ptr) {
    std::string &code = *code_ptr;
    code += "):\n";
  }

  // Recursively generate struct construction statements and instert manual
  // padding.
  void StructBuilderBody(const StructDef &struct_def,
                         const char *nameprefix, std::string *code_ptr) {
    std::string &code = *code_ptr;
    code += "    builder.Prep(" + NumToString(struct_def.minalign) + ", ";
    code += NumToString(struct_def.bytesize) + ")\n";
    for (auto it = struct_def.fields.vec.rbegin();
        it != struct_def.fields.vec.rend(); ++it) {
      auto &field = **it;
      if (field.padding)
        code += "    builder.Pad(" + NumToString(field.padding) + ")\n";
      if (IsStruct(field.value.type)) {
        StructBuilderBody(*field.value.type.struct_def,
                          (nameprefix + (NormalizedName(field) + "_")).c_str(), code_ptr);
      } else {
        code += "    builder.Prepend" + GenMethod(field) + "(";
        code += nameprefix + MakeCamel(NormalizedName(field), false) + ")\n";
      }
    }
  }

  void EndBuilderBody(std::string *code_ptr) {
    std::string &code = *code_ptr;
    code += "    return builder.Offset()\n";
  }

  // Get the value of a table's starting offset.
  void GetStartOfTable(const StructDef &struct_def,
                       std::string *code_ptr) {
    std::string &code = *code_ptr;
    code += "def " + NormalizedName(struct_def) + "Start";
    code += "(builder): ";
    code += "builder.StartObject(";
    code += NumToString(struct_def.fields.vec.size());
    code += ")\n";
  }

  // Set the value of a table's field.
  void BuildFieldOfTable(const StructDef &struct_def,
                         const FieldDef &field, const size_t offset,
                         std::string *code_ptr) {
    std::string &code = *code_ptr;
    code += "def " + NormalizedName(struct_def) + "Add" + MakeCamel(NormalizedName(field));
    code += "(builder, ";
    code += MakeCamel(NormalizedName(field), false);
    code += "): ";
    code += "builder.Prepend";
    code += GenMethod(field) + "Slot(";
    code += NumToString(offset) + ", ";
    if (!IsScalar(field.value.type.base_type) && (!struct_def.fixed)) {
      code += "flatbuffers.number_types.UOffsetTFlags.py_type";
      code += "(";
      code += MakeCamel(NormalizedName(field), false) + ")";
    } else {
      code += MakeCamel(NormalizedName(field), false);
    }
    code += ", " + field.value.constant;
    code += ")\n";
  }

  // Set the value of one of the members of a table's vector.
  void BuildVectorOfTable(const StructDef &struct_def,
                          const FieldDef &field, std::string *code_ptr) {
    std::string &code = *code_ptr;
    code += "def " + NormalizedName(struct_def) + "Start";
    code += MakeCamel(NormalizedName(field));
    code += "Vector(builder, numElems): return builder.StartVector(";
    auto vector_type = field.value.type.VectorType();
    auto alignment = InlineAlignment(vector_type);
    auto elem_size = InlineSize(vector_type);
    code += NumToString(elem_size);
    code += ", numElems, " + NumToString(alignment);
    code += ")\n";
  }

  // Get the offset of the end of a table.
  void GetEndOffsetOnTable(const StructDef &struct_def,
                           std::string *code_ptr) {
    std::string &code = *code_ptr;
    code += "def " + NormalizedName(struct_def) + "End";
    code += "(builder): ";
    code += "return builder.EndObject()\n";
  }

  // Generate the receiver for function signatures.
  void GenReceiver(const StructDef &struct_def, std::string *code_ptr) {
    std::string &code = *code_ptr;
	code += "function " + NormalizedMetaName(struct_def) + ":";
  }

  // Generate a struct field, conditioned on its child type(s).
  void GenStructAccessor(const StructDef &struct_def,
                         const FieldDef &field, std::string *code_ptr) {
    GenComment(field.doc_comment, code_ptr, nullptr, "# ");
    if (IsScalar(field.value.type.base_type)) {
      if (struct_def.fixed) {
        GetScalarFieldOfStruct(struct_def, field, code_ptr);
      } else {
        GetScalarFieldOfTable(struct_def, field, code_ptr);
      }
    } else {
      switch (field.value.type.base_type) {
        case BASE_TYPE_STRUCT:
          if (struct_def.fixed) {
            GetStructFieldOfStruct(struct_def, field, code_ptr);
          } else {
            GetStructFieldOfTable(struct_def, field, code_ptr);
          }
          break;
        case BASE_TYPE_STRING: GetStringField(struct_def, field, code_ptr); break;
        case BASE_TYPE_VECTOR: {
          auto vectortype = field.value.type.VectorType();
          if (vectortype.base_type == BASE_TYPE_STRUCT) {
            GetMemberOfVectorOfStruct(struct_def, field, code_ptr);
          } else {
            GetMemberOfVectorOfNonStruct(struct_def, field, code_ptr);
            GetVectorOfNonStructAsNumpy(struct_def, field, code_ptr);
          }
          break;
        }
        case BASE_TYPE_UNION: GetUnionField(struct_def, field, code_ptr); break;
        default: FLATBUFFERS_ASSERT(0);
      }
    }
    if (field.value.type.base_type == BASE_TYPE_VECTOR) {
      GetVectorLen(struct_def, field, code_ptr);
    }
  }

  // Generate table constructors, conditioned on its members' types.
  void GenTableBuilders(const StructDef &struct_def,
                        std::string *code_ptr) {
    GetStartOfTable(struct_def, code_ptr);

    for (auto it = struct_def.fields.vec.begin();
        it != struct_def.fields.vec.end(); ++it) {
      auto &field = **it;
      if (field.deprecated) continue;

      auto offset = it - struct_def.fields.vec.begin();
      BuildFieldOfTable(struct_def, field, offset, code_ptr);
      if (field.value.type.base_type == BASE_TYPE_VECTOR) {
        BuildVectorOfTable(struct_def, field, code_ptr);
      }
    }

    GetEndOffsetOnTable(struct_def, code_ptr);
  }

  // Generate struct or table methods.
  void GenStruct(const StructDef &struct_def, std::string *code_ptr) {
    if (struct_def.generated) return;

    GenComment(struct_def.doc_comment, code_ptr, nullptr, Comment.c_str());
    BeginClass(struct_def, code_ptr);
    if (!struct_def.fixed) {
      // Generate a special accessor for the table that has been declared as
      // the root type.
      NewRootTypeFromBuffer(struct_def, code_ptr);
    }
    // Generate the Init method that sets the field in a pre-existing
    // accessor object. This is to allow object reuse.
    InitializeExisting(struct_def, code_ptr);
    for (auto it = struct_def.fields.vec.begin();
        it != struct_def.fields.vec.end(); ++it) {
      auto &field = **it;
      if (field.deprecated) continue;

      GenStructAccessor(struct_def, field, code_ptr);
    }

    if (struct_def.fixed) {
      // create a struct constructor function
      GenStructBuilder(struct_def, code_ptr);
    } else {
      // Create a set of functions that allow table construction.
      GenTableBuilders(struct_def, code_ptr);
    }
  }

  // Generate enum declarations.
  void GenEnum(const EnumDef &enum_def, std::string *code_ptr) {
    if (enum_def.generated) return;

    GenComment(enum_def.doc_comment, code_ptr, nullptr, Comment.c_str());
    BeginEnum(NormalizedName(enum_def), code_ptr);
    for (auto it = enum_def.vals.vec.begin(); it != enum_def.vals.vec.end();
        ++it) {
      auto &ev = **it;
      GenComment(ev.doc_comment, code_ptr, nullptr, Comment.c_str());
      EnumMember(ev, code_ptr);
    }
    EndEnum(code_ptr);
  }

  // Returns the function name that is able to read a value of the given type.
  std::string GenGetter(const Type &type) {
    switch (type.base_type) {
		case BASE_TYPE_STRING: return SelfData + ":String(";
		case BASE_TYPE_UNION: return  SelfData + ":Union(";
		case BASE_TYPE_VECTOR: return GenGetter(type.VectorType());
		default:
        return SelfData + ":Get(flatbuffers.N." +
              MakeCamel(GenTypeGet(type)) + ", ";
    }
  }

  // Returns the method name for use with add/put calls.
  std::string GenMethod(const FieldDef &field) {
    return IsScalar(field.value.type.base_type)
              ? MakeCamel(GenTypeBasic(field.value.type))
              : (IsStruct(field.value.type) ? "Struct" : "UOffsetTRelative");
  }

  std::string GenTypeBasic(const Type &type) {
    static const char *ctypename[] = {
    // clang-format off
      #define FLATBUFFERS_TD(ENUM, IDLTYPE, \
        CTYPE, JTYPE, GTYPE, NTYPE, PTYPE) \
        #PTYPE,
        FLATBUFFERS_GEN_TYPES(FLATBUFFERS_TD)
      #undef FLATBUFFERS_TD
      // clang-format on
    };
    return ctypename[type.base_type];
  }

  std::string GenTypePointer(const Type &type) {
    switch (type.base_type) {
      case BASE_TYPE_STRING: return "string";
      case BASE_TYPE_VECTOR: return GenTypeGet(type.VectorType());
      case BASE_TYPE_STRUCT: return type.struct_def->name;
      case BASE_TYPE_UNION:
        // fall through
      default: return "*flatbuffers.Table";
    }
  }

  std::string GenTypeGet(const Type &type) {
    return IsScalar(type.base_type) ? GenTypeBasic(type) : GenTypePointer(type);
  }

  std::string TypeName(const FieldDef &field) {
    return GenTypeGet(field.value.type);
  }

  // Create a struct with a builder and the struct's arguments.
  void GenStructBuilder(const StructDef &struct_def,
                              std::string *code_ptr) {
    BeginBuilderArgs(struct_def, code_ptr);
    StructBuilderArgs(struct_def, "", code_ptr);
    EndBuilderArgs(code_ptr);

    StructBuilderBody(struct_def, "", code_ptr);
    EndBuilderBody(code_ptr);
  }

  bool generate() {
    if (!generateEnums()) return false;
    if (!generateStructs()) return false;
    return true;
  }

 private:
  bool generateEnums() {
    for (auto it = parser_.enums_.vec.begin(); it != parser_.enums_.vec.end();
         ++it) {
      auto &enum_def = **it;
      std::string enumcode;
      GenEnum(enum_def, &enumcode);
      if (!SaveType(enum_def, enumcode, false)) return false;
    }
    return true;
  }

  bool generateStructs() {
    for (auto it = parser_.structs_.vec.begin();
         it != parser_.structs_.vec.end(); ++it) {
      auto &struct_def = **it;
      std::string declcode;
      GenStruct(struct_def, &declcode);
      if (!SaveType(struct_def, declcode, true)) return false;
    }
    return true;
  }

  // Begin by declaring namespace and imports.
  void BeginFile(const std::string name_space_name, const bool needs_imports,
                 std::string *code_ptr) {
    std::string &code = *code_ptr;
    code += Comment + FlatBuffersGeneratedWarning() + "\n\n";
    code += Comment + "namespace: " + name_space_name + "\n\n";
    if (needs_imports) { 
		code += "local flatbuffers = require('flatbuffers')\n\n"; 
	}
  }

  // Save out the generated code for a Lua Table type.
  bool SaveType(const Definition &def, const std::string &classcode,
                bool needs_imports) {
    if (!classcode.length()) return true;

    std::string namespace_dir = path_;
    auto &namespaces = def.defined_namespace->components;
    for (auto it = namespaces.begin(); it != namespaces.end(); ++it) {
      if (it != namespaces.begin()) namespace_dir += kPathSeparator;
      namespace_dir += *it;
      std::string init_py_filename = namespace_dir + "/__init__.py";
      SaveFile(init_py_filename.c_str(), "", false);
    }

    std::string code = "";
    BeginFile(LastNamespacePart(*def.defined_namespace), needs_imports, &code);
    code += classcode;
    std::string filename =
        NamespaceDir(*def.defined_namespace) + NormalizedName(def) + ".lua";
    return SaveFile(filename.c_str(), code, false);
  }
 private:
  std::unordered_set<std::string> keywords_;
};

}  // namespace lua

bool GenerateLua(const Parser &parser, const std::string &path,
                    const std::string &file_name) {
  lua::LuaGenerator generator(parser, path, file_name);
  return generator.generate();
}

}  // namespace flatbuffers
