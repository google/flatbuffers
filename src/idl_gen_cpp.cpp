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

namespace flatbuffers {
namespace cpp {

// This tracks the current namespace so we can insert namespace declarations.
// TODO(wvo): this needs to be moved into a code generator context object.
static const Namespace *code_generator_cur_name_space = nullptr;

// Ensure that a type is prefixed with its namespace whenever it is used
// outside of its namespace.
static std::string WrapInNameSpace(const Namespace *ns,
                                   const std::string &name) {
  if (code_generator_cur_name_space != ns) {
    std::string qualified_name;
    for (auto it = ns->components.begin();
             it != ns->components.end(); ++it) {
      qualified_name += *it + "::";
    }
    return qualified_name + name;
  } else {
    return name;
  }
}

static std::string WrapInNameSpace(const Definition &def) {
  return WrapInNameSpace(def.defined_namespace, def.name);
}

// Translates a qualified name in flatbuffer text format to the same name in
// the equivalent C++ namespace.
static std::string TranslateNameSpace(const std::string &qualified_name) {
  std::string cpp_qualified_name = qualified_name;
  size_t start_pos = 0;
  while((start_pos = cpp_qualified_name.find(".", start_pos)) !=
         std::string::npos) {
    cpp_qualified_name.replace(start_pos, 1, "::");
  }
  return cpp_qualified_name;
}


// Return a C++ type from the table in idl.h
static std::string GenTypeBasic(const Type &type, bool user_facing_type) {
  static const char *ctypename[] = {
    #define FLATBUFFERS_TD(ENUM, IDLTYPE, CTYPE, JTYPE, GTYPE, NTYPE, PTYPE) \
      #CTYPE,
      FLATBUFFERS_GEN_TYPES(FLATBUFFERS_TD)
    #undef FLATBUFFERS_TD
  };
  if (user_facing_type) {
    if (type.enum_def) return WrapInNameSpace(*type.enum_def);
    if (type.base_type == BASE_TYPE_BOOL) return "bool";
  }
  return ctypename[type.base_type];
}

static std::string GenTypeWire(const Parser &parser, const Type &type,
                               const char *postfix, bool user_facing_type);

// Return a C++ pointer type, specialized to the actual struct/table types,
// and vector element types.
static std::string GenTypePointer(const Parser &parser, const Type &type) {
  switch (type.base_type) {
    case BASE_TYPE_STRING:
      return "flatbuffers::String";
    case BASE_TYPE_VECTOR:
      return "flatbuffers::Vector<" +
             GenTypeWire(parser, type.VectorType(), "", false) + ">";
    case BASE_TYPE_STRUCT: {
      return WrapInNameSpace(*type.struct_def);
    }
    case BASE_TYPE_UNION:
      // fall through
    default:
      return "void";
  }
}

// Return a C++ type for any type (scalar/pointer) specifically for
// building a flatbuffer.
static std::string GenTypeWire(const Parser &parser, const Type &type,
                               const char *postfix, bool user_facing_type) {
  return IsScalar(type.base_type)
    ? GenTypeBasic(type, user_facing_type) + postfix
    : IsStruct(type)
      ? "const " + GenTypePointer(parser, type) + " *"
      : "flatbuffers::Offset<" + GenTypePointer(parser, type) + ">" + postfix;
}

// Return a C++ type for any type (scalar/pointer) that reflects its
// serialized size.
static std::string GenTypeSize(const Parser &parser, const Type &type) {
  return IsScalar(type.base_type)
    ? GenTypeBasic(type, false)
    : IsStruct(type)
      ? GenTypePointer(parser, type)
      : "flatbuffers::uoffset_t";
}

// Return a C++ type for any type (scalar/pointer) specifically for
// using a flatbuffer.
static std::string GenTypeGet(const Parser &parser, const Type &type,
                              const char *afterbasic, const char *beforeptr,
                              const char *afterptr, bool user_facing_type) {
  return IsScalar(type.base_type)
    ? GenTypeBasic(type, user_facing_type) + afterbasic
    : beforeptr + GenTypePointer(parser, type) + afterptr;
}

static std::string GenEnumDecl(const EnumDef &enum_def,
                               const IDLOptions &opts) {
  return (opts.scoped_enums ? "enum class " : "enum ") + enum_def.name;
}

static std::string GenEnumVal(const EnumDef &enum_def,
                              const std::string &enum_val,
                              const IDLOptions &opts) {
  return opts.prefixed_enums ? enum_def.name + "_" + enum_val : enum_val;
}

static std::string GetEnumVal(const EnumDef &enum_def, const EnumVal &enum_val,
                              const IDLOptions &opts) {
  if (opts.scoped_enums) {
      return enum_def.name + "::" + enum_val.name;
  } else if (opts.prefixed_enums) {
      return enum_def.name + "_" + enum_val.name;
  } else {
      return enum_val.name;
  }
}

std::string EnumSignature(EnumDef &enum_def) {
  return "inline bool Verify" + enum_def.name +
         "(flatbuffers::Verifier &verifier, " +
         "const void *union_obj, " + enum_def.name + " type)";
}

// Generate an enum declaration and an enum string lookup table.
static void GenEnum(const Parser &parser, EnumDef &enum_def,
                    std::string *code_ptr) {
  std::string &code = *code_ptr;
  GenComment(enum_def.doc_comment, code_ptr, nullptr);
  code += GenEnumDecl(enum_def, parser.opts);
  if (parser.opts.scoped_enums)
    code += " : " + GenTypeBasic(enum_def.underlying_type, false);
  code += " {\n";
  int64_t anyv = 0;
  EnumVal *minv = nullptr, *maxv = nullptr;
  for (auto it = enum_def.vals.vec.begin();
       it != enum_def.vals.vec.end();
       ++it) {
    auto &ev = **it;
    GenComment(ev.doc_comment, code_ptr, nullptr, "  ");
    code += "  " + GenEnumVal(enum_def, ev.name, parser.opts) + " = ";
    code += NumToString(ev.value) + ",\n";
    minv = !minv || minv->value > ev.value ? &ev : minv;
    maxv = !maxv || maxv->value < ev.value ? &ev : maxv;
    anyv |= ev.value;
  }
  if (parser.opts.scoped_enums || parser.opts.prefixed_enums) {
    assert(minv && maxv);
    if (enum_def.attributes.Lookup("bit_flags")) {
      if (minv->value != 0) // If the user didn't defined NONE value
        code += "  " + GenEnumVal(enum_def, "NONE", parser.opts) + " = 0,\n";
      if (maxv->value != anyv) // If the user didn't defined ANY value
        code += "  " + GenEnumVal(enum_def, "ANY", parser.opts) + " = " + NumToString(anyv) + "\n";
    } else { // MIN & MAX are useless for bit_flags
      code += "  " + GenEnumVal(enum_def, "MIN", parser.opts) + " = ";
      code += GenEnumVal(enum_def, minv->name, parser.opts) + ",\n";
      code += "  " + GenEnumVal(enum_def, "MAX", parser.opts) + " = ";
      code += GenEnumVal(enum_def, maxv->name, parser.opts) + "\n";
    }
  }
  code += "};\n";
  if (parser.opts.scoped_enums && enum_def.attributes.Lookup("bit_flags"))
    code += "DEFINE_BITMASK_OPERATORS(" + enum_def.name + ", " +  GenTypeBasic(enum_def.underlying_type, false) + ")\n";
  code += "\n";

  // Generate a generate string table for enum values.
  // Problem is, if values are very sparse that could generate really big
  // tables. Ideally in that case we generate a map lookup instead, but for
  // the moment we simply don't output a table at all.
  auto range = enum_def.vals.vec.back()->value -
               enum_def.vals.vec.front()->value + 1;
  // Average distance between values above which we consider a table
  // "too sparse". Change at will.
  static const int kMaxSparseness = 5;
  if (range / static_cast<int64_t>(enum_def.vals.vec.size()) < kMaxSparseness) {
    code += "inline const char **EnumNames" + enum_def.name + "() {\n";
    code += "  static const char *names[] = { ";
    auto val = enum_def.vals.vec.front()->value;
    for (auto it = enum_def.vals.vec.begin();
         it != enum_def.vals.vec.end();
         ++it) {
      while (val++ != (*it)->value) code += "\"\", ";
      code += "\"" + (*it)->name + "\", ";
    }
    code += "nullptr };\n  return names;\n}\n\n";
    code += "inline const char *EnumName" + enum_def.name;
    code += "(" + enum_def.name + " e) { return EnumNames" + enum_def.name;
    code += "()[static_cast<int>(e)";
    if (enum_def.vals.vec.front()->value) {
      code += " - static_cast<int>(";
      code += GetEnumVal(enum_def, *enum_def.vals.vec.front(), parser.opts) +")";
    }
    code += "]; }\n\n";
  }

  if (enum_def.is_union) {
    code += EnumSignature(enum_def) + ";\n\n";
  }
}

static void GenEnumPost(const Parser &parser, EnumDef &enum_def,
                    std::string *code_ptr_post) {
  // Generate a verifier function for this union that can be called by the
  // table verifier functions. It uses a switch case to select a specific
  // verifier function to call, this should be safe even if the union type
  // has been corrupted, since the verifiers will simply fail when called
  // on the wrong type.
  std::string &code_post = *code_ptr_post;
  code_post += EnumSignature(enum_def) + " {\n  switch (type) {\n";
  for (auto it = enum_def.vals.vec.begin();
       it != enum_def.vals.vec.end();
       ++it) {
    auto &ev = **it;
    code_post += "    case " + GetEnumVal(enum_def, ev, parser.opts);
    if (!ev.value) {
      code_post += ": return true;\n";  // "NONE" enum value.
    } else {
      code_post += ": return verifier.VerifyTable(reinterpret_cast<const ";
      code_post += WrapInNameSpace(*ev.struct_def);
      code_post += " *>(union_obj));\n";
    }
  }
  code_post += "    default: return false;\n  }\n}\n\n";
}

// Generates a value with optionally a cast applied if the field has a
// different underlying type from its interface type (currently only the
// case for enums. "from" specify the direction, true meaning from the
// underlying type to the interface type.
std::string GenUnderlyingCast(const FieldDef &field, bool from,
                              const std::string &val) {
  if (from && field.value.type.base_type == BASE_TYPE_BOOL) {
    return val + " != 0";
  } else if ((field.value.type.enum_def &&
              IsScalar(field.value.type.base_type)) ||
             field.value.type.base_type == BASE_TYPE_BOOL) {
    return "static_cast<" + GenTypeBasic(field.value.type, from) +
           ">(" + val + ")";
  } else {
    return val;
  }
}

std::string GenFieldOffsetName(const FieldDef &field) {
  std::string uname = field.name;
  std::transform(uname.begin(), uname.end(), uname.begin(), ::toupper);
  return "VT_" + uname;
}

static void GenFullyQualifiedNameGetter(const Parser &parser, const std::string& name, std::string &code) {
  if (parser.opts.generate_name_strings) {
    code += "  static FLATBUFFERS_CONSTEXPR const char *GetFullyQualifiedName() {\n";
    code += "    return \"" + parser.namespaces_.back()->GetFullyQualifiedName(name) + "\";\n";
    code += "  }\n";
  }
}

std::string GenDefaultConstant(const FieldDef &field) {
  return field.value.type.base_type == BASE_TYPE_FLOAT
      ? field.value.constant + "f"
      : field.value.constant;
}

// Generate an accessor struct, builder structs & function for a table.
static void GenTable(const Parser &parser, StructDef &struct_def,
                     std::string *code_ptr) {
  std::string &code = *code_ptr;
  // Generate an accessor struct, with methods of the form:
  // type name() const { return GetField<type>(offset, defaultval); }
  GenComment(struct_def.doc_comment, code_ptr, nullptr);
  code += "struct " + struct_def.name;
  code += " FLATBUFFERS_FINAL_CLASS : private flatbuffers::Table";
  code += " {\n";
  // Generate GetFullyQualifiedName
  GenFullyQualifiedNameGetter(parser, struct_def.name, code);
  // Generate field id constants.
  if (struct_def.fields.vec.size() > 0) {
    code += "  enum {\n";
    bool is_first_field = true; // track the first field that's not deprecated
    for (auto it = struct_def.fields.vec.begin();
         it != struct_def.fields.vec.end();
         ++it) {
      auto &field = **it;
      if (!field.deprecated) {  // Deprecated fields won't be accessible.
        if (!is_first_field) {
          // Add trailing comma and newline to previous element. Don't add trailing comma to
          // last element since older versions of gcc complain about this.
          code += ",\n";
        } else {
          is_first_field = false;
        }
        code += "    " + GenFieldOffsetName(field) + " = ";
        code += NumToString(field.value.offset);
      }
    }
    code += "\n  };\n";
  }
  // Generate the accessors.
  for (auto it = struct_def.fields.vec.begin();
       it != struct_def.fields.vec.end();
       ++it) {
    auto &field = **it;
    if (!field.deprecated) {  // Deprecated fields won't be accessible.
      auto is_scalar = IsScalar(field.value.type.base_type);
      GenComment(field.doc_comment, code_ptr, nullptr, "  ");
      code += "  " + GenTypeGet(parser, field.value.type, " ", "const ", " *",
                                true);
      code += field.name + "() const { return ";
      // Call a different accessor for pointers, that indirects.
      auto accessor = is_scalar
        ? "GetField<"
        : (IsStruct(field.value.type) ? "GetStruct<" : "GetPointer<");
      auto offsetstr = GenFieldOffsetName(field);
      auto call =
          accessor +
          GenTypeGet(parser, field.value.type, "", "const ", " *", false) +
          ">(" + offsetstr;
      // Default value as second arg for non-pointer types.
      if (IsScalar(field.value.type.base_type))
        call += ", " + GenDefaultConstant(field);
      call += ")";
      code += GenUnderlyingCast(field, true, call);
      code += "; }\n";
      if (parser.opts.mutable_buffer) {
        if (is_scalar) {
          code += "  bool mutate_" + field.name + "(";
          code += GenTypeBasic(field.value.type, true);
          code += " _" + field.name + ") { return SetField(" + offsetstr + ", ";
          code += GenUnderlyingCast(field, false, "_" + field.name);
          code += "); }\n";
        } else {
          auto type = GenTypeGet(parser, field.value.type, " ", "", " *", true);
          code += "  " + type + "mutable_" + field.name + "() { return ";
          code += GenUnderlyingCast(field, true,
                                    accessor + type + ">(" + offsetstr + ")");
          code += "; }\n";
        }
      }
      auto nested = field.attributes.Lookup("nested_flatbuffer");
      if (nested) {
        std::string qualified_name =
            parser.namespaces_.back()->GetFullyQualifiedName(nested->constant);
        auto nested_root = parser.structs_.Lookup(qualified_name);
        assert(nested_root);  // Guaranteed to exist by parser.
        (void)nested_root;
        std::string cpp_qualified_name = TranslateNameSpace(qualified_name);

        code += "  const " + cpp_qualified_name + " *" + field.name;
        code += "_nested_root() const { return flatbuffers::GetRoot<";
        code += cpp_qualified_name + ">(" + field.name + "()->Data()); }\n";
      }
      // Generate a comparison function for this field if it is a key.
      if (field.key) {
        code += "  bool KeyCompareLessThan(const " + struct_def.name;
        code += " *o) const { return ";
        if (field.value.type.base_type == BASE_TYPE_STRING) code += "*";
        code += field.name + "() < ";
        if (field.value.type.base_type == BASE_TYPE_STRING) code += "*";
        code += "o->" + field.name + "(); }\n";
        code += "  int KeyCompareWithValue(";
        if (field.value.type.base_type == BASE_TYPE_STRING) {
          code += "const char *val) const { return strcmp(" + field.name;
          code += "()->c_str(), val); }\n";
        } else {
          if (parser.opts.scoped_enums &&
            field.value.type.enum_def &&
            IsScalar(field.value.type.base_type)) {
            code += GenTypeGet(parser, field.value.type, " ", "const ", " *",
                               true);
          } else {
            code += GenTypeBasic(field.value.type, false);
          }
          code += " val) const { return " + field.name + "() < val ? -1 : ";
          code += field.name + "() > val; }\n";
        }
      }
    }
  }
  // Generate a verifier function that can check a buffer from an untrusted
  // source will never cause reads outside the buffer.
  code += "  bool Verify(flatbuffers::Verifier &verifier) const {\n";
  code += "    return VerifyTableStart(verifier)";
  std::string prefix = " &&\n           ";
  for (auto it = struct_def.fields.vec.begin();
       it != struct_def.fields.vec.end();
       ++it) {
    auto &field = **it;
    if (!field.deprecated) {
      code += prefix + "VerifyField";
      if (field.required) code += "Required";
      code += "<" + GenTypeSize(parser, field.value.type);
      code += ">(verifier, " + GenFieldOffsetName(field) + ")";
      switch (field.value.type.base_type) {
        case BASE_TYPE_UNION:
          code += prefix + "Verify" + field.value.type.enum_def->name;
          code += "(verifier, " + field.name + "(), " + field.name + "_type())";
          break;
        case BASE_TYPE_STRUCT:
          if (!field.value.type.struct_def->fixed) {
            code += prefix + "verifier.VerifyTable(" + field.name;
            code += "())";
          }
          break;
        case BASE_TYPE_STRING:
          code += prefix + "verifier.Verify(" + field.name + "())";
          break;
        case BASE_TYPE_VECTOR:
          code += prefix + "verifier.Verify(" + field.name + "())";
          switch (field.value.type.element) {
            case BASE_TYPE_STRING: {
              code += prefix + "verifier.VerifyVectorOfStrings(" + field.name;
              code += "())";
              break;
            }
            case BASE_TYPE_STRUCT: {
              if (!field.value.type.struct_def->fixed) {
                code += prefix + "verifier.VerifyVectorOfTables(" + field.name;
                code += "())";
              }
              break;
            }
            default:
              break;
          }
          break;
        default:
          break;
      }
    }
  }
  code += prefix + "verifier.EndTable()";
  code += ";\n  }\n";
  code += "};\n\n";

  // Generate a builder struct, with methods of the form:
  // void add_name(type name) { fbb_.AddElement<type>(offset, name, default); }
  code += "struct " + struct_def.name;
  code += "Builder {\n  flatbuffers::FlatBufferBuilder &fbb_;\n";
  code += "  flatbuffers::uoffset_t start_;\n";
  for (auto it = struct_def.fields.vec.begin();
       it != struct_def.fields.vec.end();
       ++it) {
    auto &field = **it;
    if (!field.deprecated) {
      code += "  void add_" + field.name + "(";
      code += GenTypeWire(parser, field.value.type, " ", true) + field.name;
      code += ") { fbb_.Add";
      if (IsScalar(field.value.type.base_type)) {
        code += "Element<" + GenTypeWire(parser, field.value.type, "", false);
        code += ">";
      } else if (IsStruct(field.value.type)) {
        code += "Struct";
      } else {
        code += "Offset";
      }
      code += "(" + struct_def.name + "::" + GenFieldOffsetName(field) + ", ";
      code += GenUnderlyingCast(field, false, field.name);
      if (IsScalar(field.value.type.base_type))
        code += ", " + GenDefaultConstant(field);
      code += "); }\n";
    }
  }
  code += "  " + struct_def.name;
  code += "Builder(flatbuffers::FlatBufferBuilder &_fbb) : fbb_(_fbb) ";
  code += "{ start_ = fbb_.StartTable(); }\n";
  code += "  " + struct_def.name + "Builder &operator=(const ";
  code += struct_def.name + "Builder &);\n";
  code += "  flatbuffers::Offset<" + struct_def.name;
  code += "> Finish() {\n    auto o = flatbuffers::Offset<" + struct_def.name;
  code += ">(fbb_.EndTable(start_, ";
  code += NumToString(struct_def.fields.vec.size()) + "));\n";
  for (auto it = struct_def.fields.vec.begin();
       it != struct_def.fields.vec.end();
       ++it) {
    auto &field = **it;
    if (!field.deprecated && field.required) {
      code += "    fbb_.Required(o, ";
      code += struct_def.name + "::" + GenFieldOffsetName(field);
      code += ");  // " + field.name + "\n";
    }
  }
  code += "    return o;\n  }\n};\n\n";

  // Generate a convenient CreateX function that uses the above builder
  // to create a table in one go.
  code += "inline flatbuffers::Offset<" + struct_def.name + "> Create";
  code += struct_def.name;
  code += "(flatbuffers::FlatBufferBuilder &_fbb";
  for (auto it = struct_def.fields.vec.begin();
       it != struct_def.fields.vec.end();
       ++it) {
    auto &field = **it;
    if (!field.deprecated) {
      code += ",\n   " + GenTypeWire(parser, field.value.type, " ", true);
      code += field.name + " = ";
      if (field.value.type.enum_def && IsScalar(field.value.type.base_type)) {
        auto ev = field.value.type.enum_def->ReverseLookup(
           static_cast<int>(StringToInt(field.value.constant.c_str())), false);
        if (ev) {
          code += WrapInNameSpace(field.value.type.enum_def->defined_namespace,
                                  GetEnumVal(*field.value.type.enum_def, *ev,
                                             parser.opts));
        } else {
          code += GenUnderlyingCast(field, true, field.value.constant);
        }
      } else if (field.value.type.base_type == BASE_TYPE_BOOL) {
        code += field.value.constant == "0" ? "false" : "true";
      } else {
        code += GenDefaultConstant(field);
      }
    }
  }
  code += ") {\n  " + struct_def.name + "Builder builder_(_fbb);\n";
  for (size_t size = struct_def.sortbysize ? sizeof(largest_scalar_t) : 1;
       size;
       size /= 2) {
    for (auto it = struct_def.fields.vec.rbegin();
         it != struct_def.fields.vec.rend();
         ++it) {
      auto &field = **it;
      if (!field.deprecated &&
          (!struct_def.sortbysize ||
           size == SizeOf(field.value.type.base_type))) {
        code += "  builder_.add_" + field.name + "(" + field.name + ");\n";
      }
    }
  }
  code += "  return builder_.Finish();\n}\n\n";
}

static void GenPadding(const FieldDef &field, std::string &code,
                       int &padding_id,
                       const std::function<void (int bits, std::string &code,
                                                 int &padding_id)> &f) {
  if (field.padding) {
    for (int i = 0; i < 4; i++)
      if (static_cast<int>(field.padding) & (1 << i))
        f((1 << i) * 8, code, padding_id);
    assert(!(field.padding & ~0xF));
  }
}

static void PaddingDefinition(int bits, std::string &code, int &padding_id) {
  code += "  int" + NumToString(bits) +
          "_t __padding" + NumToString(padding_id++) + ";\n";
}

static void PaddingDeclaration(int bits, std::string &code, int &padding_id) {
  (void)bits;
  code += " (void)__padding" + NumToString(padding_id++) + ";";
}

static void PaddingInitializer(int bits, std::string &code, int &padding_id) {
  (void)bits;
  code += ", __padding" + NumToString(padding_id++) + "(0)";
}

// Generate an accessor struct with constructor for a flatbuffers struct.
static void GenStruct(const Parser &parser, StructDef &struct_def,
                      std::string *code_ptr) {
  if (struct_def.generated) return;
  std::string &code = *code_ptr;

  // Generate an accessor struct, with private variables of the form:
  // type name_;
  // Generates manual padding and alignment.
  // Variables are private because they contain little endian data on all
  // platforms.
  GenComment(struct_def.doc_comment, code_ptr, nullptr);
  code += "MANUALLY_ALIGNED_STRUCT(" + NumToString(struct_def.minalign) + ") ";
  code += struct_def.name + " FLATBUFFERS_FINAL_CLASS {\n private:\n";
  int padding_id = 0;
  for (auto it = struct_def.fields.vec.begin();
       it != struct_def.fields.vec.end();
       ++it) {
    auto &field = **it;
    code += "  " + GenTypeGet(parser, field.value.type, " ", "", " ", false);
    code += field.name + "_;\n";
    GenPadding(field, code, padding_id, PaddingDefinition);
  }

  // Generate GetFullyQualifiedName
  code += "\n public:\n";
  GenFullyQualifiedNameGetter(parser, struct_def.name, code);

  // Generate a constructor that takes all fields as arguments.
  code += "  " + struct_def.name + "(";
  for (auto it = struct_def.fields.vec.begin();
       it != struct_def.fields.vec.end();
       ++it) {
    auto &field = **it;
    if (it != struct_def.fields.vec.begin()) code += ", ";
    code += GenTypeGet(parser, field.value.type, " ", "const ", " &", true);
    code += "_" + field.name;
  }
  code += ")\n    : ";
  padding_id = 0;
  for (auto it = struct_def.fields.vec.begin();
       it != struct_def.fields.vec.end();
       ++it) {
    auto &field = **it;
    if (it != struct_def.fields.vec.begin()) code += ", ";
    code += field.name + "_(";
    if (IsScalar(field.value.type.base_type)) {
      code += "flatbuffers::EndianScalar(";
      code += GenUnderlyingCast(field, false, "_" + field.name);
      code += "))";
    } else {
      code += "_" + field.name + ")";
    }
    GenPadding(field, code, padding_id, PaddingInitializer);
  }

  code += " {";
  padding_id = 0;
  for (auto it = struct_def.fields.vec.begin();
       it != struct_def.fields.vec.end();
       ++it) {
    auto &field = **it;
    GenPadding(field, code, padding_id, PaddingDeclaration);
  }
  code += " }\n\n";

  // Generate accessor methods of the form:
  // type name() const { return flatbuffers::EndianScalar(name_); }
  for (auto it = struct_def.fields.vec.begin();
       it != struct_def.fields.vec.end();
       ++it) {
    auto &field = **it;
    GenComment(field.doc_comment, code_ptr, nullptr, "  ");
    auto is_scalar = IsScalar(field.value.type.base_type);
    code += "  " + GenTypeGet(parser, field.value.type, " ", "const ", " &",
                              true);
    code += field.name + "() const { return ";
    code += GenUnderlyingCast(field, true,
      is_scalar
        ? "flatbuffers::EndianScalar(" + field.name + "_)"
        : field.name + "_");
    code += "; }\n";
    if (parser.opts.mutable_buffer) {
      if (is_scalar) {
        code += "  void mutate_" + field.name + "(";
        code += GenTypeBasic(field.value.type, true);
        code += " _" + field.name + ") { flatbuffers::WriteScalar(&";
        code += field.name + "_, ";
        code += GenUnderlyingCast(field, false, "_" + field.name);
        code += "); }\n";
      } else {
        code += "  ";
        code += GenTypeGet(parser, field.value.type, "", "", " &", true);
        code += "mutable_" + field.name + "() { return " + field.name;
        code += "_; }\n";
      }
    }
  }
  code += "};\nSTRUCT_END(" + struct_def.name + ", ";
  code += NumToString(struct_def.bytesize) + ");\n\n";
}

void GenerateNestedNameSpaces(const Namespace *ns, std::string *code_ptr) {
  for (auto it = ns->components.begin(); it != ns->components.end(); ++it) {
    *code_ptr += "namespace " + *it + " {\n";
  }
}

void CloseNestedNameSpaces(const Namespace *ns, std::string *code_ptr) {
  for (auto it = ns->components.rbegin(); it != ns->components.rend(); ++it) {
    *code_ptr += "}  // namespace " + *it + "\n";
  }
}

void CheckNameSpace(const Definition &def, std::string *code_ptr) {
  // Set up the correct namespace. Only open a namespace if
  // the existing one is different.
  // TODO: this could be done more intelligently, by sorting to
  // namespace path and only opening/closing what is necessary, but that's
  // quite a bit more complexity.
  if (code_generator_cur_name_space != def.defined_namespace) {
    if (code_generator_cur_name_space) {
      CloseNestedNameSpaces(code_generator_cur_name_space, code_ptr);
      if (code_generator_cur_name_space->components.size()) *code_ptr += "\n";
    }
    GenerateNestedNameSpaces(def.defined_namespace, code_ptr);
    code_generator_cur_name_space = def.defined_namespace;
    if (code_generator_cur_name_space->components.size()) *code_ptr += "\n";
  }
}

}  // namespace cpp

struct IsAlnum {
  bool operator()(char c) {
    return !isalnum(c);
  }
};

static std::string GeneratedFileName(const std::string &path,
                                     const std::string &file_name) {
  return path + file_name + "_generated.h";
}

namespace cpp {
class CppGenerator : public BaseGenerator {
 public:
  CppGenerator(const Parser &parser, const std::string &path,
               const std::string &file_name)
      : BaseGenerator(parser, path, file_name){};
  // Iterate through all definitions we haven't generate code for (enums,
  // structs,
  // and tables) and output them to a single file.
  bool generate() {
    if (IsEverythingGenerated()) return true;

    std::string code;
    code = code + "// " + FlatBuffersGeneratedWarning();

    // Generate include guard.
    std::string include_guard_ident = file_name_;
    // Remove any non-alpha-numeric characters that may appear in a filename.
    include_guard_ident.erase(
        std::remove_if(include_guard_ident.begin(), include_guard_ident.end(),
                       IsAlnum()),
        include_guard_ident.end());
    std::string include_guard = "FLATBUFFERS_GENERATED_" + include_guard_ident;
    include_guard += "_";
    // For further uniqueness, also add the namespace.
    auto name_space = parser_.namespaces_.back();
    for (auto it = name_space->components.begin();
         it != name_space->components.end(); ++it) {
      include_guard += *it + "_";
    }
    include_guard += "H_";
    std::transform(include_guard.begin(), include_guard.end(),
                   include_guard.begin(), ::toupper);
    code += "#ifndef " + include_guard + "\n";
    code += "#define " + include_guard + "\n\n";

    code += "#include \"flatbuffers/flatbuffers.h\"\n\n";

    if (parser_.opts.include_dependence_headers) {
      int num_includes = 0;
      for (auto it = parser_.included_files_.begin();
           it != parser_.included_files_.end(); ++it) {
        auto basename =
            flatbuffers::StripPath(flatbuffers::StripExtension(it->first));
        if (basename != file_name_) {
          code += "#include \"" + basename + "_generated.h\"\n";
          num_includes++;
        }
      }
      if (num_includes) code += "\n";
    }

    assert(!code_generator_cur_name_space);

    // Generate forward declarations for all structs/tables, since they may
    // have circular references.
    for (auto it = parser_.structs_.vec.begin(); it != parser_.structs_.vec.end();
         ++it) {
      auto &struct_def = **it;
      if (!struct_def.generated) {
        CheckNameSpace(struct_def, &code);
        code += "struct " + struct_def.name + ";\n\n";
      }
    }

    // Generate code for all the enum declarations.
    for (auto it = parser_.enums_.vec.begin(); it != parser_.enums_.vec.end();
         ++it) {
      auto &enum_def = **it;
      if (!enum_def.generated) {
        CheckNameSpace(**it, &code);
        GenEnum(parser_, **it, &code);
      }
    }

    // Generate code for all structs, then all tables.
    for (auto it = parser_.structs_.vec.begin(); it != parser_.structs_.vec.end();
         ++it) {
      auto &struct_def = **it;
      if (struct_def.fixed && !struct_def.generated) {
        CheckNameSpace(struct_def, &code);
        GenStruct(parser_, struct_def, &code);
      }
    }
    for (auto it = parser_.structs_.vec.begin(); it != parser_.structs_.vec.end();
         ++it) {
      auto &struct_def = **it;
      if (!struct_def.fixed && !struct_def.generated) {
        CheckNameSpace(struct_def, &code);
        GenTable(parser_, struct_def, &code);
      }
    }

    // Generate code for union verifiers.
    for (auto it = parser_.enums_.vec.begin(); it != parser_.enums_.vec.end();
         ++it) {
      auto &enum_def = **it;
      if (enum_def.is_union && !enum_def.generated) {
        CheckNameSpace(enum_def, &code);
        GenEnumPost(parser_, enum_def, &code);
      }
    }

    // Generate convenient global helper functions:
    if (parser_.root_struct_def_) {
      CheckNameSpace(*parser_.root_struct_def_, &code);
      auto &name = parser_.root_struct_def_->name;
      std::string qualified_name =
          parser_.namespaces_.back()->GetFullyQualifiedName(name);
      std::string cpp_qualified_name = TranslateNameSpace(qualified_name);

      // The root datatype accessor:
      code += "inline const " + cpp_qualified_name + " *Get";
      code += name;
      code += "(const void *buf) { return flatbuffers::GetRoot<";
      code += cpp_qualified_name + ">(buf); }\n\n";
      if (parser_.opts.mutable_buffer) {
        code += "inline " + name + " *GetMutable";
        code += name;
        code += "(void *buf) { return flatbuffers::GetMutableRoot<";
        code += name + ">(buf); }\n\n";
      }

      // The root verifier:
      code += "inline bool Verify";
      code += name;
      code +=
          "Buffer(flatbuffers::Verifier &verifier) { "
          "return verifier.VerifyBuffer<";
      code += cpp_qualified_name + ">(); }\n\n";

      if (parser_.file_identifier_.length()) {
        // Return the identifier
        code += "inline const char *" + name;
        code += "Identifier() { return \"" + parser_.file_identifier_;
        code += "\"; }\n\n";

        // Check if a buffer has the identifier.
        code += "inline bool " + name;
        code += "BufferHasIdentifier(const void *buf) { return flatbuffers::";
        code += "BufferHasIdentifier(buf, ";
        code += name + "Identifier()); }\n\n";
      }

      if (parser_.file_extension_.length()) {
        // Return the extension
        code += "inline const char *" + name;
        code += "Extension() { return \"" + parser_.file_extension_;
        code += "\"; }\n\n";
      }

      // Finish a buffer with a given root object:
      code += "inline void Finish" + name;
      code +=
          "Buffer(flatbuffers::FlatBufferBuilder &fbb, flatbuffers::Offset<";
      code += cpp_qualified_name + "> root) { fbb.Finish(root";
      if (parser_.file_identifier_.length())
        code += ", " + name + "Identifier()";
      code += "); }\n\n";
    }

    assert(code_generator_cur_name_space);
    CloseNestedNameSpaces(code_generator_cur_name_space, &code);

    code_generator_cur_name_space = nullptr;

    // Close the include guard.
    code += "\n#endif  // " + include_guard + "\n";

    return SaveFile(GeneratedFileName(path_, file_name_).c_str(), code, false);
  }
};
}  // namespace cpp

bool GenerateCPP(const Parser &parser, const std::string &path,
                 const std::string &file_name) {
  cpp::CppGenerator generator(parser, path, file_name);
  return generator.generate();
}

std::string CPPMakeRule(const Parser &parser,
                        const std::string &path,
                        const std::string &file_name) {
  std::string filebase = flatbuffers::StripPath(
      flatbuffers::StripExtension(file_name));
  std::string make_rule = GeneratedFileName(path, filebase) + ": ";
  auto included_files = parser.GetIncludedFilesRecursive(file_name);
  for (auto it = included_files.begin();
       it != included_files.end(); ++it) {
    make_rule += " " + *it;
  }
  return make_rule;
}

}  // namespace flatbuffers

