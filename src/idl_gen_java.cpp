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

#ifdef _WIN32
#include <direct.h>
#define PATH_SEPARATOR "\\"
#define mkdir(n, m) _mkdir(n)
#else
#include <sys/stat.h>
#define PATH_SEPARATOR "/"
#endif

namespace flatbuffers {
namespace java {

static std::string GenGetter(const Type &type);
static std::string GenMethod(const FieldDef &field);
static void GenStructBuilder(const StructDef &struct_def,
                             std::string *code_ptr);
static std::string GenTypeBasic(const Type &type);
static std::string GenTypeGet(const Type &type);
static std::string TypeName(const FieldDef &field);

// Convert an underscore_based_indentifier in to camelCase.
// Also uppercases the first character if first is true.
static std::string MakeCamel(const std::string &in, bool first = true) {
  std::string s;
  for (size_t i = 0; i < in.length(); i++) {
    if (!i && first) {
      s += static_cast<char>(toupper(in[0]));
    } else if (in[i] == '_' && i + 1 < in.length()) {
      s += static_cast<char>(toupper(in[++i]));
    } else {
      s += in[i];
    }
  }
  return s;
}

// Namespace lang contains exactly the code-generating functions.
namespace lang {

// Write a comment.
static void Comment(const std::string &dc,
                       std::string *code_ptr,
                       const char *prefix = "") {
  std::string &code = *code_ptr;
  if (dc.length()) {
    code += std::string(prefix) + "///" + dc + "\n";
  }
}

// Most field accessors need to retrieve and test the field offset first,
// this is the prefix code for that.
std::string OffsetPrefix(const FieldDef &field) {
  return ") { int o = __offset(" +
         NumToString(field.value.offset) +
         "); return o != 0 ? ";
}

// Begin by declaring namespace and imports.
static void BeginFile(const std::string name_space_name,
                      const bool needs_imports,
                      std::string *code_ptr) {
  std::string &code = *code_ptr;
  code += "// automatically generated, do not modify\n\n";
  code += "package " + name_space_name + ";\n\n";
  if (needs_imports) {
    code += "import java.nio.*;\nimport java.lang.*;\nimport java.util.*;\n";
    code += "import flatbuffers.*;\n\n";
  }
}

// Begin a class declaration.
static void BeginClass(const StructDef &struct_def, std::string *code_ptr) {
  std::string &code = *code_ptr;

  code += "public class " + struct_def.name + " extends ";
  code += struct_def.fixed ? "Struct" : "Table";
  code += " {\n";
}

// Begin enum code with a class declaration.
static void BeginEnum(const EnumDef &enum_def, std::string *code_ptr) {
  std::string &code = *code_ptr;
  code += "public class " + enum_def.name + " {\n";
}

// A single enum member.
static void EnumMember(const EnumDef &enum_def, const EnumVal ev,
                       std::string *code_ptr) {
  std::string &code = *code_ptr;
  code += "  public static final " + GenTypeBasic(enum_def.underlying_type);
  code += " " + ev.name + " = ";
  code += NumToString(ev.value) + ";\n";
}

// End enum code.
static void EndEnum(std::string *code_ptr) {
  std::string &code = *code_ptr;
  code += "};\n\n";
}

// Initialize a new struct or table from existing data.
static void NewRootTypeFromBuffer(const StructDef &struct_def,
                                  std::string *code_ptr) {
  std::string &code = *code_ptr;

  code += "  public static " + struct_def.name + " getRootAs";
  code += struct_def.name;
  code += "(ByteBuffer _bb, int offset) { ";
  code += "_bb.order(ByteOrder.LITTLE_ENDIAN); ";
  code += "return (new " + struct_def.name;
  code += "()).__init(_bb.getInt(offset) + offset, _bb); }\n";
}

// Begin the set of struct accessors.
static void BeginStructAccessors(const StructDef &struct_def,
                                 std::string *code_ptr) {
}

// Initialize an existing object with other data, to avoid an allocation.
static void InitializeExisting(const StructDef &struct_def,
                               std::string *code_ptr) {
  std::string &code = *code_ptr;

  code += "  public " + struct_def.name;
  code += " __init(int _i, ByteBuffer _bb) ";
  code += "{ bb_pos = _i; bb = _bb; return this; }\n";
}

// Get a table member by allocating a new object.
static void GetFieldWithAlloc(const FieldDef &field,
                              std::string *code_ptr) {
  std::string &code = *code_ptr;

  code += "  public " + TypeName(field) + " " +
          MakeCamel(field.name, false);
  code += "() { return " + MakeCamel(field.name, false);
  code += "(new ";
  code += TypeName(field) + "()); }\n";
}

// Get a vector member by allocating a new object.
static void GetVectorFieldWithAlloc(const FieldDef &field,
                                    std::string *code_ptr) {
  std::string &code = *code_ptr;

  code += "  public " + TypeName(field) + " " +
          MakeCamel(field.name, false);
  code += "(int j) { return " + MakeCamel(field.name, false);
  code += "(new ";
  code += TypeName(field) + "(), j); }\n";
}

// Get the length of a vector.
static void GetVectorLen(const FieldDef &field,
                          std::string *code_ptr) {
  std::string &code = *code_ptr;

  code += "  public int " + MakeCamel(field.name, false) + "Length(";
  code += OffsetPrefix(field);
  code += "__vector_len(o) : 0; }\n";
}

// Get the value of a struct's scalar.
static void GetScalarFieldOfStruct(const FieldDef &field,
                                   std::string *code_ptr) {
  std::string &code = *code_ptr;
  std::string getter = GenGetter(field.value.type);
  std::string method_start = "  public " + TypeName(field) + " " +
                             MakeCamel(field.name, false);

  code += method_start + "(";
  code += ") { return " + getter;
  code += "(bb_pos + " + NumToString(field.value.offset) + ")";
  code += "; }\n";
}

// Get the value of a table's scalar.
static void GetScalarFieldOfTable(const FieldDef &field,
                                  std::string *code_ptr) {
  std::string &code = *code_ptr;
  std::string getter = GenGetter(field.value.type);
  std::string method_start = "  public " + TypeName(field) + " " +
                             MakeCamel(field.name, false);

  code += method_start + "(";
  code += OffsetPrefix(field) + getter;
  code += "(o + bb_pos) : " + field.value.constant;
  code += "; }\n";
}

// Get a struct by initializing an existing struct.
// Specific to Struct.
static void GetStructFieldOfStruct(const FieldDef &field,
                                   std::string *code_ptr) {
  std::string &code = *code_ptr;
  std::string method_start = "  public " + TypeName(field) + " " +
                             MakeCamel(field.name, false);

  code += method_start + "(";

  code += TypeName(field) + " obj";
  code += ") { return obj.__init(bb_pos + ";
  code += NumToString(field.value.offset) + ", bb)";
  code += "; }\n";
}

// Get a struct by initializing an existing struct.
// Specific to Table.
static void GetStructFieldOfTable(const FieldDef &field,
                                  std::string *code_ptr) {
  std::string &code = *code_ptr;
  code += "  public " + TypeName(field) + " " +
          MakeCamel(field.name, false);
  code += "(";
  code += TypeName(field) + " obj";
  code += OffsetPrefix(field);
  code += "obj.__init(";
  code += field.value.type.struct_def->fixed
            ? "o + bb_pos"
            : "__indirect(o + bb_pos)";
  code += ", bb) : null";
  code += "; }\n";
}

// Get the value of a string.
static void GetStringField(const FieldDef &field, std::string *code_ptr) {
  std::string &code = *code_ptr;
  std::string method_start = "  public " + TypeName(field) + " " +
                             MakeCamel(field.name, false);

  code += method_start + "(";

  code += OffsetPrefix(field) + GenGetter(field.value.type) + "(o) : null";
  code += "; }\n";
}

// Get the value of a union from an object.
static void GetUnionField(const FieldDef &field, std::string *code_ptr) {
  std::string &code = *code_ptr;
  std::string method_start = "  public " + TypeName(field) + " " +
                             MakeCamel(field.name, false);

  code += method_start + "(";

  code += TypeName(field) + " obj" + OffsetPrefix(field) +
          GenGetter(field.value.type);
  code += "(obj, o) : null";
  code += "; }\n";
}

// Get the value of a vector's struct member.
static void GetMemberOfVectorOfStruct(const FieldDef &field,
                                      std::string *code_ptr) {
  std::string &code = *code_ptr;
  auto vectortype = field.value.type.VectorType();

  code += "  public " + TypeName(field) + " " +
          MakeCamel(field.name, false);
  code += "(";
  code += TypeName(field) + " obj, ";
  code += "int j" + OffsetPrefix(field) + "obj.__init(";

  auto index = "__vector(o) + j * " +
               NumToString(InlineSize(vectortype));

  code += vectortype.struct_def->fixed
          ? index
          : "__indirect(" + index + ")";
  code += ", bb";

  code += ") : ";
  code += IsScalar(field.value.type.element) ? "0" : "null";
  code += "; }\n";
}

// Get the value of a vector's non-struct member.
static void GetMemberOfVectorOfNonStruct(const FieldDef &field,
                                         std::string *code_ptr) {
  std::string &code = *code_ptr;
  auto vectortype = field.value.type.VectorType();

  code += "  public " + TypeName(field) + " " +
          MakeCamel(field.name, false);
  code += "(";
  code += "int j" + OffsetPrefix(field) + GenGetter(field.value.type) + "(";

  code += "__vector(o) + j * " +
          NumToString(InlineSize(vectortype));

  code += ") : ";
  code += IsScalar(field.value.type.element) ? "0" : "null";
  code += "; }\n";
}

// End the set of struct accessors.
static void EndStructAccessors(const StructDef &struct_def,
                               std::string *code_ptr) {
}

// Begin the creator function signature.
static void BeginBuilderArgs(const StructDef &struct_def,
                             std::string *code_ptr) {
  std::string &code = *code_ptr;

  code += "\n";
  code += "  public static int create" + struct_def.name;
  code += "(FlatBufferBuilder builder";
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
      // a nested struct, prefix the name with the struct name.
      StructBuilderArgs(*field.value.type.struct_def,
                        (field.value.type.struct_def->name + "_").c_str(),
                        code_ptr);
    } else {
      std::string &code = *code_ptr;
      code += ", " + GenTypeBasic(field.value.type) + " " + nameprefix;
      code += MakeCamel(field.name, false);
    }
  }
}

// End the creator function signature.
static void EndBuilderArgs(const StructDef &struct_def,
                           std::string *code_ptr) {
  std::string &code = *code_ptr;
  code += ") {\n";
}

static void BeginBuilderBody(const StructDef &struct_def,
                             std::string *code_ptr) {
}

// Recusively generate struct construction statements of the form:
// builder.putType(name);
// and insert manual padding.
static void StructBuilderBody(const StructDef &struct_def,
                              const char *nameprefix,
                              std::string *code_ptr) {
  std::string &code = *code_ptr;
  code += "    builder.prep(" + NumToString(struct_def.minalign) + ", ";
  code += NumToString(struct_def.bytesize) + ");\n";
  for (auto it = struct_def.fields.vec.rbegin();
       it != struct_def.fields.vec.rend();
       ++it) {
    auto &field = **it;
    if (field.padding)
      code += "    builder.pad(" + NumToString(field.padding) + ");\n";
    if (IsStruct(field.value.type)) {
      StructBuilderBody(*field.value.type.struct_def,
                        (field.value.type.struct_def->name + "_").c_str(),
                        code_ptr);
    } else {
      code += "    builder.put" + GenMethod(field) + "(";
      code += nameprefix + MakeCamel(field.name, false) + ");\n";
    }
  }
}

static void EndBuilderBody(const StructDef &struct_def,
                           std::string *code_ptr) {
  std::string &code = *code_ptr;
  code += "    return builder.offset();\n  }\n";

  code += "};\n\n";
}

// Get the value of a table's starting offset.
static void GetStartOfTable(const StructDef &struct_def,
                            std::string *code_ptr) {
  std::string &code = *code_ptr;
  code += "\n";
  code += "  public static void start" + struct_def.name;
  code += "(FlatBufferBuilder builder) { builder.startObject(";
  code += NumToString(struct_def.fields.vec.size()) + "); }\n";
}

// Set the value of a table's field.
static void BuildFieldOfTable(const StructDef &struct_def,
                              const FieldDef &field,
                              const int offset,
                              std::string *code_ptr) {
  std::string &code = *code_ptr;
  code += "  public static void add" + MakeCamel(field.name);
  code += "(FlatBufferBuilder builder, " + GenTypeBasic(field.value.type);
  code += " " + MakeCamel(field.name, false) + ") { builder.add";
  code += GenMethod(field) + "(";
  code += NumToString(offset) + ", ";
  code += MakeCamel(field.name, false) + ", " + field.value.constant;
  code += "); }\n";
}

// Set the value of one of the members of a table's vector.
static void BuildVectorOfTable(const FieldDef &field,
                               std::string *code_ptr) {
  std::string &code = *code_ptr;
  code += "  public static void start" + MakeCamel(field.name);
  code += "Vector(FlatBufferBuilder builder, int numElems) ";
  code += "{ builder.startVector(";
  code += NumToString(InlineSize(field.value.type));
  code += ", numElems); }\n";
}

// Get the offset of the end of a table.
static void GetEndOffsetOnTable(const StructDef &struct_def,
                                std::string *code_ptr) {
  std::string &code = *code_ptr;
  code += "  public static int end" + struct_def.name;
  code += "(FlatBufferBuilder builder) { return builder.endObject(); }\n";

  code += "};\n\n";
}

// End a class declaration.
static void EndClass(const StructDef &struct_def, std::string *code_ptr) {
}

// End by noting the conclusion of generated code.
static void EndFile(std::string *code_ptr) {
}

}  // namespace lang


