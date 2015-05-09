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
#include <vector>

#include "flatbuffers/flatbuffers.h"
#include "flatbuffers/idl.h"
#include "flatbuffers/util.h"

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

// These arrays need to correspond to the GeneratorOptions::k enum.

struct LanguageParameters {
  GeneratorOptions::Language language;
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
    GeneratorOptions::kJava,
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
      "import com.google.flatbuffers.*;\n\n",
    {
      "/**",
      " *",
      " */",
    },
  },
  {
    GeneratorOptions::kCSharp,
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
    "using FlatBuffers;\n\n",
    {
      nullptr,
      "///",
      nullptr,
    },
  },
  // TODO(mystal): add Go support to the general generator.
  // WARNING: this is currently only used for generating make rules for Go.
  {
    GeneratorOptions::kGo,
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
              GeneratorOptions::kMAX,
              "Please add extra elements to the arrays above.");

static std::string FunctionStart(const LanguageParameters &lang, char upper) {
  return std::string() +
      (lang.language == GeneratorOptions::kJava
         ? static_cast<char>(tolower(upper))
         : upper);
}

static std::string GenTypeBasic(const LanguageParameters &lang,
                                const Type &type) {
  static const char *gtypename[] = {
    #define FLATBUFFERS_TD(ENUM, IDLTYPE, CTYPE, JTYPE, GTYPE, NTYPE) \
        #JTYPE, #NTYPE, #GTYPE,
      FLATBUFFERS_GEN_TYPES(FLATBUFFERS_TD)
    #undef FLATBUFFERS_TD
  };

  return gtypename[type.base_type * GeneratorOptions::kMAX + lang.language];
}

// Generate type to be used in user-facing API
static std::string GenTypeForUser(const LanguageParameters &lang,
                                  const Type &type) {
  if (lang.language == GeneratorOptions::kCSharp) {
    if (type.enum_def != nullptr &&
          type.base_type != BASE_TYPE_UNION) return type.enum_def->name;
  }
  return GenTypeBasic(lang, type);
}

static std::string GenTypeGet(const LanguageParameters &lang,
                              const Type &type);

static std::string GenTypePointer(const LanguageParameters &lang,
                                  const Type &type) {
  switch (type.base_type) {
    case BASE_TYPE_STRING:
      return lang.string_type;
    case BASE_TYPE_VECTOR:
      return GenTypeGet(lang, type.VectorType());
    case BASE_TYPE_STRUCT:
      return type.struct_def->name;
    case BASE_TYPE_UNION:
      // fall through
    default:
      return "Table";
  }
}

static std::string GenTypeGet(const LanguageParameters &lang,
                              const Type &type) {
  return IsScalar(type.base_type)
    ? GenTypeBasic(lang, type)
    : GenTypePointer(lang, type);
}

// Find the destination type the user wants to receive the value in (e.g.
// one size higher signed types for unsigned serialized values in Java).
static Type DestinationType(const LanguageParameters &lang, const Type &type,
                            bool vectorelem) {
  if (lang.language != GeneratorOptions::kJava) return type;
  switch (type.base_type) {
    case BASE_TYPE_UCHAR:  return Type(BASE_TYPE_INT);
    case BASE_TYPE_USHORT: return Type(BASE_TYPE_INT);
    case BASE_TYPE_UINT:   return Type(BASE_TYPE_LONG);
    case BASE_TYPE_VECTOR:
      if (vectorelem)
        return DestinationType(lang, type.VectorType(), vectorelem);
      // else fall thru:
    default: return type;
  }
}

// Mask to turn serialized value into destination type value.
static std::string DestinationMask(const LanguageParameters &lang,
                                   const Type &type, bool vectorelem) {
  if (lang.language != GeneratorOptions::kJava) return "";
  switch (type.base_type) {
    case BASE_TYPE_UCHAR:  return " & 0xFF";
    case BASE_TYPE_USHORT: return " & 0xFFFF";
    case BASE_TYPE_UINT:   return " & 0xFFFFFFFFL";
    case BASE_TYPE_VECTOR:
      if (vectorelem)
        return DestinationMask(lang, type.VectorType(), vectorelem);
      // else fall thru:
    default: return "";
  }
}

