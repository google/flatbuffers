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

#include "flatbuffers/flatbuffers.h"
#include "flatbuffers/idl.h"
#include "flatbuffers/util.h"
#include "flatbuffers/code_generators.h"

namespace flatbuffers {
namespace python {

static std::string GenGetter(const Type &type);
static std::string GenMethod(const FieldDef &field);
static void GenStructBuilder(const StructDef &struct_def,
                             std::string *code_ptr);
static void GenReceiver(const StructDef &struct_def, std::string *code_ptr);
static std::string GenTypeBasic(const Type &type);
static std::string GenTypeGet(const Type &type);
static std::string TypeName(const FieldDef &field);


// Hardcode spaces per indentation.
const std::string Indent = "    ";

// Most field accessors need to retrieve and test the field offset first,
// this is the prefix code for that.
std::string OffsetPrefix(const FieldDef &field) {
  return "\n" + Indent + Indent +
         "o = flatbuffers.number_types.UOffsetTFlags.py_type" +
         "(self._tab.Offset(" +
         NumToString(field.value.offset) +
         "))\n" + Indent + Indent + "if o != 0:\n";
}

// Begin a class declaration.
static void BeginClass(const StructDef &struct_def, std::string *code_ptr) {
  std::string &code = *code_ptr;
  code += "class " + struct_def.name + "(object):\n";
  code += Indent + "__slots__ = ['_tab']";
  code += "\n\n";
}

// Begin enum code with a class declaration.
static void BeginEnum(const std::string class_name, std::string *code_ptr) {
  std::string &code = *code_ptr;
  code += "class " + class_name + "(object):\n";
}

// A single enum member.
static void EnumMember(const EnumVal ev, std::string *code_ptr) {
  std::string &code = *code_ptr;
  code += Indent;
  code += ev.name;
  code += " = ";
  code += NumToString(ev.value) + "\n";
}

// End enum code.
static void EndEnum(std::string *code_ptr) {
  std::string &code = *code_ptr;
  code += "\n";
}

// Initialize a new struct or table from existing data.
static void NewRootTypeFromBuffer(const StructDef &struct_def,
                                  std::string *code_ptr) {
  std::string &code = *code_ptr;

  code += Indent + "@classmethod\n";
  code += Indent + "def GetRootAs";
  code += struct_def.name;
  code += "(cls, buf, offset):";
  code += "\n";
  code += Indent + Indent;
  code += "n = flatbuffers.encode.Get";
  code += "(flatbuffers.packer.uoffset, buf, offset)\n";
  code += Indent + Indent + "x = " + struct_def.name + "()\n";
  code += Indent + Indent + "x.Init(buf, n + offset)\n";
  code += Indent + Indent + "return x\n";
  code += "\n";
}

// Initialize an existing object with other data, to avoid an allocation.
static void InitializeExisting(const StructDef &struct_def,
                               std::string *code_ptr) {
  std::string &code = *code_ptr;

  GenReceiver(struct_def, code_ptr);
  code += "Init(self, buf, pos):\n";
  code += Indent + Indent + "self._tab = flatbuffers.table.Table(buf, pos)\n";
  code += "\n";
}

// Get the length of a vector.
static void GetVectorLen(const StructDef &struct_def,
                         const FieldDef &field,
                         std::string *code_ptr) {
  std::string &code = *code_ptr;

  GenReceiver(struct_def, code_ptr);
  code += MakeCamel(field.name) + "Length(self";
  code += "):" + OffsetPrefix(field);
  code += Indent + Indent + Indent + "return self._tab.VectorLen(o)\n";
  code += Indent + Indent + "return 0\n\n";
}

// Get the value of a struct's scalar.
static void GetScalarFieldOfStruct(const StructDef &struct_def,
                                   const FieldDef &field,
                                   std::string *code_ptr) {
  std::string &code = *code_ptr;
  std::string getter = GenGetter(field.value.type);
  GenReceiver(struct_def, code_ptr);
  code += MakeCamel(field.name);
  code += "(self): return " + getter;
  code += "self._tab.Pos + flatbuffers.number_types.UOffsetTFlags.py_type(";
  code += NumToString(field.value.offset) + "))\n";
}

// Get the value of a table's scalar.
static void GetScalarFieldOfTable(const StructDef &struct_def,
                                  const FieldDef &field,
                                  std::string *code_ptr) {
  std::string &code = *code_ptr;
  std::string getter = GenGetter(field.value.type);
  GenReceiver(struct_def, code_ptr);
  code += MakeCamel(field.name);
  code += "(self):";
  code += OffsetPrefix(field);
  code += Indent + Indent + Indent + "return " + getter;
  code += "o + self._tab.Pos)\n";
  code += Indent + Indent + "return " + field.value.constant + "\n\n";
}

// Get a struct by initializing an existing struct.
// Specific to Struct.
static void GetStructFieldOfStruct(const StructDef &struct_def,
                                   const FieldDef &field,
                                   std::string *code_ptr) {
  std::string &code = *code_ptr;
  GenReceiver(struct_def, code_ptr);
  code += MakeCamel(field.name);
  code += "(self, obj):\n";
  code += Indent + Indent + "obj.Init(self._tab.Bytes, self._tab.Pos + ";
  code += NumToString(field.value.offset) + ")";
  code += "\n" + Indent + Indent + "return obj\n\n";
}

// Get a struct by initializing an existing struct.
// Specific to Table.
static void GetStructFieldOfTable(const StructDef &struct_def,
                                  const FieldDef &field,
                                  std::string *code_ptr) {
  std::string &code = *code_ptr;
  GenReceiver(struct_def, code_ptr);
  code += MakeCamel(field.name);
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
static void GetStringField(const StructDef &struct_def,
                           const FieldDef &field,
                           std::string *code_ptr) {
  std::string &code = *code_ptr;
  GenReceiver(struct_def, code_ptr);
  code += MakeCamel(field.name);
  code += "(self):";
  code += OffsetPrefix(field);
  code += Indent + Indent + Indent + "return " + GenGetter(field.value.type);
  code += "o + self._tab.Pos)\n";
  code += Indent + Indent + "return \"\"\n\n";
}

// Get the value of a union from an object.
static void GetUnionField(const StructDef &struct_def,
                          const FieldDef &field,
                          std::string *code_ptr) {
  std::string &code = *code_ptr;
  GenReceiver(struct_def, code_ptr);
  code += MakeCamel(field.name) + "(self):";
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
static void GetMemberOfVectorOfStruct(const StructDef &struct_def,
                                      const FieldDef &field,
                                      std::string *code_ptr) {
  std::string &code = *code_ptr;
  auto vectortype = field.value.type.VectorType();

  GenReceiver(struct_def, code_ptr);
  code += MakeCamel(field.name);
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
static void GetMemberOfVectorOfNonStruct(const StructDef &struct_def,
                                         const FieldDef &field,
                                         std::string *code_ptr) {
  std::string &code = *code_ptr;
  auto vectortype = field.value.type.VectorType();

  GenReceiver(struct_def, code_ptr);
  code += MakeCamel(field.name);
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

// Begin the creator function signature.
static void BeginBuilderArgs(const StructDef &struct_def,
                             std::string *code_ptr) {
  std::string &code = *code_ptr;

  code += "\n";
  code += "def Create" + struct_def.name;
  code += "(builder";
}

// Recursively generate arguments for a constructor, to deal with nested
// structs.
static void StructBuilderArgs(const StructDef &struct_def,
                              const char *nameprefix,
                              std::string *code_ptr) {
  for (auto it = struct_def.fields.vec.begin();
       it != struct_def.fields.vec.end();
       ++it) {
    auto &field = **it;
    if (IsStruct(field.value.type)) {
      // Generate arguments for a struct inside a struct. To ensure names
      // don't clash, and to make it obvious these arguments are constructing
      // a nested struct, prefix the name with the field name.
      StructBuilderArgs(*field.value.type.struct_def,
                        (nameprefix + (field.name + "_")).c_str(),
                        code_ptr);
    } else {
      std::string &code = *code_ptr;
      code += (std::string)", " + nameprefix;
      code += MakeCamel(field.name, false);
    }
  }
}

// End the creator function signature.
static void EndBuilderArgs(std::string *code_ptr) {
  std::string &code = *code_ptr;
  code += "):\n";
}

// Recursively generate struct construction statements and instert manual
// padding.
static void StructBuilderBody(const StructDef &struct_def,
                              const char *nameprefix,
                              std::string *code_ptr) {
  std::string &code = *code_ptr;
  code += "    builder.Prep(" + NumToString(struct_def.minalign) + ", ";
  code += NumToString(struct_def.bytesize) + ")\n";
  for (auto it = struct_def.fields.vec.rbegin();
       it != struct_def.fields.vec.rend();
       ++it) {
    auto &field = **it;
    if (field.padding)
      code += "    builder.Pad(" + NumToString(field.padding) + ")\n";
    if (IsStruct(field.value.type)) {
      StructBuilderBody(*field.value.type.struct_def,
                        (nameprefix + (field.name + "_")).c_str(),
                        code_ptr);
    } else {
      code += "    builder.Prepend" + GenMethod(field) + "(";
      code += nameprefix + MakeCamel(field.name, false) + ")\n";
    }
  }
}

static void EndBuilderBody(std::string *code_ptr) {
  std::string &code = *code_ptr;
  code += "    return builder.Offset()\n";
}

// Get the value of a table's starting offset.
static void GetStartOfTable(const StructDef &struct_def,
                            std::string *code_ptr) {
  std::string &code = *code_ptr;
  code += "def " + struct_def.name + "Start";
  code += "(builder): ";
  code += "builder.StartObject(";
  code += NumToString(struct_def.fields.vec.size());
  code += ")\n";
}

// Set the value of a table's field.
static void BuildFieldOfTable(const StructDef &struct_def,
                              const FieldDef &field,
                              const size_t offset,
                              std::string *code_ptr) {
  std::string &code = *code_ptr;
  code += "def " + struct_def.name + "Add" + MakeCamel(field.name);
  code += "(builder, ";
  code += MakeCamel(field.name, false);
  code += "): ";
  code += "builder.Prepend";
  code += GenMethod(field) + "Slot(";
  code += NumToString(offset) + ", ";
  if (!IsScalar(field.value.type.base_type) && (!struct_def.fixed)) {
    code += "flatbuffers.number_types.UOffsetTFlags.py_type";
    code += "(";
    code += MakeCamel(field.name, false) + ")";
  } else {
    code += MakeCamel(field.name, false);
  }
  code += ", " + field.value.constant;
  code += ")\n";
}

// Set the value of one of the members of a table's vector.
static void BuildVectorOfTable(const StructDef &struct_def,
                               const FieldDef &field,
                               std::string *code_ptr) {
  std::string &code = *code_ptr;
  code += "def " + struct_def.name + "Start";
  code += MakeCamel(field.name);
  code += "Vector(builder, numElems): return builder.StartVector(";
  auto vector_type = field.value.type.VectorType();
  auto alignment = InlineAlignment(vector_type);
  auto elem_size = InlineSize(vector_type);
  code += NumToString(elem_size);
  code += ", numElems, " + NumToString(alignment);
  code += ")\n";
}

// Get the offset of the end of a table.
static void GetEndOffsetOnTable(const StructDef &struct_def,
                                std::string *code_ptr) {
  std::string &code = *code_ptr;
  code += "def " + struct_def.name + "End";
  code += "(builder): ";
  code += "return builder.EndObject()\n";
}

// Generate the receiver for function signatures.
static void GenReceiver(const StructDef &struct_def, std::string *code_ptr) {
  std::string &code = *code_ptr;
  code += Indent + "# " + struct_def.name + "\n";
  code += Indent + "def ";
}

// Generate a struct field, conditioned on its child type(s).
static void GenStructAccessor(const StructDef &struct_def,
                              const FieldDef &field,
                              std::string *code_ptr) {
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
      case BASE_TYPE_STRING:
        GetStringField(struct_def, field, code_ptr);
        break;
      case BASE_TYPE_VECTOR: {
        auto vectortype = field.value.type.VectorType();
        if (vectortype.base_type == BASE_TYPE_STRUCT) {
          GetMemberOfVectorOfStruct(struct_def, field, code_ptr);
        } else {
          GetMemberOfVectorOfNonStruct(struct_def, field, code_ptr);
        }
        break;
      }
      case BASE_TYPE_UNION:
        GetUnionField(struct_def, field, code_ptr);
        break;
      default:
        assert(0);
    }
  }
  if (field.value.type.base_type == BASE_TYPE_VECTOR) {
    GetVectorLen(struct_def, field, code_ptr);
  }
}

// Generate table constructors, conditioned on its members' types.
static void GenTableBuilders(const StructDef &struct_def,
                             std::string *code_ptr) {
  GetStartOfTable(struct_def, code_ptr);

  for (auto it = struct_def.fields.vec.begin();
       it != struct_def.fields.vec.end();
       ++it) {
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
static void GenStruct(const StructDef &struct_def,
                      std::string *code_ptr) {
  if (struct_def.generated) return;

  GenComment(struct_def.doc_comment, code_ptr, nullptr, "# ");
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
       it != struct_def.fields.vec.end();
       ++it) {
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
static void GenEnum(const EnumDef &enum_def, std::string *code_ptr) {
  if (enum_def.generated) return;

  GenComment(enum_def.doc_comment, code_ptr, nullptr, "# ");
  BeginEnum(enum_def.name, code_ptr);
  for (auto it = enum_def.vals.vec.begin();
       it != enum_def.vals.vec.end();
       ++it) {
    auto &ev = **it;
    GenComment(ev.doc_comment, code_ptr, nullptr, "# ");
    EnumMember(ev, code_ptr);
  }
  EndEnum(code_ptr);
}

// Returns the function name that is able to read a value of the given type.
static std::string GenGetter(const Type &type) {
  switch (type.base_type) {
    case BASE_TYPE_STRING: return "self._tab.String(";
    case BASE_TYPE_UNION: return "self._tab.Union(";
    case BASE_TYPE_VECTOR: return GenGetter(type.VectorType());
    default:
      return "self._tab.Get(flatbuffers.number_types." + \
             MakeCamel(GenTypeGet(type)) + \
             "Flags, ";
  }
}

// Returns the method name for use with add/put calls.
static std::string GenMethod(const FieldDef &field) {
  return IsScalar(field.value.type.base_type)
    ? MakeCamel(GenTypeBasic(field.value.type))
    : (IsStruct(field.value.type) ? "Struct" : "UOffsetTRelative");
}

static std::string GenTypeBasic(const Type &type) {
  static const char *ctypename[] = {
    #define FLATBUFFERS_TD(ENUM, IDLTYPE, CTYPE, JTYPE, GTYPE, NTYPE, PTYPE) \
      #PTYPE,
      FLATBUFFERS_GEN_TYPES(FLATBUFFERS_TD)
    #undef FLATBUFFERS_TD
  };
  return ctypename[type.base_type];
}

static std::string GenTypePointer(const Type &type) {
  switch (type.base_type) {
    case BASE_TYPE_STRING:
      return "string";
    case BASE_TYPE_VECTOR:
      return GenTypeGet(type.VectorType());
    case BASE_TYPE_STRUCT:
      return type.struct_def->name;
    case BASE_TYPE_UNION:
      // fall through
    default:
      return "*flatbuffers.Table";
  }
}

static std::string GenTypeGet(const Type &type) {
  return IsScalar(type.base_type)
    ? GenTypeBasic(type)
    : GenTypePointer(type);
}

static std::string TypeName(const FieldDef &field) {
  return GenTypeGet(field.value.type);
}

// Create a struct with a builder and the struct's arguments.
static void GenStructBuilder(const StructDef &struct_def,
                             std::string *code_ptr) {
  BeginBuilderArgs(struct_def, code_ptr);
  StructBuilderArgs(struct_def, "", code_ptr);
  EndBuilderArgs(code_ptr);

  StructBuilderBody(struct_def, "", code_ptr);
  EndBuilderBody(code_ptr);
}

class PythonGenerator : public BaseGenerator {
 public:
  PythonGenerator(const Parser &parser, const std::string &path,
                  const std::string &file_name)
      : BaseGenerator(parser, path, file_name, "" /* not used */,
                      "" /* not used */){};
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
    code = code + "# " + FlatBuffersGeneratedWarning();
    code += "# namespace: " + name_space_name + "\n\n";
    if (needs_imports) {
      code += "import flatbuffers\n\n";
    }
  }

  // Save out the generated code for a Python Table type.
  bool SaveType(const Definition &def, const std::string &classcode,
                bool needs_imports) {
    if (!classcode.length()) return true;

    std::string namespace_dir = path_;
    auto &namespaces = parser_.namespaces_.back()->components;
    for (auto it = namespaces.begin(); it != namespaces.end(); ++it) {
      if (it != namespaces.begin()) namespace_dir += kPathSeparator;
      namespace_dir += *it;
      std::string init_py_filename = namespace_dir + "/__init__.py";
      SaveFile(init_py_filename.c_str(), "", false);
    }

    std::string code = "";
    BeginFile(LastNamespacePart(*def.defined_namespace), needs_imports, &code);
    code += classcode;
    std::string filename = NamespaceDir(*def.defined_namespace) +
                           def.name + ".py";
    return SaveFile(filename.c_str(), code, false);
  }
};

}  // namespace python

bool GeneratePython(const Parser &parser, const std::string &path,
                    const std::string &file_name) {
  python::PythonGenerator generator(parser, path, file_name);
  return generator.generate();
}

}  // namespace flatbuffers
