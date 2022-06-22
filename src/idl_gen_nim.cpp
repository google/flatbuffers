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
#include <unordered_set>
#include <vector>

#include "flatbuffers/code_generators.h"
#include "flatbuffers/flatbuffers.h"
#include "flatbuffers/idl.h"
#include "flatbuffers/util.h"

namespace flatbuffers {
namespace nim {

// Hardcode spaces per indentation.
const CommentConfig def_comment = { nullptr, "#", nullptr };
const char *Indent = "  ";
const char *Comment = "# ";
const char *End = "\n";
const char *EndFunc = "\n";
const char *Export = "*";
const char *SelfData = "self.tab";
const char *SelfDataPos = "self.tab.Pos";
const char *SelfDataBytes = "self.tab.Bytes";

class NimGenerator : public BaseGenerator {
 public:
  NimGenerator(const Parser &parser, const std::string &path,
               const std::string &file_name)
      : BaseGenerator(parser, path, file_name, "" /* not used */,
                      "" /* not used */, "nim") {
    static const char *const keywords[] = {
      "addr",      "and",     "as",        "asm",      "bind",   "block",
      "break",     "case",    "cast",      "concept",  "const",  "continue",
      "converter", "defer",   "discard",   "distinct", "div",    "do",
      "elif",      "else",    "end",       "enum",     "except", "export",
      "finally",   "for",     "from",      "func",     "if",     "import",
      "in",        "include", "interface", "is",       "isnot",  "iterator",
      "let",       "macro",   "method",    "mixin",    "mod",    "nil",
      "not",       "notin",   "object",    "of",       "or",     "out",
      "proc",      "ptr",     "raise",     "ref",      "return", "shl",
      "shr",       "static",  "template",  "try",      "tuple",  "type",
      "using",     "var",     "when",      "while",    "xor",    "yield",
    };
    keywords_.insert(std::begin(keywords), std::end(keywords));
  }

  // Most field accessors need to retrieve and test the field offset first,
  // this is the prefix code for that.
  std::string OffsetPrefix(const FieldDef &field) {
    return std::string(Indent) + "var o = " + SelfData + ".Offset(" +
           NumToString(field.value.offset) + ")\n" + Indent + "if o != 0:\n";
  }

  // Begin a class declaration.
  void BeginClass(const StructDef &struct_def, std::string *code_ptr) {
    std::string &code = *code_ptr;
    code += "type\n";
    code += std::string(Indent) + NormalizedName(struct_def) + Export +
            " = object of FlatObj\n";
    code += "\n";
  }