// Casts necessary to correctly read serialized data
static std::string DestinationCast(const LanguageParameters &lang,
                                   const Type &type) {
  switch (lang.language) {
    case GeneratorOptions::kJava:
      // Cast necessary to correctly read serialized unsigned values.
      if (type.base_type == BASE_TYPE_UINT ||
          (type.base_type == BASE_TYPE_VECTOR &&
           type.element == BASE_TYPE_UINT)) return "(long)";
      break;

    case GeneratorOptions::kCSharp:
      // Cast from raw integral types to enum
      if (type.enum_def != nullptr && type.base_type != BASE_TYPE_UNION)
        return "(" + type.enum_def->name + ")";

    default:
      break;
  }
  return "";
}

// Read value and possibly process it to get proper value
static std::string DestinationValue(const LanguageParameters &lang,
  const std::string &name,
  const Type &type) {
  std::string type_mask = DestinationMask(lang, type, false);
  // is a typecast needed? (for C# enums and unsigned values in Java)
  if (type_mask.length() ||
    (lang.language == GeneratorOptions::kCSharp &&
    type.enum_def != nullptr &&
    type.base_type != BASE_TYPE_UNION)) {
    return "(" + GenTypeBasic(lang, type) + ")(" + name + type_mask + ")";
  } else {
    return name;
  }
}

static std::string GenDefaultValue(const Value &value) {
  return value.type.base_type == BASE_TYPE_BOOL
           ? (value.scalars.BOOL == 0 ? "false" : "true")
           : value.string;
}

static void GenEnum(const LanguageParameters &lang, const EnumDef &enum_def,
                    std::string *code_ptr) {
  std::string &code = *code_ptr;
  if (enum_def.generated) return;

  // Generate enum definitions of the form:
  // public static (final) int name = value;
  // In Java, we use ints rather than the Enum feature, because we want them
  // to map directly to how they're used in C/C++ and file formats.
  // That, and Java Enums are expensive, and not universally liked.
  GenComment(enum_def.doc_comment, code_ptr, &lang.comment_config);
  code += std::string("public ") + lang.enum_decl + enum_def.name;
  if (lang.language == GeneratorOptions::kCSharp) {
    code += lang.inheritance_marker
            + GenTypeBasic(lang, enum_def.underlying_type);
  }
  code += lang.open_curly;
  if (lang.language == GeneratorOptions::kJava) {
    code += "  private " + enum_def.name + "() { }\n";
  }
  for (auto it = enum_def.vals.vec.begin();
       it != enum_def.vals.vec.end();
       ++it) {
    auto &ev = **it;
    GenComment(ev.doc_comment, code_ptr, &lang.comment_config, "  ");
    if (lang.language != GeneratorOptions::kCSharp) {
      code += "  public static";
      code += lang.const_decl;
      code += GenTypeBasic(lang, enum_def.underlying_type);
    }
    code += " " + ev.name + " = ";
    code += NumToString(ev.value);
    code += lang.enum_separator;
  }

  // Generate a generate string table for enum values.
  // We do not do that for C# where this functionality is native.
  if (lang.language != GeneratorOptions::kCSharp) {
    // Problem is, if values are very sparse that could generate really big
    // tables. Ideally in that case we generate a map lookup instead, but for
    // the moment we simply don't output a table at all.
    auto range = enum_def.vals.vec.back()->value -
      enum_def.vals.vec.front()->value + 1;
    // Average distance between values above which we consider a table
    // "too sparse". Change at will.
    static const int kMaxSparseness = 5;
    if (range / static_cast<int64_t>(enum_def.vals.vec.size())
        < kMaxSparseness) {
      code += "\n  private static";
      code += lang.const_decl;
      code += lang.string_type;
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
      code += lang.string_type;
      code += " " + MakeCamel("name", lang.first_camel_upper);
      code += "(int e) { return names[e";
      if (enum_def.vals.vec.front()->value)
        code += " - " + enum_def.vals.vec.front()->name;
      code += "]; }\n";
    }
  }

  // Close the class
  code += "};\n\n";
}

