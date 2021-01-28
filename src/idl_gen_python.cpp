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

#include <cctype>
#include <set>
#include <string>
#include <unordered_set>
#include <vector>

#include "flatbuffers/code_generators.h"
#include "flatbuffers/flatbuffers.h"
#include "flatbuffers/idl.h"
#include "flatbuffers/util.h"

namespace flatbuffers {
namespace python {

// Hardcode spaces per indentation.
const CommentConfig def_comment = { nullptr, "#", nullptr };
const std::string Indent = "    ";

class PythonGenerator : public BaseGenerator {
 public:
  PythonGenerator(const Parser &parser, const std::string &path,
                  const std::string &file_name)
      : BaseGenerator(parser, path, file_name, "" /* not used */,
                      "" /* not used */, "py"),
        float_const_gen_("float('nan')", "float('inf')", "float('-inf')") {
    static const char *const keywords[] = {
      "False",   "None",     "True",     "and",    "as",   "assert", "break",
      "class",   "continue", "def",      "del",    "elif", "else",   "except",
      "finally", "for",      "from",     "global", "if",   "import", "in",
      "is",      "lambda",   "nonlocal", "not",    "or",   "pass",   "raise",
      "return",  "try",      "while",    "with",   "yield"
    };
    keywords_.insert(std::begin(keywords), std::end(keywords));
  }

  // Most field accessors need to retrieve and test the field offset first,
  // this is the prefix code for that.
  std::string OffsetPrefix(const FieldDef &field) {
    return "\n" + Indent + Indent +
           "o = flatbuffers.number_types.UOffsetTFlags.py_type" +
           "(self._tab.Offset(" + NumToString(field.value.offset) + "))\n" +
           Indent + Indent + "if o != 0:\n";
  }

  // Begin a class declaration.
  void BeginClass(const StructDef &struct_def, std::string *code_ptr) {
    auto &code = *code_ptr;
    code += "class " + NormalizedName(struct_def) + "(object):\n";
    code += Indent + "__slots__ = ['_tab']";
    code += "\n\n";
  }

  // Begin enum code with a class declaration.
  void BeginEnum(const std::string &class_name, std::string *code_ptr) {
    auto &code = *code_ptr;
    code += "class " + class_name + "(object):\n";
  }

  std::string EscapeKeyword(const std::string &name) const {
    return keywords_.find(name) == keywords_.end() ? name : name + "_";
  }

  std::string NormalizedName(const Definition &definition) const {
    return EscapeKeyword(definition.name);
  }

  std::string NormalizedName(const EnumVal &ev) const {
    return EscapeKeyword(ev.name);
  }

  // Converts the name of a definition into upper Camel format.
  std::string MakeUpperCamel(const Definition &definition) const {
    return MakeCamel(NormalizedName(definition), true);
  }

  // Converts the name of a definition into lower Camel format.
  std::string MakeLowerCamel(const Definition &definition) const {
    auto name = MakeCamel(NormalizedName(definition), false);
    name[0] = CharToLower(name[0]);
    return name;
  }

  // Starts a new line and then indents.
  std::string GenIndents(int num) {
    return "\n" + std::string(num * Indent.length(), ' ');
  }

  // A single enum member.
  void EnumMember(const EnumDef &enum_def, const EnumVal &ev,
                  std::string *code_ptr) {
    auto &code = *code_ptr;
    code += Indent;
    code += NormalizedName(ev);
    code += " = ";
    code += enum_def.ToString(ev) + "\n";
  }

  // End enum code.
  void EndEnum(std::string *code_ptr) {
    auto &code = *code_ptr;
    code += "\n";
  }

  // Initialize a new struct or table from existing data.
  void NewRootTypeFromBuffer(const StructDef &struct_def,
                             std::string *code_ptr) {
    auto &code = *code_ptr;

    code += Indent + "@classmethod\n";
    code += Indent + "def GetRootAs";
    code += "(cls, buf, offset=0):";
    code += "\n";
    code += Indent + Indent;
    code += "n = flatbuffers.encode.Get";
    code += "(flatbuffers.packer.uoffset, buf, offset)\n";
    code += Indent + Indent + "x = " + NormalizedName(struct_def) + "()\n";
    code += Indent + Indent + "x.Init(buf, n + offset)\n";
    code += Indent + Indent + "return x\n";
    code += "\n";

    // Add an alias with the old name
    code += Indent + "@classmethod\n";
    code += Indent + "def GetRootAs";
    code += NormalizedName(struct_def);
    code += "(cls, buf, offset=0):\n";
    code += Indent + Indent + "\"\"\"This method is deprecated. Please switch to GetRootAs.\"\"\"\n";
    code += Indent + Indent + "return cls.GetRootAs(buf, offset)\n";
  }

  // Initialize an existing object with other data, to avoid an allocation.
  void InitializeExisting(const StructDef &struct_def, std::string *code_ptr) {
    auto &code = *code_ptr;

    GenReceiver(struct_def, code_ptr);
    code += "Init(self, buf, pos):\n";
    code += Indent + Indent + "self._tab = flatbuffers.table.Table(buf, pos)\n";
    code += "\n";
  }

  // Get the length of a vector.
  void GetVectorLen(const StructDef &struct_def, const FieldDef &field,
                    std::string *code_ptr) {
    auto &code = *code_ptr;

    GenReceiver(struct_def, code_ptr);
    code += MakeCamel(NormalizedName(field)) + "Length(self";
    code += "):" + OffsetPrefix(field);
    code += Indent + Indent + Indent + "return self._tab.VectorLen(o)\n";
    code += Indent + Indent + "return 0\n\n";
  }

  // Determines whether a vector is none or not.
  void GetVectorIsNone(const StructDef &struct_def, const FieldDef &field,
                       std::string *code_ptr) {
    auto &code = *code_ptr;

    GenReceiver(struct_def, code_ptr);
    code += MakeCamel(NormalizedName(field)) + "IsNone(self";
    code += "):";
    code += GenIndents(2) +
            "o = flatbuffers.number_types.UOffsetTFlags.py_type" +
            "(self._tab.Offset(" + NumToString(field.value.offset) + "))";
    code += GenIndents(2) + "return o == 0";
    code += "\n\n";
  }

  // Get the value of a struct's scalar.
  void GetScalarFieldOfStruct(const StructDef &struct_def,
                              const FieldDef &field, std::string *code_ptr) {
    auto &code = *code_ptr;
    std::string getter = GenGetter(field.value.type);
    GenReceiver(struct_def, code_ptr);
    code += MakeCamel(NormalizedName(field));
    code += "(self): return " + getter;
    code += "self._tab.Pos + flatbuffers.number_types.UOffsetTFlags.py_type(";
    code += NumToString(field.value.offset) + "))\n";
  }

  // Get the value of a table's scalar.
  void GetScalarFieldOfTable(const StructDef &struct_def, const FieldDef &field,
                             std::string *code_ptr) {
    auto &code = *code_ptr;
    std::string getter = GenGetter(field.value.type);
    GenReceiver(struct_def, code_ptr);
    code += MakeCamel(NormalizedName(field));
    code += "(self):";
    code += OffsetPrefix(field);
    getter += "o + self._tab.Pos)";
    auto is_bool = IsBool(field.value.type.base_type);
    if (is_bool) { getter = "bool(" + getter + ")"; }
    code += Indent + Indent + Indent + "return " + getter + "\n";
    std::string default_value;
    if (is_bool) {
      default_value = field.value.constant == "0" ? "False" : "True";
    } else {
      default_value = IsFloat(field.value.type.base_type)
                          ? float_const_gen_.GenFloatConstant(field)
                          : field.value.constant;
    }
    code += Indent + Indent + "return " + default_value + "\n\n";
  }

