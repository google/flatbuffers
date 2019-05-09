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

#include <sstream>
#include <string>

#include "flatbuffers/code_generators.h"
#include "flatbuffers/flatbuffers.h"
#include "flatbuffers/idl.h"
#include "flatbuffers/util.h"

#ifdef _WIN32
#  include <direct.h>
#  define PATH_SEPARATOR "\\"
#  define mkdir(n, m) _mkdir(n)
#else
#  include <sys/stat.h>
#  define PATH_SEPARATOR "/"
#endif

namespace flatbuffers {

static std::string GeneratedFileName(const std::string &path,
                                     const std::string &file_name) {
  return path + file_name + "_generated.go";
}

namespace go {

// see https://golang.org/ref/spec#Keywords
static const char * const g_golang_keywords[] = {
  "break",  "default", "func",        "interface", "select", "case", "defer",
  "go",     "map",     "struct",      "chan",      "else",   "goto", "package",
  "switch", "const",   "fallthrough", "if",        "range",  "type", "continue",
  "for",    "import",  "return",      "var",
};

static std::string GoIdentity(const std::string &name) {
  for (size_t i = 0;
       i < sizeof(g_golang_keywords) / sizeof(g_golang_keywords[0]); i++) {
    if (name == g_golang_keywords[i]) { return MakeCamel(name + "_", false); }
  }

  return MakeCamel(name, false);
}

class GoGenerator : public BaseGenerator {
 public:
  GoGenerator(const Parser &parser, const std::string &path,
              const std::string &file_name, const std::string &go_namespace)
      : BaseGenerator(parser, path, file_name, "" /* not used*/,
                      "" /* not used */),
        cur_name_space_(nullptr) {
    std::istringstream iss(go_namespace);
    std::string component;
    while (std::getline(iss, component, '.')) {
      go_namespace_.components.push_back(component);
    }
  }

  bool generate() {
    std::string one_file_code;
    for (auto it = parser_.enums_.vec.begin(); it != parser_.enums_.vec.end();
         ++it) {
      tracked_imported_namespaces_.clear();
      std::string enumcode;
      GenEnum(**it, &enumcode);
      if (parser_.opts.one_file) {
        one_file_code += enumcode;
      } else {
        if (!SaveType(**it, enumcode, false)) return false;
      }
    }

    for (auto it = parser_.structs_.vec.begin();
         it != parser_.structs_.vec.end(); ++it) {
      tracked_imported_namespaces_.clear();
      std::string declcode;
      GenStruct(**it, &declcode);
      if (parser_.opts.one_file) {
        one_file_code += declcode;
      } else {
        if (!SaveType(**it, declcode, true)) return false;
      }
    }

    if (parser_.opts.one_file) {
      std::string code = "";
      BeginFile(LastNamespacePart(go_namespace_), true, &code);
      code += one_file_code;
      const std::string filename = GeneratedFileName(path_, file_name_);
      return SaveFile(filename.c_str(), code, false);
    }

    return true;
  }

 private:
  Namespace go_namespace_;
  Namespace *cur_name_space_;
  std::set<const Namespace*> tracked_imported_namespaces_;

  // Most field accessors need to retrieve and test the field offset first,
  // this is the prefix code for that.
  std::string OffsetPrefix(const FieldDef &field) {
    return "{\n\to := flatbuffers.UOffsetT(rcv._tab.Offset(" +
           NumToString(field.value.offset) + "))\n\tif o != 0 {\n";
  }

  // Begin a class declaration.
  void BeginClass(const StructDef &struct_def, std::string *code_ptr) {
    std::string &code = *code_ptr;

    code += "type " + struct_def.name + " struct {\n\t";

    // _ is reserved in flatbuffers field names, so no chance of name conflict:
    code += "_tab ";
    code += struct_def.fixed ? "flatbuffers.Struct" : "flatbuffers.Table";
    code += "\n}\n\n";
  }

  // Construct the name of the type alias for this enum.
  std::string GetEnumTypeName(const EnumDef &enum_def) {
    return WrapInNameSpaceAndTrack(cur_name_space_, GoIdentity(enum_def.name));
  }

  // Create a type for the enum values.
  void GenEnumType(const EnumDef &enum_def, std::string *code_ptr) {
    std::string &code = *code_ptr;
    code += "type " + GetEnumTypeName(enum_def) + " = ";
    code += GenTypeBasic(enum_def.underlying_type) + "\n";
  }

  // Begin enum code with a class declaration.
  void BeginEnum(std::string *code_ptr) {
    std::string &code = *code_ptr;
    code += "const (\n";
  }

  // A single enum member.
  void EnumMember(const EnumDef &enum_def, const EnumVal &ev,
                  std::string *code_ptr) {
    std::string &code = *code_ptr;
    code += "\t";
    code += enum_def.name;
    code += ev.name;
    code += " ";
    code += GetEnumTypeName(enum_def);
    code += " = ";
    code += NumToString(ev.value) + "\n";
  }

  // End enum code.
  void EndEnum(std::string *code_ptr) {
    std::string &code = *code_ptr;
    code += ")\n\n";
  }

  // Begin enum name code.
  void BeginEnumNames(const EnumDef &enum_def, std::string *code_ptr) {
    std::string &code = *code_ptr;
    code += "var EnumNames";
    code += enum_def.name;
    code += " = map[" + GetEnumTypeName(enum_def) + "]string{\n";
  }