// Generate a struct field, conditioned on its child type(s).
static void GenStructAccessor(const StructDef &struct_def,
                              const FieldDef &field,
                              std::string *code_ptr) {
  lang::Comment(field.doc_comment, code_ptr, "  ");
  // Generate the accessors that don't do object reuse.
  if (field.value.type.base_type == BASE_TYPE_STRUCT) {
    // Calls the accessor that takes an accessor object with a new object.
    lang::GetFieldWithAlloc(field, code_ptr);
  } else if (field.value.type.base_type == BASE_TYPE_VECTOR &&
             field.value.type.element == BASE_TYPE_STRUCT) {
    // Accessors for vectors of structs also take accessor objects, this
    // generates a variant without that argument.
    lang::GetVectorFieldWithAlloc(field, code_ptr);
  }
  if (IsScalar(field.value.type.base_type)) {
    if (struct_def.fixed) {
      lang::GetScalarFieldOfStruct(field, code_ptr);
    } else {
      lang::GetScalarFieldOfTable(field, code_ptr);
    }
  } else {
    switch (field.value.type.base_type) {
      case BASE_TYPE_STRUCT:
        if (struct_def.fixed) {
          lang::GetStructFieldOfStruct(field, code_ptr);
        } else {
          lang::GetStructFieldOfTable(field, code_ptr);
        }
        break;
      case BASE_TYPE_STRING:
        lang::GetStringField(field, code_ptr);
        break;
      case BASE_TYPE_VECTOR: {
        auto vectortype = field.value.type.VectorType();
        if (vectortype.base_type == BASE_TYPE_STRUCT) {
          lang::GetMemberOfVectorOfStruct(field, code_ptr);
        } else {
          lang::GetMemberOfVectorOfNonStruct(field, code_ptr);
        }
        break;
      }
      case BASE_TYPE_UNION:
        lang::GetUnionField(field, code_ptr);
        break;
      default:
        assert(0);
    }
  }
  if (field.value.type.base_type == BASE_TYPE_VECTOR) {
    lang::GetVectorLen(field, code_ptr);
  }
}

