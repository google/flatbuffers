/*
 * Copyright 2018 Google Inc. All rights reserved.
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

#include "idl_gen_rust.h"

#include <cmath>

#include "flatbuffers/code_generators.h"
#include "flatbuffers/flatbuffers.h"
#include "flatbuffers/idl.h"
#include "flatbuffers/util.h"
#include "idl_namer.h"

namespace flatbuffers {
namespace {

static Namer::Config RustDefaultConfig() {
  // Historical note: We've been using "keep" casing since the original
  // implementation, presumably because Flatbuffers schema style and Rust style
  // roughly align. We are not going to enforce proper casing since its an
  // unnecessary breaking change.
  return {/*types=*/Case::kKeep,
          /*constants=*/Case::kScreamingSnake,
          /*methods=*/Case::kSnake,
          /*functions=*/Case::kSnake,
          /*fields=*/Case::kKeep,
          /*variables=*/Case::kUnknown,  // Unused.
          /*variants=*/Case::kKeep,
          /*enum_variant_seperator=*/"::",
          /*escape_keywords=*/Namer::Config::Escape::BeforeConvertingCase,
          /*namespaces=*/Case::kSnake,
          /*namespace_seperator=*/"::",
          /*object_prefix=*/"",
          /*object_suffix=*/"T",
          /*keyword_prefix=*/"",
          /*keyword_suffix=*/"_",
          /*filenames=*/Case::kSnake,
          /*directories=*/Case::kSnake,
          /*output_path=*/"",
          /*filename_suffix=*/"_generated",
          /*filename_extension=*/".rs"};
}

static std::set<std::string> RustKeywords() {
  return {
      // https://doc.rust-lang.org/book/second-edition/appendix-01-keywords.html
      "as",
      "break",
      "const",
      "continue",
      "crate",
      "else",
      "enum",
      "extern",
      "false",
      "fn",
      "for",
      "if",
      "impl",
      "in",
      "let",
      "loop",
      "match",
      "mod",
      "move",
      "mut",
      "pub",
      "ref",
      "return",
      "Self",
      "self",
      "static",
      "struct",
      "super",
      "trait",
      "true",
      "type",
      "unsafe",
      "use",
      "where",
      "while",
      // future possible keywords
      "abstract",
      "alignof",
      "become",
      "box",
      "do",
      "final",
      "macro",
      "offsetof",
      "override",
      "priv",
      "proc",
      "pure",
      "sizeof",
      "typeof",
      "unsized",
      "virtual",
      "yield",
      // other rust terms we should not use
      "std",
      "usize",
      "isize",
      "u8",
      "i8",
      "u16",
      "i16",
      "u32",
      "i32",
      "u64",
      "i64",
      "u128",
      "i128",
      "f32",
      "f64",
      // Terms that we use ourselves
      "follow",
      "push",
      "to_little_endian",
      "from_little_endian",
      "ENUM_MAX",
      "ENUM_MIN",
      "ENUM_VALUES",
  };
}

// Encapsulate all logical field types in this enum. This allows us to write
// field logic based on type switches, instead of branches on the properties
// set on the Type.
// TODO(rw): for backwards compatibility, we can't use a strict `enum class`
//           declaration here. could we use the `-Wswitch-enum` warning to
//           achieve the same effect?
enum FullType {
  ftInteger = 0,
  ftFloat = 1,
  ftBool = 2,

  ftStruct = 3,
  ftTable = 4,

  ftEnumKey = 5,
  ftUnionKey = 6,

  ftUnionValue = 7,

  // TODO(rw): bytestring?
  ftString = 8,

  ftVectorOfInteger = 9,
  ftVectorOfFloat = 10,
  ftVectorOfBool = 11,
  ftVectorOfEnumKey = 12,
  ftVectorOfStruct = 13,
  ftVectorOfTable = 14,
  ftVectorOfString = 15,
  ftVectorOfUnionValue = 16,

  ftArrayOfBuiltin = 17,
  ftArrayOfEnum = 18,
  ftArrayOfStruct = 19,
};

// Convert a Type to a FullType (exhaustive).
static FullType GetFullType(const Type& type) {
  // N.B. The order of these conditionals matters for some types.

  if (IsString(type)) {
    return ftString;
  } else if (type.base_type == BASE_TYPE_STRUCT) {
    if (type.struct_def->fixed) {
      return ftStruct;
    } else {
      return ftTable;
    }
  } else if (IsVector(type)) {
    switch (GetFullType(type.VectorType())) {
      case ftInteger: {
        return ftVectorOfInteger;
      }
      case ftFloat: {
        return ftVectorOfFloat;
      }
      case ftBool: {
        return ftVectorOfBool;
      }
      case ftStruct: {
        return ftVectorOfStruct;
      }
      case ftTable: {
        return ftVectorOfTable;
      }
      case ftString: {
        return ftVectorOfString;
      }
      case ftEnumKey: {
        return ftVectorOfEnumKey;
      }
      case ftUnionKey:
      case ftUnionValue: {
        FLATBUFFERS_ASSERT(false && "vectors of unions are unsupported");
        break;
      }
      default: {
        FLATBUFFERS_ASSERT(false && "vector of vectors are unsupported");
      }
    }
  } else if (IsArray(type)) {
    switch (GetFullType(type.VectorType())) {
      case ftInteger:
      case ftFloat:
      case ftBool: {
        return ftArrayOfBuiltin;
      }
      case ftStruct: {
        return ftArrayOfStruct;
      }
      case ftEnumKey: {
        return ftArrayOfEnum;
      }
      default: {
        FLATBUFFERS_ASSERT(false && "Unsupported type for fixed array");
      }
    }
  } else if (type.enum_def != nullptr) {
    if (type.enum_def->is_union) {
      if (type.base_type == BASE_TYPE_UNION) {
        return ftUnionValue;
      } else if (IsInteger(type.base_type)) {
        return ftUnionKey;
      } else {
        FLATBUFFERS_ASSERT(false && "unknown union field type");
      }
    } else {
      return ftEnumKey;
    }
  } else if (IsScalar(type.base_type)) {
    if (IsBool(type.base_type)) {
      return ftBool;
    } else if (IsInteger(type.base_type)) {
      return ftInteger;
    } else if (IsFloat(type.base_type)) {
      return ftFloat;
    } else {
      FLATBUFFERS_ASSERT(false && "unknown number type");
    }
  }

  FLATBUFFERS_ASSERT(false && "completely unknown type");

  // this is only to satisfy the compiler's return analysis.
  return ftBool;
}

static bool IsBitFlagsEnum(const EnumDef& enum_def) {
  return enum_def.attributes.Lookup("bit_flags") != nullptr;
}

// TableArgs make required non-scalars "Option<_>".
// TODO(cneo): Rework how we do defaults and stuff.
static bool IsOptionalToBuilder(const FieldDef& field) {
  return field.IsOptional() || !IsScalar(field.value.type.base_type);
}
}  // namespace

static bool GenerateRustModuleRootFile(const Parser& parser,
                                       const std::string& output_dir) {
  if (!parser.opts.rust_module_root_file) {
    // Don't generate a root file when generating one file. This isn't an error
    // so return true.
    return true;
  }
  Namer namer(WithFlagOptions(RustDefaultConfig(), parser.opts, output_dir),
              RustKeywords());
  // We gather the symbols into a tree of namespaces (which are rust mods) and
  // generate a file that gathers them all.
  struct Module {
    std::map<std::string, Module> sub_modules;
    std::vector<std::string> generated_files;
    // Add a symbol into the tree.
    void Insert(const Namer& namer, const Definition* s) {
      const Definition& symbol = *s;
      Module* current_module = this;
      for (auto it = symbol.defined_namespace->components.begin();
           it != symbol.defined_namespace->components.end(); it++) {
        std::string ns_component = namer.Namespace(*it);
        current_module = &current_module->sub_modules[ns_component];
      }
      current_module->generated_files.push_back(
          namer.File(symbol.name, SkipFile::Extension));
    }
    // Recursively create the importer file.
    void GenerateImports(CodeWriter& code) {
      for (auto it = sub_modules.begin(); it != sub_modules.end(); it++) {
        code += "pub mod " + it->first + " {";
        code.IncrementIdentLevel();
        code += "use super::*;";
        it->second.GenerateImports(code);
        code.DecrementIdentLevel();
        code += "} // " + it->first;
      }
      for (auto it = generated_files.begin(); it != generated_files.end();
           it++) {
        code += "mod " + *it + ";";
        code += "pub use self::" + *it + "::*;";
      }
    }
  };
  Module root_module;
  for (auto it = parser.enums_.vec.begin(); it != parser.enums_.vec.end();
       it++) {
    root_module.Insert(namer, *it);
  }
  for (auto it = parser.structs_.vec.begin(); it != parser.structs_.vec.end();
       it++) {
    root_module.Insert(namer, *it);
  }
  CodeWriter code("  ");
  // TODO(caspern): Move generated warning out of BaseGenerator.
  code +=
      "// Automatically generated by the Flatbuffers compiler. "
      "Do not modify.";
  code += "// @generated";
  root_module.GenerateImports(code);
  const bool success = parser.opts.file_saver->SaveFile(
      (output_dir + "mod.rs").c_str(), code.ToString(), false);
  code.Clear();
  return success;
}

namespace rust {

class RustGenerator : public BaseGenerator {
 public:
  RustGenerator(const Parser& parser, const std::string& path,
                const std::string& file_name)
      : BaseGenerator(parser, path, file_name, "", "::", "rs"),
        cur_name_space_(nullptr),
        namer_(WithFlagOptions(RustDefaultConfig(), parser.opts, path),
               RustKeywords()) {
    // TODO: Namer flag overrides should be in flatc or flatc_main.
    code_.SetPadding("  ");
  }

  bool generate() {
    if (!parser_.opts.rust_module_root_file) {
      return GenerateOneFile();
    } else {
      return GenerateIndividualFiles();
    }
  }

  template <typename T>
  bool GenerateSymbols(const SymbolTable<T>& symbols,
                       std::function<void(const T&)> gen_symbol) {
    for (auto it = symbols.vec.begin(); it != symbols.vec.end(); it++) {
      const T& symbol = **it;
      if (symbol.generated) continue;
      code_.Clear();
      code_ += "// " + std::string(FlatBuffersGeneratedWarning());
      code_ += "// @generated";
      code_ += "extern crate alloc;";
      if (parser_.opts.rust_serialize) {
        code_ += "extern crate serde;";
        code_ +=
            "use self::serde::ser::{Serialize, Serializer, SerializeStruct};";
      }
      code_ += "use super::*;";
      cur_name_space_ = symbol.defined_namespace;
      gen_symbol(symbol);

      const std::string directories =
          namer_.Directories(*symbol.defined_namespace);
      EnsureDirExists(directories);
      const std::string file_path = directories + namer_.File(symbol);
      const bool save_success = parser_.opts.file_saver->SaveFile(
          file_path.c_str(), code_.ToString(), /*binary=*/false);
      if (!save_success) return false;
    }
    return true;
  }