  // Begin enum code with a class declaration.
  void BeginEnum(const std::string &class_name, std::string *code_ptr) {
    std::string &code = *code_ptr;
    code += "type\n";
    code += std::string(Indent) + class_name + Export + " {.pure.} = enum\n";
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
  void EnumMember(const EnumDef &enum_def, const EnumVal &ev,
                  std::string *code_ptr) {
    std::string &code = *code_ptr;
    code += std::string(Indent) + std::string(Indent) + NormalizedName(ev) +
            " = " + enum_def.ToString(ev) + "." +
            GenTypeBasic(enum_def.underlying_type) + ",\n";
  }

  // End enum code.
  void EndEnum(std::string *code_ptr) {
    std::string &code = *code_ptr;
    code += "\n";
  }

  // Initialize a new struct or table from existing data.
  void NewRootTypeFromBuffer(const StructDef &struct_def,
                             std::string *code_ptr) {
    std::string &code = *code_ptr;

    code += "function " + NormalizedName(struct_def) + ".GetRootAs" +
            NormalizedName(struct_def) + "(buf, offset)\n";
    code += std::string(Indent) + "if type(buf) == \"string\" then\n";
    code += std::string(Indent) + Indent +
            "buf = flatbuffers.binaryArray.New(buf)\n";
    code += std::string(Indent) + "end\n";
    code += std::string(Indent) +
            "local n = flatbuffers.N.UOffsetT:Unpack(buf, offset)\n";
    code += std::string(Indent) + "local o = " + NormalizedName(struct_def) +
            ".New()\n";
    code += std::string(Indent) + "o:Init(buf, n + offset)\n";
    code += std::string(Indent) + "return o\n";
    code += EndFunc;
  }

  // Get the length of a vector.
  void GetVectorLen(const StructDef &struct_def, const FieldDef &field,
                    std::string *code_ptr) {
    std::string &code = *code_ptr;
    code += "proc " + NormalizedName(field) + "Length" + Export +
            "(self: " + NormalizedName(struct_def) + "): int =\n";
    code += OffsetPrefix(field);
    code += std::string(Indent) + std::string(Indent) + "result = " + SelfData +
            ".Vectorlen(o)\n";
    code += std::string(Indent) + "else:\n";
    code += std::string(Indent) + std::string(Indent) + "result = 0\n";
    code += EndFunc;
  }

  void GetVector(const StructDef &struct_def, const FieldDef &field,
                 std::string *code_ptr) {
    std::string &code = *code_ptr;
    code += "proc " + NormalizedName(field) + Export +
            "(self: " + NormalizedName(struct_def) + "): seq[" +
            TypeName(field) + "] =\n";
    code += std::string(Indent) + "let len = self." + NormalizedName(field) +
            "Length\n";
    code += std::string(Indent) + "for i in countup(0, len - 1):\n";
    code += std::string(Indent) + std::string(Indent) + "result.add(self." +
            NormalizedName(field) + "(i))\n";
    code += EndFunc;
  }

  // Get the value of a struct's scalar.
  void GetScalarFieldOfStruct(const StructDef &struct_def,
                              const FieldDef &field, std::string *code_ptr) {
    std::string &code = *code_ptr;
    code += "proc " + NormalizedName(field) + Export +
            "(self: " + NormalizedName(struct_def) +
            "): " + GenTypeGet(field.value.type) + " =\n";
    code += std::string(Indent) + "result = " + GenGetter(field.value.type) +
            SelfDataPos + " + " + NumToString(field.value.offset) + ")\n";

    if (parser_.opts.mutable_buffer) {
      code += "proc `" + NormalizedName(field) + "=`" + Export + "(self: var " +
              NormalizedName(struct_def) +
              ", n: " + GenTypeGet(field.value.type) + ") =\n";
      code += std::string(Indent) + "discard " + SelfData + ".Mutate(" +
              SelfDataPos + " + " + NumToString(field.value.offset) + ", n)\n";
    }
    code += EndFunc;
  }

  // Get the value of a struct's struct.
  void GetStructFieldOfStruct(const StructDef &struct_def,
                              const FieldDef &field, std::string *code_ptr) {
    std::string &code = *code_ptr;
    imports_.insert(GetImport(struct_def, field));
    code += "proc " + NormalizedName(field) + Export +
            "(self: " + NormalizedName(struct_def) +
            "): " + GenTypeGet(field.value.type) + " =\n";
    code += std::string(Indent) + "result.Init(" + SelfDataBytes + ", " +
            SelfDataPos + " + " + NumToString(field.value.offset) + ")\n";

    if (parser_.opts.mutable_buffer) {
      code += "proc `" + NormalizedName(field) + "=`" + Export + "(self: var " +
              NormalizedName(struct_def) +
              ", n: " + GenTypeGet(field.value.type) + ") =\n";
      code += std::string(Indent) + "discard " + SelfData + ".Mutate(" +
              SelfDataPos + " + " + NumToString(field.value.offset) + ", n)\n";
    }
    code += EndFunc;
  }

  // Get the value of a table's scalar.
  void GetScalarFieldOfTable(const StructDef &struct_def, const FieldDef &field,
                             std::string *code_ptr) {
    std::string &code = *code_ptr;
    code += "proc " + NormalizedName(field) + Export +
            "(self: " + NormalizedName(struct_def) +
            "): " + GenTypeGet(field.value.type) + " =\n";

    code += OffsetPrefix(field);
    code += std::string(Indent) + std::string(Indent) +
            "result = " + GenGetter(field.value.type) + "o + " + SelfDataPos +
            ")\n";
    code += std::string(Indent) + "else:\n";
    code += std::string(Indent) + std::string(Indent) +
            "result = " + GetDefaultValue(field) + "\n";

    if (parser_.opts.mutable_buffer) {
      code += "proc `" + NormalizedName(field) + "=`" + Export + "(self: var " +
              NormalizedName(struct_def) +
              ", n: " + GenTypeGet(field.value.type) + "): bool =\n";
      code += std::string(Indent) + "result = " + SelfData + ".MutateSlot(" +
              NumToString(field.value.offset) + ", n)\n";
    }
    code += EndFunc;
  }

  // Get a struct by initializing an existing struct.
  // Specific to Table.
  void GetStructFieldOfTable(const StructDef &struct_def, const FieldDef &field,
                             std::string *code_ptr) {
    std::string &code = *code_ptr;
    imports_.insert(GetImport(struct_def, field));
    code += "proc " + NormalizedName(field) + Export +
            "(self: " + NormalizedName(struct_def) + "): " + TypeName(field) +
            " =\n";
    code += OffsetPrefix(field);
    code += std::string(Indent) + std::string(Indent) + "result.Init(" +
            SelfDataBytes + ", o + " + SelfDataPos + ")\n";
    code += std::string(Indent) + "else:\n";
    code += std::string(Indent) + std::string(Indent) +
            "result = default(type(result))\n";
    code += EndFunc;
  }

  // Get the value of a string.
  void GetStringField(const StructDef &struct_def, const FieldDef &field,
                      std::string *code_ptr) {
    std::string &code = *code_ptr;
    code += "proc " + NormalizedName(field) + Export +
            "(self: " + NormalizedName(struct_def) +
            "): " + GenTypeGet(field.value.type) + " =\n";
    code += OffsetPrefix(field);
    code += std::string(Indent) + std::string(Indent) +
            "result = " + GenGetter(field.value.type) + "o + " + SelfDataPos +
            ")\n";
    code += std::string(Indent) + "else:\n";
    if (field.IsDefault()) {
      code += std::string(Indent) + std::string(Indent) + "result = \"" +
              field.value.constant + "\"\n";
    } else {
      code += std::string(Indent) + std::string(Indent) + "discard\n";
    }
    code += EndFunc;
  }

  // Get the value of a union from an object.
  void GetUnionField(const StructDef &struct_def, const FieldDef &field,
                     std::string *code_ptr) {
    std::string &code = *code_ptr;
    code += "proc " + NormalizedName(field) + Export +
            "(self: " + NormalizedName(struct_def) + "): FlatObj =\n";
    code += OffsetPrefix(field);
    code += std::string(Indent) + std::string(Indent) +
            GenGetter(field.value.type) + "result.tab, o)\n";
    code += std::string(Indent) + "else:\n";
    code += std::string(Indent) + std::string(Indent) + "discard\n";
    code += EndFunc;
  }

  // Get the value of a vector's struct member.
  void GetMemberOfVectorOfStruct(const StructDef &struct_def,
                                 const FieldDef &field, std::string *code_ptr) {
    std::string &code = *code_ptr;
    imports_.insert(GetImport(struct_def, field));
    code += "proc " + NormalizedName(field) + Export +
            "(self: " + NormalizedName(struct_def) +
            ", j: int): " + TypeName(field) + " =\n";
    code += OffsetPrefix(field);
    code += std::string(Indent) + std::string(Indent) + "var x = " + SelfData +
            ".Vector(o)\n";
    code += std::string(Indent) + std::string(Indent) + "x += j.uoffset * " +
            NumToString(InlineSize(field.value.type.VectorType())) +
            ".uoffset\n";

    code += std::string(Indent) + std::string(Indent) + "result.Init(" +
            SelfDataBytes + ", x)\n";
    code += std::string(Indent) + "else:\n";
    code += std::string(Indent) + std::string(Indent) +
            "result = default(type(result))\n";
    code += EndFunc;
  }

  // Get the value of a vector's non-struct member. Uses a named return
  // argument to conveniently set the zero value for the result.
  void GetMemberOfVectorOfNonStruct(const StructDef &struct_def,
                                    const FieldDef &field,
                                    std::string *code_ptr) {
    std::string &code = *code_ptr;
    code += "proc " + NormalizedName(field) + Export +
            "(self: " + NormalizedName(struct_def) +
            ", j: int): " + TypeName(field) + " =\n";
    code += OffsetPrefix(field);
    code += std::string(Indent) + std::string(Indent) + "var x = " + SelfData +
            ".Vector(o)\n";
    code += std::string(Indent) + std::string(Indent) + "x += j.uoffset * " +
            NumToString(InlineSize(field.value.type.VectorType())) +
            ".uoffset\n";

    code += std::string(Indent) + std::string(Indent) +
            "result = " + GenGetter(field.value.type) + "x)\n";
    code += std::string(Indent) + "else:\n";
    code += std::string(Indent) + std::string(Indent) + "discard\n";
    code += EndFunc;
  }

  // Begin the creator function signature.
  void BeginBuilderArgs(const StructDef &struct_def, std::string *code_ptr) {
    std::string &code = *code_ptr;
    code += "proc Create" + NormalizedName(struct_def) + Export +
            "(self: var Builder";
  }

  // Recursively generate arguments for a constructor, to deal with nested
  // structs.
  void StructBuilderArgs(const StructDef &struct_def, const char *nameprefix,
                         std::string *code_ptr) {
    for (auto it = struct_def.fields.vec.begin();
         it != struct_def.fields.vec.end(); ++it) {
      auto &field = **it;
      if (IsStruct(field.value.type)) {
        // Generate arguments for a struct inside a struct. To ensure names
        // don't clash, and to make it obvious these arguments are constructing
        // a nested struct, prefix the name with the field name.
        StructBuilderArgs(*field.value.type.struct_def,
                          (nameprefix + (NormalizedName(field) + "_")).c_str(),
                          code_ptr);
      } else {
        std::string &code = *code_ptr;
        code += std::string(", ") + nameprefix;
        code += ConvertCase(NormalizedName(field), Case::kLowerCamel) + ": " +
                GenTypeBasic(field.value.type);
      }
    }
  }

  // End the creator function signature.
  void EndBuilderArgs(std::string *code_ptr) {
    std::string &code = *code_ptr;
    code += "): uoffset =\n";
  }

  // Recursively generate struct construction statements and instert manual
  // padding.
  void StructBuilderBody(const StructDef &struct_def, const char *nameprefix,
                         std::string *code_ptr) {
    std::string &code = *code_ptr;
    code += std::string(Indent) + "self.Prep(" +
            NumToString(struct_def.minalign) + ", ";
    code += NumToString(struct_def.bytesize) + ")\n";
    for (auto it = struct_def.fields.vec.rbegin();
         it != struct_def.fields.vec.rend(); ++it) {
      auto &field = **it;
      if (field.padding)
        code += std::string(Indent) + "self.Pad(" + NumToString(field.padding) +
                ")\n";
      if (IsStruct(field.value.type)) {
        StructBuilderBody(*field.value.type.struct_def,
                          (nameprefix + (NormalizedName(field) + "_")).c_str(),
                          code_ptr);
      } else {
        code += std::string(Indent) + "self.Prepend" + GenMethod(field) + "(";
        code += nameprefix +
                ConvertCase(NormalizedName(field), Case::kLowerCamel) + ")\n";
      }
    }
  }

  void EndBuilderBody(std::string *code_ptr) {
    std::string &code = *code_ptr;
    code += std::string(Indent) + "result = self.Offset()\n";
    code += EndFunc;
  }

  // Get the value of a table's starting offset.
  void GetStartOfTable(const StructDef &struct_def, std::string *code_ptr) {
    std::string &code = *code_ptr;
    code += "proc " + NormalizedName(struct_def) + "Start" + Export +
            "(builder: var Builder) =\n";
    code += std::string(Indent) + "builder.StartObject(" +
            NumToString(struct_def.fields.vec.size()) + ")\n";
  }

  // Set the value of a table's field.
  void BuildFieldOfTable(const StructDef &struct_def, const FieldDef &field,
                         const size_t offset, std::string *code_ptr) {
    std::string &code = *code_ptr;
    code += "proc " + NormalizedName(struct_def) + "Add" +
            ConvertCase(NormalizedName(field), Case::kUpperCamel) + Export +
            "(builder: var Builder, " +
            ConvertCase(NormalizedName(field), Case::kLowerCamel) + ": " +
            GenTypeBasic(field.value.type) + ") =\n";
    code += std::string(Indent) + "builder.Prepend" + GenMethod(field) +
            "Slot(" + NumToString(offset) + ", " +
            ConvertCase(NormalizedName(field), Case::kLowerCamel) + ", " +
            GetDefaultValue(field) + ")\n";
  }

  // Set the value of one of the members of a table's vector.
  void BuildVectorOfTable(const StructDef &struct_def, const FieldDef &field,
                          std::string *code_ptr) {
    std::string &code = *code_ptr;
    code += "proc " + NormalizedName(struct_def) + "Start" +
            ConvertCase(NormalizedName(field), Case::kUpperCamel) + "Vector" +
            Export + "(builder: var Builder, numElems: int): uoffset =\n";
    code += std::string(Indent) + "builder.StartVector(" +
            NumToString(InlineSize(field.value.type.VectorType())) +
            ", numElems, " +
            NumToString(InlineAlignment(field.value.type.VectorType())) + ")\n";
  }

  // Get the offset of the end of a table.
  void GetEndOffsetOnTable(const StructDef &struct_def, std::string *code_ptr) {
    std::string &code = *code_ptr;
    code += "proc " + NormalizedName(struct_def) + "End" + Export +
            "(builder: var Builder): uoffset =\n";
    code += std::string(Indent) + "result = builder.EndObject()\n";
    code += "\n";
  }

  // Generate a struct field, conditioned on its child type(s).
  void GenStructAccessor(const StructDef &struct_def, const FieldDef &field,
                         std::string *code_ptr) {
    GenComment(field.doc_comment, code_ptr, &def_comment);
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
        case BASE_TYPE_STRING:
          GetStringField(struct_def, field, code_ptr);
          break;
        case BASE_TYPE_VECTOR: {
          auto vectortype = field.value.type.VectorType();
          GetVectorLen(struct_def, field, code_ptr);
          if (vectortype.base_type == BASE_TYPE_STRUCT) {
            GetMemberOfVectorOfStruct(struct_def, field, code_ptr);
          } else {
            GetMemberOfVectorOfNonStruct(struct_def, field, code_ptr);
          }
          GetVector(struct_def, field, code_ptr);
          break;
        }
        case BASE_TYPE_UNION: GetUnionField(struct_def, field, code_ptr); break;
        default: FLATBUFFERS_ASSERT(0);
      }
    }
  }