  // A single enum name member.
  void EnumNameMember(const EnumDef &enum_def, const EnumVal ev,
                      std::string *code_ptr) {
    std::string &code = *code_ptr;
    code += "\t";
    code += enum_def.name;
    code += ev.name;
    code += ":\"";
    code += ev.name;
    code += "\",\n";
  }

  // End enum name code.
  void EndEnumNames(std::string *code_ptr) {
    std::string &code = *code_ptr;
    code += "}\n\n";
  }

  // Initialize a new struct or table from existing data.
  void NewRootTypeFromBuffer(const StructDef &struct_def,
                             std::string *code_ptr) {
    std::string &code = *code_ptr;

    code += "func GetRootAs";
    code += struct_def.name;
    code += "(buf []byte, offset flatbuffers.UOffsetT) ";
    code += "*" + struct_def.name + "";
    code += " {\n";
    code += "\tn := flatbuffers.GetUOffsetT(buf[offset:])\n";
    code += "\tx := &" + struct_def.name + "{}\n";
    code += "\tx.Init(buf, n+offset)\n";
    code += "\treturn x\n";
    code += "}\n\n";
  }

  // Initialize an existing object with other data, to avoid an allocation.
  void InitializeExisting(const StructDef &struct_def, std::string *code_ptr) {
    std::string &code = *code_ptr;

    GenReceiver(struct_def, code_ptr);
    code += " Init(buf []byte, i flatbuffers.UOffsetT) ";
    code += "{\n";
    code += "\trcv._tab.Bytes = buf\n";
    code += "\trcv._tab.Pos = i\n";
    code += "}\n\n";
  }

  // Implement the table accessor
  void GenTableAccessor(const StructDef &struct_def, std::string *code_ptr) {
    std::string &code = *code_ptr;

    GenReceiver(struct_def, code_ptr);
    code += " Table() flatbuffers.Table ";
    code += "{\n";

    if (struct_def.fixed) {
      code += "\treturn rcv._tab.Table\n";
    } else {
      code += "\treturn rcv._tab\n";
    }
    code += "}\n\n";
  }

  // Get the length of a vector.
  void GetVectorLen(const StructDef &struct_def, const FieldDef &field,
                    std::string *code_ptr) {
    std::string &code = *code_ptr;

    GenReceiver(struct_def, code_ptr);
    code += " " + MakeCamel(field.name) + "Length(";
    code += ") int " + OffsetPrefix(field);
    code += "\t\treturn rcv._tab.VectorLen(o)\n\t}\n";
    code += "\treturn 0\n}\n\n";
  }

  // Get a [ubyte] vector as a byte slice.
  void GetUByteSlice(const StructDef &struct_def, const FieldDef &field,
                     std::string *code_ptr) {
    std::string &code = *code_ptr;

    GenReceiver(struct_def, code_ptr);
    code += " " + MakeCamel(field.name) + "Bytes(";
    code += ") []byte " + OffsetPrefix(field);
    code += "\t\treturn rcv._tab.ByteVector(o + rcv._tab.Pos)\n\t}\n";
    code += "\treturn nil\n}\n\n";
  }

  // Get the value of a struct's scalar.
  void GetScalarFieldOfStruct(const StructDef &struct_def,
                              const FieldDef &field,
                              std::string *code_ptr) {
    std::string &code = *code_ptr;
    std::string getter = GenGetter(field.value.type);
    GenReceiver(struct_def, code_ptr);
    code += " " + MakeCamel(field.name);
    code += "() " + TypeName(field) + " {\n";
    code += "\treturn " + getter;
    code += "(rcv._tab.Pos + flatbuffers.UOffsetT(";
    code += NumToString(field.value.offset) + "))\n}\n";
  }

  // Get the value of a table's scalar.
  void GetScalarFieldOfTable(const StructDef &struct_def,
                             const FieldDef &field,
                             std::string *code_ptr) {
    std::string &code = *code_ptr;
    std::string getter = GenGetter(field.value.type);
    GenReceiver(struct_def, code_ptr);
    code += " " + MakeCamel(field.name);
    code += "() " + TypeName(field) + " ";
    code += OffsetPrefix(field) + "\t\treturn " + getter;
    code += "(o + rcv._tab.Pos)\n\t}\n";
    code += "\treturn " + GenConstant(field) + "\n";
    code += "}\n\n";
  }

  // Get a struct by initializing an existing struct.
  // Specific to Struct.
  void GetStructFieldOfStruct(const StructDef &struct_def,
                              const FieldDef &field,
                              std::string *code_ptr) {
    std::string &code = *code_ptr;
    GenReceiver(struct_def, code_ptr);
    code += " " + MakeCamel(field.name);
    code += "(obj *" + TypeName(field);
    code += ") *" + TypeName(field);
    code += " {\n";
    code += "\tif obj == nil {\n";
    code += "\t\tobj = new(" + TypeName(field) + ")\n";
    code += "\t}\n";
    code += "\tobj.Init(rcv._tab.Bytes, rcv._tab.Pos+";
    code += NumToString(field.value.offset) + ")";
    code += "\n\treturn obj\n";
    code += "}\n";
  }

