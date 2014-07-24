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
namespace csharp {

static std::string GenTypeBasic(const Type &type) {
  static const char *ctypename[] = {
    #define FLATBUFFERS_TD(ENUM, IDLTYPE, CTYPE, JTYPE) #JTYPE,
      FLATBUFFERS_GEN_TYPES(FLATBUFFERS_TD)
    #undef FLATBUFFERS_TD
  };
  return ctypename[type.base_type];
}

static std::string GenTypeGet(const Type &type);

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
      return "Table";
  }
}

static std::string GenTypeGet(const Type &type) {
  return IsScalar(type.base_type)
    ? GenTypeBasic(type)
    : GenTypePointer(type);
}

static void GenComment(const std::string &dc,
                       std::string *code_ptr,
                       const char *prefix = "") {
  std::string &code = *code_ptr;
  if (dc.length()) {
    code += std::string(prefix) + "/*" + dc + "*/\n";
  }
}

// Convert an underscore_based_indentifier in to camelCase.
// Also uppercases the first character if first is true.
static std::string MakeCamel(const std::string &in, bool first = true) {
  std::string s;
  for (size_t i = 0; i < in.length(); i++) {
    if (!i && first) s += toupper(in[0]);
    else if (in[i] == '_' && i + 1 < in.length()) s += toupper(in[++i]);
    else s += in[i];
  }
  return s;
}

static void GenEnum(EnumDef &enum_def, std::string *code_ptr) {
  std::string &code = *code_ptr;
  if (enum_def.generated) return;

  // Generate enum definitions of the form:
  // public static int Name = value;
  // We use ints rather than the C# Enum feature, because we want them
  // to map directly to how they're used in C/C++ and file formats.
  GenComment(enum_def.doc_comment, code_ptr);
  code += "public class " + enum_def.name + "\n{\n";
  for (auto it = enum_def.vals.vec.begin();
       it != enum_def.vals.vec.end();
       ++it) {
    auto &ev = **it;
    GenComment(ev.doc_comment, code_ptr, "  ");
    code += "  public static " + GenTypeBasic(enum_def.underlying_type);
    code += " " + ev.name + " = ";
    code += NumToString(ev.value) + ";\n";
  }
  code += "};\n\n";
}