  // Get a struct by initializing an existing struct.
  // Specific to Struct.
  void GetStructFieldOfStruct(const StructDef &struct_def,
                              const FieldDef &field, std::string *code_ptr) {
    auto &code = *code_ptr;
    GenReceiver(struct_def, code_ptr);
    code += MakeCamel(NormalizedName(field));
    code += "(self, obj):\n";
    code += Indent + Indent + "obj.Init(self._tab.Bytes, self._tab.Pos + ";
    code += NumToString(field.value.offset) + ")";
    code += "\n" + Indent + Indent + "return obj\n\n";
  }

  // Get the value of a fixed size array.
  void GetArrayOfStruct(const StructDef &struct_def, const FieldDef &field,
                        std::string *code_ptr) {
    auto &code = *code_ptr;
    const auto vec_type = field.value.type.VectorType();
    GenReceiver(struct_def, code_ptr);
    code += MakeCamel(NormalizedName(field));
    if (IsStruct(vec_type)) {
      code += "(self, obj, i):\n";
      code += Indent + Indent + "obj.Init(self._tab.Bytes, self._tab.Pos + ";
      code += NumToString(field.value.offset) + " + i * ";
      code += NumToString(InlineSize(vec_type));
      code += ")\n" + Indent + Indent + "return obj\n\n";
    } else {
      auto getter = GenGetter(vec_type);
      code += "(self): return [" + getter;
      code += "self._tab.Pos + flatbuffers.number_types.UOffsetTFlags.py_type(";
      code += NumToString(field.value.offset) + " + i * ";
      code += NumToString(InlineSize(vec_type));
      code += ")) for i in range(";
      code += NumToString(field.value.type.fixed_length) + ")]\n";
    }
  }

  // Get a struct by initializing an existing struct.
  // Specific to Table.
  void GetStructFieldOfTable(const StructDef &struct_def, const FieldDef &field,
                             std::string *code_ptr) {
    auto &code = *code_ptr;
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
    if (parser_.opts.include_dependence_headers) {
      code += Indent + Indent + Indent;
      code += "from " + GenPackageReference(field.value.type) + " import " +
              TypeName(field) + "\n";
    }
    code += Indent + Indent + Indent + "obj = " + TypeName(field) + "()\n";
    code += Indent + Indent + Indent + "obj.Init(self._tab.Bytes, x)\n";
    code += Indent + Indent + Indent + "return obj\n";
    code += Indent + Indent + "return None\n\n";
  }

  // Get the value of a string.
  void GetStringField(const StructDef &struct_def, const FieldDef &field,
                      std::string *code_ptr) {
    auto &code = *code_ptr;
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
    auto &code = *code_ptr;
    GenReceiver(struct_def, code_ptr);
    code += MakeCamel(NormalizedName(field)) + "(self):";
    code += OffsetPrefix(field);

    // TODO(rw): this works and is not the good way to it:
    bool is_native_table = TypeName(field) == "*flatbuffers.Table";
    if (is_native_table) {
      code +=
          Indent + Indent + Indent + "from flatbuffers.table import Table\n";
    } else if (parser_.opts.include_dependence_headers) {
      code += Indent + Indent + Indent;
      code += "from " + GenPackageReference(field.value.type) + " import " +
              TypeName(field) + "\n";
    }
    code += Indent + Indent + Indent + "obj = Table(bytearray(), 0)\n";
    code += Indent + Indent + Indent + GenGetter(field.value.type);
    code += "obj, o)\n" + Indent + Indent + Indent + "return obj\n";
    code += Indent + Indent + "return None\n\n";
  }

  // Generate the package reference when importing a struct or enum from its
  // module.
  std::string GenPackageReference(const Type &type) {
    Namespace *namespaces;
    if (type.struct_def) {
      namespaces = type.struct_def->defined_namespace;
    } else if (type.enum_def) {
      namespaces = type.enum_def->defined_namespace;
    } else {
      return "." + GenTypeGet(type);
    }

    return namespaces->GetFullyQualifiedName(GenTypeGet(type));
  }