  // Get a struct by initializing an existing struct.
  // Specific to Table.
  void GetStructFieldOfTable(const StructDef &struct_def,
                             const FieldDef &field,
                             std::string *code_ptr) {
    std::string &code = *code_ptr;
    GenReceiver(struct_def, code_ptr);
    code += " " + MakeCamel(field.name);
    code += "(obj *";
    code += TypeName(field);
    code += ") *" + TypeName(field) + " " + OffsetPrefix(field);
    if (field.value.type.struct_def->fixed) {
      code += "\t\tx := o + rcv._tab.Pos\n";
    } else {
      code += "\t\tx := rcv._tab.Indirect(o + rcv._tab.Pos)\n";
    }
    code += "\t\tif obj == nil {\n";
    code += "\t\t\tobj = new(" + TypeName(field) + ")\n";
    code += "\t\t}\n";
    code += "\t\tobj.Init(rcv._tab.Bytes, x)\n";
    code += "\t\treturn obj\n\t}\n\treturn nil\n";
    code += "}\n\n";
  }

  // Get the value of a string.
  void GetStringField(const StructDef &struct_def,
                      const FieldDef &field,
                      std::string *code_ptr) {
    std::string &code = *code_ptr;
    GenReceiver(struct_def, code_ptr);
    code += " " + MakeCamel(field.name);
    code += "() " + TypeName(field) + " ";
    code += OffsetPrefix(field) + "\t\treturn " + GenGetter(field.value.type);
    code += "(o + rcv._tab.Pos)\n\t}\n\treturn nil\n";
    code += "}\n\n";
  }

  // Get the value of a union from an object.
  void GetUnionField(const StructDef &struct_def, const FieldDef &field,
                     std::string *code_ptr) {
    std::string &code = *code_ptr;
    GenReceiver(struct_def, code_ptr);
    code += " " + MakeCamel(field.name) + "(";
    code += "obj " + TypeName(field) + ") bool ";
    code += OffsetPrefix(field);
    code += "\t\t" + GenGetter(field.value.type);
    code += "(obj, o)\n\t\treturn true\n\t}\n";
    code += "\treturn false\n";
    code += "}\n\n";
  }

  // Get the value of a vector's struct member.
  void GetMemberOfVectorOfStruct(const StructDef &struct_def,
                                 const FieldDef &field,
                                 std::string *code_ptr) {
    std::string &code = *code_ptr;
    auto vectortype = field.value.type.VectorType();

    GenReceiver(struct_def, code_ptr);
    code += " " + MakeCamel(field.name);
    code += "(obj *" + TypeName(field);
    code += ", j int) bool " + OffsetPrefix(field);
    code += "\t\tx := rcv._tab.Vector(o)\n";
    code += "\t\tx += flatbuffers.UOffsetT(j) * ";
    code += NumToString(InlineSize(vectortype)) + "\n";
    if (!(vectortype.struct_def->fixed)) {
      code += "\t\tx = rcv._tab.Indirect(x)\n";
    }
    code += "\t\tobj.Init(rcv._tab.Bytes, x)\n";
    code += "\t\treturn true\n\t}\n";
    code += "\treturn false\n";
    code += "}\n\n";
  }

  // Get the value of a vector's non-struct member. Uses a named return
  // argument to conveniently set the zero value for the result.
  void GetMemberOfVectorOfNonStruct(const StructDef &struct_def,
                                    const FieldDef &field,
                                    std::string *code_ptr) {
    std::string &code = *code_ptr;
    auto vectortype = field.value.type.VectorType();

    GenReceiver(struct_def, code_ptr);
    code += " " + MakeCamel(field.name);
    code += "(j int) " + TypeName(field) + " ";
    code += OffsetPrefix(field);
    code += "\t\ta := rcv._tab.Vector(o)\n";
    code += "\t\treturn " + GenGetter(field.value.type) + "(";
    code += "a + flatbuffers.UOffsetT(j*";
    code += NumToString(InlineSize(vectortype)) + "))\n";
    code += "\t}\n";
    if (vectortype.base_type == BASE_TYPE_STRING) {
      code += "\treturn nil\n";
    } else if (vectortype.base_type == BASE_TYPE_BOOL) {
      code += "\treturn false\n";
    } else {
      code += "\treturn 0\n";
    }
    code += "}\n\n";
  }