  // Generate table constructors, conditioned on its members' types.
  void GenTableBuilders(const StructDef &struct_def, std::string *code_ptr) {
    GetStartOfTable(struct_def, code_ptr);

    for (auto it = struct_def.fields.vec.begin();
         it != struct_def.fields.vec.end(); ++it) {
      auto &field = **it;
      if (field.deprecated) continue;

      auto offset = it - struct_def.fields.vec.begin();
      BuildFieldOfTable(struct_def, field, offset, code_ptr);
      if (IsVector(field.value.type)) {
        BuildVectorOfTable(struct_def, field, code_ptr);
      }
    }

    GetEndOffsetOnTable(struct_def, code_ptr);
  }

  // Generate struct or table methods.
  void GenStruct(const StructDef &struct_def, std::string *code_ptr) {
    if (struct_def.generated) return;

    GenComment(struct_def.doc_comment, code_ptr, &def_comment);
    BeginClass(struct_def, code_ptr);

    // Generate the Init method that sets the field in a pre-existing
    // accessor object. This is to allow object reuse.
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

    GenComment(enum_def.doc_comment, code_ptr, &def_comment);
    BeginEnum(NormalizedName(enum_def), code_ptr);
    for (auto it = enum_def.Vals().begin(); it != enum_def.Vals().end(); ++it) {
      auto &ev = **it;
      GenComment(ev.doc_comment, code_ptr, &def_comment, Indent);
      EnumMember(enum_def, ev, code_ptr);
    }
    EndEnum(code_ptr);
  }

