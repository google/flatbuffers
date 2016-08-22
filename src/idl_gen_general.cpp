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
#include "flatbuffers/code_generators.h"
#include <algorithm>

namespace flatbuffers {

// Convert an underscore_based_indentifier in to camelCase.
// Also uppercases the first character if first is true.
std::string MakeCamel(const std::string &in, bool first) {
  std::string s;
  for (size_t i = 0; i < in.length(); i++) {
    if (!i && first)
      s += static_cast<char>(toupper(in[0]));
    else if (in[i] == '_' && i + 1 < in.length())
      s += static_cast<char>(toupper(in[++i]));
    else
      s += in[i];
  }
  return s;
}

struct CommentConfig {
  const char *first_line;
  const char *content_line_prefix;
  const char *last_line;
};

// Generate a documentation comment, if available.
void GenComment(const std::vector<std::string> &dc, std::string *code_ptr,
                const CommentConfig *config, const char *prefix) {
  if (dc.begin() == dc.end()) {
    // Don't output empty comment blocks with 0 lines of comment content.
    return;
  }

  std::string &code = *code_ptr;
  if (config != nullptr && config->first_line != nullptr) {
    code += std::string(prefix) + std::string(config->first_line) + "\n";
  }
  std::string line_prefix = std::string(prefix) +
      ((config != nullptr && config->content_line_prefix != nullptr) ?
       config->content_line_prefix : "///");
  for (auto it = dc.begin();
       it != dc.end();
       ++it) {
    code += line_prefix + *it + "\n";
  }
  if (config != nullptr && config->last_line != nullptr) {
    code += std::string(prefix) + std::string(config->last_line) + "\n";
  }
}

// These arrays need to correspond to the IDLOptions::k enum.

struct LanguageParameters {
  IDLOptions::Language language;
  // Whether function names in the language typically start with uppercase.
  bool first_camel_upper;
  const char *file_extension;
  const char *string_type;
  const char *bool_type;
  const char *open_curly;
  const char *const_decl;
  const char *unsubclassable_decl;
  const char *enum_decl;
  const char *enum_separator;
  const char *getter_prefix;
  const char *getter_suffix;
  const char *inheritance_marker;
  const char *namespace_ident;
  const char *namespace_begin;
  const char *namespace_end;
  const char *set_bb_byteorder;
  const char *get_bb_position;
  const char *get_fbb_offset;
  const char *includes;
  CommentConfig comment_config;
};

LanguageParameters language_parameters[] = {
  {
    IDLOptions::kJava,
    false,
    ".java",
    "String",
    "boolean ",
    " {\n",
    " final ",
    "final ",
    "final class ",
    ";\n",
    "()",
    "",
    " extends ",
    "package ",
    ";",
    "",
    "_bb.order(ByteOrder.LITTLE_ENDIAN); ",
    "position()",
    "offset()",
    "import java.nio.*;\nimport java.lang.*;\nimport java.util.*;\n"
      "import com.google.flatbuffers.*;\n\n@SuppressWarnings(\"unused\")\n",
    {
      "/**",
      " *",
      " */",
    },
  },
  {
    IDLOptions::kCSharp,
    true,
    ".cs",
    "string",
    "bool ",
    "\n{\n",
    " readonly ",
    "sealed ",
    "enum ",
    ",\n",
    " { get",
    "} ",
    " : ",
    "namespace ",
    "\n{",
    "\n}\n",
    "",
    "Position",
    "Offset",
    "using System;\nusing FlatBuffers;\n\n",
    {
      nullptr,
      "///",
      nullptr,
    },
  },
  // TODO: add Go support to the general generator.
  // WARNING: this is currently only used for generating make rules for Go.
  {
    IDLOptions::kGo,
    true,
    ".go",
    "string",
    "bool ",
    "\n{\n",
    "const ",
    " ",
    "class ",
    ";\n",
    "()",
    "",
    "",
    "package ",
    "",
    "",
    "",
    "position()",
    "offset()",
    "import (\n\tflatbuffers \"github.com/google/flatbuffers/go\"\n)",
    {
      nullptr,
      "///",
      nullptr,
    },
  }
};

static_assert(sizeof(language_parameters) / sizeof(LanguageParameters) ==
              IDLOptions::kMAX,
              "Please add extra elements to the arrays above.");

namespace general {
class GeneralGenerator : public BaseGenerator {
 public:
  GeneralGenerator(const Parser &parser, const std::string &path,
                   const std::string &file_name)
      : BaseGenerator(parser, path, file_name, "", "."),
        lang_(language_parameters[parser_.opts.lang]) {
    assert(parser_.opts.lang <= IDLOptions::kMAX);
      };
  GeneralGenerator &operator=(const GeneralGenerator &);
  bool generate() {
    std::string one_file_code;

    for (auto it = parser_.enums_.vec.begin(); it != parser_.enums_.vec.end();
         ++it) {
      std::string enumcode;
      auto &enum_def = **it;
      GenEnum(enum_def, &enumcode);
      if (parser_.opts.one_file) {
        one_file_code += enumcode;
      } else {
        if (!SaveType(enum_def.name, *enum_def.defined_namespace,
                      enumcode, false)) return false;
      }
    }

    for (auto it = parser_.structs_.vec.begin();
         it != parser_.structs_.vec.end(); ++it) {
      std::string declcode;
      auto &struct_def = **it;
      GenStruct(struct_def, &declcode);
      if (parser_.opts.one_file) {
        one_file_code += declcode;
      } else {
        if (!SaveType(struct_def.name, *struct_def.defined_namespace,
                      declcode, true)) return false;
      }
    }

    if (parser_.opts.one_file) {
      return SaveType(file_name_, *parser_.namespaces_.back(),
                      one_file_code, true);
    }
    return true;
  }

  // Save out the generated code for a single class while adding
  // declaration boilerplate.
  bool SaveType(const std::string &defname, const Namespace &ns,
  	  const std::string &classcode, bool needs_includes) {
    if (!classcode.length()) return true;

    std::string code;
    code = code + "// " + FlatBuffersGeneratedWarning();
    std::string namespace_name = FullNamespace(".", ns);
    if (!namespace_name.empty()) {
      code += lang_.namespace_ident + namespace_name + lang_.namespace_begin;
      code += "\n\n";
    }
    if (needs_includes) code += lang_.includes;
    code += classcode;
    if (!namespace_name.empty()) code += lang_.namespace_end;
    auto filename = NamespaceDir(ns) + defname + lang_.file_extension;
    return SaveFile(filename.c_str(), code, false);
  }