  // Get the value of a vector's struct member.
  void GetMemberOfVectorOfStruct(const StructDef &struct_def,
                                 const FieldDef &field, std::string *code_ptr) {
    auto &code = *code_ptr;
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
    if (parser_.opts.include_dependence_headers) {
      code += Indent + Indent + Indent;
      code += "from " + GenPackageReference(field.value.type) + " import " +
              TypeName(field) + "\n";
    }
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
    auto &code = *code_ptr;
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
    if (IsString(vectortype)) {
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
    auto &code = *code_ptr;
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

    if (IsString(vectortype)) {
      code += Indent + Indent + "return \"\"\n";
    } else {
      code += Indent + Indent + "return 0\n";
    }
    code += "\n";
  }

  // Returns a nested flatbuffer as itself.
  void GetVectorAsNestedFlatbuffer(const StructDef &struct_def,
                                   const FieldDef &field,
                                   std::string *code_ptr) {
    auto nested = field.attributes.Lookup("nested_flatbuffer");
    if (!nested) { return; }  // There is no nested flatbuffer.

    std::string unqualified_name = nested->constant;
    std::string qualified_name = nested->constant;
    auto nested_root = parser_.LookupStruct(nested->constant);
    if (nested_root == nullptr) {
      qualified_name =
          parser_.current_namespace_->GetFullyQualifiedName(nested->constant);
      nested_root = parser_.LookupStruct(qualified_name);
    }
    FLATBUFFERS_ASSERT(nested_root);  // Guaranteed to exist by parser.
    (void)nested_root;

    auto &code = *code_ptr;
    GenReceiver(struct_def, code_ptr);
    code += MakeCamel(NormalizedName(field)) + "NestedRoot(self):";

    code += OffsetPrefix(field);

    code += Indent + Indent + Indent;
    code += "from " + qualified_name + " import " + unqualified_name + "\n";
    code += Indent + Indent + Indent + "return " + unqualified_name;
    code += ".GetRootAs";
    code += "(self._tab.Bytes, self._tab.Vector(o))\n";
    code += Indent + Indent + "return 0\n";
    code += "\n";
  }

  // Begin the creator function signature.
  void BeginBuilderArgs(const StructDef &struct_def, std::string *code_ptr) {
    auto &code = *code_ptr;

    code += "\n";
    code += "def Create" + NormalizedName(struct_def);
    code += "(builder";
  }

  // Recursively generate arguments for a constructor, to deal with nested
  // structs.
  void StructBuilderArgs(const StructDef &struct_def,
                         const std::string nameprefix,
                         const std::string namesuffix, bool has_field_name,
                         const std::string fieldname_suffix,
                         std::string *code_ptr) {
    for (auto it = struct_def.fields.vec.begin();
         it != struct_def.fields.vec.end(); ++it) {
      auto &field = **it;
      const auto &field_type = field.value.type;
      const auto &type =
          IsArray(field_type) ? field_type.VectorType() : field_type;
      if (IsStruct(type)) {
        // Generate arguments for a struct inside a struct. To ensure names
        // don't clash, and to make it obvious these arguments are constructing
        // a nested struct, prefix the name with the field name.
        auto subprefix = nameprefix;
        if (has_field_name) {
          subprefix += NormalizedName(field) + fieldname_suffix;
        }
        StructBuilderArgs(*field.value.type.struct_def, subprefix, namesuffix,
                          has_field_name, fieldname_suffix, code_ptr);
      } else {
        auto &code = *code_ptr;
        code += std::string(", ") + nameprefix;
        if (has_field_name) { code += MakeCamel(NormalizedName(field), false); }
        code += namesuffix;
      }
    }
  }

  // End the creator function signature.
  void EndBuilderArgs(std::string *code_ptr) {
    auto &code = *code_ptr;
    code += "):\n";
  }

  // Recursively generate struct construction statements and instert manual
  // padding.
  void StructBuilderBody(const StructDef &struct_def, const char *nameprefix,
                         std::string *code_ptr, size_t index = 0,
                         bool in_array = false) {
    auto &code = *code_ptr;
    std::string indent(index * 4, ' ');
    code +=
        indent + "    builder.Prep(" + NumToString(struct_def.minalign) + ", ";
    code += NumToString(struct_def.bytesize) + ")\n";
    for (auto it = struct_def.fields.vec.rbegin();
         it != struct_def.fields.vec.rend(); ++it) {
      auto &field = **it;
      const auto &field_type = field.value.type;
      const auto &type =
          IsArray(field_type) ? field_type.VectorType() : field_type;
      if (field.padding)
        code +=
            indent + "    builder.Pad(" + NumToString(field.padding) + ")\n";
      if (IsStruct(field_type)) {
        StructBuilderBody(*field_type.struct_def,
                          (nameprefix + (NormalizedName(field) + "_")).c_str(),
                          code_ptr, index, in_array);
      } else {
        const auto index_var = "_idx" + NumToString(index);
        if (IsArray(field_type)) {
          code += indent + "    for " + index_var + " in range(";
          code += NumToString(field_type.fixed_length);
          code += " , 0, -1):\n";
          in_array = true;
        }
        if (IsStruct(type)) {
          StructBuilderBody(
              *field_type.struct_def,
              (nameprefix + (NormalizedName(field) + "_")).c_str(), code_ptr,
              index + 1, in_array);
        } else {
          code += IsArray(field_type) ? "    " : "";
          code += indent + "    builder.Prepend" + GenMethod(field) + "(";
          code += nameprefix + MakeCamel(NormalizedName(field), false);
          size_t array_cnt = index + (IsArray(field_type) ? 1 : 0);
          for (size_t i = 0; in_array && i < array_cnt; i++) {
            code += "[_idx" + NumToString(i) + "-1]";
          }
          code += ")\n";
        }
      }
    }
  }

  void EndBuilderBody(std::string *code_ptr) {
    auto &code = *code_ptr;
    code += "    return builder.Offset()\n";
  }

  // Get the value of a table's starting offset.
  void GetStartOfTable(const StructDef &struct_def, std::string *code_ptr) {
    auto &code = *code_ptr;
    code += "def Start(builder): ";
    code += "builder.StartObject(";
    code += NumToString(struct_def.fields.vec.size());
    code += ")\n";

    // Add alias with the old name.
    code += "def " + NormalizedName(struct_def) + "Start(builder):\n";
    code += Indent + "\"\"\"This method is deprecated. Please switch to Start.\"\"\"\n";
    code += Indent + "return Start(builder)\n";
  }

  // Set the value of a table's field.
  void BuildFieldOfTable(const StructDef &struct_def, const FieldDef &field,
                         const size_t offset, std::string *code_ptr) {
    auto &code = *code_ptr;
    code += "def Add" + MakeCamel(NormalizedName(field));
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
    code += ", ";
    code += IsFloat(field.value.type.base_type)
                ? float_const_gen_.GenFloatConstant(field)
                : field.value.constant;
    code += ")\n";

    // Add alias with the old name.
    code += "def " + NormalizedName(struct_def) + "Add" + MakeCamel(NormalizedName(field));
    code += "(builder, ";
    code += MakeCamel(NormalizedName(field), false);
    code += "):\n";
    code += Indent + "\"\"\"This method is deprecated. Please switch to Add";
    code += MakeCamel(NormalizedName(field)) + ".\"\"\"\n";
    code += Indent + "return Add" + MakeCamel(NormalizedName(field));
    code += "(builder, ";
    code += MakeCamel(NormalizedName(field), false);
    code += ")\n";

    // Add alias with the old name.
  }

  // Set the value of one of the members of a table's vector.
  void BuildVectorOfTable(const StructDef &struct_def, const FieldDef &field,
                          std::string *code_ptr) {
    auto &code = *code_ptr;
    code += "def Start";
    code += MakeCamel(NormalizedName(field));
    code += "Vector(builder, numElems): return builder.StartVector(";
    auto vector_type = field.value.type.VectorType();
    auto alignment = InlineAlignment(vector_type);
    auto elem_size = InlineSize(vector_type);
    code += NumToString(elem_size);
    code += ", numElems, " + NumToString(alignment);
    code += ")\n";

    // Add alias with the old name.
    code += "def " + NormalizedName(struct_def) + "Start";
    code += MakeCamel(NormalizedName(field));
    code += "Vector(builder, numElems):\n";
    code += Indent + "\"\"\"This method is deprecated. Please switch to Start.\"\"\"\n";
    code += Indent + "return Start";
    code += MakeCamel(NormalizedName(field));
    code += "Vector(builder, numElems)\n";
  }

  // Set the value of one of the members of a table's vector and fills in the
  // elements from a bytearray. This is for simplifying the use of nested
  // flatbuffers.
  void BuildVectorOfTableFromBytes(const FieldDef &field, std::string *code_ptr) {
    auto nested = field.attributes.Lookup("nested_flatbuffer");
    if (!nested) { return; }  // There is no nested flatbuffer.

    std::string unqualified_name = nested->constant;
    std::string qualified_name = nested->constant;
    auto nested_root = parser_.LookupStruct(nested->constant);
    if (nested_root == nullptr) {
      qualified_name =
          parser_.current_namespace_->GetFullyQualifiedName(nested->constant);
      nested_root = parser_.LookupStruct(qualified_name);
    }
    FLATBUFFERS_ASSERT(nested_root);  // Guaranteed to exist by parser.
    (void)nested_root;

    auto &code = *code_ptr;
    code += "def MakeVectorFromBytes(builder, bytes):\n";
    code += Indent + "builder.StartVector(";
    auto vector_type = field.value.type.VectorType();
    auto alignment = InlineAlignment(vector_type);
    auto elem_size = InlineSize(vector_type);
    code += NumToString(elem_size);
    code += ", len(bytes), " + NumToString(alignment);
    code += ")\n";
    code += Indent + "builder.head = builder.head - len(bytes)\n";
    code += Indent + "builder.Bytes[builder.head : builder.head + len(bytes)]";
    code += " = bytes\n";
    code += Indent + "return builder.EndVector()\n";

    // Add alias with the old name.
    code += "def Make" + MakeCamel(NormalizedName(field));
    code += "VectorFromBytes(builder, bytes):\n";
    code += Indent + "builder.StartVector(";
    code += NumToString(elem_size);
    code += ", len(bytes), " + NumToString(alignment);
    code += ")\n";
    code += Indent + "builder.head = builder.head - len(bytes)\n";
    code += Indent + "builder.Bytes[builder.head : builder.head + len(bytes)]";
    code += " = bytes\n";
    code += Indent + "return builder.EndVector()\n";
  }

  // Get the offset of the end of a table.
  void GetEndOffsetOnTable(const StructDef &struct_def, std::string *code_ptr) {
    auto &code = *code_ptr;
    code += "def End(builder): return builder.EndObject()\n";

    // Add alias with the old name.
    code += "def " + NormalizedName(struct_def) + "End(builder):\n";
    code += Indent + "\"\"\"This method is deprecated. Please switch to End.\"\"\"\n";
    code += Indent + "return End(builder)";
  }

  // Generate the receiver for function signatures.
  void GenReceiver(const StructDef &struct_def, std::string *code_ptr) {
    auto &code = *code_ptr;
    code += Indent + "# " + NormalizedName(struct_def) + "\n";
    code += Indent + "def ";
  }

  // Generate a struct field, conditioned on its child type(s).
  void GenStructAccessor(const StructDef &struct_def, const FieldDef &field,
                         std::string *code_ptr) {
    GenComment(field.doc_comment, code_ptr, &def_comment, Indent.c_str());
    if (IsScalar(field.value.type.base_type)) {
      if (struct_def.fixed) {
        GetScalarFieldOfStruct(struct_def, field, code_ptr);
      } else {
        GetScalarFieldOfTable(struct_def, field, code_ptr);
      }
    } else if (IsArray(field.value.type)) {
      GetArrayOfStruct(struct_def, field, code_ptr);
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
          if (vectortype.base_type == BASE_TYPE_STRUCT) {
            GetMemberOfVectorOfStruct(struct_def, field, code_ptr);
          } else {
            GetMemberOfVectorOfNonStruct(struct_def, field, code_ptr);
            GetVectorOfNonStructAsNumpy(struct_def, field, code_ptr);
            GetVectorAsNestedFlatbuffer(struct_def, field, code_ptr);
          }
          break;
        }
        case BASE_TYPE_UNION: GetUnionField(struct_def, field, code_ptr); break;
        default: FLATBUFFERS_ASSERT(0);
      }
    }
    if (IsVector(field.value.type) || IsArray(field.value.type)) {
      GetVectorLen(struct_def, field, code_ptr);
      GetVectorIsNone(struct_def, field, code_ptr);
    }
  }

