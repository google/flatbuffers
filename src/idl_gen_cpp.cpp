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

struct IsAlnum {
  bool operator()(char c) { return !isalnum(c); }
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
      : BaseGenerator(parser, path, file_name, "", "::"),
        cur_name_space_(nullptr){};
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

    assert(!cur_name_space_);

    // Generate forward declarations for all structs/tables, since they may
    // have circular references.
    for (auto it = parser_.structs_.vec.begin();
         it != parser_.structs_.vec.end(); ++it) {
      auto &struct_def = **it;
      if (!struct_def.generated) {
        SetNameSpace(struct_def.defined_namespace, &code);
        code += "struct " + struct_def.name + ";\n";
        if (parser_.opts.generate_object_based_api && !struct_def.fixed) {
          code += "struct " + NativeName(struct_def.name) + ";\n";
        }
        code += "\n";
      }
    }

    // Generate code for all the enum declarations.
    for (auto it = parser_.enums_.vec.begin(); it != parser_.enums_.vec.end();
         ++it) {
      auto &enum_def = **it;
      if (!enum_def.generated) {
        SetNameSpace((**it).defined_namespace, &code);
        GenEnum(**it, &code);
      }
    }

    // Generate code for all structs, then all tables.
    for (auto it = parser_.structs_.vec.begin();
         it != parser_.structs_.vec.end(); ++it) {
      auto &struct_def = **it;
      if (struct_def.fixed && !struct_def.generated) {
        SetNameSpace(struct_def.defined_namespace, &code);
        GenStruct(struct_def, &code);
      }
    }
    for (auto it = parser_.structs_.vec.begin();
         it != parser_.structs_.vec.end(); ++it) {
      auto &struct_def = **it;
      if (!struct_def.fixed && !struct_def.generated) {
        SetNameSpace(struct_def.defined_namespace, &code);
        GenTable(struct_def, &code);
      }
    }
    for (auto it = parser_.structs_.vec.begin();
         it != parser_.structs_.vec.end(); ++it) {
      auto &struct_def = **it;
      if (!struct_def.fixed && !struct_def.generated) {
        SetNameSpace(struct_def.defined_namespace, &code);
        GenTablePost(struct_def, &code);
      }
    }

    // Generate code for union verifiers.
    for (auto it = parser_.enums_.vec.begin(); it != parser_.enums_.vec.end();
         ++it) {
      auto &enum_def = **it;
      if (enum_def.is_union && !enum_def.generated) {
        SetNameSpace(enum_def.defined_namespace, &code);
        GenUnionPost(enum_def, &code);
      }
    }

    // Generate convenient global helper functions:
    if (parser_.root_struct_def_) {
      SetNameSpace((*parser_.root_struct_def_).defined_namespace, &code);
      auto &name = parser_.root_struct_def_->name;
      std::string qualified_name =
          parser_.namespaces_.back()->GetFullyQualifiedName(name);
      std::string cpp_qualified_name = TranslateNameSpace(qualified_name);

      // The root datatype accessor:
      code += "inline const " + cpp_qualified_name + " *Get";
      code += name;
      code += "(const void *buf) {\n  return flatbuffers::GetRoot<";
      code += cpp_qualified_name + ">(buf);\n}\n\n";
      if (parser_.opts.mutable_buffer) {
        code += "inline " + name + " *GetMutable";
        code += name;
        code += "(void *buf) {\n  return flatbuffers::GetMutableRoot<";
        code += name + ">(buf);\n}\n\n";
      }

      if (parser_.file_identifier_.length()) {
        // Return the identifier
        code += "inline const char *" + name;
        code += "Identifier() {\n  return \"" + parser_.file_identifier_;
        code += "\";\n}\n\n";

        // Check if a buffer has the identifier.
        code += "inline bool " + name;
        code += "BufferHasIdentifier(const void *buf) {\n  return flatbuffers::";
        code += "BufferHasIdentifier(buf, ";
        code += name + "Identifier());\n}\n\n";
      }

      // The root verifier:
      code += "inline bool Verify";
      code += name;
      code +=
          "Buffer(flatbuffers::Verifier &verifier) {\n"
          "  return verifier.VerifyBuffer<";
      code += cpp_qualified_name + ">(";
      if (parser_.file_identifier_.length())
        code += name + "Identifier()";
      else
        code += "nullptr";
      code += ");\n}\n\n";

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
      code += cpp_qualified_name + "> root) {\n  fbb.Finish(root";
      if (parser_.file_identifier_.length())
        code += ", " + name + "Identifier()";
      code += ");\n}\n\n";

      if (parser_.opts.generate_object_based_api) {
        // A convenient root unpack function.
        auto native_name =
            NativeName(WrapInNameSpace(*parser_.root_struct_def_));
        code += "inline " + GenTypeNativePtr(native_name, nullptr, false);
        code += " UnPack" + name;
        code += "(const void *buf, const flatbuffers::resolver_function_t *";
        code += "resolver = nullptr) {\n  return ";
        code += GenTypeNativePtr(native_name, nullptr, true);
        code += "(Get" + name + "(buf)->UnPack(resolver));\n}\n\n";
      }
    }

    assert(cur_name_space_);
    SetNameSpace(nullptr, &code);

    // Close the include guard.
    code += "#endif  // " + include_guard + "\n";