// Returns the function name that is able to read a value of the given type.
static std::string GenGetter(const Type &type) {
  switch (type.base_type) {
    case BASE_TYPE_STRING: return "__string";
    case BASE_TYPE_STRUCT: return "__struct";
    case BASE_TYPE_UNION: return "__union";
    case BASE_TYPE_VECTOR: return GenGetter(type.VectorType());
    default:
      return "bb.Get" + (SizeOf(type.base_type) > 1
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

// Recursively generate arguments for a constructor, to deal with nested
// structs.
static void GenStructArgs(const StructDef &struct_def, std::string *code_ptr,
                          const char *nameprefix) {
  std::string &code = *code_ptr;
  for (auto it = struct_def.fields.vec.begin();
       it != struct_def.fields.vec.end();
       ++it) {
    auto &field = **it;
    if (IsStruct(field.value.type)) {
      // Generate arguments for a struct inside a struct. To ensure names
      // don't clash, and to make it obvious these arguments are constructing
      // a nested struct, prefix the name with the struct name.
      GenStructArgs(*field.value.type.struct_def, code_ptr,
                    (field.value.type.struct_def->name + "_").c_str());
    } else {
      code += ", " + GenTypeBasic(field.value.type) + " " + nameprefix;
	  code += MakeCamel(field.name, true);
    }
  }
}

// Recusively generate struct construction statements of the form:
// builder.PutType(name);
// and insert manual padding.
static void GenStructBody(const StructDef &struct_def, std::string *code_ptr,
                          const char *nameprefix) {
  std::string &code = *code_ptr;
  code += "    builder.Prep(" + NumToString(struct_def.minalign) + ", ";
  code +=  NumToString(struct_def.bytesize) + ");\n"; 
  for (auto it = struct_def.fields.vec.rbegin();
       it != struct_def.fields.vec.rend();
       ++it) {
    auto &field = **it;
    if (field.padding)
      code += "    builder.Pad(" + NumToString(field.padding) + ");\n";
    if (IsStruct(field.value.type)) {
      GenStructBody(*field.value.type.struct_def, code_ptr,
                    (field.value.type.struct_def->name + "_").c_str());
    } else {
      code += "    builder.Put" + GenMethod(field) + "(";
      code += nameprefix + MakeCamel(field.name, true) + ");\n";
    }
  }
}

static void GenStruct(StructDef &struct_def,
                      std::string *code_ptr,
                      StructDef *root_struct_def) {
  if (struct_def.generated) return;
  std::string &code = *code_ptr;

  // Generate a struct accessor class, with methods of the form:
  // public type Name() { return bb.GetType(i + offset); }
  // or for tables of the form:
  // public type Name() {
  //   int o = __offset(offset); return o != 0 ? bb.GetType(o + i) : default;
  // }
  GenComment(struct_def.doc_comment, code_ptr);
  code += "public class " + struct_def.name + " : ";
  code += struct_def.fixed ? "Struct" : "Table";
  code += " {\n";
  if (&struct_def == root_struct_def) {
    // Generate a special accessor for the table that has been declared as
    // the root type.
    code += "  public static " + struct_def.name + " GetRootAs";
    code += struct_def.name;
    code += "(ByteBuffer _bb, int offset) { ";
	// Endian handled by .NET ByteBuffer impl
    code += "return (new " + struct_def.name;
    code += "()).__init(_bb.GetInt(offset) + offset, _bb); }\n";
  }
  // Generate the __init method that sets the field in a pre-existing
  // accessor object. This is to allow object reuse.
  code += "  public " + struct_def.name;
  code += " __init(int _i, ByteBuffer _bb) ";
  code += "{ bb_pos = _i; bb = _bb; return this; }\n";
  for (auto it = struct_def.fields.vec.begin();
       it != struct_def.fields.vec.end();
       ++it) {
    auto &field = **it;
    if (field.deprecated) continue;
    GenComment(field.doc_comment, code_ptr, "  ");
    std::string type_name = GenTypeGet(field.value.type);
    std::string method_start = "  public " + type_name + " " +
                               MakeCamel(field.name, true);
    // Generate the accessors that don't do object reuse.
    if (field.value.type.base_type == BASE_TYPE_STRUCT) {
      // Calls the accessor that takes an accessor object with a new object.
      code += method_start + "() { return " + MakeCamel(field.name, true);
      code += "(new ";
      code += type_name + "()); }\n";
    } else if (field.value.type.base_type == BASE_TYPE_VECTOR &&
               field.value.type.element == BASE_TYPE_STRUCT) {
      // Accessors for vectors of structs also take accessor objects, this
      // generates a variant without that argument.
      code += method_start + "(int j) { return " + MakeCamel(field.name, true);
      code += "(new ";
      code += type_name + "(), j); }\n";
    }
    std::string getter = GenGetter(field.value.type);
    code += method_start + "(";
    // Most field accessors need to retrieve and test the field offset first,
    // this is the prefix code for that:
    auto offset_prefix = ") { int o = __offset(" +
                         NumToString(field.value.offset) +
                         "); return o != 0 ? ";
    if (IsScalar(field.value.type.base_type)) {
      if (struct_def.fixed) {
        code += ") { return " + getter;
        code += "(bb_pos + " + NumToString(field.value.offset) + ")";
      } else {
        code += offset_prefix + getter;
		code += "(o + bb_pos) : (";
		code += type_name;
		code += ")" + field.value.constant;
      }
    } else {
      switch (field.value.type.base_type) {
        case BASE_TYPE_STRUCT:
          code += type_name + " obj";
          if (struct_def.fixed) {
            code += ") { return obj.__init(bb_pos + ";
            code += NumToString(field.value.offset) + ", bb)";
          } else {
            code += offset_prefix;
            code += "obj.__init(";
            code += field.value.type.struct_def->fixed
                      ? "o + bb_pos"
                      : "__indirect(o + bb_pos)";
            code += ", bb) : null";
          }
          break;
        case BASE_TYPE_STRING:
          code += offset_prefix + getter +"(o) : null";
          break;
        case BASE_TYPE_VECTOR: {
          auto vectortype = field.value.type.VectorType();
          if (vectortype.base_type == BASE_TYPE_STRUCT) {
            code += type_name + " obj, ";
            getter = "obj.__init";
          }
          code += "int j" + offset_prefix + getter +"(";
          auto index = "__vector(o) + j * " +
                       NumToString(InlineSize(vectortype));
          if (vectortype.base_type == BASE_TYPE_STRUCT) {
            code += vectortype.struct_def->fixed
                      ? index
                      : "__indirect(" + index + ")";
            code += ", bb";
          } else {
            code += index;
          }
          code += ") : ";
		  code += IsScalar(field.value.type.element) ? "(" + type_name + ")0" : "null";
          break;
        }
        case BASE_TYPE_UNION:
          code += type_name + " obj" + offset_prefix + getter;
          code += "(obj, o) : null";
          break;
        default:
          assert(0);
      }
    }
    code += "; }\n";
    if (field.value.type.base_type == BASE_TYPE_VECTOR) {
      code += "  public int " + MakeCamel(field.name, true) + "Length(";
      code += offset_prefix;
      code += "__vector_len(o) : 0; }\n";
    }
  }
  code += "\n";
  if (struct_def.fixed) {
    // create a struct constructor function
    code += "  public static int Create" + struct_def.name;
    code += "(FlatBufferBuilder builder";
    GenStructArgs(struct_def, code_ptr, "");
    code += ") {\n";
    GenStructBody(struct_def, code_ptr, "");
    code += "    return builder.Offset;\n  }\n";
  } else {
    // Create a set of static methods that allow table construction,
    // of the form:
    // public static void AddName(FlatBufferBuilder builder, short name)
    // { builder.AddShort(id, name, default); }
    code += "  public static void Start" + struct_def.name;
    code += "(FlatBufferBuilder builder) { builder.StartObject(";
    code += NumToString(struct_def.fields.vec.size()) + "); }\n";
    for (auto it = struct_def.fields.vec.begin();
         it != struct_def.fields.vec.end();
         ++it) {
      auto &field = **it;
      if (field.deprecated) continue;
      code += "  public static void Add" + MakeCamel(field.name);
      code += "(FlatBufferBuilder builder, " + GenTypeBasic(field.value.type);
      code += " " + MakeCamel(field.name, false) + ") { builder.Add";
      code += GenMethod(field) + "(";
      code += NumToString(it - struct_def.fields.vec.begin()) + ", ";
	  code += MakeCamel(field.name, false) + ", " + field.value.constant;
      code += "); }\n";
      if (field.value.type.base_type == BASE_TYPE_VECTOR) {
        code += "  public static void Start" + MakeCamel(field.name);
        code += "Vector(FlatBufferBuilder builder, int numElems) ";
        code += "{ builder.StartVector(";
        code += NumToString(InlineSize(field.value.type));
        code += ", numElems); }\n";
      }
    }
    code += "  public static int End" + struct_def.name;
    code += "(FlatBufferBuilder builder) { return builder.EndObject(); }\n";
  }
  code += "};\n\n";
}

// Save out the generated code for a single Java class while adding
// declaration boilerplate.
static bool SaveClass(const Parser &parser, const Definition &def,
                      const std::string &classcode, const std::string &path) {
  if (!classcode.length()) return true;

  std::string name_space_csharp;
  std::string name_space_dir = path;
  for (auto it = parser.name_space_.begin();
        it != parser.name_space_.end(); ++it) {
    if (name_space_csharp.length()) {
      name_space_csharp += ".";
      name_space_dir += PATH_SEPARATOR;
    }
    name_space_csharp += *it;
    name_space_dir += *it;
    mkdir(name_space_dir.c_str(), S_IRWXU|S_IRGRP|S_IXGRP|S_IROTH|S_IXOTH);
  }

  std::string code = "// automatically generated, do not modify\n\n";
  code += "namespace " + name_space_csharp + "\n{\n\n";
// Other usings
  code += "using FlatBuffers;\n\n";
  code += classcode;
  code += "\n}\n";
  auto filename = name_space_dir + PATH_SEPARATOR + def.name + ".cs";
  return SaveFile(filename.c_str(), code, false);
}

}  // namespace csharp

bool GenerateCSharp(const Parser &parser,
                  const std::string &path,
                  const std::string &file_name) {
  using namespace csharp;

  for (auto it = parser.enums_.vec.begin();
       it != parser.enums_.vec.end(); ++it) {
    std::string enumcode;
    GenEnum(**it, &enumcode);
    if (!SaveClass(parser, **it, enumcode, path))
      return false;
  }

  for (auto it = parser.structs_.vec.begin();
       it != parser.structs_.vec.end(); ++it) {
    std::string declcode;
    GenStruct(**it, &declcode, parser.root_struct_def);
    if (!SaveClass(parser, **it, declcode, path))
      return false;
  }

  return true;
}

}  // namespace flatbuffers