// Generate table constructors, conditioned on its members' types.
static void GenTableBuilders(const StructDef &struct_def,
                             std::string *code_ptr) {
  lang::GetStartOfTable(struct_def, code_ptr);

  for (auto it = struct_def.fields.vec.begin();
       it != struct_def.fields.vec.end();
       ++it) {
    auto &field = **it;
    if (field.deprecated) continue;

    auto offset = it - struct_def.fields.vec.begin();
    lang::BuildFieldOfTable(struct_def, field, offset, code_ptr);
    if (field.value.type.base_type == BASE_TYPE_VECTOR) {
      lang::BuildVectorOfTable(field, code_ptr);
    }
  }

  lang::GetEndOffsetOnTable(struct_def, code_ptr);
}

// Generate struct or table methods.
static void GenStruct(const StructDef &struct_def,
                      std::string *code_ptr,
                      StructDef *root_struct_def) {
  if (struct_def.generated) return;

  // Generate a struct accessor class, with methods of the form:
  // public type name() { return bb.getType(i + offset); }
  // or for tables of the form:
  // public type name() {
  //   int o = __offset(offset); return o != 0 ? bb.getType(o + i) : default;
  // }
  lang::Comment(struct_def.doc_comment, code_ptr);
  lang::BeginClass(struct_def, code_ptr);
  lang::BeginStructAccessors(struct_def, code_ptr);
  if (&struct_def == root_struct_def) {
    // Generate a special accessor for the table that has been declared as
    // the root type.
    lang::NewRootTypeFromBuffer(struct_def, code_ptr);
  }
  // Generate the __init method that sets the field in a pre-existing
  // accessor object. This is to allow object reuse.
  lang::InitializeExisting(struct_def, code_ptr);
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
    // Create a set of static methods that allow table construction,
    // of the form:
    // public static void addName(FlatBufferBuilder builder, short name)
    // { builder.addShort(id, name, default); }
    GenTableBuilders(struct_def, code_ptr);
  }
  lang::EndStructAccessors(struct_def, code_ptr);
  lang::EndClass(struct_def, code_ptr);
}
// Generate enum declarations.
static void GenEnum(const EnumDef &enum_def, std::string *code_ptr) {
  if (enum_def.generated) return;
  std::string &code = *code_ptr;

  // Generate enum definitions of the form:
  // public static final int name = value;
  // We use ints rather than the Java Enum feature, because we want them
  // to map directly to how they're used in C/C++ and file formats.
  // That, and Java Enums are expensive, and not universally liked.
  lang::Comment(enum_def.doc_comment, code_ptr);
  lang::BeginEnum(enum_def, code_ptr);
  for (auto it = enum_def.vals.vec.begin();
       it != enum_def.vals.vec.end();
       ++it) {
    auto &ev = **it;
    lang::Comment(ev.doc_comment, code_ptr, "  ");
    lang::EnumMember(enum_def, ev, code_ptr);
  }
  lang::EndEnum(code_ptr);
}