    return SaveFile(GeneratedFileName(path_, file_name_).c_str(), code, false);
  }

 private:
  // This tracks the current namespace so we can insert namespace declarations.
  const Namespace *cur_name_space_;

  const Namespace *CurrentNameSpace() { return cur_name_space_; }

  // Translates a qualified name in flatbuffer text format to the same name in
  // the equivalent C++ namespace.
  static std::string TranslateNameSpace(const std::string &qualified_name) {
    std::string cpp_qualified_name = qualified_name;
    size_t start_pos = 0;
    while ((start_pos = cpp_qualified_name.find(".", start_pos)) !=
           std::string::npos) {
      cpp_qualified_name.replace(start_pos, 1, "::");
    }
    return cpp_qualified_name;
  }

  // Return a C++ type from the table in idl.h
  std::string GenTypeBasic(const Type &type, bool user_facing_type) {
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

  // Return a C++ pointer type, specialized to the actual struct/table types,
  // and vector element types.
  std::string GenTypePointer(const Type &type) {
    switch (type.base_type) {
      case BASE_TYPE_STRING:
        return "flatbuffers::String";
      case BASE_TYPE_VECTOR:
        return "flatbuffers::Vector<" +
               GenTypeWire(type.VectorType(), "", false) + ">";
      case BASE_TYPE_STRUCT:
        return WrapInNameSpace(*type.struct_def);
      case BASE_TYPE_UNION:
      // fall through
      default:
        return "void";
    }
  }

  // Return a C++ type for any type (scalar/pointer) specifically for
  // building a flatbuffer.
  std::string GenTypeWire(const Type &type, const char *postfix,
                          bool user_facing_type) {
    return IsScalar(type.base_type)
               ? GenTypeBasic(type, user_facing_type) + postfix
               : IsStruct(type) ? "const " + GenTypePointer(type) + " *"
                                : "flatbuffers::Offset<" +
                                      GenTypePointer(type) + ">" + postfix;
  }

  // Return a C++ type for any type (scalar/pointer) that reflects its
  // serialized size.
  std::string GenTypeSize(const Type &type) {
    return IsScalar(type.base_type) ? GenTypeBasic(type, false)
                                    : IsStruct(type) ? GenTypePointer(type)
                                                     : "flatbuffers::uoffset_t";
  }

  // TODO(wvo): make this configurable.
  std::string NativeName(const std::string &name) { return name + "T"; }

  const std::string &PtrType(const FieldDef *field) {
    auto attr = field ? field->attributes.Lookup("cpp_ptr_type") : nullptr;
    return attr ? attr->constant : parser_.opts.cpp_object_api_pointer_type;
  }

  std::string GenTypeNativePtr(const std::string &type, const FieldDef *field,
                               bool is_constructor) {
    auto &ptr_type = PtrType(field);
    if (ptr_type == "naked") return is_constructor ? "" : type + " *";
    return ptr_type + "<" + type + ">";
  }

  std::string GenPtrGet(const FieldDef &field) {
    auto &ptr_type = PtrType(&field);
    return ptr_type == "naked" ? "" : ".get()";
  }

  std::string GenTypeNative(const Type &type, bool invector,
                            const FieldDef &field) {
    switch (type.base_type) {
      case BASE_TYPE_STRING:
        return "std::string";
      case BASE_TYPE_VECTOR:
        return "std::vector<" + GenTypeNative(type.VectorType(), true, field) +
               ">";
      case BASE_TYPE_STRUCT:
        if (IsStruct(type)) {
          if (invector || field.native_inline) {
            return WrapInNameSpace(*type.struct_def);
          } else {
            return GenTypeNativePtr(WrapInNameSpace(*type.struct_def), &field,
                                    false);
          }
        } else {
          return GenTypeNativePtr(NativeName(WrapInNameSpace(*type.struct_def)),
                                  &field, false);
        }
      case BASE_TYPE_UNION:
        return type.enum_def->name + "Union";
      default:
        return GenTypeBasic(type, true);
    }
  }

  // Return a C++ type for any type (scalar/pointer) specifically for
  // using a flatbuffer.
  std::string GenTypeGet(const Type &type, const char *afterbasic,
                         const char *beforeptr, const char *afterptr,
                         bool user_facing_type) {
    return IsScalar(type.base_type)
               ? GenTypeBasic(type, user_facing_type) + afterbasic
               : beforeptr + GenTypePointer(type) + afterptr;
  }

  static std::string GenEnumDecl(const EnumDef &enum_def,
                                 const IDLOptions &opts) {
    return (opts.scoped_enums ? "enum class " : "enum ") + enum_def.name;
  }

  static std::string GenEnumValDecl(const EnumDef &enum_def,
                                    const std::string &enum_val,
                                    const IDLOptions &opts) {
    return opts.prefixed_enums ? enum_def.name + "_" + enum_val : enum_val;
  }

  static std::string GetEnumValUse(const EnumDef &enum_def,
                                   const EnumVal &enum_val,
                                   const IDLOptions &opts) {
    if (opts.scoped_enums) {
      return enum_def.name + "::" + enum_val.name;
    } else if (opts.prefixed_enums) {
      return enum_def.name + "_" + enum_val.name;
    } else {
      return enum_val.name;
    }
  }

  std::string UnionVerifySignature(EnumDef &enum_def) {
    return "inline bool Verify" + enum_def.name +
           "(flatbuffers::Verifier &verifier, const void *union_obj, " +
           enum_def.name + " type)";
  }

  std::string UnionUnPackSignature(EnumDef &enum_def, bool inclass) {
    return (inclass ? "static " : "") +
           std::string("flatbuffers::NativeTable *") +
           (inclass ? "" : enum_def.name + "Union::") +
           "UnPack(const void *union_obj, " + enum_def.name +
           " type, const flatbuffers::resolver_function_t *resolver)";
  }

  std::string UnionPackSignature(EnumDef &enum_def, bool inclass) {
    return "flatbuffers::Offset<void> " +
           (inclass ? "" : enum_def.name + "Union::") +
           "Pack(flatbuffers::FlatBufferBuilder &_fbb, " +
           "const flatbuffers::rehasher_function_t *rehasher" +
           (inclass ? " = nullptr" : "") + ") const";
  }

  std::string TableCreateSignature(StructDef &struct_def, bool predecl) {
    return "inline flatbuffers::Offset<" + struct_def.name + "> Create" +
           struct_def.name  +
           "(flatbuffers::FlatBufferBuilder &_fbb, const " +
           NativeName(struct_def.name) +
           " *_o, const flatbuffers::rehasher_function_t *rehasher" +
           (predecl ? " = nullptr" : "") + ")";
  }

  std::string TablePackSignature(StructDef &struct_def, bool inclass) {
    return std::string(inclass ? "static " : "") +
           "flatbuffers::Offset<" + struct_def.name + "> " +
           (inclass ? "" : struct_def.name + "::") +
           "Pack(flatbuffers::FlatBufferBuilder &_fbb, " +
           "const " + NativeName(struct_def.name) + "* _o, " +
           "const flatbuffers::rehasher_function_t *_rehasher" +
           (inclass ? " = nullptr" : "") + ")";
  }

  std::string TableUnPackSignature(StructDef &struct_def, bool inclass) {
    return NativeName(struct_def.name) + " *" +
           (inclass ? "" : struct_def.name + "::") +
           "UnPack(const flatbuffers::resolver_function_t *resolver" +
           (inclass ? " = nullptr" : "") + ") const";
  }

  // Generate an enum declaration and an enum string lookup table.
  void GenEnum(EnumDef &enum_def, std::string *code_ptr) {
    std::string &code = *code_ptr;
    GenComment(enum_def.doc_comment, code_ptr, nullptr);
    code += GenEnumDecl(enum_def, parser_.opts);
    if (parser_.opts.scoped_enums)
      code += " : " + GenTypeBasic(enum_def.underlying_type, false);
    code += " {\n";
    int64_t anyv = 0;
    EnumVal *minv = nullptr, *maxv = nullptr;
    for (auto it = enum_def.vals.vec.begin(); it != enum_def.vals.vec.end();
         ++it) {
      auto &ev = **it;
      GenComment(ev.doc_comment, code_ptr, nullptr, "  ");
      code += "  " + GenEnumValDecl(enum_def, ev.name, parser_.opts) + " = ";
      code += NumToString(ev.value);
      if (it != enum_def.vals.vec.end() - 1) code += ",\n";
      minv = !minv || minv->value > ev.value ? &ev : minv;
      maxv = !maxv || maxv->value < ev.value ? &ev : maxv;
      anyv |= ev.value;
    }
    if (parser_.opts.scoped_enums || parser_.opts.prefixed_enums) {
      assert(minv && maxv);
      if (enum_def.attributes.Lookup("bit_flags")) {
        if (minv->value != 0)  // If the user didn't defined NONE value
          code += ",\n  " + GenEnumValDecl(enum_def, "NONE", parser_.opts) + " = 0";
        if (maxv->value != anyv)  // If the user didn't defined ANY value
          code += ",\n  " + GenEnumValDecl(enum_def, "ANY", parser_.opts) + " = " +
                  NumToString(anyv);
      } else {  // MIN & MAX are useless for bit_flags
        code += ",\n  " + GenEnumValDecl(enum_def, "MIN", parser_.opts) + " = ";
        code += GenEnumValDecl(enum_def, minv->name, parser_.opts);
        code += ",\n  " + GenEnumValDecl(enum_def, "MAX", parser_.opts) + " = ";
        code += GenEnumValDecl(enum_def, maxv->name, parser_.opts);
      }
    }
    code += "\n};\n";
    if (parser_.opts.scoped_enums && enum_def.attributes.Lookup("bit_flags"))
      code += "DEFINE_BITMASK_OPERATORS(" + enum_def.name + ", " +
              GenTypeBasic(enum_def.underlying_type, false) + ")\n";
    code += "\n";

    // Generate a generate string table for enum values.
    // Problem is, if values are very sparse that could generate really big
    // tables. Ideally in that case we generate a map lookup instead, but for
    // the moment we simply don't output a table at all.
    auto range =
        enum_def.vals.vec.back()->value - enum_def.vals.vec.front()->value + 1;
    // Average distance between values above which we consider a table
    // "too sparse". Change at will.
    static const int kMaxSparseness = 5;
    if (range / static_cast<int64_t>(enum_def.vals.vec.size()) <
        kMaxSparseness) {
      code += "inline const char **EnumNames" + enum_def.name + "() {\n";
      code += "  static const char *names[] = { ";
      auto val = enum_def.vals.vec.front()->value;
      for (auto it = enum_def.vals.vec.begin(); it != enum_def.vals.vec.end();
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
        code += GetEnumValUse(enum_def, *enum_def.vals.vec.front(), parser_.opts) +
                ")";
      }
      code += "]; }\n\n";
    }

    // Generate type traits for unions to map from a type to union enum value.
    if (enum_def.is_union) {
      for (auto it = enum_def.vals.vec.begin();
        it != enum_def.vals.vec.end();
        ++it) {
        auto &ev = **it;
        if (it == enum_def.vals.vec.begin()) {
          code += "template<typename T> struct " + enum_def.name + "Traits {\n";
        }
        else {
          code += "template<> struct " + enum_def.name + "Traits<" +
                  WrapInNameSpace(*ev.struct_def) + "> {\n";
        }
        code += "  static const " + enum_def.name + " enum_value = " +
                GetEnumValUse(enum_def, ev, parser_.opts) + ";\n";
        code += "};\n\n";
      }
    }

    if (parser_.opts.generate_object_based_api && enum_def.is_union) {
      // Generate a union type
      code += "struct " + enum_def.name + "Union {\n";
      code += "  " + enum_def.name + " type;\n\n";
      code += "  flatbuffers::NativeTable *table;\n";
      code += "  " + enum_def.name + "Union() : type(";
      code += GetEnumValUse(enum_def, *enum_def.vals.Lookup("NONE"), parser_.opts);
      code += "), table(nullptr) {}\n";
      code += "  " + enum_def.name + "Union(const ";
      code += enum_def.name + "Union &);\n";
      code += "  " + enum_def.name + "Union &operator=(const ";
      code += enum_def.name + "Union &);\n";
      code += "  ~" + enum_def.name + "Union() { Reset(); }\n";
      code += "  void Reset();\n\n";
      code += "  template <typename T>\n";
      code += "  void Set(T&& value) {\n";
      code += "    Reset();\n";
      code += "    type = " + enum_def.name;
      code += "Traits<typename T::TableType>::enum_value;\n";
      code += "    if (type != ";
      code += GetEnumValUse(enum_def, *enum_def.vals.Lookup("NONE"), parser_.opts);
      code += ") {\n";
      code += "      table = new T(std::forward<T>(value));\n";
      code += "    }\n";
      code += "  }\n\n";
      code += "  " + UnionUnPackSignature(enum_def, true) + ";\n";
      code += "  " + UnionPackSignature(enum_def, true) + ";\n\n";
      for (auto it = enum_def.vals.vec.begin(); it != enum_def.vals.vec.end();
           ++it) {
        auto &ev = **it;
        if (ev.value) {
          auto native_name = NativeName(WrapInNameSpace(*ev.struct_def));
          code += "  " + native_name + " *As";
          code += ev.name + "() { return type == ";
          code += GetEnumValUse(enum_def, ev, parser_.opts);
          code += " ? reinterpret_cast<" + native_name;
          code += " *>(table) : nullptr; }\n";
        }
      }
      code += "};\n\n";
    }

    if (enum_def.is_union) {
      code += UnionVerifySignature(enum_def) + ";\n\n";
    }
  }

  void GenUnionPost(EnumDef &enum_def, std::string *code_ptr) {
    // Generate a verifier function for this union that can be called by the
    // table verifier functions. It uses a switch case to select a specific
    // verifier function to call, this should be safe even if the union type
    // has been corrupted, since the verifiers will simply fail when called
    // on the wrong type.
    std::string &code = *code_ptr;
    code += UnionVerifySignature(enum_def) + " {\n  switch (type) {\n";
    for (auto it = enum_def.vals.vec.begin(); it != enum_def.vals.vec.end();
         ++it) {
      auto &ev = **it;
      code += "    case " + GetEnumValUse(enum_def, ev, parser_.opts);
      if (!ev.value) {
        code += ": return true;\n";  // "NONE" enum value.
      } else {
        code += ": return verifier.VerifyTable(reinterpret_cast<const ";
        code += WrapInNameSpace(*ev.struct_def);
        code += " *>(union_obj));\n";
      }
    }
    code += "    default: return false;\n  }\n}\n\n";

    if (parser_.opts.generate_object_based_api) {
      // Generate a union pack & unpack function.
      code += "inline " + UnionUnPackSignature(enum_def, false);
      code += " {\n  switch (type) {\n";
      for (auto it = enum_def.vals.vec.begin(); it != enum_def.vals.vec.end();
           ++it) {
        auto &ev = **it;
        code += "    case " + GetEnumValUse(enum_def, ev, parser_.opts);
        if (!ev.value) {
          code += ": return nullptr;\n";  // "NONE" enum value.
        } else {
          code += ": return reinterpret_cast<const ";
          code += WrapInNameSpace(*ev.struct_def);
          code += " *>(union_obj)->UnPack(resolver);\n";
        }
      }
      code += "    default: return nullptr;\n  }\n}\n\n";
      code += "inline " + UnionPackSignature(enum_def, false);
      code += " {\n  switch (type) {\n";
      for (auto it = enum_def.vals.vec.begin(); it != enum_def.vals.vec.end();
           ++it) {
        auto &ev = **it;
        code += "    case " + GetEnumValUse(enum_def, ev, parser_.opts);
        if (!ev.value) {
          code += ": return 0;\n";  // "NONE" enum value.
        } else {
          code += ": return Create" + ev.struct_def->name;
          code += "(_fbb, reinterpret_cast<const ";
          code += NativeName(WrapInNameSpace(*ev.struct_def));
          code += " *>(table), rehasher).Union();\n";
        }
      }
      code += "    default: return 0;\n  }\n}\n\n";

      // Generate a union destructor.
      code += "inline void " + enum_def.name + "Union::Reset() {\n";
      code += "  switch (type) {\n";
      for (auto it = enum_def.vals.vec.begin(); it != enum_def.vals.vec.end();
           ++it) {
        auto &ev = **it;
        if (ev.value) {
          code += "    case " + GetEnumValUse(enum_def, ev, parser_.opts);
          code += ": delete reinterpret_cast<";
          code += NativeName(WrapInNameSpace(*ev.struct_def));
          code += " *>(table); break;\n";
        }
      }
      code += "    default: break;\n";
      code += "  }\n";
      code += "  table = nullptr;\n";
      code += "  type = ";
      code += GetEnumValUse(enum_def, *enum_def.vals.Lookup("NONE"), parser_.opts);
      code += ";\n";
      code += "}\n\n";
    }
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
      return "static_cast<" + GenTypeBasic(field.value.type, from) + ">(" +
             val + ")";
    } else {
      return val;
    }
  }

  std::string GenFieldOffsetName(const FieldDef &field) {
    std::string uname = field.name;
    std::transform(uname.begin(), uname.end(), uname.begin(), ::toupper);
    return "VT_" + uname;
  }

  void GenFullyQualifiedNameGetter(const std::string &name, std::string &code) {
    if (parser_.opts.generate_name_strings) {
      code +=
          "  static FLATBUFFERS_CONSTEXPR const char *GetFullyQualifiedName() "
          "{\n";
      code += "    return \"" +
              parser_.namespaces_.back()->GetFullyQualifiedName(name) + "\";\n";
      code += "  }\n";
    }
  }

  std::string GenDefaultConstant(const FieldDef &field) {
    return field.value.type.base_type == BASE_TYPE_FLOAT
               ? field.value.constant + "f"
               : field.value.constant;
  }

  std::string GetDefaultScalarValue(const FieldDef &field) {
    if (field.value.type.enum_def && IsScalar(field.value.type.base_type)) {
      auto ev = field.value.type.enum_def->ReverseLookup(
          static_cast<int>(StringToInt(field.value.constant.c_str())), false);
      if (ev) {
        return WrapInNameSpace(
            field.value.type.enum_def->defined_namespace,
            GetEnumValUse(*field.value.type.enum_def, *ev, parser_.opts));
      } else {
        return GenUnderlyingCast(field, true, field.value.constant);
      }
    } else if (field.value.type.base_type == BASE_TYPE_BOOL) {
      return field.value.constant == "0" ? "false" : "true";
    } else {
      return GenDefaultConstant(field);
    }
  }

  void GenSimpleParam(std::string &code, FieldDef &field) {
    code += ",\n    " + GenTypeWire(field.value.type, " ", true);
    code += field.name + " = " + GetDefaultScalarValue(field);
  }

  // Generate a member, including a default value for scalars and raw pointers.
  void GenMember(std::string& code, const FieldDef &field) {
    if (!field.deprecated &&  // Deprecated fields won't be accessible.
        field.value.type.base_type != BASE_TYPE_UTYPE) {
      auto type = GenTypeNative(field.value.type, false, field);
      auto cpp_type = field.attributes.Lookup("cpp_type");
      code += "  " + (cpp_type ? cpp_type->constant + " *" : type+ " ") +
              field.name + ";\n";
    }
  }

  // Generate the default constructor for this struct. Properly initialize all
  // scalar members with default values.
  void GenDefaultConstructor(std::string& code, const StructDef& struct_def) {
    code += "  " + NativeName(struct_def.name) + "()";
    std::string initializer_list;
    for (auto it = struct_def.fields.vec.begin();
         it != struct_def.fields.vec.end(); ++it) {
      auto &field = **it;
      if (!field.deprecated &&  // Deprecated fields won't be accessible.
          field.value.type.base_type != BASE_TYPE_UTYPE) {
        auto cpp_type = field.attributes.Lookup("cpp_type");
        // Scalar types get parsed defaults, raw pointers get nullptrs.
        if (IsScalar(field.value.type.base_type)) {
          if (!initializer_list.empty()) {
            initializer_list += ",\n      ";
          }
          initializer_list += field.name + "(" +GetDefaultScalarValue(field) +
              ")";
        } else if (cpp_type) {
          if (!initializer_list.empty()) {
            code += ",\n      ";
          }
          initializer_list += field.name + "(0)";
        }
      }
    }
    if (!initializer_list.empty()) {
      code += "\n    : " + initializer_list;
    }
    code += " {}\n";
  }

  // Generate an accessor struct, builder structs & function for a table.
  void GenTable(StructDef &struct_def, std::string *code_ptr) {
    std::string &code = *code_ptr;

    if (parser_.opts.generate_object_based_api) {
      // Generate a C++ object that can hold an unpacked version of this
      // table.
      code += "struct " + NativeName(struct_def.name);
      code += " : public flatbuffers::NativeTable {\n";
      code += "  typedef " + struct_def.name + " TableType;\n";
      // Generate GetFullyQualifiedName
      GenFullyQualifiedNameGetter(NativeName(struct_def.name), code);
      for (auto it = struct_def.fields.vec.begin();
           it != struct_def.fields.vec.end(); ++it) {
        auto &field = **it;
        GenMember(code, field);
      }
      GenDefaultConstructor(code, struct_def);
      code += "};\n\n";
    }

    // Generate an accessor struct, with methods of the form:
    // type name() const { return GetField<type>(offset, defaultval); }
    GenComment(struct_def.doc_comment, code_ptr, nullptr);
    code += "struct " + struct_def.name;
    code += " FLATBUFFERS_FINAL_CLASS : private flatbuffers::Table";
    code += " {\n";
    if (parser_.opts.generate_object_based_api) {
      code += "  typedef " + NativeName(struct_def.name) +
              " NativeTableType;\n";
    }
    // Generate GetFullyQualifiedName
    GenFullyQualifiedNameGetter(struct_def.name, code);
    // Generate field id constants.
    if (struct_def.fields.vec.size() > 0) {
      code += "  enum {\n";
      bool is_first_field =
          true;  // track the first field that's not deprecated
      for (auto it = struct_def.fields.vec.begin();
           it != struct_def.fields.vec.end(); ++it) {
        auto &field = **it;
        if (!field.deprecated) {  // Deprecated fields won't be accessible.
          if (!is_first_field) {
            // Add trailing comma and newline to previous element. Don't add
            // trailing comma to
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
         it != struct_def.fields.vec.end(); ++it) {
      auto &field = **it;
      if (!field.deprecated) {  // Deprecated fields won't be accessible.
        auto is_scalar = IsScalar(field.value.type.base_type);
        GenComment(field.doc_comment, code_ptr, nullptr, "  ");
        code += "  " + GenTypeGet(field.value.type, " ", "const ", " *", true);
        code += field.name + "() const { return ";
        // Call a different accessor for pointers, that indirects.
        auto accessor =
            is_scalar
                ? "GetField<"
                : (IsStruct(field.value.type) ? "GetStruct<" : "GetPointer<");
        auto offsetstr = GenFieldOffsetName(field);
        auto call = accessor +
                    GenTypeGet(field.value.type, "", "const ", " *", false) +
                    ">(" + offsetstr;
        // Default value as second arg for non-pointer types.
        if (IsScalar(field.value.type.base_type))
          call += ", " + GenDefaultConstant(field);
        call += ")";
        code += GenUnderlyingCast(field, true, call);
        code += "; }\n";
        if (parser_.opts.mutable_buffer) {
          if (is_scalar) {
            code += "  bool mutate_" + field.name + "(";
            code += GenTypeBasic(field.value.type, true);
            code +=
                " _" + field.name + ") { return SetField(" + offsetstr + ", ";
            code += GenUnderlyingCast(field, false, "_" + field.name);
            code += "); }\n";
          } else {
            auto type = GenTypeGet(field.value.type, " ", "", " *", true);
            code += "  " + type + "mutable_" + field.name + "() { return ";
            code += GenUnderlyingCast(field, true,
                                      accessor + type + ">(" + offsetstr + ")");
            code += "; }\n";
          }
        }
        auto nested = field.attributes.Lookup("nested_flatbuffer");
        if (nested) {
          std::string qualified_name =
              parser_.namespaces_.back()->GetFullyQualifiedName(
                  nested->constant);
          auto nested_root = parser_.structs_.Lookup(qualified_name);
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
            if (parser_.opts.scoped_enums && field.value.type.enum_def &&
                IsScalar(field.value.type.base_type)) {
              code += GenTypeGet(field.value.type, " ", "const ", " *", true);
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
         it != struct_def.fields.vec.end(); ++it) {
      auto &field = **it;
      if (!field.deprecated) {
        code += prefix + "VerifyField";
        if (field.required) code += "Required";
        code += "<" + GenTypeSize(field.value.type);
        code += ">(verifier, " + GenFieldOffsetName(field) + ")";
        switch (field.value.type.base_type) {
          case BASE_TYPE_UNION:
            code += prefix + "Verify" + field.value.type.enum_def->name;
            code += "(verifier, " + field.name + "(), " + field.name +
                    UnionTypeFieldSuffix() + "())";
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
                  code +=
                      prefix + "verifier.VerifyVectorOfTables(" + field.name;
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

    if (parser_.opts.generate_object_based_api) {
      // Generate the UnPack() pre declaration.
      code += "  " + TableUnPackSignature(struct_def, true) + ";\n";
      code += "  " + TablePackSignature(struct_def, true) + ";\n";
    }

    code += "};\n\n";  // End of table.

    // Generate a builder struct, with methods of the form:
    // void add_name(type name) { fbb_.AddElement<type>(offset, name, default);
    // }
    code += "struct " + struct_def.name;
    code += "Builder {\n  flatbuffers::FlatBufferBuilder &fbb_;\n";
    code += "  flatbuffers::uoffset_t start_;\n";
    for (auto it = struct_def.fields.vec.begin();
         it != struct_def.fields.vec.end(); ++it) {
      auto &field = **it;
      if (!field.deprecated) {
        code += "  void add_" + field.name + "(";
        code += GenTypeWire(field.value.type, " ", true) + field.name;
        code += ") { fbb_.Add";
        if (IsScalar(field.value.type.base_type)) {
          code += "Element<" + GenTypeWire(field.value.type, "", false);
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
         it != struct_def.fields.vec.end(); ++it) {
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
    bool gen_vector_pars = false;
    code += "inline flatbuffers::Offset<" + struct_def.name + "> Create";
    code += struct_def.name;
    code += "(flatbuffers::FlatBufferBuilder &_fbb";
    for (auto it = struct_def.fields.vec.begin();
         it != struct_def.fields.vec.end(); ++it) {
      auto &field = **it;
      if (!field.deprecated) {
        if (field.value.type.base_type == BASE_TYPE_STRING ||
            field.value.type.base_type == BASE_TYPE_VECTOR) {
          gen_vector_pars = true;
        }
        GenSimpleParam(code, field);
      }
    }
    code += ") {\n  " + struct_def.name + "Builder builder_(_fbb);\n";
    for (size_t size = struct_def.sortbysize ? sizeof(largest_scalar_t) : 1;
         size; size /= 2) {
      for (auto it = struct_def.fields.vec.rbegin();
           it != struct_def.fields.vec.rend(); ++it) {
        auto &field = **it;
        if (!field.deprecated && (!struct_def.sortbysize ||
                                  size == SizeOf(field.value.type.base_type))) {
          code += "  builder_.add_" + field.name + "(" + field.name + ");\n";
        }
      }
    }
    code += "  return builder_.Finish();\n}\n\n";

    // Generate a CreateXDirect function with vector types as parameters
    if (gen_vector_pars) {
      code += "inline flatbuffers::Offset<" + struct_def.name + "> Create";
      code += struct_def.name;
      code += "Direct(flatbuffers::FlatBufferBuilder &_fbb";
      for (auto it = struct_def.fields.vec.begin();
           it != struct_def.fields.vec.end(); ++it) {
        auto &field = **it;
        if (!field.deprecated) {
          if (field.value.type.base_type == BASE_TYPE_STRING) {
            code += ",\n    const char *";
            code += field.name + " = nullptr";
          } else if (field.value.type.base_type == BASE_TYPE_VECTOR) {
            code += ",\n    const std::vector<";
            code += GenTypeWire(field.value.type.VectorType(), "", false);
            code += "> *" + field.name + " = nullptr";
          } else {
            GenSimpleParam(code, field);
          }
        }
      }
      code += ") {\n  ";
      code += "return Create";
      code += struct_def.name;
      code += "(_fbb";
      for (auto it = struct_def.fields.vec.begin();
           it != struct_def.fields.vec.end(); ++it) {
        auto &field = **it;
        if (!field.deprecated) {
          if (field.value.type.base_type == BASE_TYPE_STRING) {
            code += ", " + field.name + " ? ";
            code += "_fbb.CreateString(" + field.name + ") : 0";
          } else if (field.value.type.base_type == BASE_TYPE_VECTOR) {
            code += ", " + field.name + " ? ";
            code += "_fbb.CreateVector<";
            code += GenTypeWire(field.value.type.VectorType(), "", false);
            code += ">(*" + field.name + ") : 0";
          } else {
            code += ", " + field.name;
          }
        }
      }
      code += ");\n}\n\n";
    }

    if (parser_.opts.generate_object_based_api) {
      // Generate a pre-declaration for a CreateX method that works with an
      // unpacked C++ object.
      code += TableCreateSignature(struct_def, true) + ";\n\n";
    }
  }

  std::string GenUnpackVal(const Type &type, const std::string &val,
                           bool invector, const FieldDef &afield) {
    switch (type.base_type) {
      case BASE_TYPE_STRING:
        return val + "->str()";
      case BASE_TYPE_STRUCT:
        if (IsStruct(type)) {
          if (invector || afield.native_inline) {
            return "*" + val;
          } else {
            return GenTypeNativePtr(WrapInNameSpace(*type.struct_def),
                                    &afield, true) +
                   "(new " +
                   WrapInNameSpace(*type.struct_def) + "(*" + val + "))";
          }
        } else {
          return GenTypeNativePtr(NativeName(WrapInNameSpace(
                     *type.struct_def)), &afield, true) +
                 "(" + val + "->UnPack(resolver))";
        }
      default:
        return val;
        break;
    }
  };

  // Generate code for tables that needs to come after the regular definition.
  void GenTablePost(StructDef &struct_def, std::string *code_ptr) {
    std::string &code = *code_ptr;

    if (parser_.opts.generate_object_based_api) {
      // Generate the UnPack() method.
      code += "inline " + TableUnPackSignature(struct_def, false) + " {\n";
      code += "  (void)resolver;\n";
      code += "  auto _o = new " + NativeName(struct_def.name) + "();\n";
      for (auto it = struct_def.fields.vec.begin();
           it != struct_def.fields.vec.end(); ++it) {
        auto &field = **it;
        if (!field.deprecated) {
          auto prefix = "  { auto _e = " + field.name + "(); ";
          if (!IsScalar(field.value.type.base_type)) prefix += "if (_e) ";
          auto deref = "_o->";
          auto dest = deref + field.name;
          auto assign = prefix + dest + " = ";
          switch (field.value.type.base_type) {
            case BASE_TYPE_VECTOR: {
              code += prefix;
              code += "{ for (flatbuffers::uoffset_t _i = 0;";
              code += " _i < _e->size(); _i++) { ";
              code += dest + ".push_back(";
              std::string indexing;
              if (field.value.type.enum_def) {
                indexing += "(" + field.value.type.enum_def->name + ")";
              }
              indexing += "_e->Get(_i)";
              if (field.value.type.element == BASE_TYPE_BOOL)
                indexing += "!=0";
              code += GenUnpackVal(field.value.type.VectorType(),
                                   indexing, true, field);
              code += "); } }";
              break;
            }
            case BASE_TYPE_UTYPE: {
              auto &union_field = **(it + 1);
              assert(union_field.value.type.base_type == BASE_TYPE_UNION);
              code += prefix + deref + union_field.name + ".type = _e;";
              break;
            }
            case BASE_TYPE_UNION: {
              code += prefix + dest + ".table = ";
              code += field.value.type.enum_def->name;
              code += "Union::UnPack(_e, ";
              code += field.name + UnionTypeFieldSuffix() + "(), resolver);";
              break;
            }
            default: {
              auto cpp_type = field.attributes.Lookup("cpp_type");
              if (cpp_type) {
                code += prefix;
                code += "if (resolver) (*resolver)(reinterpret_cast<void **>(&";
                code += dest;
                code += "), static_cast<flatbuffers::hash_value_t>(_e)); else ";
                code += dest + " = nullptr";
              } else {
                code += assign;
                code += GenUnpackVal(field.value.type, "_e", false, field);
              }
              code += ";";
              break;
            }
          }
          code += " };\n";
        }
      }
      code += "  return _o;\n}\n\n";

      // Generate the X::Pack member function that simply calls the global
      // CreateX function.
      code += "inline " + TablePackSignature(struct_def, false) + " {\n";
      code += "  return Create" + struct_def.name + "(_fbb, _o, _rehasher);\n";
      code += "}\n\n";

      // Generate a CreateX method that works with an unpacked C++ object.
      code += TableCreateSignature(struct_def, false) + " {\n";
      code += "  (void)rehasher;\n";
      auto before_return_statement = code.size();
      code += "  return Create";
      code += struct_def.name + "(_fbb";
      bool any_fields = false;
      for (auto it = struct_def.fields.vec.begin();
           it != struct_def.fields.vec.end(); ++it) {
        auto &field = **it;
        if (!field.deprecated) {
          any_fields = true;
          auto field_name = field.name;
          if (field.value.type.base_type == BASE_TYPE_UTYPE) {
            field_name = field_name.substr(0, field_name.size() -
                                              strlen(UnionTypeFieldSuffix()));
            field_name += ".type";
          }
          auto accessor = "_o->" + field_name;
          if (field.attributes.Lookup("cpp_type"))
            accessor = "rehasher ? static_cast<" +
                       GenTypeBasic(field.value.type, false) +
                       ">((*rehasher)(" + accessor + ")) : 0";
          auto ptrprefix = accessor + " ? ";
          auto stlprefix = accessor + ".size() ? ";
          auto postfix = " : 0";
          if (field.required &&
              (field.value.type.base_type == BASE_TYPE_STRING ||
               field.value.type.base_type == BASE_TYPE_VECTOR)) {
            stlprefix = "";
            postfix = "";
          }
          code += ",\n    ";
          switch (field.value.type.base_type) {
            case BASE_TYPE_STRING:
              code += stlprefix + "_fbb.CreateString(" + accessor + ")";
              code += postfix;
              break;
            case BASE_TYPE_VECTOR: {
              auto vector_type = field.value.type.VectorType();
              code += stlprefix;
              switch (vector_type.base_type) {
                case BASE_TYPE_STRING:
                  code += "_fbb.CreateVectorOfStrings(" + accessor + ")";
                  break;
                case BASE_TYPE_STRUCT:
                  if (IsStruct(vector_type)) {
                    code += "_fbb.CreateVectorOfStructs(" + accessor + ")";
                  } else {
                    code += "_fbb.CreateVector<flatbuffers::Offset<";
                    code += WrapInNameSpace(*vector_type.struct_def) + ">>(" +
                            accessor;
                    code += ".size(), [&](size_t i) { return Create";
                    code += vector_type.struct_def->name + "(_fbb, " + accessor;
                    code += "[i]" + GenPtrGet(field) + ", rehasher); })";
                  }
                  break;
                case BASE_TYPE_BOOL:
                  code += "_fbb.CreateVector(" + accessor + ")";
                  break;
                default: {
                  std::string args = accessor;
                  if (field.value.type.enum_def) {
                    const std::string basetype = GenTypeBasic(
                        field.value.type.enum_def->underlying_type, false);
                    args = "(const " + basetype + "*)" + accessor +
                           ".data(), " + accessor + ".size()";
                  }
                  code += "_fbb.CreateVector(" + args + ")";
                  break;
                }
              }
              code += postfix;
              break;
            }
            case BASE_TYPE_UNION:
              code += accessor + ".Pack(_fbb)";
              break;
            case BASE_TYPE_STRUCT:
              if (IsStruct(field.value.type)) {
                if (field.native_inline) {
                  code += "&" + accessor;
                } else {
                  code += ptrprefix + accessor + GenPtrGet(field) + postfix;
                }
              } else {
                code += ptrprefix + "Create";
                code += field.value.type.struct_def->name;
                code += "(_fbb, " + accessor + GenPtrGet(field) + ", rehasher)";
                code += postfix;
              }
              break;
            default:
              code += accessor;
              break;
          }
        }
      }
      code += ");\n}\n\n";
      if (!any_fields) code.insert(before_return_statement, "  (void)_o;\n");
    }
  }

  static void GenPadding(const FieldDef &field, std::string &code,
                         int &padding_id,
                         const std::function<void(int bits, std::string &code,
                                                  int &padding_id)> &f) {
    if (field.padding) {
      for (int i = 0; i < 4; i++)
        if (static_cast<int>(field.padding) & (1 << i))
          f((1 << i) * 8, code, padding_id);
      assert(!(field.padding & ~0xF));
    }
  }

  static void PaddingDefinition(int bits, std::string &code, int &padding_id) {
    code += "  int" + NumToString(bits) + "_t __padding" +
            NumToString(padding_id++) + ";\n";
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
  void GenStruct(StructDef &struct_def, std::string *code_ptr) {
    if (struct_def.generated) return;
    std::string &code = *code_ptr;

    // Generate an accessor struct, with private variables of the form:
    // type name_;
    // Generates manual padding and alignment.
    // Variables are private because they contain little endian data on all
    // platforms.
    GenComment(struct_def.doc_comment, code_ptr, nullptr);
    code +=
        "MANUALLY_ALIGNED_STRUCT(" + NumToString(struct_def.minalign) + ") ";
    code += struct_def.name + " FLATBUFFERS_FINAL_CLASS {\n private:\n";
    int padding_id = 0;
    for (auto it = struct_def.fields.vec.begin();
         it != struct_def.fields.vec.end(); ++it) {
      auto &field = **it;
      code += "  " + GenTypeGet(field.value.type, " ", "", " ", false);
      code += field.name + "_;\n";
      GenPadding(field, code, padding_id, PaddingDefinition);
    }

    // Generate GetFullyQualifiedName
    code += "\n public:\n";
    GenFullyQualifiedNameGetter(struct_def.name, code);

    // Generate a default constructor.
    code += "  " + struct_def.name + "() { memset(this, 0, sizeof(";
    code += struct_def.name + ")); }\n";

    // Generate a copy constructor.
    code += "  " + struct_def.name + "(const " + struct_def.name;
    code += " &_o) { memcpy(this, &_o, sizeof(";
    code += struct_def.name + ")); }\n";

    // Generate a constructor that takes all fields as arguments.
    if (struct_def.fields.vec.size()) {
      code += "  " + struct_def.name + "(";
      for (auto it = struct_def.fields.vec.begin();
           it != struct_def.fields.vec.end(); ++it) {
        auto &field = **it;
        if (it != struct_def.fields.vec.begin()) code += ", ";
        code += GenTypeGet(field.value.type, " ", "const ", " &", true);
        code += "_" + field.name;
      }
      code += ")\n    : ";
      padding_id = 0;
      for (auto it = struct_def.fields.vec.begin();
           it != struct_def.fields.vec.end(); ++it) {
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
           it != struct_def.fields.vec.end(); ++it) {
        auto &field = **it;
        GenPadding(field, code, padding_id, PaddingDeclaration);
      }
      code += " }\n\n";
    }

    // Generate accessor methods of the form:
    // type name() const { return flatbuffers::EndianScalar(name_); }
    for (auto it = struct_def.fields.vec.begin();
         it != struct_def.fields.vec.end(); ++it) {
      auto &field = **it;
      GenComment(field.doc_comment, code_ptr, nullptr, "  ");
      auto is_scalar = IsScalar(field.value.type.base_type);
      code += "  " + GenTypeGet(field.value.type, " ", "const ", " &", true);
      code += field.name + "() const { return ";
      code += GenUnderlyingCast(
          field, true, is_scalar
                           ? "flatbuffers::EndianScalar(" + field.name + "_)"
                           : field.name + "_");
      code += "; }\n";
      if (parser_.opts.mutable_buffer) {
        if (is_scalar) {
          code += "  void mutate_" + field.name + "(";
          code += GenTypeBasic(field.value.type, true);
          code += " _" + field.name + ") { flatbuffers::WriteScalar(&";
          code += field.name + "_, ";
          code += GenUnderlyingCast(field, false, "_" + field.name);
          code += "); }\n";
        } else {
          code += "  ";
          code += GenTypeGet(field.value.type, "", "", " &", true);
          code += "mutable_" + field.name + "() { return " + field.name;
          code += "_; }\n";
        }
      }
    }
    code += "};\nSTRUCT_END(" + struct_def.name + ", ";
    code += NumToString(struct_def.bytesize) + ");\n\n";
  }

  // Set up the correct namespace. Only open a namespace if
  // the existing one is different (closing/opening only what is necessary) :
  //
  // the file must start and end with an empty (or null) namespace
  // so that namespaces are properly opened and closed
  void SetNameSpace(const Namespace *ns, std::string *code_ptr) {
    if (cur_name_space_ == ns) return;
    // compute the size of the longest common namespace prefix.
    // if cur_name_space is A::B::C::D and ns is A::B::E::F::G,
    // the common prefix is A::B:: and we have old_size = 4, new_size = 5
    // and common_prefix_size = 2
    auto old_size =
        cur_name_space_ == nullptr ? 0 : cur_name_space_->components.size();
    auto new_size = ns == nullptr ? 0 : ns->components.size();
    std::vector<std::string>::size_type common_prefix_size = 0;
    while (common_prefix_size < old_size && common_prefix_size < new_size &&
           ns->components[common_prefix_size] ==
               cur_name_space_->components[common_prefix_size])
      common_prefix_size++;
    // close cur_name_space in reverse order to reach the common prefix
    // in the previous example, D then C are closed
    for (auto j = old_size; j > common_prefix_size; --j)
      *code_ptr +=
          "}  // namespace " + cur_name_space_->components[j - 1] + "\n";
    if (old_size != common_prefix_size) *code_ptr += "\n";
    // open namespace parts to reach the ns namespace
    // in the previous example, E, then F, then G are opened
    for (auto j = common_prefix_size; j != new_size; ++j)
      *code_ptr += "namespace " + ns->components[j] + " {\n";
    if (new_size != common_prefix_size) *code_ptr += "\n";
    cur_name_space_ = ns;
  }
};

}  // namespace cpp

bool GenerateCPP(const Parser &parser, const std::string &path,
                 const std::string &file_name) {
  cpp::CppGenerator generator(parser, path, file_name);
  return generator.generate();
}

std::string CPPMakeRule(const Parser &parser, const std::string &path,
                        const std::string &file_name) {
  std::string filebase =
      flatbuffers::StripPath(flatbuffers::StripExtension(file_name));
  std::string make_rule = GeneratedFileName(path, filebase) + ": ";
  auto included_files = parser.GetIncludedFilesRecursive(file_name);
  for (auto it = included_files.begin(); it != included_files.end(); ++it) {
    make_rule += " " + *it;
  }
  return make_rule;
}

}  // namespace flatbuffers