  // Returns the method name for use with add/put calls.
  std::string GenMethod(const FieldDef &field) const {
    if (IsStruct(field.value.type)) {
      return "Struct";
    } else {
      return "";
    }
  }

  std::string GenTypeBasic(const Type &type) {
    // clang-format off
    static const char *nimtypename[] = {
      #define FLATBUFFERS_TD(ENUM, IDLTYPE, \
              CTYPE, JTYPE, GTYPE, NTYPE, PTYPE, RTYPE, KTYPE, STYPE, NIMTYPE) \
        #NIMTYPE,
        FLATBUFFERS_GEN_TYPES(FLATBUFFERS_TD)
      #undef FLATBUFFERS_TD
    };
    // clang-format on
    return nimtypename[type.base_type];
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

  // Returns the function name that is able to read a value of the given type.
  std::string GenGetter(const Type &type) {
    switch (type.base_type) {
      case BASE_TYPE_STRING: return std::string(SelfData) + ".String(";
      case BASE_TYPE_UNION: return std::string(SelfData) + ".Union(";
      case BASE_TYPE_VECTOR: return GenGetter(type.VectorType());
      default:
        return "Get[" + GenTypeGet(type) + "](" + std::string(SelfData) + ", ";
    }
  }

  std::string GetNamespace(const Type &type) {
    return type.struct_def->defined_namespace->GetFullyQualifiedName(
        type.struct_def->name);
  }