  const Namespace *CurrentNameSpace() { return parser_.namespaces_.back(); }

  std::string FunctionStart(char upper) {
    return std::string() + (lang_.language == IDLOptions::kJava
                                ? static_cast<char>(tolower(upper))
                                : upper);
}

static bool IsEnum(const Type& type) {
  return type.enum_def != nullptr && IsInteger(type.base_type);
}

std::string GenTypeBasic(const Type &type, bool enableLangOverrides) {
  static const char *gtypename[] = {
    #define FLATBUFFERS_TD(ENUM, IDLTYPE, CTYPE, JTYPE, GTYPE, NTYPE, PTYPE) \
        #JTYPE, #NTYPE, #GTYPE,
      FLATBUFFERS_GEN_TYPES(FLATBUFFERS_TD)
    #undef FLATBUFFERS_TD
  };

  if (enableLangOverrides) {
    if (lang_.language == IDLOptions::kCSharp) {
      if (IsEnum(type)) return WrapInNameSpace(*type.enum_def);
      if (type.base_type == BASE_TYPE_STRUCT) {
          return "Offset<" + WrapInNameSpace(*type.struct_def) + ">";
      }
    }
  }

  return gtypename[type.base_type * IDLOptions::kMAX + lang_.language];
}

std::string GenTypeBasic(const Type &type) {
  return GenTypeBasic(type, true);
}

std::string GenTypePointer(const Type &type) {
  switch (type.base_type) {
    case BASE_TYPE_STRING:
      return lang_.string_type;
    case BASE_TYPE_VECTOR:
      return GenTypeGet(type.VectorType());
    case BASE_TYPE_STRUCT:
      return WrapInNameSpace(*type.struct_def);
    case BASE_TYPE_UNION:
      // Unions in C# use a generic Table-derived type for better type safety
      if (lang_.language == IDLOptions::kCSharp) return "TTable";
      // fall through
    default:
      return "Table";
  }
}

std::string GenTypeGet(const Type &type) {
  return IsScalar(type.base_type)
    ? GenTypeBasic(type)
    : GenTypePointer(type);
}

// Find the destination type the user wants to receive the value in (e.g.
// one size higher signed types for unsigned serialized values in Java).
Type DestinationType(const Type &type, bool vectorelem) {
  if (lang_.language != IDLOptions::kJava) return type;
  switch (type.base_type) {
    // We use int for both uchar/ushort, since that generally means less casting
    // than using short for uchar.
    case BASE_TYPE_UCHAR:  return Type(BASE_TYPE_INT);
    case BASE_TYPE_USHORT: return Type(BASE_TYPE_INT);
    case BASE_TYPE_UINT:   return Type(BASE_TYPE_LONG);
    case BASE_TYPE_VECTOR:
      if (vectorelem)
        return DestinationType(type.VectorType(), vectorelem);
      // else fall thru:
    default: return type;
  }
}

std::string GenOffsetType(const StructDef &struct_def) {
  if(lang_.language == IDLOptions::kCSharp) {
    return "Offset<" + WrapInNameSpace(struct_def) + ">";
  } else {
    return "int";
  }
}

std::string GenOffsetConstruct(const StructDef &struct_def,
                                      const std::string &variable_name)
{
  if(lang_.language == IDLOptions::kCSharp) {
    return "new Offset<" + WrapInNameSpace(struct_def) + ">(" + variable_name + ")";
  }
  return variable_name;
}

std::string GenVectorOffsetType() {
  if(lang_.language == IDLOptions::kCSharp) {
    return "VectorOffset";
  } else {
    return "int";
  }
}

// Generate destination type name
std::string GenTypeNameDest(const Type &type)
{
  return GenTypeGet(DestinationType(type, true));
}

// Mask to turn serialized value into destination type value.
std::string DestinationMask(const Type &type, bool vectorelem) {
  if (lang_.language != IDLOptions::kJava) return "";
  switch (type.base_type) {
    case BASE_TYPE_UCHAR:  return " & 0xFF";
    case BASE_TYPE_USHORT: return " & 0xFFFF";
    case BASE_TYPE_UINT:   return " & 0xFFFFFFFFL";
    case BASE_TYPE_VECTOR:
      if (vectorelem)
        return DestinationMask(type.VectorType(), vectorelem);
      // else fall thru:
    default: return "";
  }
}

// Casts necessary to correctly read serialized data
std::string DestinationCast(const Type &type) {
  if (type.base_type == BASE_TYPE_VECTOR) {
    return DestinationCast(type.VectorType());
  } else {
    switch (lang_.language) {
    case IDLOptions::kJava:
      // Cast necessary to correctly read serialized unsigned values.
      if (type.base_type == BASE_TYPE_UINT) return "(long)";
      break;

    case IDLOptions::kCSharp:
      // Cast from raw integral types to enum.
      if (IsEnum(type)) return "(" + WrapInNameSpace(*type.enum_def) + ")";
      break;

    default:
      break;
    }
  }
  return "";
}

// Cast statements for mutator method parameters.
// In Java, parameters representing unsigned numbers need to be cast down to their respective type.
// For example, a long holding an unsigned int value would be cast down to int before being put onto the buffer.
// In C#, one cast directly cast an Enum to its underlying type, which is essential before putting it onto the buffer.
std::string SourceCast(const Type &type, bool castFromDest) {
  if (type.base_type == BASE_TYPE_VECTOR) {
    return SourceCast(type.VectorType(), castFromDest);
  } else {
    switch (lang_.language) {
      case IDLOptions::kJava:
        if (castFromDest) {
          if (type.base_type == BASE_TYPE_UINT) return "(int)";
          else if (type.base_type == BASE_TYPE_USHORT) return "(short)";
          else if (type.base_type == BASE_TYPE_UCHAR) return "(byte)";
        }
        break;
      case IDLOptions::kCSharp:
        if (IsEnum(type)) return "(" + GenTypeBasic(type, false) + ")";
        break;
      default:
        break;
    }
  }
  return "";
}

std::string SourceCast(const Type &type) {
  return SourceCast(type, true);
}

std::string SourceCastBasic(const Type &type, bool castFromDest) {
  return IsScalar(type.base_type) ? SourceCast(type, castFromDest) : "";
}

std::string SourceCastBasic(const Type &type) {
  return SourceCastBasic(type, true);
}


std::string GenEnumDefaultValue(const Value &value) {
  auto enum_def = value.type.enum_def;
  auto vec = enum_def->vals.vec;
  auto default_value = StringToInt(value.constant.c_str());

  auto result = value.constant;
  for (auto it = vec.begin(); it != vec.end(); ++it) {
    auto enum_val = **it;
    if (enum_val.value == default_value) {
      result = WrapInNameSpace(*enum_def) + "." + enum_val.name;
      break;
    }
  }

  return result;
}

std::string GenDefaultValue(const Value &value, bool enableLangOverrides) {
  if (enableLangOverrides) {
    // handles both enum case and vector of enum case
    if (lang_.language == IDLOptions::kCSharp &&
        value.type.enum_def != nullptr &&
        value.type.base_type != BASE_TYPE_UNION) {
      return GenEnumDefaultValue(value);
    }
  }
  switch (value.type.base_type) {
    case BASE_TYPE_FLOAT: return value.constant + "f";
    case BASE_TYPE_BOOL: return value.constant == "0" ? "false" : "true";
    default: return value.constant;
  }
}

std::string GenDefaultValue(const Value &value) {
  return GenDefaultValue(value, true);
}

std::string GenDefaultValueBasic(const Value &value, bool enableLangOverrides) {
  if (!IsScalar(value.type.base_type)) {
    if (enableLangOverrides) {
      if (lang_.language == IDLOptions::kCSharp) {
        switch (value.type.base_type) {
        case BASE_TYPE_STRING:
          return "default(StringOffset)";
        case BASE_TYPE_STRUCT:
          return "default(Offset<" + WrapInNameSpace(*value.type.struct_def) + ">)";
        case BASE_TYPE_VECTOR:
          return "default(VectorOffset)";
        default:
          break;
        }
      }
    }
    return "0";
  }
  return GenDefaultValue(value, enableLangOverrides);
}

std::string GenDefaultValueBasic(const Value &value) {
  return GenDefaultValueBasic(value, true);
}

void GenEnum(EnumDef &enum_def, std::string *code_ptr) {
  std::string &code = *code_ptr;
  if (enum_def.generated) return;

  // Generate enum definitions of the form:
  // public static (final) int name = value;
  // In Java, we use ints rather than the Enum feature, because we want them
  // to map directly to how they're used in C/C++ and file formats.
  // That, and Java Enums are expensive, and not universally liked.
  GenComment(enum_def.doc_comment, code_ptr, &lang_.comment_config);
  code += std::string("public ") + lang_.enum_decl + enum_def.name;
  if (lang_.language == IDLOptions::kCSharp) {
    code += lang_.inheritance_marker + GenTypeBasic(enum_def.underlying_type, false);
  }
  code += lang_.open_curly;
  if (lang_.language == IDLOptions::kJava) {
    code += "  private " + enum_def.name + "() { }\n";
  }
  for (auto it = enum_def.vals.vec.begin();
       it != enum_def.vals.vec.end();
       ++it) {
    auto &ev = **it;
    GenComment(ev.doc_comment, code_ptr, &lang_.comment_config, "  ");
    if (lang_.language != IDLOptions::kCSharp) {
      code += "  public static";
      code += lang_.const_decl;
      code += GenTypeBasic(enum_def.underlying_type, false);
    }
    code += " " + ev.name + " = ";
    code += NumToString(ev.value);
    code += lang_.enum_separator;
  }

  // Generate a generate string table for enum values.
  // We do not do that for C# where this functionality is native.
  if (lang_.language != IDLOptions::kCSharp) {
    // Problem is, if values are very sparse that could generate really big
    // tables. Ideally in that case we generate a map lookup instead, but for
    // the moment we simply don't output a table at all.
    auto range = enum_def.vals.vec.back()->value -
      enum_def.vals.vec.front()->value + 1;
    // Average distance between values above which we consider a table
    // "too sparse". Change at will.
    static const int kMaxSparseness = 5;
    if (range / static_cast<int64_t>(enum_def.vals.vec.size()) < kMaxSparseness) {
      code += "\n  public static";
      code += lang_.const_decl;
      code += lang_.string_type;
      code += "[] names = { ";
      auto val = enum_def.vals.vec.front()->value;
      for (auto it = enum_def.vals.vec.begin();
        it != enum_def.vals.vec.end();
        ++it) {
        while (val++ != (*it)->value) code += "\"\", ";
        code += "\"" + (*it)->name + "\", ";
      }
      code += "};\n\n";
      code += "  public static ";
      code += lang_.string_type;
      code += " " + MakeCamel("name", lang_.first_camel_upper);
      code += "(int e) { return names[e";
      if (enum_def.vals.vec.front()->value)
        code += " - " + enum_def.vals.vec.front()->name;
      code += "]; }\n";
    }
  }

  // Close the class
  code += "}";
  // Java does not need the closing semi-colon on class definitions.
  code += (lang_.language != IDLOptions::kJava) ? ";" : "";
  code += "\n\n";
}

// Returns the function name that is able to read a value of the given type.
std::string GenGetter(const Type &type) {
  switch (type.base_type) {
    case BASE_TYPE_STRING: return "__string";
    case BASE_TYPE_STRUCT: return "__struct";
    case BASE_TYPE_UNION:  return "__union";
    case BASE_TYPE_VECTOR: return GenGetter(type.VectorType());
    default: {
      std::string getter = "bb." + FunctionStart('G') + "et";
      if (type.base_type == BASE_TYPE_BOOL) {
        getter = "0!=" + getter;
      } else if (GenTypeBasic(type, false) != "byte") {
        getter += MakeCamel(GenTypeBasic(type, false));
      }
      return getter;
    }
  }
}

// Direct mutation is only allowed for scalar fields.
// Hence a setter method will only be generated for such fields.
std::string GenSetter(const Type &type) {
  if (IsScalar(type.base_type)) {
    std::string setter = "bb." + FunctionStart('P') + "ut";
    if (GenTypeBasic(type, false) != "byte" &&
        type.base_type != BASE_TYPE_BOOL) {
      setter += MakeCamel(GenTypeBasic(type, false));
    }
    return setter;
  } else {
    return "";
  }
}

// Returns the method name for use with add/put calls.
std::string GenMethod(const Type &type) {
  return IsScalar(type.base_type)
    ? MakeCamel(GenTypeBasic(type, false))
    : (IsStruct(type) ? "Struct" : "Offset");
}

// Recursively generate arguments for a constructor, to deal with nested
// structs.
void GenStructArgs(const StructDef &struct_def, std::string *code_ptr, const char *nameprefix) {
  std::string &code = *code_ptr;
  for (auto it = struct_def.fields.vec.begin();
       it != struct_def.fields.vec.end();
       ++it) {
    auto &field = **it;
    if (IsStruct(field.value.type)) {
      // Generate arguments for a struct inside a struct. To ensure names
      // don't clash, and to make it obvious these arguments are constructing
      // a nested struct, prefix the name with the field name.
      GenStructArgs(*field.value.type.struct_def, code_ptr,
                    (nameprefix + (field.name + "_")).c_str());
    } else {
      code += ", ";
      code += GenTypeBasic(DestinationType(field.value.type, false));
      code += " ";
      code += nameprefix;
      code += MakeCamel(field.name, lang_.first_camel_upper);
    }
  }
}

// Recusively generate struct construction statements of the form:
// builder.putType(name);
// and insert manual padding.
void GenStructBody(const StructDef &struct_def, std::string *code_ptr, const char *nameprefix) {
  std::string &code = *code_ptr;
  code += "    builder." + FunctionStart('P') + "rep(";
  code += NumToString(struct_def.minalign) + ", ";
  code += NumToString(struct_def.bytesize) + ");\n";
  for (auto it = struct_def.fields.vec.rbegin();
       it != struct_def.fields.vec.rend(); ++it) {
    auto &field = **it;
    if (field.padding) {
      code += "    builder." + FunctionStart('P') + "ad(";
      code += NumToString(field.padding) + ");\n";
    }
    if (IsStruct(field.value.type)) {
      GenStructBody(*field.value.type.struct_def, code_ptr,
                    (nameprefix + (field.name + "_")).c_str());
    } else {
      code += "    builder." + FunctionStart('P') + "ut";
      code += GenMethod(field.value.type) + "(";
      code += SourceCast(field.value.type);
      auto argname = nameprefix + MakeCamel(field.name, lang_.first_camel_upper);
      code += argname;
      code += ");\n";
    }
  }
}

std::string GenByteBufferLength(const char *bb_name) {
  std::string bb_len = bb_name;
  if (lang_.language == IDLOptions::kCSharp) bb_len += ".Length";
  else bb_len += ".array().length";
  return bb_len;
}

std::string GenOffsetGetter(flatbuffers::FieldDef *key_field, const char *num = nullptr) {
  std::string key_offset = "";
  key_offset += "__offset(" +
    NumToString(key_field->value.offset) + ", ";
  if (num) {
    key_offset += num;
    key_offset += (lang_.language == IDLOptions::kCSharp ?
      ".Value, builder.DataBuffer)" : ", _bb)");
  }
  else {
    key_offset += GenByteBufferLength("bb");
    key_offset += " - tableOffset, bb)";
  }
  return key_offset;
}

std::string GenLookupKeyGetter(flatbuffers::FieldDef *key_field) {
  std::string key_getter = "      ";
  key_getter += "tableOffset = __indirect(vectorLocation + 4 * (start + middle)";
  key_getter += ", bb);\n      ";
  if (key_field->value.type.base_type == BASE_TYPE_STRING) {
    key_getter += "comp = " + FunctionStart('C') + "ompareStrings(";
    key_getter += GenOffsetGetter(key_field);
    key_getter += ", key, bb);\n";
  }
  else {
    auto get_val = GenGetter(key_field->value.type) +
      "(" + GenOffsetGetter(key_field) + ")";
    if (lang_.language == IDLOptions::kCSharp) {
      key_getter += "comp = " + get_val + ".CompateTo(key);\n";
    }
    else {
      key_getter += GenTypeGet(key_field->value.type) + " val = ";
      key_getter += get_val + ";\n";
      key_getter += "      comp = val > key ? 1 : val < key ? -1 : 0;\n";
    }
  }
  return key_getter;
}


std::string GenKeyGetter(flatbuffers::FieldDef *key_field) {
  std::string key_getter = "";
  auto data_buffer = (lang_.language == IDLOptions::kCSharp) ?
    "builder.DataBuffer" : "_bb";
  if (key_field->value.type.base_type == BASE_TYPE_STRING) {
    if (lang_.language == IDLOptions::kJava)
      key_getter += " return ";
    key_getter += FunctionStart('C') + "ompareStrings(";
    key_getter += GenOffsetGetter(key_field, "o1") + ", ";
    key_getter += GenOffsetGetter(key_field, "o2") + ", " + data_buffer + ")";
    if (lang_.language == IDLOptions::kJava)
      key_getter += ";";
  }
  else {
    auto field_getter = data_buffer + GenGetter(key_field->value.type).substr(2) +
      "(" + GenOffsetGetter(key_field, "o1") + ")";
    if (lang_.language == IDLOptions::kCSharp) {
      key_getter += field_getter;
      field_getter = data_buffer + GenGetter(key_field->value.type).substr(2) +
        "(" + GenOffsetGetter(key_field, "o2") + ")";
      key_getter += ".CompareTo(" + field_getter + ")";
    }
    else {
      key_getter += "\n    " + GenTypeGet(key_field->value.type) + " val_1 = ";
      key_getter += field_getter + ";\n    " + GenTypeGet(key_field->value.type);
      key_getter += " val_2 = ";
      field_getter = data_buffer + GenGetter(key_field->value.type).substr(2) +
        "(" + GenOffsetGetter(key_field, "o2") + ")";
      key_getter += field_getter + ";\n";
      key_getter += "    return val_1 > val_2 ? 1 : val_1 < val_2 ? -1 : 0;\n ";
    }
  }
  return key_getter;
}

void GenStruct(StructDef &struct_def, std::string *code_ptr) {
  if (struct_def.generated) return;
  std::string &code = *code_ptr;

  // Generate a struct accessor class, with methods of the form:
  // public type name() { return bb.getType(i + offset); }
  // or for tables of the form:
  // public type name() {
  //   int o = __offset(offset); return o != 0 ? bb.getType(o + i) : default;
  // }
  GenComment(struct_def.doc_comment, code_ptr, &lang_.comment_config);
  code += "public ";
  if (lang_.language == IDLOptions::kCSharp &&
      struct_def.attributes.Lookup("csharp_partial")) {
    // generate a partial class for this C# struct/table
    code += "partial ";
  }
  else {
    code += lang_.unsubclassable_decl;
  }
  code += "class " + struct_def.name + lang_.inheritance_marker;
  code += struct_def.fixed ? "Struct" : "Table";
  code += " {\n";
  if (!struct_def.fixed) {
    // Generate a special accessor for the table that when used as the root
    // of a FlatBuffer
    std::string method_name = FunctionStart('G') + "etRootAs" + struct_def.name;
    std::string method_signature = "  public static " + struct_def.name + " " + method_name;

    // create convenience method that doesn't require an existing object
    code += method_signature + "(ByteBuffer _bb) ";
    code += "{ return " + method_name + "(_bb, new " + struct_def.name+ "()); }\n";

    // create method that allows object reuse
    code += method_signature + "(ByteBuffer _bb, " + struct_def.name + " obj) { ";
    code += lang_.set_bb_byteorder;
    code += "return (obj.__init(_bb." + FunctionStart('G');
    code += "etInt(_bb.";
    code += lang_.get_bb_position;
    code += ") + _bb.";
    code += lang_.get_bb_position;
    code += ", _bb)); }\n";
    if (parser_.root_struct_def_ == &struct_def) {
      if (parser_.file_identifier_.length()) {
        // Check if a buffer has the identifier.
        code += "  public static ";
        code += lang_.bool_type + struct_def.name;
        code += "BufferHasIdentifier(ByteBuffer _bb) { return ";
        code += "__has_identifier(_bb, \"" + parser_.file_identifier_;
        code += "\"); }\n";
      }
    }
  }
  // Generate the __init method that sets the field in a pre-existing
  // accessor object. This is to allow object reuse.
  code += "  public " + struct_def.name;
  code += " __init(int _i, ByteBuffer _bb) ";
  code += "{ bb_pos = _i; bb = _bb; return this; }\n\n";
  for (auto it = struct_def.fields.vec.begin();
       it != struct_def.fields.vec.end();
       ++it) {
    auto &field = **it;
    if (field.deprecated) continue;
    GenComment(field.doc_comment, code_ptr, &lang_.comment_config, "  ");
    std::string type_name = GenTypeGet(field.value.type);
    std::string type_name_dest = GenTypeNameDest(field.value.type);
    std::string dest_mask = DestinationMask(field.value.type, true);
    std::string dest_cast = DestinationCast(field.value.type);
    std::string src_cast = SourceCast(field.value.type);
    std::string method_start = "  public " + type_name_dest + " " +
                               MakeCamel(field.name, lang_.first_camel_upper);

    // Most field accessors need to retrieve and test the field offset first,
    // this is the prefix code for that:
    auto offset_prefix = " { int o = __offset(" +
      NumToString(field.value.offset) +
      "); return o != 0 ? ";
    // Generate the accessors that don't do object reuse.
    if (field.value.type.base_type == BASE_TYPE_STRUCT) {
      // Calls the accessor that takes an accessor object with a new object.
      if (lang_.language == IDLOptions::kCSharp) {
        code += method_start + " { get { return Get";
        code += MakeCamel(field.name, lang_.first_camel_upper);
        code += "(new ";
        code += type_name + "()); } }\n";
        method_start = "  public " + type_name_dest + " Get" + MakeCamel(field.name, lang_.first_camel_upper);
      }
      else {
        code += method_start + "() { return ";
        code += MakeCamel(field.name, lang_.first_camel_upper);
        code += "(new ";
        code += type_name + "()); }\n";
      }
    } else if (field.value.type.base_type == BASE_TYPE_VECTOR &&
               field.value.type.element == BASE_TYPE_STRUCT) {
      // Accessors for vectors of structs also take accessor objects, this
      // generates a variant without that argument.
      if (lang_.language == IDLOptions::kCSharp) {
        method_start = "  public " + type_name_dest + " Get" + MakeCamel(field.name, lang_.first_camel_upper);
        code += method_start + "(int j) { return Get";
      } else {
        code += method_start + "(int j) { return ";
      }
      code += MakeCamel(field.name, lang_.first_camel_upper);
      code += "(new ";
      code += type_name + "(), j); }\n";
    } else if (field.value.type.base_type == BASE_TYPE_VECTOR) {
      if (lang_.language == IDLOptions::kCSharp) {
        method_start = "  public " + type_name_dest + " Get" + MakeCamel(field.name, lang_.first_camel_upper);
      }
    } else if (field.value.type.base_type == BASE_TYPE_UNION) {
      if (lang_.language == IDLOptions::kCSharp) {
        // union types in C# use generic Table-derived type for better type safety
        method_start = "  public " + type_name_dest + " Get" + MakeCamel(field.name, lang_.first_camel_upper) + "<TTable>";
        offset_prefix = " where TTable : Table" + offset_prefix;
        type_name = type_name_dest;
      }
    }
    std::string getter = dest_cast + GenGetter(field.value.type);
    code += method_start;
    std::string default_cast = "";
    // only create default casts for c# scalars or vectors of scalars
    if (lang_.language == IDLOptions::kCSharp &&
        (IsScalar(field.value.type.base_type) ||
         (field.value.type.base_type == BASE_TYPE_VECTOR && IsScalar(field.value.type.element)))) {
      // For scalars, default value will be returned by GetDefaultValue(). If the scalar is an enum, GetDefaultValue()
      // returns an actual c# enum that doesn't need to be casted. However, default values for enum elements of
      // vectors are integer literals ("0") and are still casted for clarity.
      if (field.value.type.enum_def == nullptr || field.value.type.base_type == BASE_TYPE_VECTOR) {
          default_cast = "(" + type_name_dest + ")";
      }
    }
    std::string member_suffix = "";
    if (IsScalar(field.value.type.base_type)) {
      code += lang_.getter_prefix;
      member_suffix = lang_.getter_suffix;
      if (struct_def.fixed) {
        code += " { return " + getter;
        code += "(bb_pos + " + NumToString(field.value.offset) + ")";
        code += dest_mask;
      } else {
        code += offset_prefix + getter;
        code += "(o + bb_pos)" + dest_mask + " : " + default_cast;
        code += GenDefaultValue(field.value);
      }
    } else {
      switch (field.value.type.base_type) {
        case BASE_TYPE_STRUCT:
          code += "(" + type_name + " obj";
          if (struct_def.fixed) {
            code += ") { return obj.__init(bb_pos + ";
            code += NumToString(field.value.offset) + ", bb)";
          } else {
            code += ")";
            code += offset_prefix;
            code += "obj.__init(";
            code += field.value.type.struct_def->fixed
                      ? "o + bb_pos"
                      : "__indirect(o + bb_pos)";
            code += ", bb) : null";
          }
          break;
        case BASE_TYPE_STRING:
          code += lang_.getter_prefix;
          member_suffix = lang_.getter_suffix;
          code += offset_prefix + getter + "(o + bb_pos) : null";
          break;
        case BASE_TYPE_VECTOR: {
          auto vectortype = field.value.type.VectorType();
          code += "(";
          if (vectortype.base_type == BASE_TYPE_STRUCT) {
            code += type_name + " obj, ";
            getter = "obj.__init";
          }
          code += "int j)" + offset_prefix + getter +"(";
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
          code += ")" + dest_mask + " : ";

          code += field.value.type.element == BASE_TYPE_BOOL ? "false" :
            (IsScalar(field.value.type.element) ? default_cast + "0" : "null");
          break;
        }
        case BASE_TYPE_UNION:
          code += "(" + type_name + " obj)" + offset_prefix + getter;
          code += "(obj, o) : null";
          break;
        default:
          assert(0);
      }
    }
    code += "; ";
    code += member_suffix;
    code += "}\n";
    if (field.value.type.base_type == BASE_TYPE_VECTOR) {
      code += "  public int " + MakeCamel(field.name, lang_.first_camel_upper);
      code += "Length";
      code += lang_.getter_prefix;
      code += offset_prefix;
      code += "__vector_len(o) : 0; ";
      code += lang_.getter_suffix;
      code += "}\n";
    }
    // Generate a ByteBuffer accessor for strings & vectors of scalars.
    if ((field.value.type.base_type == BASE_TYPE_VECTOR &&
         IsScalar(field.value.type.VectorType().base_type)) ||
         field.value.type.base_type == BASE_TYPE_STRING) {
      switch (lang_.language) {
        case IDLOptions::kJava:
          code += "  public ByteBuffer ";
          code += MakeCamel(field.name, lang_.first_camel_upper);
          code += "AsByteBuffer() { return __vector_as_bytebuffer(";
          code += NumToString(field.value.offset) + ", ";
          code += NumToString(field.value.type.base_type == BASE_TYPE_STRING ? 1 :
                              InlineSize(field.value.type.VectorType()));
          code += "); }\n";
          break;
        case IDLOptions::kCSharp:
          code += "  public ArraySegment<byte>? Get";
          code += MakeCamel(field.name, lang_.first_camel_upper);
          code += "Bytes() { return __vector_as_arraysegment(";
          code += NumToString(field.value.offset);
          code += "); }\n";
          break;
        default:
          break;
      }
    }
  // generate object accessors if is nested_flatbuffer
  auto nested = field.attributes.Lookup("nested_flatbuffer");
  if (nested) {
    auto nested_qualified_name =
      parser_.namespaces_.back()->GetFullyQualifiedName(nested->constant);
    auto nested_type = parser_.structs_.Lookup(nested_qualified_name);
    auto nested_type_name = WrapInNameSpace(*nested_type);
    auto nestedMethodName = MakeCamel(field.name, lang_.first_camel_upper)
      + "As" + nested_type_name;
    auto getNestedMethodName = nestedMethodName;
    if (lang_.language == IDLOptions::kCSharp) {
      getNestedMethodName = "Get" + nestedMethodName;
    }
    code += "  public " + nested_type_name + " ";
    code += nestedMethodName + "() { return ";
    code += getNestedMethodName + "(new " + nested_type_name + "()); }\n";
    code += "  public " + nested_type_name + " " + getNestedMethodName;
    code += "(" + nested_type_name + " obj) { ";
    code += "int o = __offset(" + NumToString(field.value.offset) +"); ";
    code += "return o != 0 ? obj.__init(__indirect(__vector(o)), bb) : null; }\n";
  }
    // generate mutators for scalar fields or vectors of scalars
    if (parser_.opts.mutable_buffer) {
      auto underlying_type = field.value.type.base_type == BASE_TYPE_VECTOR
                    ? field.value.type.VectorType()
                    : field.value.type;
      // boolean parameters have to be explicitly converted to byte representation
      auto setter_parameter = underlying_type.base_type == BASE_TYPE_BOOL ? "(byte)(" + field.name + " ? 1 : 0)" : field.name;
      auto mutator_prefix = MakeCamel("mutate", lang_.first_camel_upper);
      //a vector mutator also needs the index of the vector element it should mutate
      auto mutator_params = (field.value.type.base_type == BASE_TYPE_VECTOR ? "(int j, " : "(") +
                            GenTypeNameDest(underlying_type) + " " +
                            field.name + ") { ";
      auto setter_index = field.value.type.base_type == BASE_TYPE_VECTOR
                    ? "__vector(o) + j * " + NumToString(InlineSize(underlying_type))
                    : (struct_def.fixed ? "bb_pos + " + NumToString(field.value.offset) : "o + bb_pos");
      if (IsScalar(field.value.type.base_type) ||
          (field.value.type.base_type == BASE_TYPE_VECTOR &&
          IsScalar(field.value.type.VectorType().base_type))) {
        code += "  public ";
        code += struct_def.fixed ? "void " : lang_.bool_type;
        code += mutator_prefix + MakeCamel(field.name, true);
        code += mutator_params;
        if (struct_def.fixed) {
          code += GenSetter(underlying_type) + "(" + setter_index + ", ";
          code += src_cast + setter_parameter + "); }\n";
        } else {
          code += "int o = __offset(" + NumToString(field.value.offset) + ");";
          code += " if (o != 0) { " + GenSetter(underlying_type);
          code += "(" + setter_index + ", " + src_cast + setter_parameter + "); return true; } else { return false; } }\n";
        }
      }
    }
  }
  code += "\n";
  flatbuffers::FieldDef *key_field = nullptr;
  if (struct_def.fixed) {
    // create a struct constructor function
    code += "  public static " + GenOffsetType(struct_def) + " ";
    code += FunctionStart('C') + "reate";
    code += struct_def.name + "(FlatBufferBuilder builder";
    GenStructArgs(struct_def, code_ptr, "");
    code += ") {\n";
    GenStructBody(struct_def, code_ptr, "");
    code += "    return ";
    code += GenOffsetConstruct(struct_def, "builder." + std::string(lang_.get_fbb_offset));
    code += ";\n  }\n";
  } else {
    // Generate a method that creates a table in one go. This is only possible
    // when the table has no struct fields, since those have to be created
    // inline, and there's no way to do so in Java.
    bool has_no_struct_fields = true;
    int num_fields = 0;
    for (auto it = struct_def.fields.vec.begin();
         it != struct_def.fields.vec.end(); ++it) {
      auto &field = **it;
      if (field.deprecated) continue;
      if (IsStruct(field.value.type)) {
        has_no_struct_fields = false;
      } else {
        num_fields++;
      }
    }
    if (has_no_struct_fields && num_fields) {
      // Generate a table constructor of the form:
      // public static int createName(FlatBufferBuilder builder, args...)
      code += "  public static " + GenOffsetType(struct_def) + " ";
      code += FunctionStart('C') + "reate" + struct_def.name;
      code += "(FlatBufferBuilder builder";
      for (auto it = struct_def.fields.vec.begin();
           it != struct_def.fields.vec.end(); ++it) {
        auto &field = **it;
        if (field.deprecated) continue;
        code += ",\n      ";
        code += GenTypeBasic(DestinationType(field.value.type, false));
        code += " ";
        code += field.name;
        if (!IsScalar(field.value.type.base_type)) code += "Offset";

        // Java doesn't have defaults, which means this method must always
        // supply all arguments, and thus won't compile when fields are added.
        if (lang_.language != IDLOptions::kJava) {
          code += " = ";
          code += GenDefaultValueBasic(field.value);
        }
      }
      code += ") {\n    builder.";
      code += FunctionStart('S') + "tartObject(";
      code += NumToString(struct_def.fields.vec.size()) + ");\n";
      for (size_t size = struct_def.sortbysize ? sizeof(largest_scalar_t) : 1;
           size;
           size /= 2) {
        for (auto it = struct_def.fields.vec.rbegin();
             it != struct_def.fields.vec.rend(); ++it) {
          auto &field = **it;
          if (!field.deprecated &&
              (!struct_def.sortbysize ||
               size == SizeOf(field.value.type.base_type))) {
            code += "    " + struct_def.name + ".";
            code += FunctionStart('A') + "dd";
            code += MakeCamel(field.name) + "(builder, " + field.name;
            if (!IsScalar(field.value.type.base_type)) code += "Offset";
            code += ");\n";
          }
        }
      }
      code += "    return " + struct_def.name + ".";
      code += FunctionStart('E') + "nd" + struct_def.name;
      code += "(builder);\n  }\n\n";
    }
    // Generate a set of static methods that allow table construction,
    // of the form:
    // public static void addName(FlatBufferBuilder builder, short name)
    // { builder.addShort(id, name, default); }
    // Unlike the Create function, these always work.
    code += "  public static void " + FunctionStart('S') + "tart";
    code += struct_def.name;
    code += "(FlatBufferBuilder builder) { builder.";
    code += FunctionStart('S') + "tartObject(";
    code += NumToString(struct_def.fields.vec.size()) + "); }\n";
    for (auto it = struct_def.fields.vec.begin();
         it != struct_def.fields.vec.end(); ++it) {
      auto &field = **it;
      if (field.deprecated) continue;
      if (field.key) key_field = &field;
      code += "  public static void " + FunctionStart('A') + "dd";
      code += MakeCamel(field.name);
      code += "(FlatBufferBuilder builder, ";
      code += GenTypeBasic(DestinationType(field.value.type, false));
      auto argname = MakeCamel(field.name, false);
      if (!IsScalar(field.value.type.base_type)) argname += "Offset";
      code += " " + argname + ") { builder." + FunctionStart('A') + "dd";
      code += GenMethod(field.value.type) + "(";
      code += NumToString(it - struct_def.fields.vec.begin()) + ", ";
      code += SourceCastBasic(field.value.type);
      code += argname;
      if(!IsScalar(field.value.type.base_type) && field.value.type.base_type != BASE_TYPE_UNION && lang_.language == IDLOptions::kCSharp) {
        code += ".Value";
      }
      code += ", " + GenDefaultValue(field.value, false);
      code += "); }\n";
      if (field.value.type.base_type == BASE_TYPE_VECTOR) {
        auto vector_type = field.value.type.VectorType();
        auto alignment = InlineAlignment(vector_type);
        auto elem_size = InlineSize(vector_type);
        if (!IsStruct(vector_type)) {
          // Generate a method to create a vector from a Java array.
          code += "  public static " + GenVectorOffsetType() + " " + FunctionStart('C') + "reate";
          code += MakeCamel(field.name);
          code += "Vector(FlatBufferBuilder builder, ";
          code += GenTypeBasic(vector_type) + "[] data) ";
          code += "{ builder." + FunctionStart('S') + "tartVector(";
          code += NumToString(elem_size);
          code += ", data." + FunctionStart('L') + "ength, ";
          code += NumToString(alignment);
          code += "); for (int i = data.";
          code += FunctionStart('L') + "ength - 1; i >= 0; i--) builder.";
          code += FunctionStart('A') + "dd";
          code += GenMethod(vector_type);
          code += "(";
          code += SourceCastBasic(vector_type, false);
          code += "data[i]";
          if (lang_.language == IDLOptions::kCSharp &&
            (vector_type.base_type == BASE_TYPE_STRUCT || vector_type.base_type == BASE_TYPE_STRING))
            code += ".Value";
          code += "); return ";
          code += "builder." + FunctionStart('E') + "ndVector(); }\n";
        }
        // Generate a method to start a vector, data to be added manually after.
        code += "  public static void " + FunctionStart('S') + "tart";
        code += MakeCamel(field.name);
        code += "Vector(FlatBufferBuilder builder, int numElems) ";
        code += "{ builder." + FunctionStart('S') + "tartVector(";
        code += NumToString(elem_size);
        code += ", numElems, " + NumToString(alignment);
        code += "); }\n";
      }
    }
    code += "  public static " + GenOffsetType(struct_def) + " ";
    code += FunctionStart('E') + "nd" + struct_def.name;
    code += "(FlatBufferBuilder builder) {\n    int o = builder.";
    code += FunctionStart('E') + "ndObject();\n";
    for (auto it = struct_def.fields.vec.begin();
         it != struct_def.fields.vec.end();
         ++it) {
      auto &field = **it;
      if (!field.deprecated && field.required) {
        code += "    builder." + FunctionStart('R') + "equired(o, ";
        code += NumToString(field.value.offset);
        code += ");  // " + field.name + "\n";
      }
    }
    code += "    return " + GenOffsetConstruct(struct_def, "o") + ";\n  }\n";
    if (parser_.root_struct_def_ == &struct_def) {
      code += "  public static void ";
      code += FunctionStart('F') + "inish" + struct_def.name;
      code += "Buffer(FlatBufferBuilder builder, " + GenOffsetType(struct_def) + " offset) {";
      code += " builder." + FunctionStart('F') + "inish(offset";
      if (lang_.language == IDLOptions::kCSharp) {
        code += ".Value";
      }

      if (parser_.file_identifier_.length())
        code += ", \"" + parser_.file_identifier_ + "\"";
      code += "); }\n";
    }
  }
  if (struct_def.has_key) {
    if (lang_.language == IDLOptions::kJava) {
      code += "\n  @Override\n  protected int keysCompare(";
      code += "Integer o1, Integer o2, ByteBuffer _bb) {";
      code += GenKeyGetter(key_field);
      code += " }\n";
    }
    else {
      code += "\n  public static VectorOffset ";
      code += "CreateMySortedVectorOfTables(FlatBufferBuilder builder, ";
      code += "Offset<" + struct_def.name + ">";
      code += "[] offsets) {\n";
      code += "    Array.Sort(offsets, (Offset<" + struct_def.name +
        "> o1, Offset<" + struct_def.name + "> o2) => " + GenKeyGetter(key_field);
      code += ");\n";
      code += "    return builder.CreateVectorOfTables(offsets);\n  }\n";
    }

    code += "\n  public static " + struct_def.name + " " + FunctionStart('L');
    code += "ookupByKey(" + GenVectorOffsetType();
    code += " vectorOffset, " + GenTypeGet(key_field->value.type);
    code += " key, ByteBuffer bb) {\n";
    code += "    int vectorLocation = " + GenByteBufferLength("bb");
    code += " - vectorOffset.Value;\n    int span = ";
    code += "bb." + FunctionStart('G') + "etInt(vectorLocation), ";
    code += "middle, start = 0, comp, tableOffset; \n";
    code += "    vectorLocation += 4;\n";
    code += "    while (span != 0) {\n";
    code += "      int middle = span / 2;\n";
    code += GenLookupKeyGetter(key_field);
    code += "      if (comp > 0) span = middle;\n";
    code += "      else if (comp < 0) {\n";
    code += "        middle++;\n";
    code += "        start += middle;\n";
    code += "        span -= middle;\n";
    code += "      }\n";
    code += "      else return new " + struct_def.name;
    code += "().__init(tableOffset, bb);\n";
    code += "    }\n";
    code += "    return null;\n";
    code += "  }\n";
  }
  code += "}";
  // Java does not need the closing semi-colon on class definitions.
  code += (lang_.language != IDLOptions::kJava) ? ";" : "";
  code += "\n\n";
}
    const LanguageParameters & lang_;
};
}  // namespace general

bool GenerateGeneral(const Parser &parser, const std::string &path,
                     const std::string &file_name) {
  general::GeneralGenerator generator(parser, path, file_name);
  return generator.generate();
}

std::string GeneralMakeRule(const Parser &parser, const std::string &path,
                            const std::string &file_name) {
  assert(parser.opts.lang <= IDLOptions::kMAX);
  auto lang = language_parameters[parser.opts.lang];

  std::string make_rule;

  for (auto it = parser.enums_.vec.begin(); it != parser.enums_.vec.end();
       ++it) {
    auto &enum_def = **it;
    if (make_rule != "") make_rule += " ";
    std::string directory =
        BaseGenerator::NamespaceDir(parser, path, *enum_def.defined_namespace) +
        kPathSeparator;
    make_rule += directory + enum_def.name + lang.file_extension;
  }

  for (auto it = parser.structs_.vec.begin(); it != parser.structs_.vec.end();
       ++it) {
    auto &struct_def = **it;
    if (make_rule != "") make_rule += " ";
    std::string directory =
        BaseGenerator::NamespaceDir(parser, path, *struct_def.defined_namespace) +
        kPathSeparator;
    make_rule += directory + struct_def.name + lang.file_extension;
  }

  make_rule += ": ";
  auto included_files = parser.GetIncludedFilesRecursive(file_name);
  for (auto it = included_files.begin(); it != included_files.end(); ++it) {
    make_rule += " " + *it;
  }
  return make_rule;
}

std::string BinaryFileName(const Parser &parser,
                           const std::string &path,
                           const std::string &file_name) {
  auto ext = parser.file_extension_.length() ? parser.file_extension_ : "bin";
  return path + file_name + "." + ext;
}

bool GenerateBinary(const Parser &parser,
                    const std::string &path,
                    const std::string &file_name) {
  return !parser.builder_.GetSize() ||
         flatbuffers::SaveFile(
           BinaryFileName(parser, path, file_name).c_str(),
           reinterpret_cast<char *>(parser.builder_.GetBufferPointer()),
           parser.builder_.GetSize(),
           true);
}

std::string BinaryMakeRule(const Parser &parser,
                           const std::string &path,
                           const std::string &file_name) {
  if (!parser.builder_.GetSize()) return "";
  std::string filebase = flatbuffers::StripPath(
      flatbuffers::StripExtension(file_name));
  std::string make_rule = BinaryFileName(parser, path, filebase) + ": " +
      file_name;
  auto included_files = parser.GetIncludedFilesRecursive(
      parser.root_struct_def_->file);
  for (auto it = included_files.begin();
       it != included_files.end(); ++it) {
    make_rule += " " + *it;
  }
  return make_rule;
}

}  // namespace flatbuffers