  bool GenerateIndividualFiles() {
    code_.Clear();
    // Don't bother with imports. Use absolute paths everywhere.
    return GenerateSymbols<EnumDef>(
               parser_.enums_, [&](const EnumDef& e) { this->GenEnum(e); }) &&
           GenerateSymbols<StructDef>(
               parser_.structs_, [&](const StructDef& s) {
                 if (s.fixed) {
                   this->GenStruct(s);
                 } else {
                   this->GenTable(s);
                   if (this->parser_.opts.generate_object_based_api) {
                     this->GenTableObject(s);
                   }
                 }
                 if (this->parser_.root_struct_def_ == &s) {
                   this->GenRootTableFuncs(s);
                 }
               });
  }

  // Generates code organized by .fbs files. This is broken legacy behavior
  // that does not work with multiple fbs files with shared namespaces.
  // Iterate through all definitions we haven't generated code for (enums,
  // structs, and tables) and output them to a single file.
  bool GenerateOneFile() {
    code_.Clear();
    code_ += "// " + std::string(FlatBuffersGeneratedWarning());
    code_ += "// @generated";
    code_ += "extern crate alloc;";

    assert(!cur_name_space_);

    // Generate imports for the global scope in case no namespace is used
    // in the schema file.
    GenNamespaceImports(0);
    code_ += "";

    // Generate all code in their namespaces, once, because Rust does not
    // permit re-opening modules.
    //
    // TODO(rw): Use a set data structure to reduce namespace evaluations from
    //           O(n**2) to O(n).
    for (auto ns_it = parser_.namespaces_.begin();
         ns_it != parser_.namespaces_.end(); ++ns_it) {
      const auto& ns = *ns_it;

      // Generate code for all the enum declarations.
      for (auto it = parser_.enums_.vec.begin(); it != parser_.enums_.vec.end();
           ++it) {
        const auto& enum_def = **it;
        if (enum_def.defined_namespace == ns && !enum_def.generated) {
          SetNameSpace(enum_def.defined_namespace);
          GenEnum(enum_def);
        }
      }

      // Generate code for all structs.
      for (auto it = parser_.structs_.vec.begin();
           it != parser_.structs_.vec.end(); ++it) {
        const auto& struct_def = **it;
        if (struct_def.defined_namespace == ns && struct_def.fixed &&
            !struct_def.generated) {
          SetNameSpace(struct_def.defined_namespace);
          GenStruct(struct_def);
        }
      }

      // Generate code for all tables.
      for (auto it = parser_.structs_.vec.begin();
           it != parser_.structs_.vec.end(); ++it) {
        const auto& struct_def = **it;
        if (struct_def.defined_namespace == ns && !struct_def.fixed &&
            !struct_def.generated) {
          SetNameSpace(struct_def.defined_namespace);
          GenTable(struct_def);
          if (parser_.opts.generate_object_based_api) {
            GenTableObject(struct_def);
          }
        }
      }

      // Generate global helper functions.
      if (parser_.root_struct_def_) {
        auto& struct_def = *parser_.root_struct_def_;
        if (struct_def.defined_namespace != ns) {
          continue;
        }
        SetNameSpace(struct_def.defined_namespace);
        GenRootTableFuncs(struct_def);
      }
    }
    if (cur_name_space_) SetNameSpace(nullptr);

    const auto file_path = GeneratedFileName(path_, file_name_, parser_.opts);
    const auto final_code = code_.ToString();
    return parser_.opts.file_saver->SaveFile(file_path.c_str(), final_code,
                                             false);
  }

 private:
  CodeWriter code_;

  // This tracks the current namespace so we can insert namespace declarations.
  const Namespace* cur_name_space_;

  const Namespace* CurrentNameSpace() const { return cur_name_space_; }

  // Determine if a Type needs a lifetime template parameter when used in the
  // Rust builder args.
  bool TableBuilderTypeNeedsLifetime(const Type& type) const {
    switch (GetFullType(type)) {
      case ftInteger:
      case ftFloat:
      case ftBool:
      case ftEnumKey:
      case ftUnionKey:
      case ftUnionValue: {
        return false;
      }
      default: {
        return true;
      }
    }
  }

  // Determine if a table args rust type needs a lifetime template parameter.
  bool TableBuilderArgsNeedsLifetime(const StructDef& struct_def) const {
    FLATBUFFERS_ASSERT(!struct_def.fixed);

    for (auto it = struct_def.fields.vec.begin();
         it != struct_def.fields.vec.end(); ++it) {
      const auto& field = **it;
      if (field.deprecated) {
        continue;
      }

      if (TableBuilderTypeNeedsLifetime(field.value.type)) {
        return true;
      }
    }

    return false;
  }

  std::string NamespacedNativeName(const EnumDef& def) {
    return WrapInNameSpace(def.defined_namespace, namer_.ObjectType(def));
  }
  std::string NamespacedNativeName(const StructDef& def) {
    return WrapInNameSpace(def.defined_namespace, namer_.ObjectType(def));
  }

  std::string WrapInNameSpace(const Definition& def) const {
    return WrapInNameSpace(def.defined_namespace,
                           namer_.EscapeKeyword(def.name));
  }
  std::string WrapInNameSpace(const Namespace* ns,
                              const std::string& name) const {
    if (CurrentNameSpace() == ns) return name;
    std::string prefix = GetRelativeNamespaceTraversal(CurrentNameSpace(), ns);
    return prefix + name;
  }

  // Determine the relative namespace traversal needed to reference one
  // namespace from another namespace. This is useful because it does not force
  // the user to have a particular file layout. (If we output absolute
  // namespace paths, that may require users to organize their Rust crates in a
  // particular way.)
  std::string GetRelativeNamespaceTraversal(const Namespace* src,
                                            const Namespace* dst) const {
    // calculate the path needed to reference dst from src.
    // example: f(A::B::C, A::B::C) -> (none)
    // example: f(A::B::C, A::B)    -> super::
    // example: f(A::B::C, A::B::D) -> super::D
    // example: f(A::B::C, A)       -> super::super::
    // example: f(A::B::C, D)       -> super::super::super::D
    // example: f(A::B::C, D::E)    -> super::super::super::D::E
    // example: f(A, D::E)          -> super::D::E
    // does not include leaf object (typically a struct type).

    std::stringstream stream;
    size_t common = 0;
    std::vector<std::string> s, d;
    if (src) s = src->components;
    if (dst) d = dst->components;
    while (common < s.size() && common < d.size() && s[common] == d[common])
      common++;
    // If src namespace is empty, this must be an absolute path.
    for (size_t i = common; i < s.size(); i++) stream << "super::";
    for (size_t i = common; i < d.size(); i++)
      stream << namer_.Namespace(d[i]) + "::";
    return stream.str();
  }

  // Generate a comment from the schema.
  void GenComment(const std::vector<std::string>& dc, const char* prefix = "") {
    for (auto it = dc.begin(); it != dc.end(); it++) {
      code_ += std::string(prefix) + "///" + *it;
    }
  }

  // Return a Rust type from the table in idl.h.
  std::string GetTypeBasic(const Type& type) const {
    switch (GetFullType(type)) {
      case ftInteger:
      case ftFloat:
      case ftBool:
      case ftEnumKey:
      case ftUnionKey: {
        break;
      }
      default: {
        FLATBUFFERS_ASSERT(false && "incorrect type given");
      }
    }

    // clang-format off
    static const char * const ctypename[] = {
    #define FLATBUFFERS_TD(ENUM, IDLTYPE, CTYPE, JTYPE, GTYPE, NTYPE, PTYPE, \
                           RTYPE, ...) \
      #RTYPE,
      FLATBUFFERS_GEN_TYPES(FLATBUFFERS_TD)
    #undef FLATBUFFERS_TD
    };
    // clang-format on

    if (type.enum_def) {
      return WrapInNameSpace(*type.enum_def);
    }
    return ctypename[type.base_type];
  }

  // Look up the native type for an enum. This will always be an integer like
  // u8, i32, etc.
  std::string GetEnumTypeForDecl(const Type& type) {
    const auto ft = GetFullType(type);
    if (!(ft == ftEnumKey || ft == ftUnionKey)) {
      FLATBUFFERS_ASSERT(false && "precondition failed in GetEnumTypeForDecl");
    }

    // clang-format off
    static const char *ctypename[] = {
    #define FLATBUFFERS_TD(ENUM, IDLTYPE, CTYPE, JTYPE, GTYPE, NTYPE, PTYPE, \
                           RTYPE, ...) \
      #RTYPE,
      FLATBUFFERS_GEN_TYPES(FLATBUFFERS_TD)
    #undef FLATBUFFERS_TD
    };
    // clang-format on