  std::string GetNamespace(const FieldDef &field) {
    return GetNamespace(field.value.type);
  }

  std::string GetNamespace(const StructDef &struct_def) {
    return struct_def.defined_namespace->GetFullyQualifiedName(struct_def.name);
  }

  std::string TypeName(const FieldDef &field) {
    return GenTypeGet(field.value.type);
  }

  std::string TypeNameWithNamespace(const FieldDef &field) {
    return GetNamespace(field.value.type);
  }

  std::string GetDefaultValue(const FieldDef &field) {
    BaseType base_type = field.value.type.base_type;
    if (field.IsScalarOptional()) {
      return "default(" + GenTypeBasic(field.value.type) + ")";
    } else if (IsBool(base_type)) {
      return field.value.constant == "0" ? "false" : "true";
    } else if (IsFloat(base_type)) {
      return field.value.constant;
    } else if (IsInteger(base_type)) {
      return field.value.constant;
    } else {
      // For string, struct, and table.
      return "default(" + GenTypeBasic(field.value.type) + ")";
    }
  }

  std::vector<std::string> StringSplit(const std::string orig_str,
                                       const std::string token) {
    std::vector<std::string> result;
    std::string str = orig_str;
    while (str.size()) {
      size_t index = str.find(token);
      if (index != std::string::npos) {
        result.push_back(str.substr(0, index));
        str = str.substr(index + token.size());
        if (str.size() == 0) result.push_back(str);
      } else {
        result.push_back(str);
        str = "";
      }
    }
    return result;
  }
  std::string GetRelativePathFromNamespace(const std::string &relative_to,
                                           const std::string &str2) {
    std::vector<std::string> relative_to_vec = StringSplit(relative_to, ".");
    std::vector<std::string> str2_vec = StringSplit(str2, ".");
    while (relative_to_vec.size() > 0 && str2_vec.size() > 0) {
      if (relative_to_vec[0] == str2_vec[0]) {
        relative_to_vec.erase(relative_to_vec.begin());
        str2_vec.erase(str2_vec.begin());
      } else {
        break;
      }
    }
    relative_to_vec.pop_back();
    for (size_t i = 0; i < relative_to_vec.size(); ++i) {
      str2_vec.insert(str2_vec.begin(), std::string(".."));
    }
    str2_vec[str2_vec.size() - 1] =
        ConvertCase(str2_vec[str2_vec.size() - 1], Case::kAllLower);
    std::string new_path;
    for (size_t i = 0; i < str2_vec.size(); ++i) {
      new_path += str2_vec[i];
      if (i != str2_vec.size() - 1) { new_path += "/"; }
    }
    return new_path;
  }