// Returns the function name that is able to read a value of the given type.
static std::string GenGetter(const LanguageParameters &lang,
                             const Type &type) {
  switch (type.base_type) {
    case BASE_TYPE_STRING: return "__string";
    case BASE_TYPE_STRUCT: return "__struct";
    case BASE_TYPE_UNION:  return "__union";
    case BASE_TYPE_VECTOR: return GenGetter(lang, type.VectorType());
    default: {
      std::string getter = "bb." + FunctionStart(lang, 'G') + "et";
      if (type.base_type == BASE_TYPE_BOOL) {
        getter = "0!=" + getter;
      } else if (GenTypeBasic(lang, type) != "byte") {
        getter += MakeCamel(GenTypeGet(lang, type));
      }
      return getter;
    }
  }
}

// Returns the method name for use with add/put calls.
static std::string GenMethod(const LanguageParameters &lang, const Type &type) {
  return IsScalar(type.base_type)
    ? MakeCamel(GenTypeBasic(lang, type))
    : (IsStruct(type) ? "Struct" : "Offset");
}

// Recursively generate arguments for a constructor, to deal with nested
// structs.
static void GenStructArgs(const LanguageParameters &lang,
                          const StructDef &struct_def,
                          std::string *code_ptr, const char *nameprefix) {
  std::string &code = *code_ptr;
  for (auto it = struct_def.fields.vec.begin();
       it != struct_def.fields.vec.end();
       ++it) {
    auto &field = **it;
    if (IsStruct(field.value.type)) {
      // Generate arguments for a struct inside a struct. To ensure names
      // don't clash, and to make it obvious these arguments are constructing
      // a nested struct, prefix the name with the struct name.
      GenStructArgs(lang, *field.value.type.struct_def, code_ptr,
                    (field.value.type.struct_def->name + "_").c_str());
    } else {
      code += ", ";
      code += GenTypeForUser(lang,
                             DestinationType(lang, field.value.type, false));
      code += " ";
      code += nameprefix;
      code += MakeCamel(field.name, lang.first_camel_upper);
    }
  }
}

// Recusively generate struct construction statements of the form:
// builder.putType(name);
// and insert manual padding.
static void GenStructBody(const LanguageParameters &lang,
                          const StructDef &struct_def,
                          std::string *code_ptr, const char *nameprefix) {
  std::string &code = *code_ptr;
  code += "    builder." + FunctionStart(lang, 'P') + "rep(";
  code += NumToString(struct_def.minalign) + ", ";
  code += NumToString(struct_def.bytesize) + ");\n";
  for (auto it = struct_def.fields.vec.rbegin();
       it != struct_def.fields.vec.rend(); ++it) {
    auto &field = **it;
    if (field.padding) {
      code += "    builder." + FunctionStart(lang, 'P') + "ad(";
      code += NumToString(field.padding) + ");\n";
    }
    if (IsStruct(field.value.type)) {
      GenStructBody(lang, *field.value.type.struct_def, code_ptr,
                    (field.value.type.struct_def->name + "_").c_str());
    } else {
      code += "    builder." + FunctionStart(lang, 'P') + "ut";
      code += GenMethod(lang, field.value.type) + "(";
      auto argname = nameprefix + MakeCamel(field.name, lang.first_camel_upper);
      code += DestinationValue(lang, argname, field.value.type);
      code += ");\n";
    }
  }
}