    // Enums can be bools, but their Rust representation must be a u8, as used
    // in the repr attribute (#[repr(bool)] is an invalid attribute).
    if (type.base_type == BASE_TYPE_BOOL) return "u8";
    return ctypename[type.base_type];
  }

  // Return a Rust type for any type (scalar, table, struct) specifically for
  // using a FlatBuffer.
  std::string GetTypeGet(const Type& type) const {
    switch (GetFullType(type)) {
      case ftInteger:
      case ftFloat:
      case ftBool:
      case ftEnumKey:
      case ftUnionKey: {
        return GetTypeBasic(type);
      }
      case ftArrayOfBuiltin:
      case ftArrayOfEnum:
      case ftArrayOfStruct: {
        return "[" + GetTypeGet(type.VectorType()) + "; " +
               NumToString(type.fixed_length) + "]";
      }
      case ftTable: {
        return WrapInNameSpace(type.struct_def->defined_namespace,
                               type.struct_def->name) +
               "<'a>";
      }
      default: {
        return WrapInNameSpace(type.struct_def->defined_namespace,
                               type.struct_def->name);
      }
    }
  }

  std::string GetEnumValue(const EnumDef& enum_def,
                           const EnumVal& enum_val) const {
    return namer_.EnumVariant(enum_def, enum_val);
  }

  // 1 suffix since old C++ can't figure out the overload.
  void ForAllEnumValues1(const EnumDef& enum_def,
                         std::function<void(const EnumVal&)> cb) {
    for (auto it = enum_def.Vals().begin(); it != enum_def.Vals().end(); ++it) {
      const auto& ev = **it;
      code_.SetValue("VARIANT", namer_.Variant(ev));
      code_.SetValue("VALUE", enum_def.ToString(ev));
      code_.IncrementIdentLevel();
      cb(ev);
      code_.DecrementIdentLevel();
    }
  }
  void ForAllEnumValues(const EnumDef& enum_def, std::function<void()> cb) {
    std::function<void(const EnumVal&)> wrapped = [&](const EnumVal& unused) {
      (void)unused;
      cb();
    };
    ForAllEnumValues1(enum_def, wrapped);
  }
  // Generate an enum declaration,
  // an enum string lookup table,
  // an enum match function,
  // and an enum array of values
  void GenEnum(const EnumDef& enum_def) {
    const bool is_private = parser_.opts.no_leak_private_annotations &&
                            (enum_def.attributes.Lookup("private") != nullptr);
    code_.SetValue("ACCESS_TYPE", is_private ? "pub(crate)" : "pub");
    code_.SetValue("ENUM_TY", namer_.Type(enum_def));
    code_.SetValue("BASE_TYPE", GetEnumTypeForDecl(enum_def.underlying_type));
    code_.SetValue("ENUM_NAMESPACE", namer_.Namespace(enum_def.name));
    code_.SetValue("ENUM_CONSTANT", namer_.Constant(enum_def.name));
    const EnumVal* minv = enum_def.MinValue();
    const EnumVal* maxv = enum_def.MaxValue();
    FLATBUFFERS_ASSERT(minv && maxv);
    code_.SetValue("ENUM_MIN_BASE_VALUE", enum_def.ToString(*minv));
    code_.SetValue("ENUM_MAX_BASE_VALUE", enum_def.ToString(*maxv));

    if (IsBitFlagsEnum(enum_def)) {
      // Defer to the convenient and canonical bitflags crate. We declare it in
      // a module to #allow camel case constants in a smaller scope. This
      // matches Flatbuffers c-modeled enums where variants are associated
      // constants but in camel case.
      code_ += "#[allow(non_upper_case_globals)]";
      code_ += "mod bitflags_{{ENUM_NAMESPACE}} {";
      code_ += "  ::flatbuffers::bitflags::bitflags! {";
      GenComment(enum_def.doc_comment, "    ");
      code_ += "    #[derive(Default, Debug, Clone, Copy, PartialEq)]";
      code_ += "    {{ACCESS_TYPE}} struct {{ENUM_TY}}: {{BASE_TYPE}} {";
      ForAllEnumValues1(enum_def, [&](const EnumVal& ev) {
        this->GenComment(ev.doc_comment, "    ");
        code_ += "    const {{VARIANT}} = {{VALUE}};";
      });
      code_ += "    }";
      code_ += "  }";
      code_ += "}";
      code_ += "pub use self::bitflags_{{ENUM_NAMESPACE}}::{{ENUM_TY}};";
      code_ += "";

      code_.SetValue("INTO_BASE", "self.bits()");
    } else {
      // Normal, c-modelled enums.
      // Deprecated associated constants;
      const std::string deprecation_warning =
          "#[deprecated(since = \"2.0.0\", note = \"Use associated constants"
          " instead. This will no longer be generated in 2021.\")]";
      code_ += deprecation_warning;
      code_ +=
          "pub const ENUM_MIN_{{ENUM_CONSTANT}}: {{BASE_TYPE}}"
          " = {{ENUM_MIN_BASE_VALUE}};";
      code_ += deprecation_warning;
      code_ +=
          "pub const ENUM_MAX_{{ENUM_CONSTANT}}: {{BASE_TYPE}}"
          " = {{ENUM_MAX_BASE_VALUE}};";
      auto num_fields = NumToString(enum_def.size());
      code_ += deprecation_warning;
      code_ += "#[allow(non_camel_case_types)]";
      code_ += "pub const ENUM_VALUES_{{ENUM_CONSTANT}}: [{{ENUM_TY}}; " +
               num_fields + "] = [";
      ForAllEnumValues1(enum_def, [&](const EnumVal& ev) {
        code_ += namer_.EnumVariant(enum_def, ev) + ",";
      });
      code_ += "];";
      code_ += "";

      GenComment(enum_def.doc_comment);
      // Derive Default to be 0. flatc enforces this when the enum
      // is put into a struct, though this isn't documented behavior, it is
      // needed to derive defaults in struct objects.
      code_ +=
          "#[derive(Clone, Copy, PartialEq, Eq, PartialOrd, Ord, Hash, "
          "Default)]";
      code_ += "#[repr(transparent)]";
      code_ += "{{ACCESS_TYPE}} struct {{ENUM_TY}}(pub {{BASE_TYPE}});";
      code_ += "#[allow(non_upper_case_globals)]";
      code_ += "impl {{ENUM_TY}} {";
      ForAllEnumValues1(enum_def, [&](const EnumVal& ev) {
        this->GenComment(ev.doc_comment);
        code_ += "pub const {{VARIANT}}: Self = Self({{VALUE}});";
      });
      code_ += "";
      // Generate Associated constants
      code_ += "  pub const ENUM_MIN: {{BASE_TYPE}} = {{ENUM_MIN_BASE_VALUE}};";
      code_ += "  pub const ENUM_MAX: {{BASE_TYPE}} = {{ENUM_MAX_BASE_VALUE}};";
      code_ += "  pub const ENUM_VALUES: &'static [Self] = &[";
      ForAllEnumValues(enum_def, [&]() { code_ += "  Self::{{VARIANT}},"; });
      code_ += "  ];";
      code_ += "  /// Returns the variant's name or \"\" if unknown.";
      code_ += "  pub fn variant_name(self) -> Option<&'static str> {";
      code_ += "    match self {";
      ForAllEnumValues(enum_def, [&]() {
        code_ += "    Self::{{VARIANT}} => Some(\"{{VARIANT}}\"),";
      });
      code_ += "      _ => None,";
      code_ += "    }";
      code_ += "  }";
      code_ += "}";

      // Generate Debug. Unknown variants are printed like "<UNKNOWN 42>".
      code_ += "impl ::core::fmt::Debug for {{ENUM_TY}} {";
      code_ +=
          "  fn fmt(&self, f: &mut ::core::fmt::Formatter) ->"
          " ::core::fmt::Result {";
      code_ += "    if let Some(name) = self.variant_name() {";
      code_ += "      f.write_str(name)";
      code_ += "    } else {";
      code_ += "      f.write_fmt(format_args!(\"<UNKNOWN {:?}>\", self.0))";
      code_ += "    }";
      code_ += "  }";
      code_ += "}";

      code_.SetValue("INTO_BASE", "self.0");
    }

    // Implement serde::Serialize
    if (parser_.opts.rust_serialize) {
      code_ += "impl Serialize for {{ENUM_TY}} {";
      code_ +=
          "  fn serialize<S>(&self, serializer: S) -> Result<S::Ok, S::Error>";
      code_ += "  where";
      code_ += "    S: Serializer,";
      code_ += "  {";
      if (IsBitFlagsEnum(enum_def)) {
        code_ += "    serializer.serialize_u32(self.bits() as u32)";
      } else {
        code_ +=
            "    serializer.serialize_unit_variant(\"{{ENUM_TY}}\", self.0 "
            "as "
            "u32, self.variant_name().unwrap())";
      }
      code_ += "  }";
      code_ += "}";
      code_ += "";

      if (!IsBitFlagsEnum(enum_def)) {
        code_ += "impl<'de> serde::Deserialize<'de> for {{ENUM_TY}} {";
        code_ +=
            "    fn deserialize<D>(deserializer: D) -> Result<Self, D::Error>";
        code_ += "    where";
        code_ += "        D: serde::Deserializer<'de>,";
        code_ += "    {";
        code_ += "        let s = String::deserialize(deserializer)?;";
        code_ += "        for item in {{ENUM_TY}}::ENUM_VALUES {";
        code_ += "            if let Some(item_name) = item.variant_name() {";
        code_ += "                if item_name == s {";
        code_ += "                    return Ok(item.clone());";
        code_ += "                }";
        code_ += "            }";
        code_ += "        }";
        code_ += "        Err(serde::de::Error::custom(format!(";
        code_ += "            \"Unknown {{ENUM_TY}} variant: {s}\"";
        code_ += "        )))";
        code_ += "    }";
        code_ += "}";
        code_ += "";
      }
    }

    // Generate Follow and Push so we can serialize and stuff.
    code_ += "impl<'a> ::flatbuffers::Follow<'a> for {{ENUM_TY}} {";
    code_ += "  type Inner = Self;";
    code_ += "  #[inline]";
    code_ += "  unsafe fn follow(buf: &'a [u8], loc: usize) -> Self::Inner {";
    code_ +=
        "    let b = unsafe { "
        "::flatbuffers::read_scalar_at::<{{BASE_TYPE}}>(buf, loc) };";
    if (IsBitFlagsEnum(enum_def)) {
      code_ += "    Self::from_bits_retain(b)";
    } else {
      code_ += "    Self(b)";
    }
    code_ += "  }";
    code_ += "";
    code_ += "impl ::flatbuffers::Push for {{ENUM_TY}} {";
    code_ += "    type Output = {{ENUM_TY}};";
    code_ += "    #[inline]";
    code_ += "    unsafe fn push(&self, dst: &mut [u8], _written_len: usize) {";
    code_ +=
        "        unsafe { ::flatbuffers::emplace_scalar::<{{BASE_TYPE}}>(dst, "
        "{{INTO_BASE}}) };";
    code_ += "    }";
    code_ += "";
    code_ += "impl ::flatbuffers::EndianScalar for {{ENUM_TY}} {";
    code_ += "  type Scalar = {{BASE_TYPE}};";
    code_ += "  #[inline]";
    code_ += "  fn to_little_endian(self) -> {{BASE_TYPE}} {";
    code_ += "    {{INTO_BASE}}.to_le()";
    code_ += "  }";
    code_ += "  #[inline]";
    code_ += "  #[allow(clippy::wrong_self_convention)]";
    code_ += "  fn from_little_endian(v: {{BASE_TYPE}}) -> Self {";
    code_ += "    let b = {{BASE_TYPE}}::from_le(v);";
    if (IsBitFlagsEnum(enum_def)) {
      code_ += "    Self::from_bits_retain(b)";
    } else {
      code_ += "    Self(b)";
    }
    code_ += "  }";
    code_ += "";

    // Generate verifier - deferring to the base type.
    code_ += "impl<'a> ::flatbuffers::Verifiable for {{ENUM_TY}} {";
    code_ += "  #[inline]";
    code_ += "  fn run_verifier(";
    code_ += "    v: &mut ::flatbuffers::Verifier, pos: usize";
    code_ += "  ) -> Result<(), ::flatbuffers::InvalidFlatbuffer> {";
    code_ += "    {{BASE_TYPE}}::run_verifier(v, pos)";
    code_ += "  }";
    code_ += "";
    // Enums are basically integers.
    code_ += "impl ::flatbuffers::SimpleToVerifyInSlice for {{ENUM_TY}} {}";

    if (enum_def.is_union) {
      // Generate typesafe offset(s) for unions
      code_.SetValue("UNION_TYPE", namer_.Type(enum_def));
      code_ += "{{ACCESS_TYPE}} struct {{UNION_TYPE}}UnionTableOffset {}";
      code_ += "";
      if (parser_.opts.generate_object_based_api) {
        GenUnionObject(enum_def);
      }
    }
  }

  // TODO(cneo): dedup Object versions from non object versions.
  void ForAllUnionObjectVariantsBesidesNone(const EnumDef& enum_def,
                                            std::function<void()> cb) {
    for (auto it = enum_def.Vals().begin(); it != enum_def.Vals().end(); ++it) {
      auto& enum_val = **it;
      if (enum_val.union_type.base_type == BASE_TYPE_NONE) continue;
      code_.SetValue("VARIANT_NAME", namer_.Variant(enum_val));
      // For legacy reasons, enum variants are Keep case while enum native
      // variants are UpperCamel case.
      code_.SetValue("NATIVE_VARIANT",
                     namer_.LegacyRustNativeVariant(enum_val));
      code_.SetValue("U_ELEMENT_NAME", namer_.Method(enum_val));
      code_.SetValue("U_ELEMENT_TABLE_TYPE",
                     NamespacedNativeName(*enum_val.union_type.struct_def));
      code_.IncrementIdentLevel();
      cb();
      code_.DecrementIdentLevel();
    }
  }
  void GenUnionObject(const EnumDef& enum_def) {
    code_.SetValue("ENUM_TY", namer_.Type(enum_def));
    code_.SetValue("ENUM_FN", namer_.Function(enum_def));
    code_.SetValue("ENUM_OTY", namer_.ObjectType(enum_def));

    // Generate native union.
    code_ += "#[allow(clippy::upper_case_acronyms)]";  // NONE's spelling is
                                                       // intended.
    code_ += "#[non_exhaustive]";
    code_ += "#[derive(Debug, Clone, PartialEq)]";
    code_ += "{{ACCESS_TYPE}} enum {{ENUM_OTY}} {";
    code_ += "  NONE,";
    ForAllUnionObjectVariantsBesidesNone(enum_def, [&] {
      code_ +=
          "{{NATIVE_VARIANT}}(alloc::boxed::Box<{{U_ELEMENT_TABLE_TYPE}}>),";
    });
    code_ += "}";
    // Generate Default (NONE).
    code_ += "impl Default for {{ENUM_OTY}} {";
    code_ += "  fn default() -> Self {";
    code_ += "    Self::NONE";
    code_ += "  }";
    code_ += "}";

    // Generate native union methods.
    code_ += "impl {{ENUM_OTY}} {";

    // Get flatbuffers union key.
    // TODO(cneo): add docstrings?
    code_ += "  pub fn {{ENUM_FN}}_type(&self) -> {{ENUM_TY}} {";
    code_ += "    match self {";
    code_ += "      Self::NONE => {{ENUM_TY}}::NONE,";
    ForAllUnionObjectVariantsBesidesNone(enum_def, [&] {
      code_ +=
          "    Self::{{NATIVE_VARIANT}}(_) => {{ENUM_TY}}::"
          "{{VARIANT_NAME}},";
    });
    code_ += "    }";
    code_ += "  }";
    // Pack flatbuffers union value
    code_ +=
        "  pub fn pack<'b, A: ::flatbuffers::Allocator + 'b>(&self, fbb: &mut "
        "::flatbuffers::FlatBufferBuilder<'b, A>)"
        " -> Option<::flatbuffers::WIPOffset<::flatbuffers::UnionWIPOffset>>"
        " {";
    code_ += "    match self {";
    code_ += "      Self::NONE => None,";
    ForAllUnionObjectVariantsBesidesNone(enum_def, [&] {
      code_ += "    Self::{{NATIVE_VARIANT}}(v) => \\";
      code_ += "Some(v.pack(fbb).as_union_value()),";
    });
    code_ += "    }";
    code_ += "  }";

    // Generate some accessors;
    ForAllUnionObjectVariantsBesidesNone(enum_def, [&] {
      // Move accessor.
      code_ +=
          "/// If the union variant matches, return the owned "
          "{{U_ELEMENT_TABLE_TYPE}}, setting the union to NONE.";
      code_ +=
          "pub fn take_{{U_ELEMENT_NAME}}(&mut self) -> "
          "Option<alloc::boxed::Box<{{U_ELEMENT_TABLE_TYPE}}>> {";
      code_ += "  if let Self::{{NATIVE_VARIANT}}(_) = self {";
      code_ += "    let v = ::core::mem::replace(self, Self::NONE);";
      code_ += "    if let Self::{{NATIVE_VARIANT}}(w) = v {";
      code_ += "      Some(w)";
      code_ += "    } else {";
      code_ += "      unreachable!()";
      code_ += "    }";
      code_ += "  } else {";
      code_ += "    None";
      code_ += "  }";
      code_ += "}";
      // Immutable reference accessor.
      code_ +=
          "/// If the union variant matches, return a reference to the "
          "{{U_ELEMENT_TABLE_TYPE}}.";
      code_ +=
          "pub fn as_{{U_ELEMENT_NAME}}(&self) -> "
          "Option<&{{U_ELEMENT_TABLE_TYPE}}> {";
      code_ +=
          "  if let Self::{{NATIVE_VARIANT}}(v) = self "
          "{ Some(v.as_ref()) } else { None }";
      code_ += "}";
      // Mutable reference accessor.
      code_ +=
          "/// If the union variant matches, return a mutable reference"
          " to the {{U_ELEMENT_TABLE_TYPE}}.";
      code_ +=
          "pub fn as_{{U_ELEMENT_NAME}}_mut(&mut self) -> "
          "Option<&mut {{U_ELEMENT_TABLE_TYPE}}> {";
      code_ +=
          "  if let Self::{{NATIVE_VARIANT}}(v) = self "
          "{ Some(v.as_mut()) } else { None }";
      code_ += "}";
    });
    code_ += "}";  // End union methods impl.
  }

  enum DefaultContext { kBuilder, kAccessor, kObject };
  std::string GetDefaultValue(const FieldDef& field,
                              const DefaultContext context) {
    if (context == kBuilder) {
      // Builders and Args structs model nonscalars "optional" even if they're
      // required or have defaults according to the schema. I guess its because
      // WIPOffset is not nullable.
      if (!IsScalar(field.value.type.base_type) || field.IsOptional()) {
        return "None";
      }
    } else {
      // This for defaults in objects.
      // Unions have a NONE variant instead of using Rust's None.
      if (field.IsOptional() && !IsUnion(field.value.type)) {
        return "None";
      }
    }
    switch (GetFullType(field.value.type)) {
      case ftInteger: {
        return field.value.constant;
      }
      case ftFloat: {
        const std::string float_prefix =
            (field.value.type.base_type == BASE_TYPE_FLOAT) ? "f32::" : "f64::";
        if (StringIsFlatbufferNan(field.value.constant)) {
          return float_prefix + "NAN";
        } else if (StringIsFlatbufferPositiveInfinity(field.value.constant)) {
          return float_prefix + "INFINITY";
        } else if (StringIsFlatbufferNegativeInfinity(field.value.constant)) {
          return float_prefix + "NEG_INFINITY";
        }
        return field.value.constant;
      }
      case ftBool: {
        return field.value.constant == "0" ? "false" : "true";
      }
      case ftUnionKey:
      case ftEnumKey: {
        auto ev = field.value.type.enum_def->FindByValue(field.value.constant);
        if (!ev) return "Default::default()";  // Bitflags enum.
        return WrapInNameSpace(
            field.value.type.enum_def->defined_namespace,
            namer_.EnumVariant(*field.value.type.enum_def, *ev));
      }
      case ftUnionValue: {
        return ObjectFieldType(field, true) + "::NONE";
      }
      case ftString: {
        // Required fields do not have defaults defined by the schema, but we
        // need one for Rust's Default trait so we use empty string. The usual
        // value of field.value.constant is `0`, which is non-sensical except
        // maybe to c++ (nullptr == 0).
        // TODO: Escape strings?
        const std::string defval =
            field.IsRequired() ? "\"\"" : "\"" + field.value.constant + "\"";
        if (context == kObject) {
          return "alloc::string::ToString::to_string(" + defval + ")";
        }
        if (context == kAccessor) return "&" + defval;
        FLATBUFFERS_ASSERT(false);
        return "INVALID_CODE_GENERATION";
      }

      case ftArrayOfStruct:
      case ftArrayOfEnum:
      case ftArrayOfBuiltin:
      case ftVectorOfBool:
      case ftVectorOfFloat:
      case ftVectorOfInteger:
      case ftVectorOfString:
      case ftVectorOfStruct:
      case ftVectorOfTable:
      case ftVectorOfEnumKey:
      case ftVectorOfUnionValue:
      case ftStruct:
      case ftTable: {
        // We only support empty vectors which matches the defaults for
        // &[T] and Vec<T> anyway.
        //
        // For required structs and tables fields, we defer to their object API
        // defaults. This works so long as there's nothing recursive happening,
        // but `table Infinity { i: Infinity (required); }` does compile.
        return "Default::default()";
      }
    }
    FLATBUFFERS_ASSERT(false);
    return "INVALID_CODE_GENERATION";
  }

  // Create the return type for fields in the *BuilderArgs structs that are
  // used to create Tables.
  //
  // Note: we could make all inputs to the BuilderArgs be an Option, as well
  // as all outputs. But, the UX of Flatbuffers is that the user doesn't get to
  // know if the value is default or not, because there are three ways to
  // return a default value:
  // 1) return a stored value that happens to be the default,
  // 2) return a hardcoded value because the relevant vtable field is not in
  //    the vtable, or
  // 3) return a hardcoded value because the vtable field value is set to zero.
  std::string TableBuilderArgsDefnType(const FieldDef& field,
                                       const std::string& lifetime) {
    const Type& type = field.value.type;
    auto WrapOption = [&](std::string s) {
      return IsOptionalToBuilder(field) ? "Option<" + s + ">" : s;
    };
    auto WrapVector = [&](std::string ty) {
      return WrapOption("::flatbuffers::WIPOffset<::flatbuffers::Vector<" +
                        lifetime + ", " + ty + ">>");
    };
    auto WrapUOffsetsVector = [&](std::string ty) {
      return WrapVector("::flatbuffers::ForwardsUOffset<" + ty + ">");
    };

    switch (GetFullType(type)) {
      case ftInteger:
      case ftFloat:
      case ftBool: {
        return WrapOption(GetTypeBasic(type));
      }
      case ftStruct: {
        const auto typname = WrapInNameSpace(*type.struct_def);
        return WrapOption("&" + lifetime + " " + typname);
      }
      case ftTable: {
        const auto typname = WrapInNameSpace(*type.struct_def);
        return WrapOption("::flatbuffers::WIPOffset<" + typname + "<" +
                          lifetime + ">>");
      }
      case ftString: {
        return WrapOption("::flatbuffers::WIPOffset<&" + lifetime + " str>");
      }
      case ftEnumKey:
      case ftUnionKey: {
        return WrapOption(WrapInNameSpace(*type.enum_def));
      }
      case ftUnionValue: {
        return "Option<::flatbuffers::WIPOffset<::flatbuffers::UnionWIPOffset>"
               ">";
      }

      case ftVectorOfInteger:
      case ftVectorOfBool:
      case ftVectorOfFloat: {
        const auto typname = GetTypeBasic(type.VectorType());
        return WrapVector(typname);
      }
      case ftVectorOfEnumKey: {
        const auto typname = WrapInNameSpace(*type.enum_def);
        return WrapVector(typname);
      }
      case ftVectorOfStruct: {
        const auto typname = WrapInNameSpace(*type.struct_def);
        return WrapVector(typname);
      }
      case ftVectorOfTable: {
        const auto typname = WrapInNameSpace(*type.struct_def);
        return WrapUOffsetsVector(typname + "<" + lifetime + ">");
      }
      case ftVectorOfString: {
        return WrapUOffsetsVector("&" + lifetime + " str");
      }
      case ftVectorOfUnionValue: {
        return WrapUOffsetsVector("::flatbuffers::Table<" + lifetime + ">");
      }
      case ftArrayOfEnum:
      case ftArrayOfStruct:
      case ftArrayOfBuiltin: {
        FLATBUFFERS_ASSERT(false && "arrays are not supported within tables");
        return "ARRAYS_NOT_SUPPORTED_IN_TABLES";
      }
    }
    return "INVALID_CODE_GENERATION";  // for return analysis
  }

  std::string ObjectFieldType(const FieldDef& field, bool in_a_table) {
    const Type& type = field.value.type;
    std::string ty;
    switch (GetFullType(type)) {
      case ftInteger:
      case ftBool:
      case ftFloat: {
        ty = GetTypeBasic(type);
        break;
      }
      case ftString: {
        ty = "alloc::string::String";
        break;
      }
      case ftStruct: {
        ty = NamespacedNativeName(*type.struct_def);
        break;
      }
      case ftTable: {
        // Since Tables can contain themselves, Box is required to avoid
        // infinite types.
        ty =
            "alloc::boxed::Box<" + NamespacedNativeName(*type.struct_def) + ">";
        break;
      }
      case ftUnionKey: {
        // There is no native "UnionKey", natively, unions are rust enums with
        // newtype-struct-variants.
        return "INVALID_CODE_GENERATION";
      }
      case ftUnionValue: {
        ty = NamespacedNativeName(*type.enum_def);
        break;
      }
      case ftEnumKey: {
        ty = WrapInNameSpace(*type.enum_def);
        break;
      }
      // Vectors are in tables and are optional
      case ftVectorOfEnumKey: {
        ty = "alloc::vec::Vec<" + WrapInNameSpace(*type.VectorType().enum_def) +
             ">";
        break;
      }
      case ftVectorOfInteger:
      case ftVectorOfBool:
      case ftVectorOfFloat: {
        ty = "alloc::vec::Vec<" + GetTypeBasic(type.VectorType()) + ">";
        break;
      }
      case ftVectorOfString: {
        ty = "alloc::vec::Vec<alloc::string::String>";
        break;
      }
      case ftVectorOfTable:
      case ftVectorOfStruct: {
        ty = NamespacedNativeName(*type.VectorType().struct_def);
        ty = "alloc::vec::Vec<" + ty + ">";
        break;
      }
      case ftVectorOfUnionValue: {
        FLATBUFFERS_ASSERT(false && "vectors of unions are not yet supported");
        return "INVALID_CODE_GENERATION";  // OH NO!
      }
      case ftArrayOfEnum: {
        ty = "[" + WrapInNameSpace(*type.VectorType().enum_def) + "; " +
             NumToString(type.fixed_length) + "]";
        break;
      }
      case ftArrayOfStruct: {
        ty = "[" + NamespacedNativeName(*type.VectorType().struct_def) + "; " +
             NumToString(type.fixed_length) + "]";
        break;
      }
      case ftArrayOfBuiltin: {
        ty = "[" + GetTypeBasic(type.VectorType()) + "; " +
             NumToString(type.fixed_length) + "]";
        break;
      }
    }
    if (in_a_table && !IsUnion(type) && field.IsOptional()) {
      return "Option<" + ty + ">";
    } else {
      return ty;
    }
  }

  std::string TableBuilderArgsAddFuncType(const FieldDef& field,
                                          const std::string& lifetime) {
    const Type& type = field.value.type;

    switch (GetFullType(field.value.type)) {
      case ftVectorOfStruct: {
        const auto typname = WrapInNameSpace(*type.struct_def);
        return "::flatbuffers::WIPOffset<::flatbuffers::Vector<" + lifetime +
               ", " + typname + ">>";
      }
      case ftVectorOfTable: {
        const auto typname = WrapInNameSpace(*type.struct_def);
        return "::flatbuffers::WIPOffset<::flatbuffers::Vector<" + lifetime +
               ", ::flatbuffers::ForwardsUOffset<" + typname + "<" + lifetime +
               ">>>>";
      }
      case ftVectorOfInteger:
      case ftVectorOfBool:
      case ftVectorOfFloat: {
        const auto typname = GetTypeBasic(type.VectorType());
        return "::flatbuffers::WIPOffset<::flatbuffers::Vector<" + lifetime +
               ", " + typname + ">>";
      }
      case ftVectorOfString: {
        return "::flatbuffers::WIPOffset<::flatbuffers::Vector<" + lifetime +
               ", ::flatbuffers::ForwardsUOffset<&" + lifetime + " str>>>";
      }
      case ftVectorOfEnumKey: {
        const auto typname = WrapInNameSpace(*type.enum_def);
        return "::flatbuffers::WIPOffset<::flatbuffers::Vector<" + lifetime +
               ", " + typname + ">>";
      }
      case ftVectorOfUnionValue: {
        return "::flatbuffers::WIPOffset<::flatbuffers::Vector<" + lifetime +
               ", ::flatbuffers::ForwardsUOffset<::flatbuffers::Table<" +
               lifetime + ">>>";
      }
      case ftEnumKey:
      case ftUnionKey: {
        const auto typname = WrapInNameSpace(*type.enum_def);
        return typname;
      }
      case ftStruct: {
        const auto typname = WrapInNameSpace(*type.struct_def);
        return "&" + typname + "";
      }
      case ftTable: {
        const auto typname = WrapInNameSpace(*type.struct_def);
        return "::flatbuffers::WIPOffset<" + typname + "<" + lifetime + ">>";
      }
      case ftInteger:
      case ftBool:
      case ftFloat: {
        return GetTypeBasic(type);
      }
      case ftString: {
        return "::flatbuffers::WIPOffset<&" + lifetime + " str>";
      }
      case ftUnionValue: {
        return "::flatbuffers::WIPOffset<::flatbuffers::UnionWIPOffset>";
      }
      case ftArrayOfBuiltin: {
        const auto typname = GetTypeBasic(type.VectorType());
        return "::flatbuffers::Array<" + lifetime + ", " + typname + ", " +
               NumToString(type.fixed_length) + ">";
      }
      case ftArrayOfEnum: {
        const auto typname = WrapInNameSpace(*type.enum_def);
        return "::flatbuffers::Array<" + lifetime + ", " + typname + ", " +
               NumToString(type.fixed_length) + ">";
      }
      case ftArrayOfStruct: {
        const auto typname = WrapInNameSpace(*type.struct_def);
        return "::flatbuffers::Array<" + lifetime + ", " + typname + ", " +
               NumToString(type.fixed_length) + ">";
      }
    }

    return "INVALID_CODE_GENERATION";  // for return analysis
  }

  std::string TableBuilderArgsAddFuncBody(const FieldDef& field) {
    const Type& type = field.value.type;

    switch (GetFullType(field.value.type)) {
      case ftInteger:
      case ftBool:
      case ftFloat: {
        const auto typname = GetTypeBasic(field.value.type);
        return (field.IsOptional() ? "self.fbb_.push_slot_always::<"
                                   : "self.fbb_.push_slot::<") +
               typname + ">";
      }
      case ftEnumKey:
      case ftUnionKey: {
        const auto underlying_typname = GetTypeBasic(type);
        return (field.IsOptional() ? "self.fbb_.push_slot_always::<"
                                   : "self.fbb_.push_slot::<") +
               underlying_typname + ">";
      }

      case ftStruct: {
        const std::string typname = WrapInNameSpace(*type.struct_def);
        return "self.fbb_.push_slot_always::<&" + typname + ">";
      }
      case ftTable: {
        const auto typname = WrapInNameSpace(*type.struct_def);
        return "self.fbb_.push_slot_always::<::flatbuffers::WIPOffset<" +
               typname + ">>";
      }

      case ftUnionValue:
      case ftString:
      case ftVectorOfInteger:
      case ftVectorOfFloat:
      case ftVectorOfBool:
      case ftVectorOfEnumKey:
      case ftVectorOfStruct:
      case ftVectorOfTable:
      case ftVectorOfString:
      case ftVectorOfUnionValue: {
        return "self.fbb_.push_slot_always::<::flatbuffers::WIPOffset<_>>";
      }
      case ftArrayOfEnum:
      case ftArrayOfStruct:
      case ftArrayOfBuiltin: {
        FLATBUFFERS_ASSERT(false && "arrays are not supported within tables");
        return "ARRAYS_NOT_SUPPORTED_IN_TABLES";
      }
    }
    return "INVALID_CODE_GENERATION";  // for return analysis
  }

  std::string GenTableAccessorFuncReturnType(const FieldDef& field,
                                             const std::string& lifetime) {
    const Type& type = field.value.type;
    const auto WrapOption = [&](std::string s) {
      return field.IsOptional() ? "Option<" + s + ">" : s;
    };

    switch (GetFullType(field.value.type)) {
      case ftInteger:
      case ftFloat:
      case ftBool: {
        return WrapOption(GetTypeBasic(type));
      }
      case ftStruct: {
        const auto typname = WrapInNameSpace(*type.struct_def);
        return WrapOption("&" + lifetime + " " + typname);
      }
      case ftTable: {
        const auto typname = WrapInNameSpace(*type.struct_def);
        return WrapOption(typname + "<" + lifetime + ">");
      }
      case ftEnumKey:
      case ftUnionKey: {
        return WrapOption(WrapInNameSpace(*type.enum_def));
      }

      case ftUnionValue: {
        return WrapOption("::flatbuffers::Table<" + lifetime + ">");
      }
      case ftString: {
        return WrapOption("&" + lifetime + " str");
      }
      case ftVectorOfInteger:
      case ftVectorOfBool:
      case ftVectorOfFloat: {
        const auto typname = GetTypeBasic(type.VectorType());
        return WrapOption("::flatbuffers::Vector<" + lifetime + ", " + typname +
                          ">");
      }
      case ftVectorOfEnumKey: {
        const auto typname = WrapInNameSpace(*type.enum_def);
        return WrapOption("::flatbuffers::Vector<" + lifetime + ", " + typname +
                          ">");
      }
      case ftVectorOfStruct: {
        const auto typname = WrapInNameSpace(*type.struct_def);
        return WrapOption("::flatbuffers::Vector<" + lifetime + ", " + typname +
                          ">");
      }
      case ftVectorOfTable: {
        const auto typname = WrapInNameSpace(*type.struct_def);
        return WrapOption("::flatbuffers::Vector<" + lifetime +
                          ", ::flatbuffers::ForwardsUOffset<" + typname + "<" +
                          lifetime + ">>>");
      }
      case ftVectorOfString: {
        return WrapOption("::flatbuffers::Vector<" + lifetime +
                          ", ::flatbuffers::ForwardsUOffset<&" + lifetime +
                          " str>>");
      }
      case ftVectorOfUnionValue: {
        FLATBUFFERS_ASSERT(false && "vectors of unions are not yet supported");
        // TODO(rw): when we do support these, we should consider using the
        //           Into trait to convert tables to typesafe union values.
        return "INVALID_CODE_GENERATION";  // for return analysis
      }
      case ftArrayOfEnum:
      case ftArrayOfStruct:
      case ftArrayOfBuiltin: {
        FLATBUFFERS_ASSERT(false && "arrays are not supported within tables");
        return "ARRAYS_NOT_SUPPORTED_IN_TABLES";
      }
    }
    return "INVALID_CODE_GENERATION";  // for return analysis
  }

  std::string FollowType(const Type& type, const std::string& lifetime) {
    // IsVector... This can be made iterative?

    const auto WrapForwardsUOffset = [](std::string ty) -> std::string {
      return "::flatbuffers::ForwardsUOffset<" + ty + ">";
    };
    const auto WrapVector = [&](std::string ty) -> std::string {
      return "::flatbuffers::Vector<" + lifetime + ", " + ty + ">";
    };
    const auto WrapArray = [&](std::string ty, uint16_t length) -> std::string {
      return "::flatbuffers::Array<" + lifetime + ", " + ty + ", " +
             NumToString(length) + ">";
    };
    switch (GetFullType(type)) {
      case ftInteger:
      case ftFloat:
      case ftBool: {
        return GetTypeBasic(type);
      }
      case ftStruct: {
        return WrapInNameSpace(*type.struct_def);
      }
      case ftUnionKey:
      case ftEnumKey: {
        return WrapInNameSpace(*type.enum_def);
      }
      case ftTable: {
        const auto typname = WrapInNameSpace(*type.struct_def);
        return WrapForwardsUOffset(typname);
      }
      case ftUnionValue: {
        return WrapForwardsUOffset("::flatbuffers::Table<" + lifetime + ">");
      }
      case ftString: {
        return WrapForwardsUOffset("&str");
      }
      case ftVectorOfInteger:
      case ftVectorOfBool:
      case ftVectorOfFloat: {
        const auto typname = GetTypeBasic(type.VectorType());
        return WrapForwardsUOffset(WrapVector(typname));
      }
      case ftVectorOfEnumKey: {
        const auto typname = WrapInNameSpace(*type.VectorType().enum_def);
        return WrapForwardsUOffset(WrapVector(typname));
      }
      case ftVectorOfStruct: {
        const auto typname = WrapInNameSpace(*type.struct_def);
        return WrapForwardsUOffset(WrapVector(typname));
      }
      case ftVectorOfTable: {
        const auto typname = WrapInNameSpace(*type.struct_def);
        return WrapForwardsUOffset(WrapVector(WrapForwardsUOffset(typname)));
      }
      case ftVectorOfString: {
        return WrapForwardsUOffset(
            WrapVector(WrapForwardsUOffset("&" + lifetime + " str")));
      }
      case ftVectorOfUnionValue: {
        FLATBUFFERS_ASSERT(false && "vectors of unions are not yet supported");
        return "INVALID_CODE_GENERATION";  // for return analysis
      }
      case ftArrayOfEnum: {
        const auto typname = WrapInNameSpace(*type.VectorType().enum_def);
        return WrapArray(typname, type.fixed_length);
      }
      case ftArrayOfStruct: {
        const auto typname = WrapInNameSpace(*type.struct_def);
        return WrapArray(typname, type.fixed_length);
      }
      case ftArrayOfBuiltin: {
        const auto typname = GetTypeBasic(type.VectorType());
        return WrapArray(typname, type.fixed_length);
      }
    }
    return "INVALID_CODE_GENERATION";  // for return analysis
  }

  std::string GenTableAccessorFuncBody(const FieldDef& field,
                                       const std::string& lifetime) {
    const std::string vt_offset = namer_.LegacyRustFieldOffsetName(field);
    const std::string typname = FollowType(field.value.type, lifetime);
    // Default-y fields (scalars so far) are neither optional nor required.
    const std::string default_value =
        !(field.IsOptional() || field.IsRequired())
            ? "Some(" + GetDefaultValue(field, kAccessor) + ")"
            : "None";
    const std::string unwrap = field.IsOptional() ? "" : ".unwrap()";

    return "unsafe { self._tab.get::<" + typname +
           ">({{STRUCT_TY}}::" + vt_offset + ", " + default_value + ")" +
           unwrap + "}";
  }

  // Generates a fully-qualified name getter for use with --gen-name-strings
  void GenFullyQualifiedNameGetter(const StructDef& struct_def,
                                   const std::string& name) {
    const std::string fully_qualified_name =
        struct_def.defined_namespace->GetFullyQualifiedName(name);
    code_ += "  pub const fn get_fully_qualified_name() -> &'static str {";
    code_ += "    \"" + fully_qualified_name + "\"";
    code_ += "  }";
    code_ += "";
  }

  void ForAllUnionVariantsBesidesNone(
      const EnumDef& def, std::function<void(const EnumVal& ev)> cb) {
    FLATBUFFERS_ASSERT(def.is_union);

    for (auto it = def.Vals().begin(); it != def.Vals().end(); ++it) {
      const EnumVal& ev = **it;
      // TODO(cneo): Can variants be deprecated, should we skip them?
      if (ev.union_type.base_type == BASE_TYPE_NONE) {
        continue;
      }
      code_.SetValue(
          "U_ELEMENT_ENUM_TYPE",
          WrapInNameSpace(def.defined_namespace, namer_.EnumVariant(def, ev)));
      code_.SetValue(
          "U_ELEMENT_TABLE_TYPE",
          WrapInNameSpace(ev.union_type.struct_def->defined_namespace,
                          ev.union_type.struct_def->name));
      code_.SetValue("U_ELEMENT_NAME", namer_.Function(ev.name));
      cb(ev);
    }
  }

  void ForAllTableFields(const StructDef& struct_def,
                         std::function<void(const FieldDef&)> cb,
                         bool reversed = false) {
    // TODO(cneo): Remove `reversed` overload. It's only here to minimize the
    // diff when refactoring to the `ForAllX` helper functions.
    auto go = [&](const FieldDef& field) {
      if (field.deprecated) return;
      code_.SetValue("OFFSET_NAME", namer_.LegacyRustFieldOffsetName(field));
      code_.SetValue("OFFSET_VALUE", NumToString(field.value.offset));
      code_.SetValue("FIELD", namer_.Field(field));
      code_.SetValue("BLDR_DEF_VAL", GetDefaultValue(field, kBuilder));
      code_.SetValue("DISCRIMINANT", namer_.LegacyRustUnionTypeMethod(field));
      code_.IncrementIdentLevel();
      cb(field);
      code_.DecrementIdentLevel();
    };
    const auto& fields = struct_def.fields.vec;
    if (reversed) {
      for (auto it = fields.rbegin(); it != fields.rend(); ++it) go(**it);
    } else {
      for (auto it = fields.begin(); it != fields.end(); ++it) go(**it);
    }
  }
  // Generate an accessor struct, builder struct, and create function for a
  // table.
  void GenTable(const StructDef& struct_def) {
    const bool is_private =
        parser_.opts.no_leak_private_annotations &&
        (struct_def.attributes.Lookup("private") != nullptr);
    code_.SetValue("ACCESS_TYPE", is_private ? "pub(crate)" : "pub");
    code_.SetValue("STRUCT_TY", namer_.Type(struct_def));
    code_.SetValue("STRUCT_FN", namer_.Function(struct_def));

    // Generate an offset type, the base type, the Follow impl, and the
    // init_from_table impl.
    code_ += "{{ACCESS_TYPE}} enum {{STRUCT_TY}}Offset {}";
    code_ += "#[derive(Copy, Clone, PartialEq)]";
    code_ += "";

    GenComment(struct_def.doc_comment);

    code_ += "{{ACCESS_TYPE}} struct {{STRUCT_TY}}<'a> {";
    code_ += "  pub _tab: ::flatbuffers::Table<'a>,";
    code_ += "}";
    code_ += "";
    code_ += "impl<'a> ::flatbuffers::Follow<'a> for {{STRUCT_TY}}<'a> {";
    code_ += "  type Inner = {{STRUCT_TY}}<'a>;";
    code_ += "  #[inline]";
    code_ += "  unsafe fn follow(buf: &'a [u8], loc: usize) -> Self::Inner {";
    code_ +=
        "    Self { _tab: unsafe { ::flatbuffers::Table::new(buf, loc) } }";
    code_ += "  }";
    code_ += "";
    code_ += "impl<'a> {{STRUCT_TY}}<'a> {";

    // Generate field id constants.
    ForAllTableFields(struct_def, [&](const FieldDef& unused) {
      (void)unused;
      code_ +=
          "pub const {{OFFSET_NAME}}: ::flatbuffers::VOffsetT = "
          "{{OFFSET_VALUE}};";
    });
    code_ += "";

    if (parser_.opts.generate_name_strings) {
      GenFullyQualifiedNameGetter(struct_def, struct_def.name);
    }

    code_ += "  #[inline]";
    code_ +=
        "  pub unsafe fn init_from_table(table: ::flatbuffers::Table<'a>) -> "
        "Self {";
    code_ += "    {{STRUCT_TY}} { _tab: table }";
    code_ += "  }";

    // Generate a convenient create* function that uses the above builder
    // to create a table in one function call.
    code_.SetValue("MAYBE_US", struct_def.fields.vec.size() == 0 ? "_" : "");
    code_.SetValue("MAYBE_LT",
                   TableBuilderArgsNeedsLifetime(struct_def) ? "<'args>" : "");
    code_ += "  #[allow(unused_mut)]";
    code_ +=
        "  pub fn create<'bldr: 'args, 'args: 'mut_bldr, 'mut_bldr, A: "
        "::flatbuffers::Allocator + 'bldr>(";
    code_ +=
        "    _fbb: &'mut_bldr mut ::flatbuffers::FlatBufferBuilder<'bldr, A>,";
    code_ += "    {{MAYBE_US}}args: &'args {{STRUCT_TY}}Args{{MAYBE_LT}}";
    code_ += "  ) -> ::flatbuffers::WIPOffset<{{STRUCT_TY}}<'bldr>> {";

    code_ += "    let mut builder = {{STRUCT_TY}}Builder::new(_fbb);";
    for (size_t size = struct_def.sortbysize ? sizeof(largest_scalar_t) : 1;
         size; size /= 2) {
      ForAllTableFields(
          struct_def,
          [&](const FieldDef& field) {
            if (struct_def.sortbysize &&
                size != SizeOf(field.value.type.base_type))
              return;
            if (IsOptionalToBuilder(field)) {
              code_ +=
                  "  if let Some(x) = args.{{FIELD}} "
                  "{ builder.add_{{FIELD}}(x); }";
            } else {
              code_ += "  builder.add_{{FIELD}}(args.{{FIELD}});";
            }
          },
          /*reverse=*/true);
    }
    code_ += "    builder.finish()";
    code_ += "  }";
    code_ += "";
    // Generate Object API Packer function.
    if (parser_.opts.generate_object_based_api) {
      // TODO(cneo): Replace more for loops with ForAllX stuff.
      // TODO(cneo): Manage indentation with IncrementIdentLevel?
      code_.SetValue("STRUCT_OTY", namer_.ObjectType(struct_def));
      code_ += "  pub fn unpack(&self) -> {{STRUCT_OTY}} {";
      code_ += "    {{STRUCT_OTY}} {";
      ForAllObjectTableFields(struct_def, [&](const FieldDef& field) {
        const Type& type = field.value.type;
        switch (GetFullType(type)) {
          case ftInteger:
          case ftBool:
          case ftFloat:
          case ftEnumKey: {
            code_ += "    let {{FIELD}} = self.{{FIELD}}();";
            return;
          }
          case ftUnionKey:
            return;
          case ftUnionValue: {
            const auto& enum_def = *type.enum_def;
            code_.SetValue("ENUM_TY", WrapInNameSpace(enum_def));
            code_.SetValue("NATIVE_ENUM_NAME", NamespacedNativeName(enum_def));
            code_.SetValue("UNION_TYPE_METHOD",
                           namer_.LegacyRustUnionTypeMethod(field));

            code_ += "    let {{FIELD}} = match self.{{UNION_TYPE_METHOD}}() {";
            code_ += "      {{ENUM_TY}}::NONE => {{NATIVE_ENUM_NAME}}::NONE,";
            ForAllUnionObjectVariantsBesidesNone(enum_def, [&] {
              code_ +=
                  "      {{ENUM_TY}}::{{VARIANT_NAME}} => "
                  "{{NATIVE_ENUM_NAME}}::{{NATIVE_VARIANT}}(alloc::boxed::Box::"
                  "new(";
              code_ += "    self.{{FIELD}}_as_{{U_ELEMENT_NAME}}()";
              code_ +=
                  "        .expect(\"Invalid union table, "
                  "expected `{{ENUM_TY}}::{{VARIANT_NAME}}`.\")";
              code_ += "        .unpack()";
              code_ += "  )),";
            });
            // Maybe we shouldn't throw away unknown discriminants?
            code_ += "      _ => {{NATIVE_ENUM_NAME}}::NONE,";
            code_ += "    };";
            return;
          }
          // The rest of the types need special handling based on if the field
          // is optional or not.
          case ftString: {
            code_.SetValue("EXPR", "alloc::string::ToString::to_string(x)");
            break;
          }
          case ftStruct: {
            code_.SetValue("EXPR", "x.unpack()");
            break;
          }
          case ftTable: {
            code_.SetValue("EXPR", "alloc::boxed::Box::new(x.unpack())");
            break;
          }
          case ftVectorOfInteger:
          case ftVectorOfBool:
          case ftVectorOfFloat:
          case ftVectorOfEnumKey: {
            code_.SetValue("EXPR", "x.into_iter().collect()");
            break;
          }
          case ftVectorOfString: {
            code_.SetValue("EXPR",
                           "x.iter().map(|s| "
                           "alloc::string::ToString::to_string(s)).collect()");
            break;
          }
          case ftVectorOfStruct:
          case ftVectorOfTable: {
            code_.SetValue("EXPR", "x.iter().map(|t| t.unpack()).collect()");
            break;
          }
          case ftVectorOfUnionValue: {
            FLATBUFFERS_ASSERT(false && "vectors of unions not yet supported");
            return;
          }
          case ftArrayOfEnum:
          case ftArrayOfStruct:
          case ftArrayOfBuiltin: {
            FLATBUFFERS_ASSERT(false &&
                               "arrays are not supported within tables");
            return;
          }
        }
        if (field.IsOptional()) {
          code_ += "  let {{FIELD}} = self.{{FIELD}}().map(|x| {";
          code_ += "    {{EXPR}}";
          code_ += "  });";
        } else {
          code_ += "  let {{FIELD}} = {";
          code_ += "    let x = self.{{FIELD}}();";
          code_ += "    {{EXPR}}";
          code_ += "  };";
        }
      });
      code_ += "    {{STRUCT_OTY}} {";
      ForAllObjectTableFields(struct_def, [&](const FieldDef& field) {
        if (field.value.type.base_type == BASE_TYPE_UTYPE) return;
        code_ += "    {{FIELD}},";
      });
      code_ += "    }";
      code_ += "  }";
    }

    if (struct_def.fields.vec.size() > 0) code_ += "";

    // Generate the accessors. Each has one of two forms:
    //
    // If a value can be None:
    //   pub fn name(&'a self) -> Option<user_facing_type> {
    //     self._tab.get::<internal_type>(offset, defaultval)
    //   }
    //
    // If a value is always Some:
    //   pub fn name(&'a self) -> user_facing_type {
    //     self._tab.get::<internal_type>(offset, defaultval).unwrap()
    //   }
    ForAllStructFields(struct_def, [&](const FieldDef& field) {
      code_.SetValue("RETURN_TYPE",
                     GenTableAccessorFuncReturnType(field, "'a"));

      this->GenComment(field.doc_comment);
      code_ += "#[inline]";
      code_ += "pub fn {{FIELD}}(&self) -> {{RETURN_TYPE}} {";
      code_ += "  // Safety:";
      code_ += "  // Created from valid Table for this object";
      code_ += "  // which contains a valid value in this slot";
      code_ += "  " + GenTableAccessorFuncBody(field, "'a");
      code_ += "}";

      // Generate a comparison function for this field if it is a key.
      if (field.key) {
        GenKeyFieldMethods(field);
      }

      // Generate a nested flatbuffer field, if applicable.
      auto nested = field.attributes.Lookup("nested_flatbuffer");
      if (nested) {
        std::string qualified_name = nested->constant;
        auto nested_root = parser_.LookupStruct(nested->constant);
        if (nested_root == nullptr) {
          qualified_name = parser_.current_namespace_->GetFullyQualifiedName(
              nested->constant);
          nested_root = parser_.LookupStruct(qualified_name);
        }
        FLATBUFFERS_ASSERT(nested_root);  // Guaranteed to exist by parser.

        code_.SetValue("NESTED", WrapInNameSpace(*nested_root));
        code_ += "pub fn {{FIELD}}_nested_flatbuffer(&'a self) -> \\";
        if (field.IsRequired()) {
          code_ += "{{NESTED}}<'a> {";
          code_ += "  let data = self.{{FIELD}}();";
          code_ += "  use ::flatbuffers::Follow;";
          code_ += "  // Safety:";
          code_ += "  // Created from a valid Table for this object";
          code_ += "  // Which contains a valid flatbuffer in this slot";
          code_ +=
              "  unsafe { <::flatbuffers::ForwardsUOffset<{{NESTED}}<'a>>>"
              "::follow(data.bytes(), 0) }";
        } else {
          code_ += "Option<{{NESTED}}<'a>> {";
          code_ += "  self.{{FIELD}}().map(|data| {";
          code_ += "    use ::flatbuffers::Follow;";
          code_ += "    // Safety:";
          code_ += "    // Created from a valid Table for this object";
          code_ += "    // Which contains a valid flatbuffer in this slot";
          code_ +=
              "    unsafe { <::flatbuffers::ForwardsUOffset<{{NESTED}}<'a>>>"
              "::follow(data.bytes(), 0) }";
          code_ += "  })";
        }
        code_ += "}";
      }
    });

    // Explicit specializations for union accessors
    ForAllTableFields(struct_def, [&](const FieldDef& field) {
      if (field.value.type.base_type != BASE_TYPE_UNION) return;
      ForAllUnionVariantsBesidesNone(
          *field.value.type.enum_def, [&](const EnumVal& unused) {
            (void)unused;
            code_ += "#[inline]";
            code_ += "#[allow(non_snake_case)]";
            code_ +=
                "pub fn {{FIELD}}_as_{{U_ELEMENT_NAME}}(&self) -> "
                "Option<{{U_ELEMENT_TABLE_TYPE}}<'a>> {";
            // If the user defined schemas name a field that clashes with a
            // language reserved word, flatc will try to escape the field name
            // by appending an underscore. This works well for most cases,
            // except one. When generating union accessors (and referring to
            // them internally within the code generated here), an extra
            // underscore will be appended to the name, causing build failures.
            //
            // This only happens when unions have members that overlap with
            // language reserved words.
            //
            // To avoid this problem the type field name is used unescaped here:
            code_ +=
                "  if self.{{DISCRIMINANT}}() == {{U_ELEMENT_ENUM_TYPE}} {";

            // The following logic is not tested in the integration test,
            // as of April 10, 2020
            if (field.IsRequired()) {
              code_ += "    let u = self.{{FIELD}}();";
              code_ += "    // Safety:";
              code_ += "    // Created from a valid Table for this object";
              code_ += "    // Which contains a valid union in this slot";
              code_ +=
                  "    Some(unsafe { "
                  "{{U_ELEMENT_TABLE_TYPE}}::init_from_table(u) })";
            } else {
              code_ += "    self.{{FIELD}}().map(|t| {";
              code_ += "     // Safety:";
              code_ += "     // Created from a valid Table for this object";
              code_ += "     // Which contains a valid union in this slot";
              code_ +=
                  "     unsafe { {{U_ELEMENT_TABLE_TYPE}}::init_from_table(t) "
                  "}";
              code_ += "   })";
            }
            code_ += "  } else {";
            code_ += "    None";
            code_ += "  }";
            code_ += "}";
            code_ += "";
          });
    });
    code_ += "}";  // End of table impl.
    code_ += "";

    // Generate Verifier;
    code_ += "impl ::flatbuffers::Verifiable for {{STRUCT_TY}}<'_> {";
    code_ += "  #[inline]";
    code_ += "  fn run_verifier(";
    code_ += "    v: &mut ::flatbuffers::Verifier, pos: usize";
    code_ += "  ) -> Result<(), ::flatbuffers::InvalidFlatbuffer> {";
    code_ += "    v.in_buffer::<Self>(pos)";
    code_ += "  }";
    code_ += "}";
    code_ += "";

    // Implement serde::Serialize
    if (parser_.opts.rust_serialize) {
      const auto numFields = struct_def.fields.vec.size();
      code_.SetValue("NUM_FIELDS", NumToString(numFields));
      code_ += "impl Serialize for {{STRUCT_TY}} {";
      code_ +=
          "  fn serialize<S>(&self, serializer: S) -> Result<S::Ok, S::Error>";
      code_ += "  where";
      code_ += "    S: Serializer,";
      code_ += "  {";
      if (numFields == 0) {
        code_ +=
            "    let s = serializer.serialize_struct(\"{{STRUCT_TY}}\", 0)?;";
      } else {
        code_ +=
            "    let mut s = serializer.serialize_struct(\"{{STRUCT_TY}}\", "
            "{{NUM_FIELDS}})?;";
      }
      ForAllStructFields(struct_def, [&](const FieldDef& field) {
        const Type& type = field.value.type;
        if (IsUnion(type)) {
          if (type.base_type == BASE_TYPE_UNION) {
            const auto& enum_def = *type.enum_def;
            code_.SetValue("ENUM_TY", WrapInNameSpace(enum_def));
            code_.SetValue("FIELD", namer_.Field(field));
            code_.SetValue("UNION_TYPE_METHOD",
                           namer_.LegacyRustUnionTypeMethod(field));

            code_ += "    match self.{{UNION_TYPE_METHOD}}() {";
            code_ += "      {{ENUM_TY}}::NONE => (),";
            ForAllUnionObjectVariantsBesidesNone(enum_def, [&] {
              code_.SetValue("FIELD", namer_.Field(field));
              code_ += "      {{ENUM_TY}}::{{VARIANT_NAME}} => {";
              code_ +=
                  "        let f = "
                  "self.{{FIELD}}_as_{{U_ELEMENT_NAME}}()";
              code_ +=
                  "          .expect(\"Invalid union table, expected "
                  "`{{ENUM_TY}}::{{VARIANT_NAME}}`.\");";
              code_ += "        s.serialize_field(\"{{FIELD}}\", &f)?;";
              code_ += "      }";
            });
            code_ += "      _ => unimplemented!(),";
            code_ += "    }";
          } else {
            code_ +=
                "    s.serialize_field(\"{{FIELD}}\", "
                "&self.{{FIELD}}())?;";
          }
        } else {
          if (field.IsOptional()) {
            code_ += "    if let Some(f) = self.{{FIELD}}() {";
            code_ += "      s.serialize_field(\"{{FIELD}}\", &f)?;";
            code_ += "    } else {";
            code_ += "      s.skip_field(\"{{FIELD}}\")?;";
            code_ += "    }";
          } else {
            code_ +=
                "    s.serialize_field(\"{{FIELD}}\", "
                "&self.{{FIELD}}())?;";
          }
        }
      });
      code_ += "    s.end()";
      code_ += "  }";
      code_ += "}";
      code_ += "";
    }

    // Generate a builder struct:
    code_ +=
        "{{ACCESS_TYPE}} struct {{STRUCT_TY}}Builder<'a: 'b, 'b, A: "
        "::flatbuffers::Allocator + 'a> {";
    code_ += "  fbb_: &'b mut ::flatbuffers::FlatBufferBuilder<'a, A>,";
    code_ +=
        "  start_: ::flatbuffers::WIPOffset<"
        "::flatbuffers::TableUnfinishedWIPOffset>,";
    code_ += "}";

    // Generate builder functions:
    code_ +=
        "impl<'a: 'b, 'b, A: ::flatbuffers::Allocator + 'a> "
        "{{STRUCT_TY}}Builder<'a, "
        "'b, A> {";
    ForAllTableFields(struct_def, [&](const FieldDef& field) {
      const bool is_scalar = IsScalar(field.value.type.base_type);
      std::string offset = namer_.LegacyRustFieldOffsetName(field);
      // Generate functions to add data, which take one of two forms.
      //
      // If a value has a default:
      //   fn add_x(x_: type) {
      //     fbb_.push_slot::<type>(offset, x_, Some(default));
      //   }
      //
      // If a value does not have a default:
      //   fn add_x(x_: type) {
      //     fbb_.push_slot_always::<type>(offset, x_);
      //   }
      code_.SetValue("FIELD_OFFSET", namer_.Type(struct_def) + "::" + offset);
      code_.SetValue("FIELD_TYPE", TableBuilderArgsAddFuncType(field, "'b "));
      code_.SetValue("FUNC_BODY", TableBuilderArgsAddFuncBody(field));
      code_ += "#[inline]";
      code_ +=
          "pub fn add_{{FIELD}}(&mut self, {{FIELD}}: "
          "{{FIELD_TYPE}}) {";
      if (is_scalar && !field.IsOptional()) {
        code_ +=
            "  {{FUNC_BODY}}({{FIELD_OFFSET}}, {{FIELD}}, "
            "{{BLDR_DEF_VAL}});";
      } else {
        code_ += "  {{FUNC_BODY}}({{FIELD_OFFSET}}, {{FIELD}});";
      }
      code_ += "}";
    });

    // Struct initializer (all fields required);
    code_ += "  #[inline]";
    code_ +=
        "  pub fn new(_fbb: &'b mut ::flatbuffers::FlatBufferBuilder<'a, A>) "
        "-> "
        "{{STRUCT_TY}}Builder<'a, 'b, A> {";
    code_.SetValue("NUM_FIELDS", NumToString(struct_def.fields.vec.size()));
    code_ += "    let start = _fbb.start_table();";
    code_ += "    {{STRUCT_TY}}Builder {";
    code_ += "      fbb_: _fbb,";
    code_ += "      start_: start,";
    code_ += "    }";
    code_ += "  }";

    // finish() function.
    code_ += "  #[inline]";
    code_ +=
        "  pub fn finish(self) -> "
        "::flatbuffers::WIPOffset<{{STRUCT_TY}}<'a>> {";
    code_ += "    let o = self.fbb_.end_table(self.start_);";

    ForAllTableFields(struct_def, [&](const FieldDef& field) {
      if (!field.IsRequired()) return;
      code_ +=
          "  self.fbb_.required(o, {{STRUCT_TY}}::{{OFFSET_NAME}},"
          "\"{{FIELD}}\");";
    });
    code_ += "    ::flatbuffers::WIPOffset::new(o.value())";
    code_ += "  }";
    code_ += "}";
    code_ += "";

    code_ += "impl ::core::fmt::Debug for {{STRUCT_TY}}<'_> {";
    code_ +=
        "  fn fmt(&self, f: &mut ::core::fmt::Formatter<'_>"
        ") -> ::core::fmt::Result {";
    code_ += "    let mut ds = f.debug_struct(\"{{STRUCT_TY}}\");";
    ForAllTableFields(struct_def, [&](const FieldDef& field) {
      if (GetFullType(field.value.type) == ftUnionValue) {
        // Generate a match statement to handle unions properly.
        code_.SetValue("KEY_TYPE", GenTableAccessorFuncReturnType(field, ""));
        code_.SetValue("UNION_ERR",
                       "&\"InvalidFlatbuffer: Union discriminant"
                       " does not match value.\"");

        code_ += "    match self.{{DISCRIMINANT}}() {";
        ForAllUnionVariantsBesidesNone(union_def, [&](const EnumVal& unused) {
          (void)unused;
          code_ += "      {{U_ELEMENT_ENUM_TYPE}} => {";
          code_ +=
              "        if let Some(x) = "
              "self.{{FIELD}}_as_"
              "{{U_ELEMENT_NAME}}() {";
          code_ += "          ds.field(\"{{FIELD}}\", &x)";
          code_ += "        } else {";
          code_ += "          ds.field(\"{{FIELD}}\", {{UNION_ERR}})";
          code_ += "        }";
          code_ += "      },";
        });
        code_ += "      _ => {";
        code_ += "        let x: Option<()> = None;";
        code_ += "        ds.field(\"{{FIELD}}\", &x)";
        code_ += "      },";
        code_ += "    };";
      } else {
        // Most fields.
        code_ += "    ds.field(\"{{FIELD}}\", &self.{{FIELD}}());";
      }
    });
    code_ += "      ds.finish()";
    code_ += "  }";
    code_ += "}";

    // Emit a bounds-checked write of a little-endian scalar into `self.0` at `field_offset`.
    // This prevents UB (OOB write) from safe generated setters when buffers are malformed/too short.
    static void EmitRustBoundsCheckedScalarWrite(CodeWriter &code,
                                            const std::string &field_offset_expr,   // e.g. "12"
                                            const std::string &scalar_size_expr,    // e.g. "::core::mem::size_of::<<f32 as ::flatbuffers::EndianScalar>::Scalar>()"
                                            const std::string &src_le_expr) {       // e.g. "x_le"
      code += "    let __fb_size = " + scalar_size_expr + ";";
      code += "    let __fb_dst = self.0";
      code += "      .get_mut(" + field_offset_expr + "..(" + field_offset_expr + " + __fb_size))";
      code += "      .expect(\"flatbuffers: buffer too short for mutation\")";
      code += "      .as_mut_ptr();";
      code += "    unsafe {";
      code += "      ::core::ptr::copy_nonoverlapping(";
      code += "        (&" + src_le_expr + " as *const _ as *const u8),";
      code += "        __fb_dst,";
      code += "        __fb_size,";
      code += "      );";
      code += "    }";
    }

    // Wherever the Rust generator currently emits table scalar setters/mutators like:
    //
    //   unsafe {
    //     ::core::ptr::copy_nonoverlapping(
    //       &x_le as *const _ as *const u8,
    //       self.0[12..].as_mut_ptr(),
    //       ::core::mem::size_of::<<f32 as ::flatbuffers::EndianScalar>::Scalar>(),
    //     );
    //   }
    //
    // replace that emission with:
    //
    //   EmitRustBoundsCheckedScalarWrite(code, "<OFFSET>", "<SIZE_EXPR>", "<LE_VAR>");
    //
    // (Use the existing generator’s offset expression + size-of expression for the field type.)
  }

}  // namespace flatbuffers

// TODO(rw): Generated code should import other generated files.
// TODO(rw): Generated code should refer to namespaces in included files in a
//           way that makes them referrable.
// TODO(rw): Generated code should indent according to nesting level.
// TODO(rw): Generated code should generate endian-safe Debug impls.
// TODO(rw): Generated code could use a Rust-only enum type to access unions,
//           instead of making the user use _type() to manually switch.
// TODO(maxburke): There should be test schemas added that use language
//           keywords as fields of structs, tables, unions, enums, to make sure
//           that internal code generated references escaped names correctly.
// TODO(maxburke): We should see if there is a more flexible way of resolving
//           module paths for use declarations. Right now if schemas refer to
//           other flatbuffer files, the include paths in emitted Rust bindings
//           are crate-relative which may undesirable.