  std::string GetImport(const StructDef &struct_def, const FieldDef &field) {
    if (GetNamespace(field) != GetNamespace(struct_def)) {
      return "from " +
             GetRelativePathFromNamespace(GetNamespace(struct_def),
                                          GetNamespace(field)) +
             "_generated import " + TypeName(field) + "\n";
    } else {
      return std::string();
    }
  }

  // Create a struct with a builder and the struct's arguments.
  void GenStructBuilder(const StructDef &struct_def, std::string *code_ptr) {
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
  void BeginFile(const std::string &name_space_name, const bool needs_imports,
                 std::string *code_ptr) {
    std::string &code = *code_ptr;
    code += std::string(Comment) + FlatBuffersGeneratedWarning() + "\n\n";
    code += std::string(Comment) + "namespace: " + name_space_name + "\n\n";
    if (needs_imports) { code += "import flatbuffers \n\n"; }
    for (std::string import : imports_) { code += import; }
    imports_.clear();
    code += "\n";
  }

  // Save out the generated code for a Nim Table type.
  bool SaveType(const Definition &def, const std::string &classcode,
                bool needs_imports) {
    if (!classcode.length()) return true;

    std::string namespace_dir = path_;
    auto &namespaces = def.defined_namespace->components;
    for (auto it = namespaces.begin(); it != namespaces.end(); ++it) {
      if (it != namespaces.begin()) namespace_dir += kPathSeparator;
      namespace_dir += *it;
    }

    std::string code = "";
    BeginFile(LastNamespacePart(*def.defined_namespace), needs_imports, &code);
    code += classcode;

    std::string filename = NamespaceDir(*def.defined_namespace) +
                           ConvertCase(NormalizedName(def), Case::kAllLower) +
                           "_generated.nim";
    return SaveFile(filename.c_str(), code, false);
  }

 private:
  std::unordered_set<std::string> keywords_;
  std::unordered_set<std::string> imports_;
};

}  // namespace nim

bool GenerateNim(const Parser &parser, const std::string &path,
                 const std::string &file_name) {
  nim::NimGenerator generator(parser, path, file_name);
  return generator.generate();
}

}  // namespace flatbuffers
