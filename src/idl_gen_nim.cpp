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
#include "idl_namer.h"

namespace flatbuffers {
namespace nim {

std::set<std::string> NimKeywords() {
  return {
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
}

Namer::Config NimDefaultConfig() {
  return { /*types=*/Case::kKeep,
           /*constants=*/Case::kScreamingSnake,
           /*methods=*/Case::kUpperCamel,
           /*functions=*/Case::kUpperCamel,
           /*fields=*/Case::kLowerCamel,
           /*variable=*/Case::kLowerCamel,
           /*variants=*/Case::kKeep,
           /*enum_variant_seperator=*/".",
           /*escape_keywords=*/Namer::Config::Escape::AfterConvertingCase,
           /*namespaces=*/Case::kKeep,
           /*namespace_seperator=*/"/",
           /*object_prefix=*/"",
           /*object_suffix=*/"T",
           /*keyword_prefix=*/"",
           /*keyword_suffix=*/"_",
           /*filenames=*/Case::kKeep,
           /*directories=*/Case::kKeep,
           /*output_path=*/"",
           /*filename_suffix=*/"",
           /*filename_extension=*/".nim" };
}
// Hardcode spaces per indentation.
const CommentConfig def_comment = { nullptr, "#", nullptr };
const std::string Indent = "  ";
const std::string Comment = "# ";
const std::string End = "\n";
const std::string EndFunc = "\n";
const std::string Export = "*";
const std::string SelfData = "self.tab";
const std::string SelfDataPos = "self.tab.Pos";
const std::string SelfDataBytes = "self.tab.Bytes";

class NimGenerator : public BaseGenerator {
 public:
  NimGenerator(const Parser &parser, const std::string &path,
               const std::string &file_name)
      : BaseGenerator(parser, path, file_name, "" /* not used */,
                      "" /* not used */, "nim"),
        namer_(WithFlagOptions(NimDefaultConfig(), parser.opts, path),
               NimKeywords()) {}

  // Most field accessors need to retrieve and test the field offset first,
  // this is the prefix code for that.
  std::string OffsetPrefix(const FieldDef &field) {
    return Indent + "let o = " + SelfData + ".Offset(" +
           NumToString(field.value.offset) + ")\n" + Indent + "if o != 0:\n";
  }

  // Begin a class declaration.
  void BeginClass(const StructDef &struct_def, std::string &code) {
    code +=
        "type " + namer_.Type(struct_def) + Export + " = object of FlatObj\n";
    code += "\n";
  }

  // Begin enum code with a class declaration.
  void BeginEnum(const std::string &class_name, std::string &code) {
    code += "type " + class_name + Export + " {.pure.} = enum\n";
  }

  // Starts a new line and then indents.
  std::string GenIndents(int num) const {
    return "\n" + std::string(num * Indent.length(), ' ');
  }

  // A single enum member.
  void EnumMember(const EnumDef &enum_def, const EnumVal &ev,
                  std::string &code) {
    code += Indent + namer_.Variant(ev) + " = " + enum_def.ToString(ev) + "." +
            GenTypeBasic(enum_def.underlying_type) + ",\n";
  }

  // End enum code.
  void EndEnum(std::string &code) { code += "\n"; }

  // Get the length of a vector.
  void GetVectorLen(const StructDef &struct_def, const FieldDef &field,
                    std::string &code) {
    code += "proc " + namer_.Field(field) + "Length" + Export +
            "(self: " + namer_.Type(struct_def) + "): int =\n";
    code += OffsetPrefix(field);
    code += Indent + Indent + "result = " + SelfData + ".Vectorlen(o)\n";
    code += EndFunc;
  }

  void GetStructVector(const StructDef &struct_def, const FieldDef &field,
                       std::string &code) {
    imports_.insert(GetImport(struct_def, field));
    code += "proc " + namer_.Field(field) + Export +
            "(self: " + namer_.Type(struct_def) + "): seq[" +
            ImportedName(struct_def, field) + "] =\n";
    code += Indent + "let len = self." + namer_.Field(field) + "Length\n";
    code += Indent + "for i in countup(0, len - 1):\n";
    code +=
        Indent + Indent + "result.add(self." + namer_.Field(field) + "(i))\n";
    code += EndFunc;
  }

  void GetEnumVector(const StructDef &struct_def, const FieldDef &field,
                     std::string &code) {
    imports_.insert(GetImport(struct_def, field));
    code += "proc " + namer_.Field(field) + Export +
            "(self: " + namer_.Type(struct_def) + "): seq[" +
            ImportedName(struct_def, field) + "] =\n";
    code += Indent + "let len = self." + namer_.Field(field) + "Length\n";
    code += Indent + "for i in countup(0, len - 1):\n";
    code +=
        Indent + Indent + "result.add(self." + namer_.Field(field) + "(i))\n";
    code += EndFunc;
  }

  void GetMemberOfVectorOfEnum(const StructDef &struct_def,
                               const FieldDef &field, std::string &code) {
    imports_.insert(GetImport(struct_def, field));
    code += "proc " + namer_.Field(field) + Export +
            "(self: " + namer_.Type(struct_def) +
            ", j: int): " + ImportedName(struct_def, field) + " =\n";
    code += OffsetPrefix(field);
    code += Indent + Indent + "var x = " + SelfData + ".Vector(o)\n";
    code += Indent + Indent + "x += j.uoffset * " +
            NumToString(InlineSize(field.value.type.VectorType())) +
            ".uoffset\n";

    code += Indent + Indent + "result = " + ImportedName(struct_def, field) +
            "(" + GenGetter(field.value.type) + "x))\n";
    code += EndFunc;
  }

  void GetNonStructVector(const StructDef &struct_def, const FieldDef &field,
                          std::string &code) {
    code += "proc " + namer_.Field(field) + Export +
            "(self: " + namer_.Type(struct_def) + "): seq[" + TypeName(field) +
            "] =\n";
    code += Indent + "let len = self." + namer_.Field(field) + "Length\n";
    code += Indent + "for i in countup(0, len - 1):\n";
    code +=
        Indent + Indent + "result.add(self." + namer_.Field(field) + "(i))\n";
    code += EndFunc;
  }

  // Get the value of a struct's scalar.
  void GetScalarFieldOfStruct(const StructDef &struct_def,
                              const FieldDef &field, std::string &code) {
    std::string field_type = GenTypeGet(field.value.type);
    if (IsEnum(field.value.type)) {
      imports_.insert(GetImport(struct_def, field));
      field_type = ImportedName(struct_def, field);
    }
    code += "proc " + namer_.Field(field) + Export +
            "(self: " + namer_.Type(struct_def) + "): " + field_type + " =\n";
    code += Indent + "result = ";
    if (IsEnum(field.value.type)) { code += field_type + "("; }
    code += GenGetter(field.value.type) + SelfDataPos + " + " +
            NumToString(field.value.offset) + ")";
    if (IsEnum(field.value.type)) { code += ")"; }
    code += "\n";

    if (parser_.opts.mutable_buffer) {
      code += "proc `" + namer_.Field(field) + "=`" + Export + "(self: var " +
              namer_.Type(struct_def) + ", n: " + GenTypeGet(field.value.type) +
              "): bool =\n";
      code += Indent + "result = ";
      code += SelfData + ".Mutate(" + SelfDataPos + " + " +
              NumToString(field.value.offset) + ", n";
      if (IsEnum(field.value.type)) {
        code += "." + GenTypeGet(field.value.type);
      }
      code += ")\n";
    }
    code += EndFunc;
  }

  // Get the value of a struct's struct.
  void GetStructFieldOfStruct(const StructDef &struct_def,
                              const FieldDef &field, std::string &code) {
    imports_.insert(GetImport(struct_def, field));
    code += "proc " + namer_.Field(field) + Export +
            "(self: " + namer_.Type(struct_def) +
            "): " + ImportedName(struct_def, field) + " =\n";
    code += Indent + "result.Init(" + SelfDataBytes + ", " + SelfDataPos +
            " + " + NumToString(field.value.offset) + ")\n";

    if (parser_.opts.mutable_buffer) {
      code += "proc `" + namer_.Field(field) + "=`" + Export + "(self: var " +
              namer_.Type(struct_def) +
              ", n: " + ImportedName(struct_def, field) + ") =\n";
      code += Indent + "discard " + SelfData + ".Mutate(" + SelfDataPos +
              " + " + NumToString(field.value.offset) + ", n)\n";
    }
    code += EndFunc;
  }

  // Get the value of a table's scalar.
  void GetScalarFieldOfTable(const StructDef &struct_def, const FieldDef &field,
                             std::string &code) {
    std::string field_type = GenTypeGet(field.value.type);
    if (IsEnum(field.value.type)) {
      imports_.insert(GetImport(struct_def, field));
      field_type = ImportedName(struct_def, field);
    }
    std::string returntype = field_type;
    if (field.IsScalarOptional()) {
      ImportOptions();
      returntype = "Option[" + field_type + "]";
    }

    code += "proc " + namer_.Field(field) + Export +
            "(self: " + namer_.Type(struct_def) + "): " + returntype + " =\n";

    code += OffsetPrefix(field);
    code += Indent + Indent + "result = ";
    if (field.IsScalarOptional()) { code += "some("; }
    if (IsEnum(field.value.type)) { code += field_type + "("; }
    code += GenGetter(field.value.type) + "o + " + SelfDataPos + ")";
    if (IsEnum(field.value.type)) { code += ")"; }
    if (field.IsScalarOptional()) { code += ")"; }
    code += "\n";
    if (!field.IsScalarOptional()) {
      std::string default_value;
      if (IsBool(field.value.type.base_type)) {
        default_value = field.value.constant == "0" ? "false" : "true";
      } else {
        default_value = field.value.constant;
      }
      code += Indent + "else:\n";
      code += Indent + Indent + "result = ";
      if (IsEnum(field.value.type)) { code += field_type + "("; }
      code += default_value;
      if (IsEnum(field.value.type)) { code += ")"; }
      code += "\n\n";
    }

    if (parser_.opts.mutable_buffer) {
      code += "proc `" + namer_.Field(field) + "=`" + Export + "(self: var " +
              namer_.Type(struct_def) + ", n: " + field_type + "): bool =\n";
      code += Indent + "result = " + SelfData + ".MutateSlot(" +
              NumToString(field.value.offset) + ", n";
      if (IsEnum(field.value.type)) {
        code += "." + GenTypeGet(field.value.type);
      }
      code += ")\n";
    }
    code += EndFunc;
  }

  // Get a struct by initializing an existing struct.
  // Specific to Table.
  void GetStructFieldOfTable(const StructDef &struct_def, const FieldDef &field,
                             std::string &code) {
    ImportOptions();
    imports_.insert(GetImport(struct_def, field));
    code += "proc " + namer_.Field(field) + Export +
            "(self: " + namer_.Type(struct_def) + "): Option[" +
            ImportedName(struct_def, field) + "] =\n";
    code += OffsetPrefix(field);
    // var x: Property.Property
    // x.Init(self.tab.Bytes, o + self.tab.Pos)
    // result = some(x)
    code +=
        Indent + Indent + "var x: " + ImportedName(struct_def, field) + "\n";
    code += Indent + Indent + "x.Init(" + SelfDataBytes + ", o + " +
            SelfDataPos + ")\n";
    code += Indent + Indent + "result = some(x)\n";
    code += EndFunc;
  }

  // Get the value of a string.
  void GetStringField(const StructDef &struct_def, const FieldDef &field,
                      std::string &code) {
    ImportOptions();
    code += "proc " + namer_.Field(field) + Export +
            "(self: " + namer_.Type(struct_def) + "): Option[" +
            GenTypeGet(field.value.type) + "] =\n";
    code += OffsetPrefix(field);
    code += Indent + Indent + "result = some(" + GenGetter(field.value.type) +
            "o + " + SelfDataPos + "))\n";
    if (field.IsDefault()) {
      code += Indent + "else:\n";
      code +=
          Indent + Indent + "result = some(\"" + field.value.constant + "\")\n";
    }
    code += EndFunc;
  }

  // Get the value of a union from an object.
  void GetUnionField(const StructDef &struct_def, const FieldDef &field,
                     std::string &code) {
    ImportOptions();
    code += "proc " + namer_.Field(field) + Export +
            "(self: " + namer_.Type(struct_def) + "): Option[Vtable] =\n";
    code += OffsetPrefix(field);
    code += Indent + Indent + "result = some(" + GenGetter(field.value.type) +
            "o))\n";
    code += EndFunc;
  }

  // Get the value of a vector's struct member.
  void GetMemberOfVectorOfStruct(const StructDef &struct_def,
                                 const FieldDef &field, std::string &code) {
    imports_.insert(GetImport(struct_def, field));
    code += "proc " + namer_.Field(field) + Export +
            "(self: " + namer_.Type(struct_def) +
            ", j: int): " + ImportedName(struct_def, field) + " =\n";
    code += OffsetPrefix(field);
    code += Indent + Indent + "var x = " + SelfData + ".Vector(o)\n";
    code += Indent + Indent + "x += j.uoffset * " +
            NumToString(InlineSize(field.value.type.VectorType())) +
            ".uoffset\n";

    code += Indent + Indent + "result.Init(" + SelfDataBytes + ", x)\n";
    code += EndFunc;
  }

  // Get the value of a vector's non-struct member. Uses a named return
  // argument to conveniently set the zero value for the result.
  void GetMemberOfVectorOfNonStruct(const StructDef &struct_def,
                                    const FieldDef &field, std::string &code) {
    code += "proc " + namer_.Field(field) + Export +
            "(self: " + namer_.Type(struct_def) +
            ", j: int): " + TypeName(field) + " =\n";
    code += OffsetPrefix(field);
    code += Indent + Indent + "var x = " + SelfData + ".Vector(o)\n";
    code += Indent + Indent + "x += j.uoffset * " +
            NumToString(InlineSize(field.value.type.VectorType())) +
            ".uoffset\n";

    code +=
        Indent + Indent + "result = " + GenGetter(field.value.type) + "x)\n";
    code += EndFunc;
  }

  // Begin the creator function signature.
  void BeginBuilderArgs(const StructDef &struct_def, std::string &code) {
    code += "proc Create" + namer_.Type(struct_def) + Export +
            "(self: var Builder, ";
  }

  // Recursively generate arguments for a constructor, to deal with nested
  // structs.
  void StructBuilderArgs(const StructDef &struct_def,
                         const std::string nameprefix,
                         const std::string namesuffix, bool has_field_name,
                         bool with_type, const std::string fieldname_suffix,
                         std::string &code) {
    for (auto it = struct_def.fields.vec.begin();
         it != struct_def.fields.vec.end(); ++it) {
      auto &field = **it;
      const auto &field_type = field.value.type;
      const auto &type =
          IsArray(field_type) ? field_type.VectorType() : field_type;
      if (it != struct_def.fields.vec.begin()) { code += std::string(", "); }
      if (IsStruct(type)) {
        // Generate arguments for a struct inside a struct. To ensure names
        // don't clash, and to make it obvious these arguments are constructing
        // a nested struct, prefix the name with the field name.
        auto subprefix = nameprefix;
        if (has_field_name) {
          subprefix += namer_.Field(field) + fieldname_suffix;
        }
        StructBuilderArgs(*field.value.type.struct_def, subprefix, namesuffix,
                          has_field_name, with_type, fieldname_suffix, code);
      } else {
        code += nameprefix;
        if (has_field_name) { code += namer_.Field(field); }
        code += namesuffix;
        if (with_type) {
          code += ": ";

          if (IsEnum(field.value.type)) {
            imports_.insert(GetImport(struct_def, field));
            code += ImportedName(struct_def, field);
          } else {
            code += GenTypeBasic(field.value.type);
          }
        };
      }
    }
  }

  // End the creator function signature.
  void EndBuilderArgs(std::string &code) { code += "): uoffset =\n"; }

  // Recursively generate struct construction statements and instert manual
  // padding.
  void StructBuilderBody(const StructDef &struct_def, const char *nameprefix,
                         std::string &code) {
    code += Indent + "self.Prep(" + NumToString(struct_def.minalign) + ", ";
    code += NumToString(struct_def.bytesize) + ")\n";
    for (auto it = struct_def.fields.vec.rbegin();
         it != struct_def.fields.vec.rend(); ++it) {
      auto &field = **it;
      if (field.padding)
        code += Indent + "self.Pad(" + NumToString(field.padding) + ")\n";
      if (IsStruct(field.value.type)) {
        StructBuilderBody(*field.value.type.struct_def,
                          (nameprefix + (namer_.Field(field) + "_")).c_str(),
                          code);
      } else {
        code += Indent + "self.Prepend" + GenMethod(field) + "(";
        code += nameprefix + namer_.Field(field);
        if (IsEnum(field.value.type)) {
          code += "." + GenTypeGet(field.value.type);
        }
        code += ")\n";
      }
    }
  }

  void EndBuilderBody(std::string &code) {
    code += Indent + "result = self.Offset()\n";
    code += EndFunc;
  }

  // Get the value of a table's starting offset.
  void GetStartOfTable(const StructDef &struct_def, std::string &code) {
    code += "proc " + namer_.Type(struct_def) + "Start" + Export +
            "(builder: var Builder) =\n";
    code += Indent + "builder.StartObject(" +
            NumToString(struct_def.fields.vec.size()) + ")\n";
  }

  // Set the value of a table's field.
  void BuildFieldOfTable(const StructDef &struct_def, const FieldDef &field,
                         const size_t offset, std::string &code) {
    std::string field_type = GenTypeBasic(field.value.type);
    std::string variable = namer_.Variable(field);
    if (IsEnum(field.value.type)) {
      imports_.insert(GetImport(struct_def, field));
      field_type = ImportedName(struct_def, field);
      variable += "." + GenTypeBasic(field.value.type);
    }
    code += "proc " + namer_.Type(struct_def) + "Add" + namer_.Method(field) +
            Export + "(builder: var Builder, " + namer_.Variable(field) + ": " +
            field_type + ") =\n";
    code += Indent + "builder.Prepend" + GenMethod(field) + "Slot(" +
            NumToString(offset) + ", " + variable + ", " +
            GetDefaultValue(field) + ")\n";
  }

  // Set the value of one of the members of a table's vector.
  void BuildVectorOfTable(const StructDef &struct_def, const FieldDef &field,
                          std::string &code) {
    code += "proc " + namer_.Type(struct_def) + "Start" + namer_.Method(field) +
            "Vector" + Export +
            "(builder: var Builder, numElems: uoffset): uoffset =\n";
    code += Indent + "builder.StartVector(" +
            NumToString(InlineSize(field.value.type.VectorType())) +
            ", numElems, " +
            NumToString(InlineAlignment(field.value.type.VectorType())) + ")\n";
  }

  // Get the offset of the end of a table.
  void GetEndOffsetOnTable(const StructDef &struct_def, std::string &code) {
    code += "proc " + namer_.Type(struct_def) + "End" + Export +
            "(builder: var Builder): uoffset =\n";
    code += Indent + "result = builder.EndObject()\n";
    code += "\n";
  }

  // Generate a struct field, conditioned on its child type(s).
  void GenStructAccessor(const StructDef &struct_def, const FieldDef &field,
                         std::string &code) {
    GenComment(field.doc_comment, &code, &def_comment);
    if (IsScalar(field.value.type.base_type)) {
      if (struct_def.fixed) {
        GetScalarFieldOfStruct(struct_def, field, code);
      } else {
        GetScalarFieldOfTable(struct_def, field, code);
      }
    } else {
      switch (field.value.type.base_type) {
        case BASE_TYPE_STRUCT:
          if (struct_def.fixed) {
            GetStructFieldOfStruct(struct_def, field, code);
          } else {
            GetStructFieldOfTable(struct_def, field, code);
          }
          break;
        case BASE_TYPE_STRING: GetStringField(struct_def, field, code); break;
        case BASE_TYPE_VECTOR: {
          auto vectortype = field.value.type.VectorType();
          GetVectorLen(struct_def, field, code);
          if (vectortype.base_type == BASE_TYPE_STRUCT) {
            GetMemberOfVectorOfStruct(struct_def, field, code);
            GetStructVector(struct_def, field, code);
          } else if (IsEnum(vectortype) ||
                     vectortype.base_type == BASE_TYPE_UNION) {
            GetMemberOfVectorOfEnum(struct_def, field, code);
            GetEnumVector(struct_def, field, code);
          } else {
            GetMemberOfVectorOfNonStruct(struct_def, field, code);
            GetNonStructVector(struct_def, field, code);
          }
          break;
        }
        case BASE_TYPE_UNION: GetUnionField(struct_def, field, code); break;
        default: FLATBUFFERS_ASSERT(0);
      }
    }
  }

  // Generate table constructors, conditioned on its members' types.
  void GenTableBuilders(const StructDef &struct_def, std::string &code) {
    GetStartOfTable(struct_def, code);

    for (auto it = struct_def.fields.vec.begin();
         it != struct_def.fields.vec.end(); ++it) {
      auto &field = **it;
      if (field.deprecated) continue;

      auto offset = it - struct_def.fields.vec.begin();
      BuildFieldOfTable(struct_def, field, offset, code);
      if (IsVector(field.value.type)) {
        BuildVectorOfTable(struct_def, field, code);
      }
    }

    GetEndOffsetOnTable(struct_def, code);
  }

  // Generate struct or table methods.
  void GenStruct(const StructDef &struct_def, std::string &code) {
    if (struct_def.generated) return;

    GenComment(struct_def.doc_comment, &code, &def_comment);
    BeginClass(struct_def, code);

    // Generate the Init method that sets the field in a pre-existing
    // accessor object. This is to allow object reuse.
    for (auto it = struct_def.fields.vec.begin();
         it != struct_def.fields.vec.end(); ++it) {
      auto &field = **it;
      if (field.deprecated) continue;

      GenStructAccessor(struct_def, field, code);
    }

    if (struct_def.fixed) {
      // create a struct constructor function
      GenStructBuilder(struct_def, code);
    } else {
      // Create a set of functions that allow table construction.
      GenTableBuilders(struct_def, code);
    }
  }

  // Generate enum declarations.
  void GenEnum(const EnumDef &enum_def, std::string &code) {
    if (enum_def.generated) return;

    GenComment(enum_def.doc_comment, &code, &def_comment);
    BeginEnum(namer_.Type(enum_def), code);
    for (auto it = enum_def.Vals().begin(); it != enum_def.Vals().end(); ++it) {
      auto &ev = **it;
      GenComment(ev.doc_comment, &code, &def_comment, Indent.c_str());
      EnumMember(enum_def, ev, code);
    }
    EndEnum(code);
  }

  // Returns the method name for use with add/put calls.
  std::string GenMethod(const FieldDef &field) const {
    if (IsStruct(field.value.type)) {
      return "Struct";
    } else {
      return "";
    }
  }

  std::string GenTypeBasic(const Type &type) const {
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

  std::string GenTypePointer(const Type &type) const {
    switch (type.base_type) {
      case BASE_TYPE_STRING: return "string";
      case BASE_TYPE_VECTOR: return GenTypeGet(type.VectorType());
      case BASE_TYPE_STRUCT: return type.struct_def->name;
      case BASE_TYPE_UNION:
        // fall through
      default: return "*flatbuffers.Table";
    }
  }

  std::string GenTypeGet(const Type &type) const {
    return IsScalar(type.base_type) ? GenTypeBasic(type) : GenTypePointer(type);
  }

  // Returns the function name that is able to read a value of the given type.
  std::string GenGetter(const Type &type) const {
    switch (type.base_type) {
      case BASE_TYPE_STRING: return SelfData + ".String(";
      case BASE_TYPE_UNION: return SelfData + ".Union(";
      case BASE_TYPE_VECTOR: return GenGetter(type.VectorType());
      default: return "Get[" + GenTypeGet(type) + "](" + SelfData + ", ";
    }
  }

  // std::string GetNamespace(const Type &type) const {
  //   return type.struct_def->defined_namespace->GetFullyQualifiedName(
  //       type.struct_def->name);
  // }

  // std::string GetNamespace(const FieldDef &field) const {
  //   return GetNamespace(field.value.type);
  // }

  // std::string GetNamespace(const EnumVal &val) const {
  //   return
  //   val.union_type.struct_def->defined_namespace->GetFullyQualifiedName(
  //       val.name);
  // }

  // std::string GetNamespace(const StructDef &struct_def) const {
  //   return
  //   struct_def.defined_namespace->GetFullyQualifiedName(struct_def.name);
  // }

  // std::string GetNamespace(const EnumDef &enum_def) const {
  //   return enum_def.defined_namespace->GetFullyQualifiedName(enum_def.name);
  // }

  std::string TypeName(const FieldDef &field) const {
    if (field.value.type.struct_def) {
      return namer_.Type(*field.value.type.struct_def);
    } else if (field.value.type.enum_def) {
      return namer_.Type(*field.value.type.enum_def);
    } else {
      return GenTypeGet(field.value.type);
    }
  }

  std::string GetDefaultValue(const FieldDef &field) const {
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
    std::vector<std::string> relative_to_vec = StringSplit(relative_to, "/");
    std::vector<std::string> str2_vec = StringSplit(str2, "/");
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

    std::string new_path;
    for (size_t i = 0; i < str2_vec.size(); ++i) {
      new_path += str2_vec[i];
      if (i != str2_vec.size() - 1) { new_path += "/"; }
    }
    return new_path;
  }

  std::string GenPackageReference(const StructDef &struct_def) const {
    return namer_.NamespacedType(struct_def);
  }

  std::string GenPackageReference(const EnumDef &enum_def) const {
    return namer_.NamespacedType(enum_def);
  }

  std::string GenPackageReference(const Type &type) const {
    if (type.struct_def) {
      return GenPackageReference(*type.struct_def);
    } else if (type.enum_def) {
      return GenPackageReference(*type.enum_def);
    } else {
      return GenTypeGet(type);
    }
  }

  std::string GenPackageReference(const EnumVal &val) const {
    return GenPackageReference(val.union_type);
  }

  std::string GenPackageReference(const FieldDef &field) const {
    return GenPackageReference(field.value.type);
  }

  std::string ImportName(const EnumVal &val) {
    std::string package_name = GenPackageReference(val);
    std::replace(package_name.begin(), package_name.end(), '/', '_');
    return package_name;
  }

  std::string ImportName(const FieldDef &field) {
    std::string package_name = GenPackageReference(field);
    std::replace(package_name.begin(), package_name.end(), '/', '_');
    return package_name;
  }

  std::string ImportedName(const EnumDef &enum_def, const EnumVal &val) {
    return ImportName(val) + "." + namer_.Variant(val);
  }

  std::string ImportedName(const StructDef &struct_def, const FieldDef &field) {
    if (namer_.Type(struct_def) == TypeName(field)) {
      return TypeName(field);
    } else {
      return ImportName(field) + "." + TypeName(field);
    }
  }

  std::string GetImport(const EnumDef &enum_def, const EnumVal &val) {
    if (GenPackageReference(val) != GenPackageReference(enum_def)) {
      return "import " +
             GetRelativePathFromNamespace(GenPackageReference(enum_def),
                                          GenPackageReference(val)) +
             " as " + ImportName(val) + "\n";
    } else {
      return std::string();
    }
  }

  std::string GetImport(const StructDef &struct_def, const FieldDef &field) {
    if (GenPackageReference(field) != GenPackageReference(struct_def)) {
      return "import " +
             GetRelativePathFromNamespace(GenPackageReference(struct_def),
                                          GenPackageReference(field)) +
             " as " + ImportName(field) + "\n";
    } else {
      return std::string();
    }
  }

  void ImportOptions() { import_options_ = true; }

  // Create a struct with a builder and the struct's arguments.
  void GenStructBuilder(const StructDef &struct_def, std::string &code) {
    BeginBuilderArgs(struct_def, code);
    StructBuilderArgs(struct_def,
                      /* nameprefix = */ "",
                      /* namesuffix = */ "",
                      /* has_field_name = */ true,
                      /* with_type = */ true,
                      /* fieldname_suffix = */ "_", code);
    EndBuilderArgs(code);

    StructBuilderBody(struct_def, "", code);
    EndBuilderBody(code);
  }

  void GenUnionInit(const StructDef &struct_def, const FieldDef &field,
                    std::string &field_type) {
    field_type = TypeName(field);
    if (parser_.opts.include_dependence_headers) {
      imports_.insert(GetImport(struct_def, field));
      field_type = ImportedName(struct_def, field);
    }
    field_type += "T";
  }

  void GenStructInit(const StructDef &struct_def, const FieldDef &field,
                     std::string &output) {
    const Type &type = field.value.type;
    const std::string object_type = namer_.ObjectType(*type.struct_def);
    if (parser_.opts.include_dependence_headers) {
      imports_.insert(GetImport(struct_def, field));
      output = ImportedName(struct_def, field) + "T";
    } else {
      output = object_type;
    }
  }

  void GenVectorInit(const StructDef &struct_def, const FieldDef &field,
                     std::string &field_type) {
    const Type &vector_type = field.value.type.VectorType();
    const BaseType base_type = vector_type.base_type;
    if (base_type == BASE_TYPE_STRUCT) {
      const std::string object_type =
          namer_.ObjectType(*vector_type.struct_def);
      field_type = object_type;
      if (parser_.opts.include_dependence_headers) {
        imports_.insert(GetImport(struct_def, field));
        field_type = ImportedName(struct_def, field) + "T";
      }
      field_type = "seq[" + field_type + "]";
    } else if (base_type == BASE_TYPE_STRING) {
      field_type = "seq[string]";
    } else {
      field_type = GenTypeBasic(vector_type);
      if (IsEnum(vector_type)) {
        imports_.insert(GetImport(struct_def, field));
        field_type = ImportedName(struct_def, field);
      }
      field_type = "seq[" + field_type + "]";
    }
  }

  void GenInitialize(const StructDef &struct_def, std::string &code) {
    std::string object_fields_code;
    std::string init_fields_code;
    for (auto it = struct_def.fields.vec.begin();
         it != struct_def.fields.vec.end(); ++it) {
      auto &field = **it;
      if (field.deprecated) continue;

      // Determines field type, default value, and typing imports.
      auto base_type = field.value.type.base_type;
      std::string field_type;
      switch (base_type) {
        case BASE_TYPE_UNION: {
          GenUnionInit(struct_def, field, field_type);
          break;
        }
        case BASE_TYPE_STRUCT: {
          GenStructInit(struct_def, field, field_type);
          break;
        }
        case BASE_TYPE_VECTOR:
        case BASE_TYPE_ARRAY: {
          GenVectorInit(struct_def, field, field_type);
          break;
        }
        case BASE_TYPE_STRING: {
          field_type = "string";
          break;
        }
        default:
          if (IsEnum(field.value.type)) {
            imports_.insert(GetImport(struct_def, field));
            field_type = ImportedName(struct_def, field);
          } else {
            field_type = GenTypeBasic(field.value.type);
          }
          // Scalar or sting fields.
          break;
      }

      if ((field.IsOptional() || base_type == BASE_TYPE_STRUCT ||
           base_type == BASE_TYPE_UNION || base_type == BASE_TYPE_STRING) &&
          !struct_def.fixed && !(base_type == BASE_TYPE_VECTOR)) {
        ImportOptions();
        field_type = "Option[" + field_type + "]";
      }

      const auto field_field = namer_.Field(field);
      std::string default_value;
      if (field.IsScalarOptional()) {
        default_value = "default(type(result." + field_field + "))";
      } else if (IsBool(base_type)) {
        default_value = (field.value.constant == "0" ? "false" : "true");
      } else if (IsFloat(base_type) || IsInteger(base_type)) {
        default_value = field.value.constant;
      } else if (base_type == BASE_TYPE_STRING && field.IsDefault()) {
        default_value = "some(\"" + field.value.constant + "\")";
      } else {
        // For string, struct, and table.
        default_value = "default(type(result." + field_field + "))";
      }
      if (IsEnum(field.value.type)) {
        default_value = field_type + "(" + default_value + ")";
      }
      // Wrties the init statement.
      object_fields_code +=
          GenIndents(1) + field_field + Export + ": " + field_type;
      init_fields_code +=
          GenIndents(1) + "result." + field_field + " = " + default_value;
    }

    // Writes Init method.
    code += "type " + namer_.ObjectType(struct_def) + Export + " = ref object";
    code += object_fields_code;
    code += "\n";
    code += "\n";

    code += "proc new" + namer_.ObjectType(struct_def) + Export +
            "(): " + namer_.ObjectType(struct_def) + " = ";
    code += GenIndents(1) + "new(result)";

    if (init_fields_code.empty()) {
      code += GenIndents(1) + "discard\n";
    } else {
      code += init_fields_code;
      code += "\n";
    }
    code += "\n";

    // Merges the typing imports into import_list.
    // if (!import_typing_list.empty()) {
    //   // Adds the try statement.
    //   std::string typing_imports = "try:";
    //   typing_imports += GenIndents(1) + "from typing import ";
    //   std::string separator_string = ", ";
    //   for (auto it = import_typing_list.begin(); it !=
    //   import_typing_list.end();
    //        ++it) {
    //     const std::string &im = *it;
    //     typing_imports += im + separator_string;
    //   }
    //   // Removes the last separator_string.
    //   typing_imports.erase(typing_imports.length() -
    //   separator_string.size());

    //   // Adds the except statement.
    //   typing_imports += "\n";
    //   typing_imports += "except:";
    //   typing_imports += GenIndents(1) + "pass";
    //   import_list->insert(typing_imports);
    // }

    // Removes the import of the struct itself, if applied.
    // auto struct_import = "import " + namer_.NamespacedType(struct_def);
    // import_list->erase(struct_import);
  }

  void InitializeFromBuf(const StructDef &struct_def, std::string &code) const {
    const auto struct_var = namer_.Variable(struct_def);
    const auto struct_type = namer_.Type(struct_def);

    code += "proc InitFromBuf" + Export + "(self: var " +
            namer_.ObjectType(struct_def) + ", buf: seq[byte], pos: uoffset) =";
    code += GenIndents(1) + "var " + struct_var + ": " + struct_type;
    code += GenIndents(1) + struct_var + ".Init(buf, pos)";
    code += GenIndents(1) + "self.InitFromObj(" + struct_var + ")";
    code += "\n";
    code += "\n";
  }

  void InitializeFromObjForObject(const StructDef &struct_def,
                                  std::string &code) const {
    const auto struct_var = namer_.Variable(struct_def);
    const auto struct_object = namer_.ObjectType(struct_def);

    code += "proc InitFromObj" + Export + "(self: var " +
            namer_.ObjectType(struct_def) + ", " + struct_var + ": " +
            namer_.Type(struct_def) + ") =";
    code += GenIndents(1) + "self.UnPack(" + struct_var + ")";
    code += "\n";
    code += "\n";
  }

  void GenUnPackForStruct(const StructDef &struct_def, const FieldDef &field,
                          std::string &code) {
    const auto struct_var = namer_.Variable(struct_def);
    const auto field_field = namer_.Field(field);
    const auto field_var = namer_.Variable(field);
    auto field_type = TypeName(field);

    if (parser_.opts.include_dependence_headers) {
      imports_.insert(GetImport(struct_def, field));
      field_type = ImportedName(struct_def, field);
    }
    if (struct_def.fixed) {
      // Struct fields are not optional
      code += GenIndents(1) + "self." + field_field + ".InitFromObj(" +
              struct_var + "." + field_field + ")";
    } else {
      code +=
          GenIndents(1) + "if " + struct_var + "." + field_field + ".isSome:";
      code += GenIndents(2) + "var " + field_var + ": " + field_type + "T";
      code += GenIndents(2) + field_var + ".InitFromObj(" + struct_var + "." +
              field_field + ".get())";
      code +=
          GenIndents(2) + "self." + field_field + " = some(" + field_var + ")";
    }
    // A struct's accessor requires a struct buf instance.
    //         if (struct_def.fixed &&
    //             field.value.type.base_type == BASE_TYPE_STRUCT) {
    //   code += field_type + "()";
    // }
    // code += "))";
  }

  void GenUnPackForUnion(const StructDef &struct_def, const FieldDef &field,
                         std::string &code) {
    const auto field_field = namer_.Field(field);
    const auto field_method = namer_.Method(field);
    const auto struct_var = namer_.Variable(struct_def);
    // const EnumDef &enum_def = *field.value.type.enum_def;
    // auto union_type = namer_.Type(enum_def);

    // if (parser_.opts.include_dependence_headers) {
    // union_type = namer_.NamespacedType(enum_def) + "." + union_type;
    imports_.insert(GetImport(struct_def, field));
    const auto field_type = ImportedName(struct_def, field);
    // }
    code += GenIndents(1) + "if " + struct_var + "." + field_field + ".isSome:";
    code += GenIndents(2) + "self." + field_field + " = some(" + field_type +
            "Creator(" + struct_var + "." + field_field + "Type, " +
            struct_var + "." + field_field + ".get()))";
    // TODO: ^^^^^^
    // code += GenIndents(1) + "self." + field_field + " = " + union_type +
    //         "Creator" + "(" + "self." + field_field + "Type, " + struct_var +
    //         "." + field_method + "())";
  }

  void GenUnPackForStructVector(const StructDef &struct_def,
                                const FieldDef &field, std::string &code) {
    const auto field_field = namer_.Field(field);
    const auto field_var = namer_.Variable(field);
    const auto struct_var = namer_.Variable(struct_def);
    auto field_type = TypeName(field);

    if (parser_.opts.include_dependence_headers) {
      imports_.insert(GetImport(struct_def, field));
      field_type = ImportedName(struct_def, field);
    }

    code += GenIndents(1) + "for " + field_var + " in " + struct_var + "." +
            field_field + ":";
    code += GenIndents(2) + "var " + field_var + "t: " + field_type + "T";
    code += GenIndents(2) + field_var + "t.InitFromObj(" + field_var + ")";
    code += GenIndents(2) + "self." + field_field + ".add(" + field_var + "t)";
  }

  void GenUnPackForScalarVector(const StructDef &struct_def,
                                const FieldDef &field, std::string &code) {
    const auto field_field = namer_.Field(field);
    const auto struct_var = namer_.Variable(struct_def);

    code += GenIndents(1) + "self." + field_field + " = " + struct_var + "." +
            field_field;
  }

  void GenUnPackForScalar(const StructDef &struct_def, const FieldDef &field,
                          std::string &code) const {
    const auto field_field = namer_.Field(field);
    const auto struct_var = namer_.Variable(struct_def);

    code += GenIndents(1) + "self." + field_field + " = " + struct_var + "." +
            field_field;
  }

  // Generates the UnPack method for the object class.
  void GenUnPack(const StructDef &struct_def, std::string &code_base) {
    std::string code;
    // Items that needs to be imported. No duplicate modules will be imported.
    std::set<std::string> import_list;

    for (auto it = struct_def.fields.vec.begin();
         it != struct_def.fields.vec.end(); ++it) {
      auto &field = **it;
      if (field.deprecated) continue;

      auto field_type = TypeName(field);
      switch (field.value.type.base_type) {
        case BASE_TYPE_STRUCT: {
          GenUnPackForStruct(struct_def, field, code);
          break;
        }
        case BASE_TYPE_UNION: {
          GenUnPackForUnion(struct_def, field, code);
          break;
        }
        case BASE_TYPE_VECTOR: {
          auto vectortype = field.value.type.VectorType();
          if (vectortype.base_type == BASE_TYPE_STRUCT) {
            GenUnPackForStructVector(struct_def, field, code);
          } else {
            GenUnPackForScalarVector(struct_def, field, code);
          }
          break;
        }
        case BASE_TYPE_ARRAY: {
          GenUnPackForScalarVector(struct_def, field, code);
          break;
        }
        default: GenUnPackForScalar(struct_def, field, code);
      }
    }

    // Writes import statements and code into the generated file.
    const auto struct_var = namer_.Variable(struct_def);

    code_base += "proc UnPack" + Export + "(self: var " +
                 namer_.ObjectType(struct_def) + ", " + struct_var + ": " +
                 namer_.Type(struct_def) + ") =";
    if (code.empty()) { code += GenIndents(1) + "discard"; }
    // Write the code.
    code_base += code;
    code_base += "\n";
    code_base += "\n";
  }

  void GenPackForStruct(const StructDef &struct_def, std::string &code) {
    code += "proc Pack" + Export + "(self: " + namer_.ObjectType(struct_def) +
            ", builder: var Builder): uoffset =";
    code += GenIndents(1) + "result = builder.Create" +
            namer_.Type(struct_def) + "(";

    StructBuilderArgs(struct_def,
                      /* nameprefix = */ "self.",
                      /* namesuffix = */ "",
                      /* has_field_name = */ true,
                      /* with_type = */ false,
                      /* fieldname_suffix = */ ".", code);
    code += ")\n";
    code += "\n";
  }

  void GenPackForStructVectorField(const StructDef &struct_def,
                                   const FieldDef &field,
                                   std::string &code_prefix,
                                   std::string &code) {
    const auto field_field = namer_.Field(field);
    const auto struct_type = namer_.Type(struct_def);
    const auto field_method = namer_.Method(field);

    // Creates the field.
    code_prefix += GenIndents(1) + "var " + field_field + ": uoffset";
    code_prefix += GenIndents(1) + "if self." + field_field + ".len > 0:";
    if (field.value.type.struct_def->fixed) {
      code_prefix += GenIndents(2) + "discard builder." + struct_type +
                     "Start" + field_method + "Vector(self." + field_field +
                     ".len.uoffset)";
      code_prefix += GenIndents(2) + "for i in countdown(self." + field_field +
                     ".len - 1, 0):";
      code_prefix +=
          GenIndents(3) + "discard self." + field_field + "[i].Pack(builder)";
    } else {
      // If the vector is a struct vector, we need to first build accessor for
      // each struct element.
      code_prefix +=
          GenIndents(2) + "var " + field_field + "list: seq[uoffset] = @[]";
      code_prefix += GenIndents(2) + "for i in countup(0, self." + field_field +
                     ".len - 1):";
      code_prefix += GenIndents(3) + field_field + "list.add(self." +
                     field_field + "[i].Pack(builder))";

      code_prefix += GenIndents(2) + "discard builder." + struct_type +
                     "Start" + field_method + "Vector(self." + field_field +
                     ".len.uoffset)";
      code_prefix += GenIndents(2) + "for i in countdown(" + field_field +
                     "list.len - 1, 0):";
      code_prefix += GenIndents(3) + "builder.PrependOffsetRelative" + "(" +
                     field_field + "list[i])";
    }
    code_prefix += GenIndents(2) + field_field + " = builder.EndVector()";

    // Adds the field into the struct.
    code += GenIndents(1) + "if self." + field_field + ".len > 0:";
    code += GenIndents(2) + "builder." + struct_type + "Add" + field_method +
            "(" + field_field + ")";
  }

  void GenPackForScalarVectorFieldHelper(const StructDef &struct_def,
                                         const FieldDef &field,
                                         std::string &code, int indents) const {
    const auto field_field = namer_.Field(field);
    const auto field_method = namer_.Method(field);
    const auto struct_type = namer_.Type(struct_def);
    const auto vectortype = field.value.type.VectorType();

    code += GenIndents(indents) + "discard builder." + struct_type + "Start" +
            field_method + "Vector(self." + field_field + ".len.uoffset)";
    code += GenIndents(indents) + "for i in countdown(self." + field_field +
            ".len - 1, 0):";
    code += GenIndents(indents + 1) + "builder.Prepend";

    std::string type_name;
    switch (vectortype.base_type) {
      case BASE_TYPE_BOOL:
      case BASE_TYPE_CHAR:
      case BASE_TYPE_UCHAR:
      case BASE_TYPE_SHORT:
      case BASE_TYPE_USHORT:
      case BASE_TYPE_INT:
      case BASE_TYPE_UINT:
      case BASE_TYPE_LONG:
      case BASE_TYPE_ULONG:
      case BASE_TYPE_FLOAT:
      case BASE_TYPE_DOUBLE: break;
      case BASE_TYPE_STRING: type_name = "OffsetRelative"; break;
      default: type_name = "OffsetRelative"; break;
    }
    code += type_name;
  }

  void GenPackForScalarVectorField(const StructDef &struct_def,
                                   const FieldDef &field,
                                   std::string &code_prefix,
                                   std::string &code) const {
    const auto field_field = namer_.Field(field);
    const auto field_method = namer_.Method(field);
    const auto struct_type = namer_.Type(struct_def);

    // Adds the field into the struct.
    code += GenIndents(1) + "if self." + field_field + ".len > 0:";
    code += GenIndents(2) + "builder." + struct_type + "Add" + field_method +
            "(" + field_field + ")";

    // Creates the field.
    code_prefix += GenIndents(1) + "var " + field_field + ": uoffset ";
    code_prefix += GenIndents(1) + "if self." + field_field + ".len > 0:";
    // If the vector is a string vector, we need to first build accessor for
    // each string element. And this generated code, needs to be
    // placed ahead of code_prefix.
    auto vectortype = field.value.type.VectorType();
    if (IsString(vectortype)) {
      code_prefix +=
          GenIndents(2) + "var " + field_field + "list: seq[uoffset] = @[]";
      code_prefix +=
          GenIndents(2) + "for i in countup(0, self." + field_field + ".len):";
      code_prefix += GenIndents(3) + field_field +
                     "list.add(builder.Create(self." + field_field + "[i]))";
      GenPackForScalarVectorFieldHelper(struct_def, field, code_prefix, 2);
      code_prefix += "(" + field_field + "list[i])";
      code_prefix += GenIndents(2) + field_field + " = builder.EndVector()";
    } else {
      GenPackForScalarVectorFieldHelper(struct_def, field, code_prefix, 2);
      code_prefix += "(self." + field_field + "[i])";
      code_prefix += GenIndents(2) + field_field + " = builder.EndVector()";
    }
  }

  void GenPackForStructField(const StructDef &struct_def, const FieldDef &field,
                             std::string &code_prefix, std::string &code) {
    const auto field_field = namer_.Field(field);
    const auto field_method = namer_.Method(field);
    const auto struct_type = namer_.Type(struct_def);

    // if (field.value.type.struct_def->fixed) {
    //   // Pure struct fields need to be created along with their parent
    //   // structs.
    //   code += GenIndents(1) + "var " + field_field + ": uoffset";
    //   code += GenIndents(1) + "if self." + field_field + ".isSome:";
    //   code += GenIndents(2) + field_field + " = self." + field_field +
    //           ".get().Pack(builder)";
    // } else {
    // Tables need to be created before their parent structs are created.
    code_prefix += GenIndents(1) + "var " + field_field + ": uoffset";
    code_prefix += GenIndents(1) + "if self." + field_field + ".isSome:";
    code_prefix += GenIndents(2) + field_field + " = self." + field_field +
                   ".get().Pack(builder)";
    code += GenIndents(1) + "if self." + field_field + ".isSome:";
    // }

    code += GenIndents(2) + "builder." + struct_type + "Add" + field_method +
            "(" + field_field + ")";
  }

  void GenPackForUnionField(const StructDef &struct_def, const FieldDef &field,
                            std::string &code_prefix, std::string &code) {
    const auto field_field = namer_.Field(field);
    const auto field_method = namer_.Method(field);
    const auto struct_type = namer_.Type(struct_def);

    // TODO(luwa): TypeT should be moved under the None check as well.
    code_prefix += GenIndents(1) + "var " + field_field + ": uoffset";
    code_prefix += GenIndents(1) + "if self." + field_field + ".isSome:";
    code_prefix += GenIndents(2) + field_field + " = self." + field_field +
                   ".get().Pack(self." + field_field + "Type, builder)";
    code += GenIndents(1) + "if self." + field_field + ".isSome:";
    code += GenIndents(2) + "builder." + struct_type + "Add" + field_method +
            "(" + field_field + ")";
  }

  void GenPackForTable(const StructDef &struct_def, std::string &code_base) {
    std::string code, code_prefix;
    const auto struct_type = namer_.Type(struct_def);

    code_base += "proc Pack" + Export +
                 "(self: " + namer_.ObjectType(struct_def) +
                 ", builder: var Builder): uoffset =";
    code += GenIndents(1) + "builder." + struct_type + "Start()";
    for (auto it = struct_def.fields.vec.begin();
         it != struct_def.fields.vec.end(); ++it) {
      auto &field = **it;
      if (field.deprecated) continue;

      const auto field_method = namer_.Method(field);
      const auto field_field = namer_.Field(field);

      switch (field.value.type.base_type) {
        case BASE_TYPE_STRUCT: {
          GenPackForStructField(struct_def, field, code_prefix, code);
          break;
        }
        case BASE_TYPE_UNION: {
          GenPackForUnionField(struct_def, field, code_prefix, code);
          break;
        }
        case BASE_TYPE_VECTOR: {
          auto vectortype = field.value.type.VectorType();
          if (vectortype.base_type == BASE_TYPE_STRUCT) {
            GenPackForStructVectorField(struct_def, field, code_prefix, code);
          } else {
            GenPackForScalarVectorField(struct_def, field, code_prefix, code);
          }
          break;
        }
        case BASE_TYPE_ARRAY: {
          GenPackForScalarVectorField(struct_def, field, code_prefix, code);
          break;
        }
        case BASE_TYPE_STRING: {
          code_prefix += GenIndents(1) + "var " + field_field + ": uoffset";
          code_prefix += GenIndents(1) + "if self." + field_field + ".isSome:";
          code_prefix += GenIndents(2) + field_field +
                         " = builder.Create(self." + field_field + ".get())";
          code += GenIndents(1) + "if self." + field_field + ".isSome:";
          code += GenIndents(2) + "builder." + struct_type + "Add" +
                  field_method + "(" + field_field + ")";
          break;
        }
        default:
          // Generates code for scalar values. If the value equals to the
          // default value, builder will automatically ignore it. So we don't
          // need to check the value ahead.
          code += GenIndents(1) + "builder." + struct_type + "Add" +
                  field_method + "(self." + field_field + ")";
          break;
      }
    }

    code += GenIndents(1) + "result = builder." + struct_type + "End()";

    code_base += code_prefix + code;
    code_base += "\n";
    code_base += "\n";
  }
  void GenForwardDeclarations(const StructDef &struct_def, std::string &code) {
    std::string forward_declare_comment = " # forward declaration\n";
    std::string type = namer_.Type(struct_def);
    std::string object_type = namer_.ObjectType(struct_def);
    std::string struct_var = namer_.Variable(struct_def);

    code += "proc Pack" + Export + "(self: " + object_type +
            ", builder: var Builder): uoffset" + forward_declare_comment;
    code += "proc UnPack" + Export + "(self: var " + object_type + ", " +
            struct_var + ": " + type + ")" + forward_declare_comment;
    code += "proc InitFromObj" + Export + "(self: var " + object_type + ", " +
            struct_var + ": " + type + ")" + forward_declare_comment;
    code += "proc InitFromBuf" + Export + "(self: var " + object_type +
            ", buf: seq[byte], pos: uoffset)" + forward_declare_comment;
    code += "\n";
  }

  void GenStructForObjectAPI(const StructDef &struct_def,
                             std::string &code_base) {
    std::string code;
    // Creates an object class for a struct or a table
    GenInitialize(struct_def, code);

    GenForwardDeclarations(struct_def, code);
    if (struct_def.fixed) {
      GenPackForStruct(struct_def, code);
    } else {
      GenPackForTable(struct_def, code);
    }

    GenUnPack(struct_def, code);

    InitializeFromObjForObject(struct_def, code);

    InitializeFromBuf(struct_def, code);

    // Adds the imports at top.
    code_base += code;
  }

  void GenUnionCreatorForStruct(const EnumDef &enum_def, const EnumVal &ev,
                                std::string &object_code,
                                std::string &creator_code,
                                std::string &pack_code) {
    const std::string variant = namer_.Variant(ev);
    const std::string field_var = namer_.Variable(*ev.union_type.struct_def);
    const std::string enum_objectType = namer_.ObjectType(enum_def);
    std::string field_type = namer_.ObjectType(*ev.union_type.struct_def);
    if (parser_.opts.include_dependence_headers) {
      imports_.insert(GetImport(enum_def, ev));
      field_type = ImportName(ev) + "." + field_type;
    }
    object_code += GenIndents(1) + "as" + variant + Export + ": " + field_type;

    creator_code +=
        GenIndents(1) + "of " + namer_.EnumVariant(enum_def, ev) + ":";
    creator_code += GenIndents(2) + "result.as" + variant +
                    ".InitFromBuf(table.Bytes, table.Pos)";

    pack_code += GenIndents(1) + "of " + namer_.EnumVariant(enum_def, ev) + ":";
    pack_code +=
        GenIndents(2) + "result = self.as" + variant + ".Pack(builder)";
  }

  void GenUnionCreatorForString(const EnumDef &enum_def, const EnumVal &ev,
                                std::string &object_code,
                                std::string &creator_code,
                                std::string &pack_code) {
    const auto union_type = namer_.Type(enum_def);
    const auto variant = namer_.Variant(ev);
    object_code += GenIndents(1) + "as" + variant + Export + ": string";

    creator_code +=
        GenIndents(1) + "of " + namer_.EnumVariant(enum_def, ev) + ":";
    creator_code +=
        GenIndents(2) + "result.as" + variant + " = table.String(table.Pos)";

    pack_code += GenIndents(1) + "of " + namer_.EnumVariant(enum_def, ev) + ":";
    pack_code +=
        GenIndents(2) + "result = builder.Create(self.as" + variant + ")";
  }

  // Creates an union object based on union type.
  void GenUnionCreator(const EnumDef &enum_def, std::string &code) {
    if (enum_def.generated) return;
    std::string object_code;
    std::string creator_code;
    std::string pack_code;

    const auto enum_fn = namer_.Function(enum_def);
    const auto enum_type = namer_.Type(enum_def);
    const auto enum_objectType = namer_.ObjectType(enum_def);

    object_code += "\n";
    object_code +=
        "type " + enum_objectType + Export + " {.union.} = ref object";

    creator_code += "proc " + enum_type + "Creator" + Export +
                    "(unionType: " + enum_type +
                    ", table: Vtable): " + enum_objectType + " =";
    creator_code += GenIndents(1) + "case unionType";

    pack_code += "proc Pack" + Export + "(self: " + enum_objectType +
                 ", unionType: " + enum_type +
                 ", builder: var Builder): uoffset =";
    pack_code += GenIndents(1) + "case unionType";

    for (auto it = enum_def.Vals().begin(); it != enum_def.Vals().end(); ++it) {
      auto &ev = **it;
      switch (ev.union_type.base_type) {
        case BASE_TYPE_STRUCT:
          GenUnionCreatorForStruct(enum_def, ev, object_code, creator_code,
                                   pack_code);
          break;
        case BASE_TYPE_STRING:
          GenUnionCreatorForString(enum_def, ev, object_code, creator_code,
                                   pack_code);
          break;
        default: break;
      }
    }
    creator_code += GenIndents(1) + "else:";
    creator_code += GenIndents(2) + "discard";
    pack_code += GenIndents(1) + "else:";
    pack_code += GenIndents(2) + "discard";
    code += object_code + "\n\n";
    code += creator_code + "\n\n";
    code += pack_code + "\n\n";
  }

  bool generate() {
    std::string one_file_code;
    if (!generateEnums(one_file_code)) return false;
    if (!generateStructs(one_file_code)) return false;
    if (parser_.opts.one_file) {
      // Legacy file format uses keep casing.
      return SaveType(file_name_ + "_generated.py", *parser_.current_namespace_,
                      one_file_code, true);
    }
    return true;
  }

 private:
  bool generateEnums(std::string &one_file_code) {
    for (auto it = parser_.enums_.vec.begin(); it != parser_.enums_.vec.end();
         ++it) {
      bool needs_imports = false;
      auto &enum_def = **it;
      std::string enumcode;
      GenEnum(enum_def, enumcode);
      if (parser_.opts.generate_object_based_api & enum_def.is_union) {
        GenUnionCreator(enum_def, enumcode);
        needs_imports = true;
      }

      if (parser_.opts.one_file && !enumcode.empty()) {
        one_file_code += enumcode + "\n\n";
      } else {
        if (!SaveType(namer_.File(enum_def, SkipFile::Suffix),
                      *enum_def.defined_namespace, enumcode, needs_imports))
          return false;
      }
    }
    return true;
  }

  bool generateStructs(std::string &one_file_code) {
    for (auto it = parser_.structs_.vec.begin();
         it != parser_.structs_.vec.end(); ++it) {
      auto &struct_def = **it;
      std::string declcode;
      GenStruct(struct_def, declcode);
      if (parser_.opts.generate_object_based_api && !struct_def.generated) {
        GenStructForObjectAPI(struct_def, declcode);
      }

      if (parser_.opts.one_file && !declcode.empty()) {
        one_file_code += declcode + "\n\n";
      } else {
        if (!SaveType(namer_.File(struct_def, SkipFile::Suffix),
                      *struct_def.defined_namespace, declcode, true))
          return false;
      }
    }
    return true;
  }

  // Begin by declaring namespace and imports.
  void BeginFile(const std::string &name_space_name, const bool needs_imports,
                 std::string &code) {
    code += "{.warning[HoleEnumConv]: off.}\n";
    code += Comment + FlatBuffersGeneratedWarning() + "\n\n";
    code += Comment + "namespace: " + name_space_name + "\n\n";
    if (import_options_) {
      code += "import std/options\n";
      import_options_ = false;
    }
    if (needs_imports) { code += "import flatbuffers\n"; }
    code += "\n";
    for (std::string import : imports_) { code += import; }
    imports_.clear();
    code += "\n";
  }

  // Save out the generated code for a Table type.
  bool SaveType(const std::string &defname, const Namespace &ns,
                const std::string &classcode, bool needs_imports) {
    if (!classcode.length()) return true;

    std::string code = "";
    BeginFile(LastNamespacePart(ns), needs_imports, code);
    code += classcode;

    const std::string directories =
        parser_.opts.one_file ? path_ : namer_.Directories(ns.components);
    EnsureDirExists(directories);

    const std::string filename = directories + defname;
    return SaveFile(filename.c_str(), code, false);
  }

 private:
  const IdlNamer namer_;
  std::unordered_set<std::string> imports_;
  bool import_options_;
};

}  // namespace nim

bool GenerateNim(const Parser &parser, const std::string &path,
                 const std::string &file_name) {
  nim::NimGenerator generator(parser, path, file_name);
  return generator.generate();
}

}  // namespace flatbuffers