  // Generate struct sizeof.
  void GenStructSizeOf(const StructDef &struct_def, std::string *code_ptr) {
    auto &code = *code_ptr;
    code += Indent + "@classmethod\n";
    code += Indent + "def SizeOf(cls):\n";
    code +=
        Indent + Indent + "return " + NumToString(struct_def.bytesize) + "\n";
    code += "\n";
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
        BuildVectorOfTableFromBytes(field, code_ptr);
      }
    }

    GetEndOffsetOnTable(struct_def, code_ptr);
  }

  // Generate function to check for proper file identifier
  void GenHasFileIdentifier(const StructDef &struct_def,
                            std::string *code_ptr) {
    auto &code = *code_ptr;
    std::string escapedID;
    // In the event any of file_identifier characters are special(NULL, \, etc),
    // problems occur. To prevent this, convert all chars to their hex-escaped
    // equivalent.
    for (auto it = parser_.file_identifier_.begin();
         it != parser_.file_identifier_.end(); ++it) {
      escapedID += "\\x" + IntToStringHex(*it, 2);
    }

    code += Indent + "@classmethod\n";
    code += Indent + "def " + NormalizedName(struct_def);
    code += "BufferHasIdentifier(cls, buf, offset, size_prefixed=False):";
    code += "\n";
    code += Indent + Indent;
    code += "return flatbuffers.util.BufferHasIdentifier(buf, offset, b\"";
    code += escapedID;
    code += "\", size_prefixed=size_prefixed)\n";
    code += "\n";
  }

  // Generates struct or table methods.
  void GenStruct(const StructDef &struct_def, std::string *code_ptr) {
    if (struct_def.generated) return;

    GenComment(struct_def.doc_comment, code_ptr, &def_comment);
    BeginClass(struct_def, code_ptr);
    if (!struct_def.fixed) {
      // Generate a special accessor for the table that has been declared as
      // the root type.
      NewRootTypeFromBuffer(struct_def, code_ptr);
      if (parser_.file_identifier_.length()) {
        // Generate a special function to test file_identifier
        GenHasFileIdentifier(struct_def, code_ptr);
      }
    } else {
      // Generates the SizeOf method for all structs.
      GenStructSizeOf(struct_def, code_ptr);
    }
    // Generates the Init method that sets the field in a pre-existing
    // accessor object. This is to allow object reuse.
    InitializeExisting(struct_def, code_ptr);
    for (auto it = struct_def.fields.vec.begin();
         it != struct_def.fields.vec.end(); ++it) {
      auto &field = **it;
      if (field.deprecated) continue;

      GenStructAccessor(struct_def, field, code_ptr);
    }

    if (struct_def.fixed) {
      // creates a struct constructor function
      GenStructBuilder(struct_def, code_ptr);
    } else {
      // Creates a set of functions that allow table construction.
      GenTableBuilders(struct_def, code_ptr);
    }
  }

  void GenReceiverForObjectAPI(const StructDef &struct_def,
                               std::string *code_ptr) {
    auto &code = *code_ptr;
    code += GenIndents(1) + "# " + NormalizedName(struct_def) + "T";
    code += GenIndents(1) + "def ";
  }

  void BeginClassForObjectAPI(const StructDef &struct_def,
                              std::string *code_ptr) {
    auto &code = *code_ptr;
    code += "\n";
    code += "class " + NormalizedName(struct_def) + "T(object):";
    code += "\n";
  }

  // Gets the accoresponding python builtin type of a BaseType for scalars and
  // string.
  std::string GetBasePythonTypeForScalarAndString(const BaseType &base_type) {
    if (IsBool(base_type)) {
      return "bool";
    } else if (IsFloat(base_type)) {
      return "float";
    } else if (IsInteger(base_type)) {
      return "int";
    } else if (base_type == BASE_TYPE_STRING) {
      return "str";
    } else {
      FLATBUFFERS_ASSERT(false && "base_type is not a scalar or string type.");
      return "";
    }
  }

  std::string GetDefaultValue(const FieldDef &field) {
    BaseType base_type = field.value.type.base_type;
    if (IsBool(base_type)) {
      return field.value.constant == "0" ? "False" : "True";
    } else if (IsFloat(base_type)) {
      return float_const_gen_.GenFloatConstant(field);
    } else if (IsInteger(base_type)) {
      return field.value.constant;
    } else {
      // For string, struct, and table.
      return "None";
    }
  }

  void GenUnionInit(const FieldDef &field, std::string *field_types_ptr,
                    std::set<std::string> *import_list,
                    std::set<std::string> *import_typing_list) {
    // Gets all possible types in the union.
    import_typing_list->insert("Union");
    auto &field_types = *field_types_ptr;
    field_types = "Union[";

    std::string separator_string = ", ";
    auto enum_def = field.value.type.enum_def;
    for (auto it = enum_def->Vals().begin(); it != enum_def->Vals().end();
         ++it) {
      auto &ev = **it;
      // Union only supports string and table.
      std::string field_type;
      switch (ev.union_type.base_type) {
        case BASE_TYPE_STRUCT:
          field_type = GenTypeGet(ev.union_type) + "T";
          if (parser_.opts.include_dependence_headers) {
            auto package_reference = GenPackageReference(ev.union_type);
            field_type = package_reference + "." + field_type;
            import_list->insert("import " + package_reference);
          }
          break;
        case BASE_TYPE_STRING: field_type += "str"; break;
        case BASE_TYPE_NONE: field_type += "None"; break;
        default: break;
      }
      field_types += field_type + separator_string;
    }

    // Removes the last separator_string.
    field_types.erase(field_types.length() - separator_string.size());
    field_types += "]";

    // Gets the import lists for the union.
    if (parser_.opts.include_dependence_headers) {
      // The package reference is generated based on enum_def, instead
      // of struct_def in field.type. That's why GenPackageReference() is
      // not used.
      Namespace *namespaces = field.value.type.enum_def->defined_namespace;
      auto package_reference = namespaces->GetFullyQualifiedName(
          MakeUpperCamel(*(field.value.type.enum_def)));
      auto union_name = MakeUpperCamel(*(field.value.type.enum_def));
      import_list->insert("import " + package_reference);
    }
  }

  void GenStructInit(const FieldDef &field, std::string *field_type_ptr,
                     std::set<std::string> *import_list,
                     std::set<std::string> *import_typing_list) {
    import_typing_list->insert("Optional");
    auto &field_type = *field_type_ptr;
    if (parser_.opts.include_dependence_headers) {
      auto package_reference = GenPackageReference(field.value.type);
      field_type = package_reference + "." + TypeName(field) + "T]";
      import_list->insert("import " + package_reference);
    } else {
      field_type = TypeName(field) + "T]";
    }
    field_type = "Optional[" + field_type;
  }

  void GenVectorInit(const FieldDef &field, std::string *field_type_ptr,
                     std::set<std::string> *import_list,
                     std::set<std::string> *import_typing_list) {
    import_typing_list->insert("List");
    auto &field_type = *field_type_ptr;
    auto base_type = field.value.type.VectorType().base_type;
    if (base_type == BASE_TYPE_STRUCT) {
      field_type = GenTypeGet(field.value.type.VectorType()) + "T]";
      if (parser_.opts.include_dependence_headers) {
        auto package_reference =
            GenPackageReference(field.value.type.VectorType());
        field_type = package_reference + "." +
                     GenTypeGet(field.value.type.VectorType()) + "T]";
        import_list->insert("import " + package_reference);
      }
      field_type = "List[" + field_type;
    } else {
      field_type =
          "List[" + GetBasePythonTypeForScalarAndString(base_type) + "]";
    }
  }

  void GenInitialize(const StructDef &struct_def, std::string *code_ptr,
                     std::set<std::string> *import_list) {
    std::string code;
    std::set<std::string> import_typing_list;
    for (auto it = struct_def.fields.vec.begin();
         it != struct_def.fields.vec.end(); ++it) {
      auto &field = **it;
      if (field.deprecated) continue;

      // Determines field type, default value, and typing imports.
      auto base_type = field.value.type.base_type;
      std::string field_type;
      switch (base_type) {
        case BASE_TYPE_UNION: {
          GenUnionInit(field, &field_type, import_list, &import_typing_list);
          break;
        }
        case BASE_TYPE_STRUCT: {
          GenStructInit(field, &field_type, import_list, &import_typing_list);
          break;
        }
        case BASE_TYPE_VECTOR:
        case BASE_TYPE_ARRAY: {
          GenVectorInit(field, &field_type, import_list, &import_typing_list);
          break;
        }
        default:
          // Scalar or sting fields.
          field_type = GetBasePythonTypeForScalarAndString(base_type);
          break;
      }

      auto default_value = GetDefaultValue(field);
      // Wrties the init statement.
      auto field_instance_name = MakeLowerCamel(field);
      code += GenIndents(2) + "self." + field_instance_name + " = " +
              default_value + "  # type: " + field_type;
    }

    // Writes __init__ method.
    auto &code_base = *code_ptr;
    GenReceiverForObjectAPI(struct_def, code_ptr);
    code_base += "__init__(self):";
    if (code.empty()) {
      code_base += GenIndents(2) + "pass";
    } else {
      code_base += code;
    }
    code_base += "\n";

    // Merges the typing imports into import_list.
    if (!import_typing_list.empty()) {
      // Adds the try statement.
      std::string typing_imports = "try:";
      typing_imports += GenIndents(1) + "from typing import ";
      std::string separator_string = ", ";
      for (auto it = import_typing_list.begin(); it != import_typing_list.end();
           ++it) {
        const std::string &im = *it;
        typing_imports += im + separator_string;
      }
      // Removes the last separator_string.
      typing_imports.erase(typing_imports.length() - separator_string.size());

      // Adds the except statement.
      typing_imports += "\n";
      typing_imports += "except:";
      typing_imports += GenIndents(1) + "pass";
      import_list->insert(typing_imports);
    }

    // Removes the import of the struct itself, if applied.
    auto package_reference =
        struct_def.defined_namespace->GetFullyQualifiedName(
            MakeUpperCamel(struct_def));
    auto struct_import = "import " + package_reference;
    import_list->erase(struct_import);
  }

  void InitializeFromBuf(const StructDef &struct_def, std::string *code_ptr) {
    auto &code = *code_ptr;
    auto instance_name = MakeLowerCamel(struct_def);
    auto struct_name = NormalizedName(struct_def);

    code += GenIndents(1) + "@classmethod";
    code += GenIndents(1) + "def InitFromBuf(cls, buf, pos):";
    code += GenIndents(2) + instance_name + " = " + struct_name + "()";
    code += GenIndents(2) + instance_name + ".Init(buf, pos)";
    code += GenIndents(2) + "return cls.InitFromObj(" + instance_name + ")";
    code += "\n";
  }

  void InitializeFromObjForObject(const StructDef &struct_def,
                                  std::string *code_ptr) {
    auto &code = *code_ptr;
    auto instance_name = MakeLowerCamel(struct_def);
    auto struct_name = NormalizedName(struct_def);

    code += GenIndents(1) + "@classmethod";
    code += GenIndents(1) + "def InitFromObj(cls, " + instance_name + "):";
    code += GenIndents(2) + "x = " + struct_name + "T()";
    code += GenIndents(2) + "x._UnPack(" + instance_name + ")";
    code += GenIndents(2) + "return x";
    code += "\n";
  }

  void GenUnPackForStruct(const StructDef &struct_def, const FieldDef &field,
                          std::string *code_ptr) {
    auto &code = *code_ptr;
    auto struct_instance_name = MakeLowerCamel(struct_def);
    auto field_instance_name = MakeLowerCamel(field);
    auto field_accessor_name = MakeUpperCamel(field);
    auto field_type = TypeName(field);

    if (parser_.opts.include_dependence_headers) {
      auto package_reference = GenPackageReference(field.value.type);
      field_type = package_reference + "." + TypeName(field);
    }

    code += GenIndents(2) + "if " + struct_instance_name + "." +
            field_accessor_name + "(";
    // if field is a struct, we need to create an instance for it first.
    if (struct_def.fixed && field.value.type.base_type == BASE_TYPE_STRUCT) {
      code += field_type + "()";
    }
    code += ") is not None:";
    code += GenIndents(3) + "self." + field_instance_name + " = " + field_type +
            "T.InitFromObj(" + struct_instance_name + "." +
            field_accessor_name + "(";
    // A struct's accessor requires a struct buf instance.
    if (struct_def.fixed && field.value.type.base_type == BASE_TYPE_STRUCT) {
      code += field_type + "()";
    }
    code += "))";
  }

  void GenUnPackForUnion(const StructDef &struct_def, const FieldDef &field,
                         std::string *code_ptr) {
    auto &code = *code_ptr;
    auto field_instance_name = MakeLowerCamel(field);
    auto field_accessor_name = MakeUpperCamel(field);
    auto struct_instance_name = MakeLowerCamel(struct_def);
    auto union_name = MakeUpperCamel(*(field.value.type.enum_def));

    if (parser_.opts.include_dependence_headers) {
      Namespace *namespaces = field.value.type.enum_def->defined_namespace;
      auto package_reference = namespaces->GetFullyQualifiedName(
          MakeUpperCamel(*(field.value.type.enum_def)));
      union_name = package_reference + "." + union_name;
    }
    code += GenIndents(2) + "self." + field_instance_name + " = " + union_name +
            "Creator(" + "self." + field_instance_name + "Type, " +
            struct_instance_name + "." + field_accessor_name + "())";
  }

  void GenUnPackForStructVector(const StructDef &struct_def,
                                const FieldDef &field, std::string *code_ptr) {
    auto &code = *code_ptr;
    auto field_instance_name = MakeLowerCamel(field);
    auto field_accessor_name = MakeUpperCamel(field);
    auto struct_instance_name = MakeLowerCamel(struct_def);

    code += GenIndents(2) + "if not " + struct_instance_name + "." +
            field_accessor_name + "IsNone():";
    code += GenIndents(3) + "self." + field_instance_name + " = []";
    code += GenIndents(3) + "for i in range(" + struct_instance_name + "." +
            field_accessor_name + "Length()):";

    auto field_type_name = TypeName(field);
    auto one_instance = field_type_name + "_";
    one_instance[0] = CharToLower(one_instance[0]);

    if (parser_.opts.include_dependence_headers) {
      auto package_reference = GenPackageReference(field.value.type);
      field_type_name = package_reference + "." + TypeName(field);
    }

    code += GenIndents(4) + "if " + struct_instance_name + "." +
            field_accessor_name + "(i) is None:";
    code += GenIndents(5) + "self." + field_instance_name + ".append(None)";
    code += GenIndents(4) + "else:";
    code += GenIndents(5) + one_instance + " = " + field_type_name +
            "T.InitFromObj(" + struct_instance_name + "." +
            field_accessor_name + "(i))";
    code += GenIndents(5) + "self." + field_instance_name + ".append(" +
            one_instance + ")";
  }

  void GenUnpackforScalarVectorHelper(const StructDef &struct_def,
                                      const FieldDef &field,
                                      std::string *code_ptr, int indents) {
    auto &code = *code_ptr;
    auto field_instance_name = MakeLowerCamel(field);
    auto field_accessor_name = MakeUpperCamel(field);
    auto struct_instance_name = MakeLowerCamel(struct_def);

    code += GenIndents(indents) + "self." + field_instance_name + " = []";
    code += GenIndents(indents) + "for i in range(" + struct_instance_name +
            "." + field_accessor_name + "Length()):";
    code += GenIndents(indents + 1) + "self." + field_instance_name +
            ".append(" + struct_instance_name + "." + field_accessor_name +
            "(i))";
  }

  void GenUnPackForScalarVector(const StructDef &struct_def,
                                const FieldDef &field, std::string *code_ptr) {
    auto &code = *code_ptr;
    auto field_instance_name = MakeLowerCamel(field);
    auto field_accessor_name = MakeUpperCamel(field);
    auto struct_instance_name = MakeLowerCamel(struct_def);

    code += GenIndents(2) + "if not " + struct_instance_name + "." +
            field_accessor_name + "IsNone():";

    // String does not have the AsNumpy method.
    if (!(IsScalar(field.value.type.VectorType().base_type))) {
      GenUnpackforScalarVectorHelper(struct_def, field, code_ptr, 3);
      return;
    }

    code += GenIndents(3) + "if np is None:";
    GenUnpackforScalarVectorHelper(struct_def, field, code_ptr, 4);

    // If numpy exists, use the AsNumpy method to optimize the unpack speed.
    code += GenIndents(3) + "else:";
    code += GenIndents(4) + "self." + field_instance_name + " = " +
            struct_instance_name + "." + field_accessor_name + "AsNumpy()";
  }

  void GenUnPackForScalar(const StructDef &struct_def, const FieldDef &field,
                          std::string *code_ptr) {
    auto &code = *code_ptr;
    auto field_instance_name = MakeLowerCamel(field);
    auto field_accessor_name = MakeUpperCamel(field);
    auto struct_instance_name = MakeLowerCamel(struct_def);

    code += GenIndents(2) + "self." + field_instance_name + " = " +
            struct_instance_name + "." + field_accessor_name + "()";
  }

  // Generates the UnPack method for the object class.
  void GenUnPack(const StructDef &struct_def, std::string *code_ptr) {
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
          GenUnPackForStruct(struct_def, field, &code);
          break;
        }
        case BASE_TYPE_UNION: {
          GenUnPackForUnion(struct_def, field, &code);
          break;
        }
        case BASE_TYPE_VECTOR: {
          auto vectortype = field.value.type.VectorType();
          if (vectortype.base_type == BASE_TYPE_STRUCT) {
            GenUnPackForStructVector(struct_def, field, &code);
          } else {
            GenUnPackForScalarVector(struct_def, field, &code);
          }
          break;
        }
        case BASE_TYPE_ARRAY: {
          GenUnPackForScalarVector(struct_def, field, &code);
          break;
        }
        default: GenUnPackForScalar(struct_def, field, &code);
      }
    }

    // Writes import statements and code into the generated file.
    auto &code_base = *code_ptr;
    auto struct_instance_name = MakeLowerCamel(struct_def);
    auto struct_name = MakeUpperCamel(struct_def);

    GenReceiverForObjectAPI(struct_def, code_ptr);
    code_base += "_UnPack(self, " + struct_instance_name + "):";
    code_base += GenIndents(2) + "if " + struct_instance_name + " is None:";
    code_base += GenIndents(3) + "return";

    // Write the import statements.
    for (std::set<std::string>::iterator it = import_list.begin();
         it != import_list.end(); ++it) {
      code_base += GenIndents(2) + *it;
    }

    // Write the code.
    code_base += code;
    code_base += "\n";
  }

  void GenPackForStruct(const StructDef &struct_def, std::string *code_ptr) {
    auto &code = *code_ptr;
    auto struct_name = MakeUpperCamel(struct_def);

    GenReceiverForObjectAPI(struct_def, code_ptr);
    code += "Pack(self, builder):";
    code += GenIndents(2) + "return Create" + struct_name + "(builder";

    StructBuilderArgs(struct_def,
                      /* nameprefix = */ "self.",
                      /* namesuffix = */ "",
                      /* has_field_name = */ true,
                      /* fieldname_suffix = */ ".", code_ptr);
    code += ")\n";
  }

  void GenPackForStructVectorField(const StructDef &struct_def,
                                   const FieldDef &field,
                                   std::string *code_prefix_ptr,
                                   std::string *code_ptr) {
    auto &code_prefix = *code_prefix_ptr;
    auto &code = *code_ptr;
    auto field_instance_name = MakeLowerCamel(field);
    auto struct_name = NormalizedName(struct_def);
    auto field_accessor_name = MakeUpperCamel(field);

    // Creates the field.
    code_prefix +=
        GenIndents(2) + "if self." + field_instance_name + " is not None:";
    if (field.value.type.struct_def->fixed) {
      code_prefix += GenIndents(3) + "Start" +
                     field_accessor_name + "Vector(builder, len(self." +
                     field_instance_name + "))";
      code_prefix += GenIndents(3) + "for i in reversed(range(len(self." +
                     field_instance_name + "))):";
      code_prefix +=
          GenIndents(4) + "self." + field_instance_name + "[i].Pack(builder)";
      code_prefix +=
          GenIndents(3) + field_instance_name + " = builder.EndVector()";
    } else {
      // If the vector is a struct vector, we need to first build accessor for
      // each struct element.
      code_prefix += GenIndents(3) + field_instance_name + "list = []";
      code_prefix += GenIndents(3);
      code_prefix += "for i in range(len(self." + field_instance_name + ")):";
      code_prefix += GenIndents(4) + field_instance_name + "list.append(self." +
                     field_instance_name + "[i].Pack(builder))";

      code_prefix += GenIndents(3) + "Start" +
                     field_accessor_name + "Vector(builder, len(self." +
                     field_instance_name + "))";
      code_prefix += GenIndents(3) + "for i in reversed(range(len(self." +
                     field_instance_name + "))):";
      code_prefix += GenIndents(4) + "builder.PrependUOffsetTRelative" + "(" +
                     field_instance_name + "list[i])";
      code_prefix +=
          GenIndents(3) + field_instance_name + " = builder.EndVector()";
    }

    // Adds the field into the struct.
    code += GenIndents(2) + "if self." + field_instance_name + " is not None:";
    code += GenIndents(3) + "Add" + field_accessor_name +
            "(builder, " + field_instance_name + ")";
  }

  void GenPackForScalarVectorFieldHelper(const StructDef &struct_def,
                                         const FieldDef &field,
                                         std::string *code_ptr, int indents) {
    auto &code = *code_ptr;
    auto field_instance_name = MakeLowerCamel(field);
    auto field_accessor_name = MakeUpperCamel(field);
    auto struct_name = NormalizedName(struct_def);
    auto vectortype = field.value.type.VectorType();

    code += GenIndents(indents) + "Start" + field_accessor_name +
            "Vector(builder, len(self." + field_instance_name + "))";
    code += GenIndents(indents) + "for i in reversed(range(len(self." +
            field_instance_name + "))):";
    code += GenIndents(indents + 1) + "builder.Prepend";

    std::string type_name;
    switch (vectortype.base_type) {
      case BASE_TYPE_BOOL: type_name = "Bool"; break;
      case BASE_TYPE_CHAR: type_name = "Byte"; break;
      case BASE_TYPE_UCHAR: type_name = "Uint8"; break;
      case BASE_TYPE_SHORT: type_name = "Int16"; break;
      case BASE_TYPE_USHORT: type_name = "Uint16"; break;
      case BASE_TYPE_INT: type_name = "Int32"; break;
      case BASE_TYPE_UINT: type_name = "Uint32"; break;
      case BASE_TYPE_LONG: type_name = "Int64"; break;
      case BASE_TYPE_ULONG: type_name = "Uint64"; break;
      case BASE_TYPE_FLOAT: type_name = "Float32"; break;
      case BASE_TYPE_DOUBLE: type_name = "Float64"; break;
      case BASE_TYPE_STRING: type_name = "UOffsetTRelative"; break;
      default: type_name = "VOffsetT"; break;
    }
    code += type_name;
  }

  void GenPackForScalarVectorField(const StructDef &struct_def,
                                   const FieldDef &field,
                                   std::string *code_prefix_ptr,
                                   std::string *code_ptr) {
    auto &code = *code_ptr;
    auto &code_prefix = *code_prefix_ptr;
    auto field_instance_name = MakeLowerCamel(field);
    auto field_accessor_name = MakeUpperCamel(field);
    auto struct_name = NormalizedName(struct_def);

    // Adds the field into the struct.
    code += GenIndents(2) + "if self." + field_instance_name + " is not None:";
    code += GenIndents(3) + "Add" + field_accessor_name +
            "(builder, " + field_instance_name + ")";

    // Creates the field.
    code_prefix +=
        GenIndents(2) + "if self." + field_instance_name + " is not None:";
    // If the vector is a string vector, we need to first build accessor for
    // each string element. And this generated code, needs to be
    // placed ahead of code_prefix.
    auto vectortype = field.value.type.VectorType();
    if (IsString(vectortype)) {
      code_prefix += GenIndents(3) + MakeLowerCamel(field) + "list = []";
      code_prefix += GenIndents(3) + "for i in range(len(self." +
                     field_instance_name + ")):";
      code_prefix += GenIndents(4) + MakeLowerCamel(field) +
                     "list.append(builder.CreateString(self." +
                     field_instance_name + "[i]))";
      GenPackForScalarVectorFieldHelper(struct_def, field, code_prefix_ptr, 3);
      code_prefix += "(" + MakeLowerCamel(field) + "list[i])";
      code_prefix +=
          GenIndents(3) + field_instance_name + " = builder.EndVector()";
      return;
    }

    code_prefix += GenIndents(3) + "if np is not None and type(self." +
                   field_instance_name + ") is np.ndarray:";
    code_prefix += GenIndents(4) + field_instance_name +
                   " = builder.CreateNumpyVector(self." + field_instance_name +
                   ")";
    code_prefix += GenIndents(3) + "else:";
    GenPackForScalarVectorFieldHelper(struct_def, field, code_prefix_ptr, 4);
    code_prefix += "(self." + field_instance_name + "[i])";
    code_prefix +=
        GenIndents(4) + field_instance_name + " = builder.EndVector()";
  }

  void GenPackForStructField(const StructDef &struct_def, const FieldDef &field,
                             std::string *code_prefix_ptr,
                             std::string *code_ptr) {
    auto &code_prefix = *code_prefix_ptr;
    auto &code = *code_ptr;
    auto field_instance_name = MakeLowerCamel(field);

    auto field_accessor_name = MakeUpperCamel(field);
    auto struct_name = NormalizedName(struct_def);

    if (field.value.type.struct_def->fixed) {
      // Pure struct fields need to be created along with their parent
      // structs.
      code +=
          GenIndents(2) + "if self." + field_instance_name + " is not None:";
      code += GenIndents(3) + field_instance_name + " = self." +
              field_instance_name + ".Pack(builder)";
    } else {
      // Tables need to be created before their parent structs are created.
      code_prefix +=
          GenIndents(2) + "if self." + field_instance_name + " is not None:";
      code_prefix += GenIndents(3) + field_instance_name + " = self." +
                     field_instance_name + ".Pack(builder)";
      code +=
          GenIndents(2) + "if self." + field_instance_name + " is not None:";
    }

    code += GenIndents(3) + "Add" + field_accessor_name +
            "(builder, " + field_instance_name + ")";
  }

  void GenPackForUnionField(const StructDef &struct_def, const FieldDef &field,
                            std::string *code_prefix_ptr,
                            std::string *code_ptr) {
    auto &code_prefix = *code_prefix_ptr;
    auto &code = *code_ptr;
    auto field_instance_name = MakeLowerCamel(field);

    auto field_accessor_name = MakeUpperCamel(field);
    auto struct_name = NormalizedName(struct_def);

    // TODO(luwa): TypeT should be moved under the None check as well.
    code_prefix +=
        GenIndents(2) + "if self." + field_instance_name + " is not None:";
    code_prefix += GenIndents(3) + field_instance_name + " = self." +
                   field_instance_name + ".Pack(builder)";
    code += GenIndents(2) + "if self." + field_instance_name + " is not None:";
    code += GenIndents(3) + "Add" + field_accessor_name +
            "(builder, " + field_instance_name + ")";
  }

  void GenPackForTable(const StructDef &struct_def, std::string *code_ptr) {
    auto &code_base = *code_ptr;
    std::string code, code_prefix;
    auto struct_instance_name = MakeLowerCamel(struct_def);
    auto struct_name = NormalizedName(struct_def);

    GenReceiverForObjectAPI(struct_def, code_ptr);
    code_base += "Pack(self, builder):";
    code += GenIndents(2) + "Start(builder)";
    for (auto it = struct_def.fields.vec.begin();
         it != struct_def.fields.vec.end(); ++it) {
      auto &field = **it;
      if (field.deprecated) continue;

      auto field_accessor_name = MakeUpperCamel(field);
      auto field_instance_name = MakeLowerCamel(field);

      switch (field.value.type.base_type) {
        case BASE_TYPE_STRUCT: {
          GenPackForStructField(struct_def, field, &code_prefix, &code);
          break;
        }
        case BASE_TYPE_UNION: {
          GenPackForUnionField(struct_def, field, &code_prefix, &code);
          break;
        }
        case BASE_TYPE_VECTOR: {
          auto vectortype = field.value.type.VectorType();
          if (vectortype.base_type == BASE_TYPE_STRUCT) {
            GenPackForStructVectorField(struct_def, field, &code_prefix, &code);
          } else {
            GenPackForScalarVectorField(struct_def, field, &code_prefix, &code);
          }
          break;
        }
        case BASE_TYPE_ARRAY: {
          GenPackForScalarVectorField(struct_def, field, &code_prefix, &code);
          break;
        }
        case BASE_TYPE_STRING: {
          code_prefix += GenIndents(2) + "if self." + field_instance_name +
                         " is not None:";
          code_prefix += GenIndents(3) + field_instance_name +
                         " = builder.CreateString(self." + field_instance_name +
                         ")";
          code += GenIndents(2) + "if self." + field_instance_name +
                  " is not None:";
          code += GenIndents(3) + "Add" + field_accessor_name +
                  "(builder, " + field_instance_name + ")";
          break;
        }
        default:
          // Generates code for scalar values. If the value equals to the
          // default value, builder will automatically ignore it. So we don't
          // need to check the value ahead.
          code += GenIndents(2) + "Add" + field_accessor_name +
                  "(builder, self." + field_instance_name + ")";
          break;
      }
    }

    code += GenIndents(2) + struct_instance_name + " = " + "End(builder)";
    code += GenIndents(2) + "return " + struct_instance_name;

    code_base += code_prefix + code;
    code_base += "\n";
  }

  void GenStructForObjectAPI(const StructDef &struct_def,
                             std::string *code_ptr) {
    if (struct_def.generated) return;

    std::set<std::string> import_list;
    std::string code;

    // Creates an object class for a struct or a table
    BeginClassForObjectAPI(struct_def, &code);

    GenInitialize(struct_def, &code, &import_list);

    InitializeFromBuf(struct_def, &code);

    InitializeFromObjForObject(struct_def, &code);

    GenUnPack(struct_def, &code);

    if (struct_def.fixed) {
      GenPackForStruct(struct_def, &code);
    } else {
      GenPackForTable(struct_def, &code);
    }

    // Adds the imports at top.
    auto &code_base = *code_ptr;
    code_base += "\n";
    for (auto it = import_list.begin(); it != import_list.end(); it++) {
      auto im = *it;
      code_base += im + "\n";
    }
    code_base += code;
  }

  void GenUnionCreatorForStruct(const EnumDef &enum_def, const EnumVal &ev,
                                std::string *code_ptr) {
    auto &code = *code_ptr;
    auto union_name = NormalizedName(enum_def);
    auto field_name = NormalizedName(ev);
    auto field_type = GenTypeGet(ev.union_type) + "T";

    code += GenIndents(1) + "if unionType == " + union_name + "()." +
            field_name + ":";
    if (parser_.opts.include_dependence_headers) {
      auto package_reference = GenPackageReference(ev.union_type);
      code += GenIndents(2) + "import " + package_reference;
      field_type = package_reference + "." + field_type;
    }
    code += GenIndents(2) + "return " + field_type +
            ".InitFromBuf(table.Bytes, table.Pos)";
  }

  void GenUnionCreatorForString(const EnumDef &enum_def, const EnumVal &ev,
                                std::string *code_ptr) {
    auto &code = *code_ptr;
    auto union_name = NormalizedName(enum_def);
    auto field_name = NormalizedName(ev);

    code += GenIndents(1) + "if unionType == " + union_name + "()." +
            field_name + ":";
    code += GenIndents(2) + "tab = Table(table.Bytes, table.Pos)";
    code += GenIndents(2) + "union = tab.String(table.Pos)";
    code += GenIndents(2) + "return union";
  }

  // Creates an union object based on union type.
  void GenUnionCreator(const EnumDef &enum_def, std::string *code_ptr) {
    auto &code = *code_ptr;
    auto union_name = MakeUpperCamel(enum_def);

    code += "\n";
    code += "def " + union_name + "Creator(unionType, table):";
    code += GenIndents(1) + "from flatbuffers.table import Table";
    code += GenIndents(1) + "if not isinstance(table, Table):";
    code += GenIndents(2) + "return None";

    for (auto it = enum_def.Vals().begin(); it != enum_def.Vals().end(); ++it) {
      auto &ev = **it;
      // Union only supports string and table.
      switch (ev.union_type.base_type) {
        case BASE_TYPE_STRUCT:
          GenUnionCreatorForStruct(enum_def, ev, &code);
          break;
        case BASE_TYPE_STRING:
          GenUnionCreatorForString(enum_def, ev, &code);
          break;
        default: break;
      }
    }
    code += GenIndents(1) + "return None";
    code += "\n";
  }

  // Generate enum declarations.
  void GenEnum(const EnumDef &enum_def, std::string *code_ptr) {
    if (enum_def.generated) return;

    GenComment(enum_def.doc_comment, code_ptr, &def_comment);
    BeginEnum(NormalizedName(enum_def), code_ptr);
    for (auto it = enum_def.Vals().begin(); it != enum_def.Vals().end(); ++it) {
      auto &ev = **it;
      GenComment(ev.doc_comment, code_ptr, &def_comment, Indent.c_str());
      EnumMember(enum_def, ev, code_ptr);
    }
    EndEnum(code_ptr);
  }

  // Returns the function name that is able to read a value of the given type.
  std::string GenGetter(const Type &type) {
    switch (type.base_type) {
      case BASE_TYPE_STRING: return "self._tab.String(";
      case BASE_TYPE_UNION: return "self._tab.Union(";
      case BASE_TYPE_VECTOR: return GenGetter(type.VectorType());
      default:
        return "self._tab.Get(flatbuffers.number_types." +
               MakeCamel(GenTypeGet(type)) + "Flags, ";
    }
  }

  // Returns the method name for use with add/put calls.
  std::string GenMethod(const FieldDef &field) {
    return (IsScalar(field.value.type.base_type) || IsArray(field.value.type))
               ? MakeCamel(GenTypeBasic(field.value.type))
               : (IsStruct(field.value.type) ? "Struct" : "UOffsetTRelative");
  }

  std::string GenTypeBasic(const Type &type) {
    // clang-format off
    static const char *ctypename[] = {
      #define FLATBUFFERS_TD(ENUM, IDLTYPE, \
              CTYPE, JTYPE, GTYPE, NTYPE, PTYPE, ...) \
        #PTYPE,
        FLATBUFFERS_GEN_TYPES(FLATBUFFERS_TD)
      #undef FLATBUFFERS_TD
    };
    // clang-format on
    return ctypename[IsArray(type) ? type.VectorType().base_type
                                   : type.base_type];
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
  void GenStructBuilder(const StructDef &struct_def, std::string *code_ptr) {
    BeginBuilderArgs(struct_def, code_ptr);
    StructBuilderArgs(struct_def,
                      /* nameprefix = */ "",
                      /* namesuffix = */ "",
                      /* has_field_name = */ true,
                      /* fieldname_suffix = */ "_", code_ptr);
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
      if (parser_.opts.generate_object_based_api & enum_def.is_union) {
        GenUnionCreator(enum_def, &enumcode);
      }
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
      if (parser_.opts.generate_object_based_api) {
        GenStructForObjectAPI(struct_def, &declcode);
      }
      if (!SaveType(struct_def, declcode, true)) return false;
    }
    return true;
  }

  // Begin by declaring namespace and imports.
  void BeginFile(const std::string &name_space_name, const bool needs_imports,
                 std::string *code_ptr) {
    auto &code = *code_ptr;
    code = code + "# " + FlatBuffersGeneratedWarning() + "\n\n";
    code += "# namespace: " + name_space_name + "\n\n";
    if (needs_imports) {
      code += "import flatbuffers\n";
      code += "from flatbuffers.compat import import_numpy\n";
      code += "np = import_numpy()\n\n";
    }
  }

  // Save out the generated code for a Python Table type.
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
        NamespaceDir(*def.defined_namespace) + NormalizedName(def) + ".py";
    return SaveFile(filename.c_str(), code, false);
  }

 private:
  std::unordered_set<std::string> keywords_;
  const SimpleFloatConstantGenerator float_const_gen_;
};

}  // namespace python

bool GeneratePython(const Parser &parser, const std::string &path,
                    const std::string &file_name) {
  python::PythonGenerator generator(parser, path, file_name);
  return generator.generate();
}

}  // namespace flatbuffers