// Returns the function name that is able to read a value of the given type.
static std::string GenGetter(const Type &type) {
  switch (type.base_type) {
    case BASE_TYPE_STRING: return "__string";
    case BASE_TYPE_STRUCT: return "__struct";
    case BASE_TYPE_UNION: return "__union";
    case BASE_TYPE_VECTOR: return GenGetter(type.VectorType());
    default:
      return "bb.get" + (SizeOf(type.base_type) > 1
        ? MakeCamel(GenTypeGet(type))
        : "");
  }
}

// Returns the method name for use with add/put calls.
static std::string GenMethod(const FieldDef &field) {
  return IsScalar(field.value.type.base_type)
    ? MakeCamel(GenTypeBasic(field.value.type))
    : (IsStruct(field.value.type) ? "Struct" : "Offset");
}


// Save out the generated code for a single Java class while adding
// declaration boilerplate.
static bool SaveType(const Parser &parser, const Definition &def,
                     const std::string &classcode, const std::string &path,
                     bool needs_imports) {
  if (!classcode.length()) return true;

  std::string name_space_name;
  std::string name_space_dir = path;
  for (auto it = parser.name_space_.begin();
        it != parser.name_space_.end(); ++it) {
    if (name_space_name.length()) {
      name_space_name += ".";
      name_space_dir += PATH_SEPARATOR;
    }
    name_space_name += *it;
    name_space_dir += *it;
    mkdir(name_space_dir.c_str(), S_IRWXU|S_IRGRP|S_IXGRP|S_IROTH|S_IXOTH);
  }


  std::string code = "";
  lang::BeginFile(name_space_name, needs_imports, &code);
  code += classcode;
  lang::EndFile(&code);
  auto filename = name_space_dir + PATH_SEPARATOR + def.name + ".java";
  return SaveFile(filename.c_str(), code, false);
}