static void GenStruct(const LanguageParameters &lang, const Parser &parser,
                      const StructDef &struct_def, std::string *code_ptr) {
  if (struct_def.generated) return;
  std::string &code = *code_ptr;

  // Generate a struct accessor class, with methods of the form:
  // public type name() { return bb.getType(i + offset); }
  // or for tables of the form:
  // public type name() {
  //   int o = __offset(offset); return o != 0 ? bb.getType(o + i) : default;
  // }
  GenComment(struct_def.doc_comment, code_ptr, &lang.comment_config);
  code += std::string("public ") + lang.unsubclassable_decl;
  code += "class " + struct_def.name + lang.inheritance_marker;
  code += struct_def.fixed ? "Struct" : "Table";
  code += " {\n";
  if (!struct_def.fixed) {
    // Generate a special accessor for the table that when used as the root
    // of a FlatBuffer
    std::string method_name = FunctionStart(lang, 'G')
                              + "etRootAs" + struct_def.name;
    std::string method_signature = "  public static " + struct_def.name
                                   + " " + method_name;

    // create convenience method that doesn't require an existing object
    code += method_signature + "(ByteBuffer _bb) ";
    code += "{ return " + method_name + "(_bb, new "
            + struct_def.name+ "()); }\n";

    // create method that allows object reuse
    code += method_signature + "(ByteBuffer _bb, "
            + struct_def.name + " obj) { ";
    code += lang.set_bb_byteorder;
    code += "return (obj.__init(_bb." + FunctionStart(lang, 'G');
    code += "etInt(_bb.";
    code += lang.get_bb_position;
    code += ") + _bb.";
    code += lang.get_bb_position;
    code += ", _bb)); }\n";
    if (parser.root_struct_def == &struct_def) {
      if (parser.file_identifier_.length()) {
        // Check if a buffer has the identifier.
        code += "  public static ";
        code += lang.bool_type + struct_def.name;
        code += "BufferHasIdentifier(ByteBuffer _bb) { return ";
        code += "__has_identifier(_bb, \"" + parser.file_identifier_;
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
    GenComment(field.doc_comment, code_ptr, &lang.comment_config, "  ");
    std::string type_name = GenTypeGet(lang, field.value.type);
    std::string type_name_dest =
      lang.language == GeneratorOptions::kCSharp &&
        field.value.type.enum_def != nullptr &&
        field.value.type.base_type != BASE_TYPE_UNION
          ? field.value.type.enum_def->name
          : GenTypeGet(lang, DestinationType(lang, field.value.type, true));
    std::string dest_mask = DestinationMask(lang, field.value.type, true);
    std::string dest_cast = DestinationCast(lang, field.value.type);
    std::string method_start = "  public " + type_name_dest + " " +
                               MakeCamel(field.name, lang.first_camel_upper);
    // Generate the accessors that don't do object reuse.
    if (field.value.type.base_type == BASE_TYPE_STRUCT) {
      // Calls the accessor that takes an accessor object with a new object.
      if (lang.language == GeneratorOptions::kCSharp) {
        code += method_start + " { get { return Get";
        code += MakeCamel(field.name, lang.first_camel_upper);
        code += "(new ";
        code += type_name + "()); } }\n";
        method_start = "  public " + type_name_dest + " Get"
                       + MakeCamel(field.name, lang.first_camel_upper);
      } else {
        code += method_start + "() { return ";
        code += MakeCamel(field.name, lang.first_camel_upper);
        code += "(new ";
        code += type_name + "()); }\n";
      }
    } else if (field.value.type.base_type == BASE_TYPE_VECTOR &&
               field.value.type.element == BASE_TYPE_STRUCT) {
      // Accessors for vectors of structs also take accessor objects, this
      // generates a variant without that argument.
      if (lang.language == GeneratorOptions::kCSharp) {
        method_start = "  public " + type_name_dest + " Get"
                       + MakeCamel(field.name, lang.first_camel_upper);
        code += method_start + "(int j) { return Get";
      } else {
        code += method_start + "(int j) { return ";
      }
      code += MakeCamel(field.name, lang.first_camel_upper);
      code += "(new ";
      code += type_name + "(), j); }\n";
    } else if (field.value.type.base_type == BASE_TYPE_UNION ||
               field.value.type.base_type == BASE_TYPE_VECTOR) {
      if (lang.language == GeneratorOptions::kCSharp) {
        method_start = "  public " + type_name_dest + " Get"
                       + MakeCamel(field.name, lang.first_camel_upper);
      }
    }
    std::string getter = dest_cast + GenGetter(lang, field.value.type);
    code += method_start;
    // Most field accessors need to retrieve and test the field offset first,
    // this is the prefix code for that:
    auto offset_prefix = " { int o = __offset(" +
                         NumToString(field.value.offset) +
                         "); return o != 0 ? ";
    std::string default_cast = "";
    if (lang.language == GeneratorOptions::kCSharp)
      default_cast = "(" + type_name_dest + ")";
    std::string member_suffix = "";
    if (IsScalar(field.value.type.base_type)) {
      code += lang.getter_prefix;
      member_suffix = lang.getter_suffix;
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
          code += lang.getter_prefix;
          member_suffix = lang.getter_suffix;
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
          code += IsScalar(field.value.type.element)
                  ? default_cast + "0"
                  : "null";
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
      code += "  public int " + MakeCamel(field.name, lang.first_camel_upper);
      code += "Length";
      code += lang.getter_prefix;
      code += offset_prefix;
      code += "__vector_len(o) : 0; ";
      code += lang.getter_suffix;
      code += "}\n";
    }
    // Generate a ByteBuffer accessor for strings & vectors of scalars.
    if (((field.value.type.base_type == BASE_TYPE_VECTOR &&
          IsScalar(field.value.type.VectorType().base_type)) ||
         field.value.type.base_type == BASE_TYPE_STRING) &&
        lang.language == GeneratorOptions::kJava) {
      code += "  public ByteBuffer ";
      code += MakeCamel(field.name, lang.first_camel_upper);
      code += "AsByteBuffer() { return __vector_as_bytebuffer(";
      code += NumToString(field.value.offset) + ", ";
      code += NumToString(field.value.type.base_type == BASE_TYPE_STRING ? 1 :
                          InlineSize(field.value.type.VectorType()));
      code += "); }\n";
    }
  }
  code += "\n";
  if (struct_def.fixed) {
    // create a struct constructor function
    code += "  public static int " + FunctionStart(lang, 'C') + "reate";
    code += struct_def.name + "(FlatBufferBuilder builder";
    GenStructArgs(lang, struct_def, code_ptr, "");
    code += ") {\n";
    GenStructBody(lang, struct_def, code_ptr, "");
    code += "    return builder.";
    code += lang.get_fbb_offset;
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
      // public static void createName(FlatBufferBuilder builder, args...)
      code += "  public static int " + FunctionStart(lang, 'C') + "reate";
      code += struct_def.name;
      code += "(FlatBufferBuilder builder";
      for (auto it = struct_def.fields.vec.begin();
           it != struct_def.fields.vec.end(); ++it) {
        auto &field = **it;
        if (field.deprecated) continue;
        code += ",\n      ";
        code += GenTypeForUser(lang,
                               DestinationType(lang, field.value.type, false));
        code += " ";
        code += field.name;
        // Java doesn't have defaults, which means this method must always
        // supply all arguments, and thus won't compile when fields are added.
        if (lang.language != GeneratorOptions::kJava) {
          code += " = " + GenDefaultValue(field.value);
        }
      }
      code += ") {\n    builder.";
      code += FunctionStart(lang, 'S') + "tartObject(";
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
            code += FunctionStart(lang, 'A') + "dd";
            code += MakeCamel(field.name) + "(builder, " + field.name + ");\n";
          }
        }
      }
      code += "    return " + struct_def.name + ".";
      code += FunctionStart(lang, 'E') + "nd" + struct_def.name;
      code += "(builder);\n  }\n\n";
    }
    // Generate a set of static methods that allow table construction,
    // of the form:
    // public static void addName(FlatBufferBuilder builder, short name)
    // { builder.addShort(id, name, default); }
    // Unlike the Create function, these always work.
    code += "  public static void " + FunctionStart(lang, 'S') + "tart";
    code += struct_def.name;
    code += "(FlatBufferBuilder builder) { builder.";
    code += FunctionStart(lang, 'S') + "tartObject(";
    code += NumToString(struct_def.fields.vec.size()) + "); }\n";
    for (auto it = struct_def.fields.vec.begin();
         it != struct_def.fields.vec.end(); ++it) {
      auto &field = **it;
      if (field.deprecated) continue;
      code += "  public static void " + FunctionStart(lang, 'A') + "dd";
      code += MakeCamel(field.name);
      code += "(FlatBufferBuilder builder, ";
      code += GenTypeForUser(lang,
                             DestinationType(lang, field.value.type, false));
      auto argname = MakeCamel(field.name, false);
      if (!IsScalar(field.value.type.base_type)) argname += "Offset";
      code += " " + argname + ") { builder." + FunctionStart(lang, 'A') + "dd";
      code += GenMethod(lang, field.value.type) + "(";
      code += NumToString(it - struct_def.fields.vec.begin()) + ", ";
      code += DestinationValue(lang, argname, field.value.type);
      code += ", " + GenDefaultValue(field.value);
      code += "); }\n";
      if (field.value.type.base_type == BASE_TYPE_VECTOR) {
        auto vector_type = field.value.type.VectorType();
        auto alignment = InlineAlignment(vector_type);
        auto elem_size = InlineSize(vector_type);
        if (!IsStruct(vector_type)) {
          // Generate a method to create a vector from a Java array.
          code += "  public static int " + FunctionStart(lang, 'C') + "reate";
          code += MakeCamel(field.name);
          code += "Vector(FlatBufferBuilder builder, ";
          code += GenTypeBasic(lang, vector_type) + "[] data) ";
          code += "{ builder." + FunctionStart(lang, 'S') + "tartVector(";
          code += NumToString(elem_size);
          code += ", data." + FunctionStart(lang, 'L') + "ength, ";
          code += NumToString(alignment);
          code += "); for (int i = data.";
          code += FunctionStart(lang, 'L') + "ength - 1; i >= 0; i--) builder.";
          code += FunctionStart(lang, 'A') + "dd";
          code += GenMethod(lang, vector_type);
          code += "(data[i]); return builder.";
          code += FunctionStart(lang, 'E') + "ndVector(); }\n";
        }
        // Generate a method to start a vector, data to be added manually after.
        code += "  public static void " + FunctionStart(lang, 'S') + "tart";
        code += MakeCamel(field.name);
        code += "Vector(FlatBufferBuilder builder, int numElems) ";
        code += "{ builder." + FunctionStart(lang, 'S') + "tartVector(";
        code += NumToString(elem_size);
        code += ", numElems, " + NumToString(alignment);
        code += "); }\n";
      }
    }
    code += "  public static int ";
    code += FunctionStart(lang, 'E') + "nd" + struct_def.name;
    code += "(FlatBufferBuilder builder) {\n    int o = builder.";
    code += FunctionStart(lang, 'E') + "ndObject();\n";
    for (auto it = struct_def.fields.vec.begin();
         it != struct_def.fields.vec.end();
         ++it) {
      auto &field = **it;
      if (!field.deprecated && field.required) {
        code += "    builder." + FunctionStart(lang, 'R') + "equired(o, ";
        code += NumToString(field.value.offset);
        code += ");  // " + field.name + "\n";
      }
    }
    code += "    return o;\n  }\n";
    if (parser.root_struct_def == &struct_def) {
      code += "  public static void ";
      code += FunctionStart(lang, 'F') + "inish" + struct_def.name;
      code += "Buffer(FlatBufferBuilder builder, int offset) { ";
      code += "builder." + FunctionStart(lang, 'F') + "inish(offset";
      if (parser.file_identifier_.length())
        code += ", \"" + parser.file_identifier_ + "\"";
      code += "); }\n";
    }
  }
  code += "};\n\n";
}

