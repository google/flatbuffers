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

namespace flatbuffers {
namespace cpp {

// Return a C++ type from the table in idl.h
static std::string GenTypeBasic(const Type &type) {
  static const char *ctypename[] = {
    #define FLATBUFFERS_TD(ENUM, IDLTYPE, CTYPE, JTYPE, GTYPE) #CTYPE,
      FLATBUFFERS_GEN_TYPES(FLATBUFFERS_TD)
    #undef FLATBUFFERS_TD
  };
  return ctypename[type.base_type];
}

static std::string GenTypeWire(const Parser &parser, const Type &type,
                               const char *postfix);

// Return a C++ pointer type, specialized to the actual struct/table types,
// and vector element types.
static std::string GenTypePointer(const Parser &parser, const Type &type) {
  switch (type.base_type) {
    case BASE_TYPE_STRING:
      return "flatbuffers::String";
    case BASE_TYPE_VECTOR:
      return "flatbuffers::Vector<" +
             GenTypeWire(parser, type.VectorType(), "") + ">";
    case BASE_TYPE_STRUCT: {
      auto ns = type.struct_def->defined_namespace;
      if (parser.namespaces_.back() != ns) {
        std::string qualified_name;
        for (auto it = ns->components.begin();
                 it != ns->components.end(); ++it) {
          qualified_name += *it + "::";
        }
        return qualified_name + type.struct_def->name;
      } else {
        return type.struct_def->name;
      }
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
                               const char *postfix) {
  return IsScalar(type.base_type)
    ? GenTypeBasic(type) + postfix
    : IsStruct(type)
      ? "const " + GenTypePointer(parser, type) + " *"
      : "flatbuffers::Offset<" + GenTypePointer(parser, type) + ">" + postfix;
}

// Return a C++ type for any type (scalar/pointer) that reflects its
// serialized size.
static std::string GenTypeSize(const Parser &parser, const Type &type) {
  return IsScalar(type.base_type)
    ? GenTypeBasic(type)
    : IsStruct(type)
      ? GenTypePointer(parser, type)
      : "flatbuffers::uoffset_t";
}

// Return a C++ type for any type (scalar/pointer) specifically for
// using a flatbuffer.
static std::string GenTypeGet(const Parser &parser, const Type &type,
                              const char *afterbasic, const char *beforeptr,
                              const char *afterptr) {
  return IsScalar(type.base_type)
    ? GenTypeBasic(type) + afterbasic
    : beforeptr + GenTypePointer(parser, type) + afterptr;
}

// Generate a documentation comment, if available.
static void GenComment(const std::string &dc,
                       std::string *code_ptr,
                       const char *prefix = "") {
  std::string &code = *code_ptr;
  if (dc.length()) {
    code += std::string(prefix) + "///" + dc + "\n";
  }
}

static std::string GenEnumVal(const EnumDef &enum_def, const EnumVal &enum_val,
                              const GeneratorOptions &opts) {
  return opts.prefixed_enums ? enum_def.name + "_" + enum_val.name
                             : enum_val.name;
}

// Generate an enum declaration and an enum string lookup table.
static void GenEnum(EnumDef &enum_def, std::string *code_ptr,
                    std::string *code_ptr_post,
                    const GeneratorOptions &opts) {
  if (enum_def.generated) return;
  std::string &code = *code_ptr;
  std::string &code_post = *code_ptr_post;
  GenComment(enum_def.doc_comment, code_ptr);
  code += "enum {\n";
  for (auto it = enum_def.vals.vec.begin();
       it != enum_def.vals.vec.end();
       ++it) {
    auto &ev = **it;
    GenComment(ev.doc_comment, code_ptr, "  ");
    code += "  " + GenEnumVal(enum_def, ev, opts) + " = ";
    code += NumToString(ev.value);
    code += (it + 1) != enum_def.vals.vec.end() ? ",\n" : "\n";
  }
  code += "};\n\n";

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
    code += "(int e) { return EnumNames" + enum_def.name + "()[e";
    if (enum_def.vals.vec.front()->value)
      code += " - " + GenEnumVal(enum_def, *enum_def.vals.vec.front(), opts);
    code += "]; }\n\n";
  }

  if (enum_def.is_union) {
    // Generate a verifier function for this union that can be called by the
    // table verifier functions. It uses a switch case to select a specific
    // verifier function to call, this should be safe even if the union type
    // has been corrupted, since the verifiers will simply fail when called
    // on the wrong type.
    auto signature = "bool Verify" + enum_def.name +
                     "(const flatbuffers::Verifier &verifier, " +
                     "const void *union_obj, uint8_t type)";
    code += signature + ";\n\n";
    code_post += signature + " {\n  switch (type) {\n";
    for (auto it = enum_def.vals.vec.begin();
         it != enum_def.vals.vec.end();
         ++it) {
      auto &ev = **it;
      code_post += "    case " + GenEnumVal(enum_def, ev, opts);
      if (!ev.value) {
        code_post += ": return true;\n";  // "NONE" enum value.
      } else {
        code_post += ": return verifier.VerifyTable(reinterpret_cast<const ";
        code_post += ev.struct_def->name + " *>(union_obj));\n";
      }
    }
    code_post += "    default: return false;\n  }\n}\n\n";
  }
}

// Generate an accessor struct, builder structs & function for a table.
static void GenTable(const Parser &parser, StructDef &struct_def,
                     std::string *code_ptr) {
  if (struct_def.generated) return;
  std::string &code = *code_ptr;

  // Generate an accessor struct, with methods of the form:
  // type name() const { return GetField<type>(offset, defaultval); }
  GenComment(struct_def.doc_comment, code_ptr);
  code += "struct " + struct_def.name + " : private flatbuffers::Table";
  code += " {\n";
  for (auto it = struct_def.fields.vec.begin();
       it != struct_def.fields.vec.end();
       ++it) {
    auto &field = **it;
    if (!field.deprecated) {  // Deprecated fields won't be accessible.
      GenComment(field.doc_comment, code_ptr, "  ");
      code += "  " + GenTypeGet(parser, field.value.type, " ", "const ", " *");
      code += field.name + "() const { return ";
      // Call a different accessor for pointers, that indirects.
      code += IsScalar(field.value.type.base_type)
        ? "GetField<"
        : (IsStruct(field.value.type) ? "GetStruct<" : "GetPointer<");
      code += GenTypeGet(parser, field.value.type, "", "const ", " *") + ">(";
      code += NumToString(field.value.offset);
      // Default value as second arg for non-pointer types.
      if (IsScalar(field.value.type.base_type))
        code += ", " + field.value.constant;
      code += "); }\n";
      auto nested = field.attributes.Lookup("nested_flatbuffer");
      if (nested) {
        auto nested_root = parser.structs_.Lookup(nested->constant);
        assert(nested_root);  // Guaranteed to exist by parser.
        code += "  const " + nested_root->name + " *" + field.name;
        code += "_nested_root() { return flatbuffers::GetRoot<";
        code += nested_root->name + ">(" + field.name + "()->Data()); }\n";
      }
    }
  }
  // Generate a verifier function that can check a buffer from an untrusted
  // source will never cause reads outside the buffer.
  code += "  bool Verify(const flatbuffers::Verifier &verifier) const {\n";
  code += "    return VerifyTable(verifier)";
  std::string prefix = " &&\n           ";
  for (auto it = struct_def.fields.vec.begin();
       it != struct_def.fields.vec.end();
       ++it) {
    auto &field = **it;
    if (!field.deprecated) {
      code += prefix + "VerifyField<" + GenTypeSize(parser, field.value.type);
      code += ">(verifier, " + NumToString(field.value.offset);
      code += " /* " + field.name + " */)";
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
      code += GenTypeWire(parser, field.value.type, " ") + field.name;
      code += ") { fbb_.Add";
      if (IsScalar(field.value.type.base_type))
        code += "Element<" + GenTypeWire(parser, field.value.type, "") + ">";
      else if (IsStruct(field.value.type))
        code += "Struct";
      else
        code += "Offset";
      code += "(" + NumToString(field.value.offset) + ", " + field.name;
      if (IsScalar(field.value.type.base_type))
        code += ", " + field.value.constant;
      code += "); }\n";
    }
  }
  code += "  " + struct_def.name;
  code += "Builder(flatbuffers::FlatBufferBuilder &_fbb) : fbb_(_fbb) ";
  code += "{ start_ = fbb_.StartTable(); }\n";
  code += "  " + struct_def.name + "Builder &operator=(const ";
  code += struct_def.name + "Builder &);\n";
  code += "  flatbuffers::Offset<" + struct_def.name;
  code += "> Finish() { return flatbuffers::Offset<" + struct_def.name;
  code += ">(fbb_.EndTable(start_, ";
  code += NumToString(struct_def.fields.vec.size()) + ")); }\n};\n\n";

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
      code += ",\n   " + GenTypeWire(parser, field.value.type, " ");
      code += field.name + " = " + field.value.constant;
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
  GenComment(struct_def.doc_comment, code_ptr);
  code += "MANUALLY_ALIGNED_STRUCT(" + NumToString(struct_def.minalign) + ") ";
  code += struct_def.name + " {\n private:\n";
  int padding_id = 0;
  for (auto it = struct_def.fields.vec.begin();
       it != struct_def.fields.vec.end();
       ++it) {
    auto &field = **it;
    code += "  " + GenTypeGet(parser, field.value.type, " ", "", " ");
    code += field.name + "_;\n";
    if (field.padding) {
      for (int i = 0; i < 4; i++)
        if (static_cast<int>(field.padding) & (1 << i))
          code += "  int" + NumToString((1 << i) * 8) +
                  "_t __padding" + NumToString(padding_id++) + ";\n";
      assert(!(field.padding & ~0xF));
    }
  }

  // Generate a constructor that takes all fields as arguments.
  code += "\n public:\n  " + struct_def.name + "(";
  for (auto it = struct_def.fields.vec.begin();
       it != struct_def.fields.vec.end();
       ++it) {
    auto &field = **it;
    if (it != struct_def.fields.vec.begin()) code += ", ";
    code += GenTypeGet(parser, field.value.type, " ", "const ", " &");
    code += field.name;
  }
  code += ")\n    : ";
  padding_id = 0;
  for (auto it = struct_def.fields.vec.begin();
       it != struct_def.fields.vec.end();
       ++it) {
    auto &field = **it;
    if (it != struct_def.fields.vec.begin()) code += ", ";
    code += field.name + "_(";
    if (IsScalar(field.value.type.base_type))
      code += "flatbuffers::EndianScalar(" + field.name + "))";
    else
      code += field.name + ")";
    if (field.padding)
      code += ", __padding" + NumToString(padding_id++) + "(0)";
  }
  code += " {";
  padding_id = 0;
  for (auto it = struct_def.fields.vec.begin();
       it != struct_def.fields.vec.end();
       ++it) {
    auto &field = **it;
    if (field.padding)
      code += " (void)__padding" + NumToString(padding_id++) + ";";
  }
  code += " }\n\n";

  // Generate accessor methods of the form:
  // type name() const { return flatbuffers::EndianScalar(name_); }
  for (auto it = struct_def.fields.vec.begin();
       it != struct_def.fields.vec.end();
       ++it) {
    auto &field = **it;
    GenComment(field.doc_comment, code_ptr, "  ");
    code += "  " + GenTypeGet(parser, field.value.type, " ", "const ", " &");
    code += field.name + "() const { return ";
    if (IsScalar(field.value.type.base_type))
      code += "flatbuffers::EndianScalar(" + field.name + "_)";
    else
      code += field.name + "_";
    code += "; }\n";
  }
  code += "};\nSTRUCT_END(" + struct_def.name + ", ";
  code += NumToString(struct_def.bytesize) + ");\n\n";
}

void GenerateNestedNameSpaces(Namespace *ns, std::string *code_ptr) {
  for (auto it = ns->components.begin(); it != ns->components.end(); ++it) {
    *code_ptr += "namespace " + *it + " {\n";
  }
}

void CloseNestedNameSpaces(Namespace *ns, std::string *code_ptr) {
  for (auto it = ns->components.rbegin(); it != ns->components.rend(); ++it) {
    *code_ptr += "}  // namespace " + *it + "\n";
  }
}

}  // namespace cpp

// Iterate through all definitions we haven't generate code for (enums, structs,
// and tables) and output them to a single file.
std::string GenerateCPP(const Parser &parser,
                        const std::string &include_guard_ident,
                        const GeneratorOptions &opts) {
  using namespace cpp;

  // Generate code for all the enum declarations.
  std::string enum_code, enum_code_post;
  for (auto it = parser.enums_.vec.begin();
       it != parser.enums_.vec.end(); ++it) {
    GenEnum(**it, &enum_code, &enum_code_post, opts);
  }

  // Generate forward declarations for all structs/tables, since they may
  // have circular references.
  std::string forward_decl_code_same_namespace;
  std::string forward_decl_code_other_namespace;
  Namespace *cur_name_space = nullptr;
  for (auto it = parser.structs_.vec.begin();
       it != parser.structs_.vec.end(); ++it) {
    auto &struct_def = **it;
    auto decl = "struct " + struct_def.name + ";\n";
    if (struct_def.defined_namespace == parser.namespaces_.back()) {
      forward_decl_code_same_namespace += decl;
    } else {
      // Wrap this decl in the correct namespace. Only open a namespace if
      // the adjacent one is different.
      // TODO: this could be done more intelligently, by sorting to
      // namespace path and only opening/closing what is necessary, but that's
      // quite a bit more complexity.
      if (cur_name_space != struct_def.defined_namespace) {
        if (cur_name_space) {
          CloseNestedNameSpaces(cur_name_space,
                                &forward_decl_code_other_namespace);
        }
        GenerateNestedNameSpaces(struct_def.defined_namespace,
                                 &forward_decl_code_other_namespace);
        cur_name_space = struct_def.defined_namespace;
      }
      forward_decl_code_other_namespace += decl;
    }
  }
  if (cur_name_space) {
    CloseNestedNameSpaces(cur_name_space,
                          &forward_decl_code_other_namespace);
  }

  // Generate code for all structs, then all tables.
  std::string decl_code;
  for (auto it = parser.structs_.vec.begin();
       it != parser.structs_.vec.end(); ++it) {
    if ((**it).fixed) GenStruct(parser, **it, &decl_code);
  }
  for (auto it = parser.structs_.vec.begin();
       it != parser.structs_.vec.end(); ++it) {
    if (!(**it).fixed) GenTable(parser, **it, &decl_code);
  }

  // Only output file-level code if there were any declarations.
  if (enum_code.length() || decl_code.length()) {
    std::string code;
    code = "// automatically generated by the FlatBuffers compiler,"
           " do not modify\n\n";

    // Generate include guard.
    std::string include_guard = "FLATBUFFERS_GENERATED_" + include_guard_ident;
    include_guard += "_";
    auto name_space = parser.namespaces_.back();
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

    code += forward_decl_code_other_namespace;
    code += "\n";

    GenerateNestedNameSpaces(name_space, &code);
    code += "\n";

    code += forward_decl_code_same_namespace;
    code += "\n";

    // Output the main declaration code from above.
    code += enum_code;
    code += decl_code;
    code += enum_code_post;

    // Generate convenient global helper functions:
    if (parser.root_struct_def) {
      // The root datatype accessor:
      code += "inline const " + parser.root_struct_def->name + " *Get";
      code += parser.root_struct_def->name;
      code += "(const void *buf) { return flatbuffers::GetRoot<";
      code += parser.root_struct_def->name + ">(buf); }\n\n";

      // The root verifier:
      code += "inline bool Verify";
      code += parser.root_struct_def->name;
      code += "Buffer(const flatbuffers::Verifier &verifier) { "
              "return verifier.VerifyBuffer<";
      code += parser.root_struct_def->name + ">(); }\n\n";

      // Finish a buffer with a given root object:
      code += "inline void Finish" + parser.root_struct_def->name;
      code += "Buffer(flatbuffers::FlatBufferBuilder &fbb, flatbuffers::Offset<";
      code += parser.root_struct_def->name + "> root) { fbb.Finish(root";
      if (parser.file_identifier_.length())
        code += ", \"" + parser.file_identifier_ + "\"";
      code += "); }\n\n";

      if (parser.file_identifier_.length()) {
        // Check if a buffer has the identifier.
        code += "inline bool " + parser.root_struct_def->name;
        code += "BufferHasIdentifier(const void *buf) { return flatbuffers::";
        code += "BufferHasIdentifier(buf, \"" + parser.file_identifier_;
        code += "\"); }\n\n";
      }
    }

    CloseNestedNameSpaces(name_space, &code);

    // Close the include guard.
    code += "\n#endif  // " + include_guard + "\n";

    return code;
  }

  return std::string();
}

bool GenerateCPP(const Parser &parser,
                 const std::string &path,
                 const std::string &file_name,
                 const GeneratorOptions &opts) {
    auto code = GenerateCPP(parser, file_name, opts);
    return !code.length() ||
           SaveFile((path + file_name + "_generated.h").c_str(), code, false);
}

}  // namespace flatbuffers