static std::string GenTypeBasic(const Type &type) {
  static const char *ctypename[] = {
    #define FLATBUFFERS_TD(ENUM, IDLTYPE, CTYPE, JTYPE) #JTYPE,
      FLATBUFFERS_GEN_TYPES(FLATBUFFERS_TD)
    #undef FLATBUFFERS_TD
  };
  return ctypename[type.base_type];
}

static std::string GenTypePointer(const Type &type) {
  switch (type.base_type) {
    case BASE_TYPE_STRING:
      return "String";
    case BASE_TYPE_VECTOR:
      return GenTypeGet(type.VectorType());
    case BASE_TYPE_STRUCT:
      return type.struct_def->name;
    case BASE_TYPE_UNION:
      // fall through
    default:
      return "Table";
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
  std::string &code = *code_ptr;

  lang::BeginBuilderArgs(struct_def, code_ptr);
  lang::StructBuilderArgs(struct_def, "", code_ptr);
  lang::EndBuilderArgs(struct_def, code_ptr);

  lang::BeginBuilderBody(struct_def, code_ptr);
  lang::StructBuilderBody(struct_def, "", code_ptr);
  lang::EndBuilderBody(struct_def, code_ptr);
}

}  // namespace java

bool GenerateJava(const Parser &parser,
                  const std::string &path,
                  const std::string &file_name) {
  for (auto it = parser.enums_.vec.begin();
       it != parser.enums_.vec.end(); ++it) {
    std::string enumcode;
    java::GenEnum(**it, &enumcode);
    if (!java::SaveType(parser, **it, enumcode, path, false))
      return false;
  }

  for (auto it = parser.structs_.vec.begin();
       it != parser.structs_.vec.end(); ++it) {
    std::string declcode;
    java::GenStruct(**it, &declcode, parser.root_struct_def);
    if (!java::SaveType(parser, **it, declcode, path, true))
      return false;
  }

  return true;
}

}  // namespace flatbuffers