// Save out the generated code for a single class while adding
// declaration boilerplate.
static bool SaveClass(const LanguageParameters &lang, const Parser &parser,
                      const Definition &def, const std::string &classcode,
                      const std::string &path, bool needs_includes) {
  if (!classcode.length()) return true;

  std::string namespace_general;
  std::string namespace_dir = path;  // Either empty or ends in separator.
  auto &namespaces = parser.namespaces_.back()->components;
  for (auto it = namespaces.begin(); it != namespaces.end(); ++it) {
    if (namespace_general.length()) {
      namespace_general += ".";
    }
    namespace_general += *it;
    namespace_dir += *it + kPathSeparator;
  }
  EnsureDirExists(namespace_dir);

  std::string code = "// automatically generated, do not modify\n\n";
  code += lang.namespace_ident + namespace_general + lang.namespace_begin;
  code += "\n\n";
  if (needs_includes) code += lang.includes;
  code += classcode;
  code += lang.namespace_end;
  auto filename = namespace_dir + def.name + lang.file_extension;
  return SaveFile(filename.c_str(), code, false);
}

bool GenerateGeneral(const Parser &parser,
                     const std::string &path,
                     const std::string & /*file_name*/,
                     const GeneratorOptions &opts) {
  assert(opts.lang <= GeneratorOptions::kMAX);
  auto lang = language_parameters[opts.lang];

  for (auto it = parser.enums_.vec.begin();
       it != parser.enums_.vec.end(); ++it) {
    std::string enumcode;
    GenEnum(lang, **it, &enumcode);
    if (!SaveClass(lang, parser, **it, enumcode, path, false))
      return false;
  }

  for (auto it = parser.structs_.vec.begin();
       it != parser.structs_.vec.end(); ++it) {
    std::string declcode;
    GenStruct(lang, parser, **it, &declcode);
    if (!SaveClass(lang, parser, **it, declcode, path, true))
      return false;
  }

  return true;
}