  // Begin the creator function signature.
  void BeginBuilderArgs(const StructDef &struct_def, std::string *code_ptr) {
    std::string &code = *code_ptr;

    if (code.substr(code.length() - 2) != "\n\n") {
      // a previous mutate has not put an extra new line
      code += "\n";
    }
    code += "func Create" + struct_def.name;
    code += "(builder *flatbuffers.Builder";
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
                          (nameprefix + (field.name + "_")).c_str(), code_ptr);
      } else {
        std::string &code = *code_ptr;
        code += std::string(", ") + nameprefix;
        code += GoIdentity(field.name);
        code += " " + GenTypeBasic(field.value.type);
      }
    }
  }

  // End the creator function signature.
  void EndBuilderArgs(std::string *code_ptr) {
    std::string &code = *code_ptr;
    code += ") flatbuffers.UOffsetT {\n";
  }

  // Recursively generate struct construction statements and instert manual
  // padding.
  void StructBuilderBody(const StructDef &struct_def,
                         const char *nameprefix, std::string *code_ptr) {
    std::string &code = *code_ptr;
    code += "\tbuilder.Prep(" + NumToString(struct_def.minalign) + ", ";
    code += NumToString(struct_def.bytesize) + ")\n";
    for (auto it = struct_def.fields.vec.rbegin();
         it != struct_def.fields.vec.rend(); ++it) {
      auto &field = **it;
      if (field.padding)
        code += "\tbuilder.Pad(" + NumToString(field.padding) + ")\n";
      if (IsStruct(field.value.type)) {
        StructBuilderBody(*field.value.type.struct_def,
                          (nameprefix + (field.name + "_")).c_str(), code_ptr);
      } else {
        code += "\tbuilder.Prepend" + GenMethod(field) + "(";
        code += nameprefix + GoIdentity(field.name) + ")\n";
      }
    }
  }

  void EndBuilderBody(std::string *code_ptr) {
    std::string &code = *code_ptr;
    code += "\treturn builder.Offset()\n";
    code += "}\n";
  }

  // Get the value of a table's starting offset.
  void GetStartOfTable(const StructDef &struct_def, std::string *code_ptr) {
    std::string &code = *code_ptr;
    code += "func " + struct_def.name + "Start";
    code += "(builder *flatbuffers.Builder) {\n";
    code += "\tbuilder.StartObject(";
    code += NumToString(struct_def.fields.vec.size());
    code += ")\n}\n";
  }

  // Set the value of a table's field.
  void BuildFieldOfTable(const StructDef &struct_def, const FieldDef &field,
                         const size_t offset, std::string *code_ptr) {
    std::string &code = *code_ptr;
    code += "func " + struct_def.name + "Add" + MakeCamel(field.name);
    code += "(builder *flatbuffers.Builder, ";
    code += GoIdentity(field.name) + " ";
    if (!IsScalar(field.value.type.base_type) && (!struct_def.fixed)) {
      code += "flatbuffers.UOffsetT";
    } else {
      code += GenTypeBasic(field.value.type);
    }
    code += ") {\n";
    code += "\tbuilder.Prepend";
    code += GenMethod(field) + "Slot(";
    code += NumToString(offset) + ", ";
    if (!IsScalar(field.value.type.base_type) && (!struct_def.fixed)) {
      code += "flatbuffers.UOffsetT";
      code += "(";
      code += GoIdentity(field.name) + ")";
    } else {
      code += GoIdentity(field.name);
    }
    code += ", " + GenConstant(field);
    code += ")\n}\n";
  }

  // Set the value of one of the members of a table's vector.
  void BuildVectorOfTable(const StructDef &struct_def,
                          const FieldDef &field, std::string *code_ptr) {
    std::string &code = *code_ptr;
    code += "func " + struct_def.name + "Start";
    code += MakeCamel(field.name);
    code += "Vector(builder *flatbuffers.Builder, numElems int) ";
    code += "flatbuffers.UOffsetT {\n\treturn builder.StartVector(";
    auto vector_type = field.value.type.VectorType();
    auto alignment = InlineAlignment(vector_type);
    auto elem_size = InlineSize(vector_type);
    code += NumToString(elem_size);
    code += ", numElems, " + NumToString(alignment);
    code += ")\n}\n";
  }

  // Get the offset of the end of a table.
  void GetEndOffsetOnTable(const StructDef &struct_def, std::string *code_ptr) {
    std::string &code = *code_ptr;
    code += "func " + struct_def.name + "End";
    code += "(builder *flatbuffers.Builder) flatbuffers.UOffsetT ";
    code += "{\n\treturn builder.EndObject()\n}\n";
  }

  // Generate the receiver for function signatures.
  void GenReceiver(const StructDef &struct_def, std::string *code_ptr) {
    std::string &code = *code_ptr;
    code += "func (rcv *" + struct_def.name + ")";
  }

  // Generate a struct field getter, conditioned on its child type(s).
  void GenStructAccessor(const StructDef &struct_def,
                         const FieldDef &field, std::string *code_ptr) {
    GenComment(field.doc_comment, code_ptr, nullptr, "");
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
          }
          break;
        }
        case BASE_TYPE_UNION: GetUnionField(struct_def, field, code_ptr); break;
        default: FLATBUFFERS_ASSERT(0);
      }
    }
    if (field.value.type.base_type == BASE_TYPE_VECTOR) {
      GetVectorLen(struct_def, field, code_ptr);
      if (field.value.type.element == BASE_TYPE_UCHAR) {
        GetUByteSlice(struct_def, field, code_ptr);
      }
    }
  }

  // Mutate the value of a struct's scalar.
  void MutateScalarFieldOfStruct(const StructDef &struct_def,
                                 const FieldDef &field,
                                 std::string *code_ptr) {
    std::string &code = *code_ptr;
    std::string type = MakeCamel(GenTypeBasic(field.value.type));
    std::string setter = "rcv._tab.Mutate" + type;
    GenReceiver(struct_def, code_ptr);
    code += " Mutate" + MakeCamel(field.name);
    code += "(n " + TypeName(field) + ") bool {\n\treturn " + setter;
    code += "(rcv._tab.Pos+flatbuffers.UOffsetT(";
    code += NumToString(field.value.offset) + "), n)\n}\n\n";
  }

  // Mutate the value of a table's scalar.
  void MutateScalarFieldOfTable(const StructDef &struct_def,
                                const FieldDef &field,
                                std::string *code_ptr) {
    std::string &code = *code_ptr;
    std::string type = MakeCamel(GenTypeBasic(field.value.type));
    std::string setter = "rcv._tab.Mutate" + type + "Slot";
    GenReceiver(struct_def, code_ptr);
    code += " Mutate" + MakeCamel(field.name);
    code += "(n " + TypeName(field) + ") bool {\n\treturn ";
    code += setter + "(" + NumToString(field.value.offset) + ", n)\n";
    code += "}\n\n";
  }

  // Mutate an element of a vector of scalars.
  void MutateElementOfVectorOfNonStruct(const StructDef &struct_def,
                                        const FieldDef &field,
                                        std::string *code_ptr) {
    std::string &code = *code_ptr;
    auto vectortype = field.value.type.VectorType();
    std::string type = MakeCamel(GenTypeBasic(vectortype));
    std::string setter = "rcv._tab.Mutate" + type;
    GenReceiver(struct_def, code_ptr);
    code += " Mutate" + MakeCamel(field.name);
    code += "(j int, n " + TypeName(field) + ") bool ";
    code += OffsetPrefix(field);
    code += "\t\ta := rcv._tab.Vector(o)\n";
    code += "\t\treturn " + setter + "(";
    code += "a+flatbuffers.UOffsetT(j*";
    code += NumToString(InlineSize(vectortype)) + "), n)\n";
    code += "\t}\n";
    code += "\treturn false\n";
    code += "}\n\n";
  }

  // Generate a struct field setter, conditioned on its child type(s).
  void GenStructMutator(const StructDef &struct_def, const FieldDef &field,
                        std::string *code_ptr) {
    GenComment(field.doc_comment, code_ptr, nullptr, "");
    if (IsScalar(field.value.type.base_type)) {
      if (struct_def.fixed) {
        MutateScalarFieldOfStruct(struct_def, field, code_ptr);
      } else {
        MutateScalarFieldOfTable(struct_def, field, code_ptr);
      }
    } else if (field.value.type.base_type == BASE_TYPE_VECTOR) {
      if (IsScalar(field.value.type.element)) {
        MutateElementOfVectorOfNonStruct(struct_def, field, code_ptr);
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
      if (field.value.type.base_type == BASE_TYPE_VECTOR) {
        BuildVectorOfTable(struct_def, field, code_ptr);
      }
    }

    GetEndOffsetOnTable(struct_def, code_ptr);
  }

  // Generate struct or table methods.
  void GenStruct(const StructDef &struct_def, std::string *code_ptr) {
    if (struct_def.generated) return;

    cur_name_space_ = struct_def.defined_namespace;

    GenComment(struct_def.doc_comment, code_ptr, nullptr);
    if (parser_.opts.generate_object_based_api) {
      GenNativeStruct(struct_def, code_ptr);
    }
    BeginClass(struct_def, code_ptr);
    if (!struct_def.fixed) {
      // Generate a special accessor for the table that has been declared as
      // the root type.
      NewRootTypeFromBuffer(struct_def, code_ptr);
    }
    // Generate the Init method that sets the field in a pre-existing
    // accessor object. This is to allow object reuse.
    InitializeExisting(struct_def, code_ptr);
    // Generate _tab accessor
    GenTableAccessor(struct_def, code_ptr);

    // Generate struct fields accessors
    for (auto it = struct_def.fields.vec.begin();
         it != struct_def.fields.vec.end(); ++it) {
      auto &field = **it;
      if (field.deprecated) continue;

      GenStructAccessor(struct_def, field, code_ptr);
      GenStructMutator(struct_def, field, code_ptr);
    }

    // Generate builders
    if (struct_def.fixed) {
      // create a struct constructor function
      GenStructBuilder(struct_def, code_ptr);
    } else {
      // Create a set of functions that allow table construction.
      GenTableBuilders(struct_def, code_ptr);
    }
  }

  void GenNativeStruct(const StructDef &struct_def, std::string *code_ptr) {
    std::string &code = *code_ptr;

    code += "type " + NativeName(struct_def) + " struct {\n";
    for (auto it = struct_def.fields.vec.begin();
         it != struct_def.fields.vec.end(); ++it) {
      const FieldDef &field = **it;
      if (field.deprecated) continue;
      code += "\t" + MakeCamel(field.name) + " " +
              NativeType(field.value.type) + "\n";
    }
    code += "}\n\n";

    if (!struct_def.fixed) {
      GenNativeTablePack(struct_def, code_ptr);
      GenNativeTableUnPack(struct_def, code_ptr);
    } else {
      GenNativeStructPack(struct_def, code_ptr);
      GenNativeStructUnPack(struct_def, code_ptr);
    }
  }

  void GenNativeTablePack(const StructDef &struct_def, std::string *code_ptr) {
    std::string &code = *code_ptr;

    code += "func " + struct_def.name +
            "Pack(builder *flatbuffers.Builder, t *" + NativeName(struct_def) +
            ") flatbuffers.UOffsetT {\n";
    code += "\tif t == nil { return 0 }\n";
    for (auto it = struct_def.fields.vec.begin();
         it != struct_def.fields.vec.end(); ++it) {
      const FieldDef &field = **it;
      if (field.deprecated) continue;
      if (IsScalar(field.value.type.base_type)) continue;

      std::string offset = MakeCamel(field.name, false) + "Offset";

      if (field.value.type.base_type == BASE_TYPE_STRING) {
        code += "\t" + offset + " := builder.CreateString(t." +
                MakeCamel(field.name) + ")\n";
      } else if (field.value.type.base_type == BASE_TYPE_VECTOR &&
                 field.value.type.element == BASE_TYPE_UCHAR) {
        code += "\t" + offset + " := builder.CreateByteString(t." +
                MakeCamel(field.name) + ")\n";
      } else if (field.value.type.base_type == BASE_TYPE_VECTOR) {
        std::string length = MakeCamel(field.name, false) + "Length";
        std::string offsets = MakeCamel(field.name, false) + "Offsets";
        code += "\t" + length + " := len(t." + MakeCamel(field.name) + ")\n";
        if (!IsScalar(field.value.type.element)) {
          code += "\t" + offsets + " := []flatbuffers.UOffsetT{}\n";
          code += "\tfor j := 0; j < " + length + "; j++ {\n";
          code += "\t\t" + offsets + " = append(" + offsets + ", ";
          if (field.value.type.element == BASE_TYPE_STRING) {
            code += "builder.CreateString(t." + MakeCamel(field.name) + "[j])";
          } else if (field.value.type.element == BASE_TYPE_STRUCT) {
            code += field.value.type.struct_def->name + "Pack(builder, t." +
                    MakeCamel(field.name) + "[j])";
          } else {
            // TODO
          }
          code += ")\n";
          code += "\t}\n";
        }
        code += "\t" + struct_def.name + "Start" + MakeCamel(field.name) +
                "Vector(builder, " + length + ")\n";
        code += "\tfor j := " + length + " - 1; j >= 0; j-- {\n";
        if (IsScalar(field.value.type.element)) {
          code += "\t\tbuilder.Prepend" +
                  MakeCamel(GenTypeBasic(field.value.type.VectorType())) +
                  "(t." + MakeCamel(field.name) + "[j])\n";
        } else {
          code += "\t\tbuilder.PrependUOffsetT(" + offsets + "[j])\n";
        }
        code += "\t}\n";
        code += "\t" + offset + " := builder.EndVector(" + length + ")\n";
      } else if (field.value.type.base_type == BASE_TYPE_STRUCT) {
        code += "\t" + offset + " := " + field.value.type.struct_def->name +
                "Pack(builder, t." + MakeCamel(field.name) + ")\n";
      } else if (field.value.type.base_type == BASE_TYPE_UNION) {
        // TODO
      } else {
        FLATBUFFERS_ASSERT(0);
      }
    }
    code += "\t" + struct_def.name + "Start(builder)\n";
    for (auto it = struct_def.fields.vec.begin();
         it != struct_def.fields.vec.end(); ++it) {
      const FieldDef &field = **it;
      if (field.deprecated) continue;
      if (IsScalar(field.value.type.base_type)) {
        code += "\t" + struct_def.name + "Add" + MakeCamel(field.name) +
                "(builder, t." + MakeCamel(field.name) + ")\n";
      } else {
        code += "\t" + struct_def.name + "Add" + MakeCamel(field.name) +
                "(builder, " + MakeCamel(field.name, false) + "Offset)\n";
      }
    }
    code += "\treturn " + struct_def.name + "End(builder)\n";
    code += "}\n\n";
  }

  void GenNativeTableUnPack(
    const StructDef &struct_def, std::string *code_ptr) {
    std::string &code = *code_ptr;

    code += "func (rcv *" + struct_def.name + ") UnPack() *" +
            NativeName(struct_def) + " {\n";
    code += "\tif rcv == nil { return nil }\n";
    code += "\tt := &" + NativeName(struct_def) + "{}\n";
    for (auto it = struct_def.fields.vec.begin();
         it != struct_def.fields.vec.end(); ++it) {
      const FieldDef &field = **it;
      if (field.deprecated) continue;
      std::string field_name_camel = MakeCamel(field.name);
      if (IsScalar(field.value.type.base_type)) {
        code += "\tt." + field_name_camel + " = rcv." + field_name_camel +
                "()\n";
      } else if (field.value.type.base_type == BASE_TYPE_STRING) {
        code += "\tt." + field_name_camel + " = string(rcv." +
                field_name_camel + "())\n";
      } else if (field.value.type.base_type == BASE_TYPE_VECTOR &&
                 field.value.type.element == BASE_TYPE_UCHAR) {
        code += "\tt." + field_name_camel + " = rcv." + field_name_camel +
                "Bytes()\n";
      } else if (field.value.type.base_type == BASE_TYPE_VECTOR) {
        code += "\tfor j := 0; j < rcv." + field_name_camel +
                "Length(); j++ {\n";
        if (field.value.type.element == BASE_TYPE_STRUCT) {
          code += "\tx := " + field.value.type.struct_def->name + "{}\n";
          code += "\tif !rcv." + field_name_camel + "(&x, j) { return nil }\n";
        }
        code += "\t\tt." + field_name_camel + " = append(t." +
                field_name_camel + ", ";
        if (IsScalar(field.value.type.element)) {
          code += "rcv." + field_name_camel + "(j)";
        } else if (field.value.type.element == BASE_TYPE_STRING) {
          code += "string(rcv." + field_name_camel + "(j))";
        } else if (field.value.type.element == BASE_TYPE_STRUCT) {
          code += "x.UnPack()";
        } else {
          // TODO(iceboy): BASE_TYPE_UNION
          FLATBUFFERS_ASSERT(0);
        }
        code += ")\n";
        code += "\t}\n";
      } else if (field.value.type.base_type == BASE_TYPE_STRUCT) {
        code += "\tt." + field_name_camel + " = rcv." + field_name_camel +
                "(nil).UnPack()\n";
      } else {
        // TODO(iceboy): BASE_TYPE_UNION
        FLATBUFFERS_ASSERT(0);
      }
    }
    code += "\treturn t\n";
    code += "}\n\n";
  }

  void GenNativeStructPack(const StructDef &struct_def, std::string *code_ptr) {
    std::string &code = *code_ptr;

    code += "func " + struct_def.name +
            "Pack(builder *flatbuffers.Builder, t *" + NativeName(struct_def) +
            ") flatbuffers.UOffsetT {\n";
    code += "\tif t == nil { return 0 }\n";
    code += "\treturn Create" + struct_def.name + "(builder";
    StructPackArgs(struct_def, "", code_ptr);
    code += ")\n";
    code += "}\n";
  }

  void StructPackArgs(const StructDef &struct_def, const char *nameprefix,
                      std::string *code_ptr) {
    std::string &code = *code_ptr;
    for (auto it = struct_def.fields.vec.begin();
         it != struct_def.fields.vec.end(); ++it) {
      const FieldDef &field = **it;
      if (field.value.type.base_type != BASE_TYPE_STRUCT) {
        code += std::string(", t.") + nameprefix + MakeCamel(field.name);
      } else {
        StructPackArgs(*field.value.type.struct_def,
                       (nameprefix + MakeCamel(field.name) + ".").c_str(),
                       code_ptr);
      }
    }
  }

  void GenNativeStructUnPack(
    const StructDef &struct_def, std::string *code_ptr) {
    std::string &code = *code_ptr;

    code += "func (rcv *" + struct_def.name + ") UnPack() *" +
            NativeName(struct_def) + " {\n";
    code += "\tif rcv == nil { return nil }\n";
    code += "\tt := &" + NativeName(struct_def) + "{}\n";
    for (auto it = struct_def.fields.vec.begin();
         it != struct_def.fields.vec.end(); ++it) {
      const FieldDef &field = **it;
      if (field.value.type.base_type != BASE_TYPE_STRUCT) {
        code += "\tt." + MakeCamel(field.name) + " = rcv." +
                MakeCamel(field.name) + "()\n";
      } else {
        code += "\tt." + MakeCamel(field.name) + " = rcv." +
                MakeCamel(field.name) + "(nil).UnPack()\n";
      }
    }
    code += "\treturn t\n";
    code += "}\n\n";
  }

  // Generate enum declarations.
  void GenEnum(const EnumDef &enum_def, std::string *code_ptr) {
    if (enum_def.generated) return;

    cur_name_space_ = enum_def.defined_namespace;

    GenComment(enum_def.doc_comment, code_ptr, nullptr);
    GenEnumType(enum_def, code_ptr);
    BeginEnum(code_ptr);
    for (auto it = enum_def.Vals().begin(); it != enum_def.Vals().end(); ++it) {
      auto &ev = **it;
      GenComment(ev.doc_comment, code_ptr, nullptr, "\t");
      EnumMember(enum_def, ev, code_ptr);
    }
    EndEnum(code_ptr);

    BeginEnumNames(enum_def, code_ptr);
    for (auto it = enum_def.Vals().begin(); it != enum_def.Vals().end(); ++it) {
      auto &ev = **it;
      EnumNameMember(enum_def, ev, code_ptr);
    }
    EndEnumNames(code_ptr);
  }

  // Returns the function name that is able to read a value of the given type.
  std::string GenGetter(const Type &type) {
    switch (type.base_type) {
      case BASE_TYPE_STRING: return "rcv._tab.ByteVector";
      case BASE_TYPE_UNION: return "rcv._tab.Union";
      case BASE_TYPE_VECTOR: return GenGetter(type.VectorType());
      default: return "rcv._tab.Get" + MakeCamel(GenTypeBasic(type));
    }
  }

  // Returns the method name for use with add/put calls.
  std::string GenMethod(const FieldDef &field) {
    return IsScalar(field.value.type.base_type)
               ? MakeCamel(GenTypeBasic(field.value.type))
               : (IsStruct(field.value.type) ? "Struct" : "UOffsetT");
  }

  std::string GenTypeBasic(const Type &type) {
    static const char *ctypename[] = {
    // clang-format off
      #define FLATBUFFERS_TD(ENUM, IDLTYPE, \
        CTYPE, JTYPE, GTYPE, NTYPE, PTYPE, RTYPE) \
        #GTYPE,
        FLATBUFFERS_GEN_TYPES(FLATBUFFERS_TD)
      #undef FLATBUFFERS_TD
      // clang-format on
    };
    return ctypename[type.base_type];
  }

  std::string GenTypePointer(const Type &type) {
    switch (type.base_type) {
      case BASE_TYPE_STRING: return "[]byte";
      case BASE_TYPE_VECTOR: return GenTypeGet(type.VectorType());
      case BASE_TYPE_STRUCT: return WrapInNameSpaceAndTrack(*type.struct_def);
      case BASE_TYPE_UNION:
        // fall through
      default: return "*flatbuffers.Table";
    }
  }

  std::string GenTypeGet(const Type &type) {
    if (type.enum_def != nullptr && !type.enum_def->is_union) {
      return GetEnumTypeName(*type.enum_def);
    }
    return IsScalar(type.base_type) ? GenTypeBasic(type) : GenTypePointer(type);
  }

  std::string TypeName(const FieldDef &field) {
    return GenTypeGet(field.value.type);
  }

  std::string GenConstant(const FieldDef &field) {
    switch (field.value.type.base_type) {
      case BASE_TYPE_BOOL: return field.value.constant == "0" ? "false" : "true";
      default: return field.value.constant;
    }
  }

  std::string NativeName(const StructDef &struct_def) {
    return parser_.opts.object_prefix + struct_def.name +
           parser_.opts.object_suffix;
  }

  std::string NativeType(const Type &type) {
    if (IsScalar(type.base_type)) {
      return GenTypeBasic(type);
    } else if (type.base_type == BASE_TYPE_STRING) {
      return "string";
    } else if (type.base_type == BASE_TYPE_VECTOR) {
      return "[]" + NativeType(type.VectorType());
    } else if (type.base_type == BASE_TYPE_STRUCT) {
      return "*" + NativeName(*type.struct_def);
    } else if (type.base_type == BASE_TYPE_UNION) {
      // TODO
    }
    FLATBUFFERS_ASSERT(0);
    return std::string();
  }

  // Create a struct with a builder and the struct's arguments.
  void GenStructBuilder(const StructDef &struct_def, std::string *code_ptr) {
    BeginBuilderArgs(struct_def, code_ptr);
    StructBuilderArgs(struct_def, "", code_ptr);
    EndBuilderArgs(code_ptr);

    StructBuilderBody(struct_def, "", code_ptr);
    EndBuilderBody(code_ptr);
  }
  // Begin by declaring namespace and imports.
  void BeginFile(const std::string name_space_name, const bool needs_imports,
                 std::string *code_ptr) {
    std::string &code = *code_ptr;
    code = code + "// Code generated by the FlatBuffers compiler. DO NOT EDIT.\n\n";
    code += "package " + name_space_name + "\n\n";
    if (needs_imports) {
      code += "import (\n";
      if (!parser_.opts.go_import.empty()) {
        code += "\tflatbuffers \"" + parser_.opts.go_import + "\"\n";
      } else {
        code += "\tflatbuffers \"github.com/google/flatbuffers/go\"\n";
      }
      if (tracked_imported_namespaces_.size() > 0) {
        code += "\n";
        for (auto it = tracked_imported_namespaces_.begin();
             it != tracked_imported_namespaces_.end();
             ++it) {
        code += "\t" + NamespaceImportName(*it) + " \"" + \
                NamespaceImportPath(*it) + "\"\n";
        }
      }
      code += ")\n\n";
    }
  }

  // Save out the generated code for a Go Table type.
  bool SaveType(const Definition &def, const std::string &classcode,
                bool needs_imports) {
    if (!classcode.length()) return true;

    Namespace &ns = go_namespace_.components.empty() ? *def.defined_namespace
                                                     : go_namespace_;
    std::string code = "";
    BeginFile(LastNamespacePart(ns), needs_imports, &code);
    code += classcode;
    std::string filename = NamespaceDir(ns) + def.name + ".go";
    return SaveFile(filename.c_str(), code, false);
  }

  // Create the full name of the imported namespace (format: A__B__C).
  std::string NamespaceImportName(const Namespace *ns) {
    std::string s = "";
    for (auto it = ns->components.begin(); it != ns->components.end(); ++it) {
      if (s.size() == 0) {
        s += *it;
      } else {
        s += "__" + *it;
      }
    }
    return s;
  }

  // Create the full path for the imported namespace (format: A/B/C).
  std::string NamespaceImportPath(const Namespace *ns) {
    std::string s = "";
    for (auto it = ns->components.begin(); it != ns->components.end(); ++it) {
      if (s.size() == 0) {
        s += *it;
      } else {
        s += "/" + *it;
      }
    }
    return s;
  }

  // Ensure that a type is prefixed with its go package import name if it is
  // used outside of its namespace.
  std::string WrapInNameSpaceAndTrack(const Namespace *ns,
                                      const std::string &name) {
    if (CurrentNameSpace() == ns) return name;

    tracked_imported_namespaces_.insert(ns);

    std::string import_name = NamespaceImportName(ns);
    return import_name + "." + name;
  }

  std::string WrapInNameSpaceAndTrack(const Definition &def) {
    return WrapInNameSpaceAndTrack(def.defined_namespace, def.name);
  }

  const Namespace *CurrentNameSpace() const { return cur_name_space_; }
};
}  // namespace go

bool GenerateGo(const Parser &parser, const std::string &path,
                const std::string &file_name) {
  go::GoGenerator generator(parser, path, file_name, parser.opts.go_namespace);
  return generator.generate();
}

}  // namespace flatbuffers