static std::string ClassFileName(const LanguageParameters &lang,
                                 const Parser &parser, const Definition &def,
                                 const std::string &path) {
  std::string namespace_general;
  std::string namespace_dir = path;
  auto &namespaces = parser.namespaces_.back()->components;
  for (auto it = namespaces.begin(); it != namespaces.end(); ++it) {
    if (namespace_general.length()) {
      namespace_general += ".";
      namespace_dir += kPathSeparator;
    }
    namespace_general += *it;
    namespace_dir += *it;
  }

  return namespace_dir + kPathSeparator + def.name + lang.file_extension;
}

std::string GeneralMakeRule(const Parser &parser,
                            const std::string &path,
                            const std::string &file_name,
                            const GeneratorOptions &opts) {
  assert(opts.lang <= GeneratorOptions::kMAX);
  auto lang = language_parameters[opts.lang];

  std::string make_rule;

  for (auto it = parser.enums_.vec.begin();
       it != parser.enums_.vec.end(); ++it) {
    if (make_rule != "")
      make_rule += " ";
    make_rule += ClassFileName(lang, parser, **it, path);
  }

  for (auto it = parser.structs_.vec.begin();
       it != parser.structs_.vec.end(); ++it) {
    if (make_rule != "")
      make_rule += " ";
    make_rule += ClassFileName(lang, parser, **it, path);
  }

  make_rule += ": ";
  auto included_files = parser.GetIncludedFilesRecursive(file_name);
  for (auto it = included_files.begin();
       it != included_files.end(); ++it) {
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
                    const std::string &file_name,
                    const GeneratorOptions & /*opts*/) {
  return !parser.builder_.GetSize() ||
         flatbuffers::SaveFile(
           BinaryFileName(parser, path, file_name).c_str(),
           reinterpret_cast<char *>(parser.builder_.GetBufferPointer()),
           parser.builder_.GetSize(),
           true);
}

std::string BinaryMakeRule(const Parser &parser,
                           const std::string &path,
                           const std::string &file_name,
                           const GeneratorOptions & /*opts*/) {
  if (!parser.builder_.GetSize()) return "";
  std::string filebase = flatbuffers::StripPath(
      flatbuffers::StripExtension(file_name));
  std::string make_rule = BinaryFileName(parser, path, filebase) + ": " +
      file_name;
  auto included_files = parser.GetIncludedFilesRecursive(
      parser.root_struct_def->file);
  for (auto it = included_files.begin();
       it != included_files.end(); ++it) {
    make_rule += " " + *it;
  }
  return make_rule;
}

}  // namespace flatbuffers
