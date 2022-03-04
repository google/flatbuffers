/*
 * Copyright 2021 Google Inc. All rights reserved.
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

#include "bfbs_gen_rust.h"

#include <cstdint>
#include <map>
#include <memory>
#include <string>
#include <unordered_set>
#include <vector>

// Ensure no includes to flatc internals. bfbs_gen.h and generator.h are OK.
#include "bfbs_gen.h"
#include "flatbuffers/bfbs_generator.h"

// The intermediate representation schema.
#include "flatbuffers/reflection_generated.h"

namespace flatbuffers {
namespace {

// To reduce typing
namespace r = ::reflection;

class CaseManager {
 public:
  struct Config {
    Case types;
    Case constants;
    Case methods;
    Case functions;
    Case fields;
    Case variants;
  };
  CaseManager(Config config, std::set<std::string> keywords)
    : config_(config), keywords_(std::move(keywords)) {}

  std::string Type(const flatbuffers::String* s) const {
    return Format(s->str(), config_.types);
  }
  std::string Method(const flatbuffers::String* s) const {
    return Format(s->str(), config_.methods);
  }
  std::string Constant(const flatbuffers::String* s) const {
    return Format(s->str(), config_.constants);
  }
  std::string Function(const flatbuffers::String* s) const {
    return Format(s->str(), config_.functions);
  }
  std::string Field(const flatbuffers::String* s) const {
    return Format(s->str(), config_.fields);
  }
  std::string Variant(const flatbuffers::String* s) const {
    return Format(s->str(), config_.variants);
  }

 private:
  std::string Escape(const std::string& name) const {
    return keywords_.find(name) == keywords_.end() ? name : name + "_";
  }
  std::string Format(const std::string& fullname, Case casing) const {
    size_t d = fullname.find_last_of('.');
    std::string name = d == std::string::npos ? fullname : fullname.substr(d+1);
    return Escape(ConvertCase(name, casing, Case::kLowerCamel));
  }

  Config config_;
  std::set<std::string> keywords_;
};

CaseManager MakeRustCaseManager() {
    std::set<std::string> keywords = {
    // currently-used keywords
    "as", "break", "const", "continue", "crate", "else", "enum", "extern",
    "false", "fn", "for", "if", "impl", "in", "let", "loop", "match", "mod",
    "move", "mut", "pub", "ref", "return", "Self", "self", "static", "struct",
    "super", "trait", "true", "type", "unsafe", "use", "where", "while",
    "abstract", "alignof", "become", "box", "do",
    // future possible keywords
    "final", "macro", "offsetof", "override", "priv", "proc", "pure",
    "sizeof", "typeof", "unsized", "virtual", "yield",
    // other rust terms we should not use
    "std", "usize", "isize", "u8", "i8", "u16", "i16", "u32", "i32", "u64",
    "i64", "u128", "i128", "f32", "f64", "follow", "push", "size",
    "alignment", "to_little_endian", "from_little_endian", "ENUM_MAX",
    "ENUM_MIN", "ENUM_VALUES",
  };
  CaseManager::Config config = {
    /*types=*/Case::kUpperCamel,
    /*constants=*/Case::kScreamingSnake,
    /*methods=*/Case::kSnake,
    /*functions=*/Case::kSnake,
    /*fields=*/Case::kSnake,
    /*variants=*/Case::kUpperCamel,
  };
  return CaseManager(config, std::move(keywords));
}


std::string NativeType(r::BaseType bt) {
  switch (bt) {
    case r::None: return "u8";
    case r::UType: return "u8";
    case r::Byte: return "i8";
    case r::UByte: return "u8";
    case r::Short: return "i16";
    case r::UShort: return "u16";
    case r::Int: return "i32";
    case r::UInt: return "u32";
    case r::Long: return "i64";
    case r::ULong: return "u64";
    case r::Float: return "f32";
    case r::Double: return "f64";
    default: return "INVALID_CODE_GENERATION_SORRY";
  }
}

class RustBfbsGenerator : public BaseBfbsGenerator {
 public:
  explicit RustBfbsGenerator(const std::string &flatc_version)
      : BaseBfbsGenerator(),
        case_(MakeRustCaseManager()),
        flatc_version_(flatc_version) {
    // static const char *const keywords[] = ;
    // keywords_.insert(std::begin(keywords), std::end(keywords));
  }
  GeneratorStatus GenerateFromSchema(const r::Schema *schema) FLATBUFFERS_OVERRIDE {
    schema_ = schema;
    // TODO(cneo): Figure out error propagation later.
    ForAllEnums(schema->enums(), [&](const r::Enum *e) {
      bool bit_flags = false;
      ForAllKeyValues(e->attributes(), [&](const r::KeyValue* kv) {
        bit_flags |= (kv->key()->str() == "bit_flags");
      });
      if (bit_flags) {
        this->GenBitFlagsEnum(*e);
      } else {
        this->GenEnum(*e);
      }
    });
    ForAllObjects(schema->objects(), [&](const r::Object *object) {
      if (object->is_struct()) {
        this->GenStruct(*object);
      } else {
        this->GenTable(*object);
      }
    });
    GenModuleRootFile(*schema);
    return OK;
  }
  uint64_t SupportedAdvancedFeatures() const FLATBUFFERS_OVERRIDE {
    return r::AdvancedFeatures::OptionalScalars
      | r::AdvancedFeatures::AdvancedArrayFeatures
      | r::AdvancedFeatures::DefaultVectorsAndStrings;
  }
 private:
  void GenModuleRootFile(const r::Schema& schema) {
    // We gather the symbols into a tree of namespaces (which are rust mods) and
    // generate a file that gathers them all.
    struct Module {
      // Map from namespace part to submodules.
      std::map<std::string, Module> sub_modules;
      // The generated files within the namespace part.
      std::vector<std::string> generated_files;
      void Insert(std::string type, std::string file) {
        // traverse namespace.
        size_t a = 0;
        size_t b = type.find('.');
        Module* current = this;
        while (b != std::string::npos) {
          current = &current->sub_modules[type.substr(a, b)];
          a = b;
          b = type.find(b + 1, '.');
        }
        // Add the generated file name.
        // TODO: Refactor out generation of generated-file-name
        if ((a = type.find_last_of('.')) == std::string::npos) {
          current->generated_files.push_back(type);
        } else {
          current->generated_files.push_back(type.substr(a + 1));
        }
      }
      void GenerateImports(std::string& code, std::string padding) {

        for (auto it = sub_modules.begin(); it != sub_modules.end(); it++) {
          code.append(
            padding + "pub mod " + it->first + " {\n  " +
            padding + "use super::*;\n"
          );
          it->second.GenerateImports(code, padding + "  ");
          code.append(padding + "}  // " + it->first  + "\n");
        }
        for (auto it = generated_files.begin(); it != generated_files.end();
             it++) {
          code.append(
            padding + "mod " + *it + ";\n" +
            padding + "pub use self::" + *it + "::*;\n"
          );
        }
      }
    };
    Module root;
    ForAllEnums(schema.enums(), [&](const r::Enum *e) {
      root.Insert(e->name()->str(), e->declaration_file()->str());
    });
    ForAllObjects(schema.objects(), [&](const r::Object *o) {
      root.Insert(o->name()->str(), o->declaration_file()->str());
    });
    // Walk all modules into the root file.
    std::string code;
    AddDisclaimer(code);
    // TODO: Get rid of unused imports!!!
    code.append("#![allow(dead_code, unused_imports, non_snake_case)]\n");
    root.GenerateImports(code, "");
    SaveCode("mod", code);
  }
  void AddEnumTraits(
    const std::string& name,
    const std::string& base_type,
    const std::string& into_base,
    const std::string& from_base,
    std::string& code
  ) {
    // Define Follow, Push, EndianScalar to get in/out of the binary fomrat.
    code.append(
      "impl<'a> flatbuffers::Follow<'a> for " + name + " {\n"
      "  type Inner = Self;\n"
      "  #[inline]\n"
      "  fn follow(buf: &'a [u8], loc: usize) -> Self::Inner {\n"
      "    let b = unsafe {\n"
      "      flatbuffers::read_scalar_at::<" + base_type + ">(buf, loc)\n"
      "    };\n"
      "    " + from_base + "\n"
      "  }\n"
      "}\n"
    );
    code.append(
      "impl flatbuffers::Push for " + name + " {\n"
      "    type Output = " + name + ";\n"
      "    #[inline]\n"
      "    fn push(&self, dst: &mut [u8], _rest: &[u8]) {\n"
      "        unsafe { flatbuffers::emplace_scalar::<" + base_type + ">(dst, "
      + into_base + "); }\n"
      "    }\n"
      "}\n"
    );
    code.append(
      "impl flatbuffers::EndianScalar for " + name + " {\n"
      "  #[inline]\n"
      "  fn to_little_endian(self) -> Self {\n"
      "    let b = " + base_type + "::to_le(" + into_base + ");\n"
      "    " + from_base + "\n"
      "  }\n"
      "  #[inline]\n"
      "  #[allow(clippy::wrong_self_convention)]\n"
      "  fn from_little_endian(self) -> Self {\n"
      "    let b = " + base_type + "::from_le(" + into_base + ");\n"
      "    " + from_base + "\n"
      "  }\n"
      "}\n"
    );

    // Define Verifiable.
    code.append(
      "impl<'a> flatbuffers::Verifiable for " + name + " {\n"
      "  #[inline]\n"
      "  fn run_verifier(\n"
      "    v: &mut flatbuffers::Verifier, pos: usize\n"
      "  ) -> Result<(), flatbuffers::InvalidFlatbuffer> {\n"
      "    use self::flatbuffers::Verifiable;\n"
      "    " + base_type + "::run_verifier(v, pos)\n"
      "  }\n"
      "}\n"
    );

    // Define Serialize.
    if (serde_) {
      code.append(
        "impl Serialize for " + name + " {\n"
        "  fn serialize<S>(&self, serializer: S) -> Result<S::Ok, S::Error>\n"
        "    where S: Serializer\n"
        "  {\n"
        "    let b = " + into_base + ";\n"
        "    serializer.serialize_unit_variant(\"" + name + "\","
        "  b, format!(self))\n"
        "  }\n"
        "}\n"
      );
    }
  }
  void GenEnum(const r::Enum& e) {
    std::string code;
    AddDisclaimer(code);
    AddImports(code);
    const std::string name = case_.Type(e.name());
    const std::string base_type = NativeType(e.underlying_type()->base_type());

    // Define the struct itself.
    ForAllDocumentation(e.documentation(), [&](const flatbuffers::String *d) {
      code.append("/// " + d->str() + "\n");
    });
    code.append(
      "#[derive(Clone, Copy, PartialEq, Eq, PartialOrd, Ord, Hash, Default)]\n"
      "#[repr(transparent)]\n"
      "pub struct " + name + "(" + base_type + ");\n"
      "impl " + name + " {\n"
    );
    // Define the individual enum variants, and their maximums.
    int64_t min = INT64_MAX;
    int64_t max = INT64_MIN;
    ForAllEnumValues(&e, [&](const r::EnumVal* v) {
      const std::string vname = v->name()->str();
      min = min < v->value() ? min : v->value();
      max = max > v->value() ? max : v->value();
      const std::string val = NumToString(v->value());
      code += "  pub const " + vname + ": Self = Self(" + val + ");\n";
    });
    code.append(
      "  pub const ENUM_MIN: " + base_type + " = " + NumToString(max) + ";\n"
      "  pub const ENUM_MAX: " + base_type + " = " + NumToString(min) + ";\n"
    );
    code.append("  pub const ENUM_VALUES: &'static [Self] = &[\n");
    ForAllEnumValues(&e, [&](const r::EnumVal* v) {
      code.append("    Self::" + v->name()->str() + ",\n");
    });
    code.append("  ];\n");

    // Define to-string functionality.
    code.append(
      "  /// Returns the variant's name or None if unknown.\n"
      "  fn variant_name(self) -> Option<&'static str> {\n"
      "    match self {\n"
    );
    ForAllEnumValues(&e, [&](const r::EnumVal* v) {
      const std::string vname = v->name()->str();
      code.append("      Self::" + vname + " => Some(\"" + vname + "\"),\n");
    });
    code.append(
      "      _ => None,\n"
      "    }\n"
      "  }\n"
      "}\n"
    );

    // Define Debug implementation.
    code.append(
      "impl std::fmt::Debug for " + name + " {\n"
      "  fn fmt(&self, f: &mut std::fmt::Formatter) -> std::fmt::Result {\n"
      "    if let Some(name) = self.variant_name() {\n"
      "      f.write_str(name)\n"
      "    } else {\n"
      "      f.write_fmt(format_args!(\"<UNKNOWN {:?}>\", self.0))\n"
      "    }\n"
      "  }\n"
      "}\n"
    );
    const std::string into_base = "self.0";
    const std::string from_base = "Self(b)";
    AddEnumTraits(name, base_type, into_base, from_base, code);

    if (e.is_union()) AddUnionObject(e, code);
    SaveCode(e.name()->c_str(), code);
  }
  void GenBitFlagsEnum(const r::Enum& e) {
    // TODO: Same boilerplate as Enum!
    std::string code;
    AddDisclaimer(code);
    AddImports(code);
    const std::string name = case_.Type(e.name());
    const std::string base_type = NativeType(e.underlying_type()->base_type());

    // Defer to the convenient and canonical bitflags crate. We declare it in
    // a module to #allow camel case constants in a smaller scope. This
    // matches Flatbuffers c-modeled enums where variants are associated
    // constants but in camel case.
    code.append(
      "#[allow(non_upper_case_globals)]\n"
      "mod bitflags {\n"
      "  flatbuffers::bitflags::bitflags! {\n"
      "    pub struct " + name + ": " + base_type + " {\n"
    );
    ForAllEnumValues(&e, [&](const r::EnumVal* v) {
      // CamelCase!
      std::string vname = case_.Type(v->name());
      std::string vval = NumToString(v->value());
      code.append("      const " + vname + " = " + vval + ";\n");
    });
    code.append(
      "    }\n"
      "  }\n"
      "}\n"
      "pub use self::bitflags::" + name + ";\n"
    );
    const std::string from_base = "unsafe { Self::from_bits_unchecked(b) }";
    const std::string into_base = "self.bits()";

    AddEnumTraits(name, base_type, into_base, from_base, code);
    SaveCode(e.name()->c_str(), code);
  }
  void AddUnionObject(const r::Enum& e, std::string& code) {
    FLATBUFFERS_ASSERT(e.is_union());  // TODO: implement
    const std::string type_name = case_.Type(e.name());
    const std::string native_name = case_.Type(e.name()) + "T";
    const std::string method_name = case_.Method(e.name());
    // Generate native union.
    // NONE's upper case spelling is intended.
    code.append(
      "#[allow(clippy::upper_case_acronyms)]\n"
      "#[non_exhaustive]\n"
      "#[derive(Debug, Clone, PartialEq)]\n"
      "pub enum " + native_name + " {\n"
      "  NONE,\n"
    );
    ForAllUnionVariantsBesidesNone(e, [&](const r::EnumVal* v) {
      // TODO: To get the table type, we need r::Schema.
      // TODO: Documentation?
      const std::string vname = case_.Type(v->name());
      code.append("  " + vname + "(Box<TodoTableType>),\n");
    });
    code.append(
      "}\n"
      "impl Default for " + native_name + " {\n"
      "  fn default() -> Self {\n"
      "    Self::NONE\n"
      "  }\n"
      "}\n\n"
      // Generate native union methods. Get flatbuffers union key.
      // CASPER: add docstrings?
      "impl " + native_name + " {\n"
      "  pub fn " + method_name + "_type(&self) -> " + type_name + " {\n"
      "    match self {\n"
      "      Self::NONE => " + type_name + "::NONE,\n"
    );
    ForAllUnionVariantsBesidesNone(e, [&](const r::EnumVal* v) {
      const std::string vname = case_.Type(v->name());
      code.append(
        "      Self::" + vname + "(_) => " + type_name + "::{{VARIANT_NAME}},\n"
      );
    });
    code.append(
      "    }\n"
      "  }\n\n"
      // Pack flatbuffers union value
      "  pub fn pack(&self, fbb: &mut flatbuffers::FlatBufferBuilder)"
      " -> Option<flatbuffers::WIPOffset<flatbuffers::UnionWIPOffset>> {\n"
      "    match self {\n"
      "      Self::NONE => None,\n"
    );
    ForAllUnionVariantsBesidesNone(e, [&](const r::EnumVal* v) {
      const std::string vname = case_.Type(v->name());
      code.append(
        "    Self::" + vname + "(v) => Some(v.pack(fbb).as_union_value()),\n"
      );
    });
    code.append(
      "    }\n"
      "  }\n"
    );

    // Generate some accessors;
    ForAllUnionVariantsBesidesNone(e, [&](const r::EnumVal* v) {
      const std::string vtype = case_.Type(v->name());
      const std::string vvariant = case_.Variant(v->name());
      const std::string vmethod = case_.Method(v->name());
      code.append(
        // Move accessor.
        "  /// If the union variant matches, return the owned " +
        vtype + ", setting the union to NONE.\n"
        "  pub fn take_" + vmethod + "(&mut self) -> Option<Box<"
        + vtype + ">> {\n"
        "    if let Self::" + vvariant + "(_) = self {\n"
        "      let v = std::mem::replace(self, Self::NONE);\n"
        "      if let Self::" + vvariant + "(w) = v {\n"
        "        Some(w)\n"
        "      } else {\n"
        "        unreachable!()\n"
        "      }\n"
        "    } else {\n"
        "      None\n"
        "    }\n"
        "  }\n"
        // Immutable reference accessor.
        "  /// If the union variant matches, return a reference to the " +
        vtype + ".\n"
        "  pub fn as_" + vmethod + "(&self) -> Option<&" + vtype +
        "> {\n"
        "    if let Self::" + vvariant + "(v) = self {\n"
        "      Some(v.as_ref())\n"
        "    } else {\n"
        "      None\n"
        "    }\n "
        "  }\n"
        // Mutable reference accessor.
        "  /// If the union variant matches, return a mutable reference to the "
        + vtype + ".\n"
        "  pub fn as_" + vmethod + "_mut(&mut self) -> Option<&mut " +
        vtype + "> {\n"
        "    if let Self::" + vvariant + "(v) = self {\n"
        "      Some(v.as_mut())\n"
        "    } else {\n"
        "      None\n"
        "    }\n "
        "  }\n"
     );
    });
    code.append("}\n");  // End union methods impl.

  }
  void GenStruct(const r::Object& o) {}
  void GenTable(const r::Object& o) {}
  // Builder Object Verifier Serializer etc etc etc.

  void AddDisclaimer(std::string& code) {
    code.append(
      "// Automatically generated by the flatbuffers compiler.\n"
      "// Do not modify... or do, I'm a message, not a cop.\n"
      "// flatc_version: " + flatc_version_ + "\n"
    );
  }
  void AddImports(std::string& code) {
    code.append(
      "use std::mem;\n"
      "use std::cmp::Ordering;\n"
      "use flatbuffers;\n"
      "use flatbuffers::{EndianScalar, Follow};\n"
    );
    if (serde_) {
      code.append(
        "use serde::ser::{Serialize, Serializer, SerializeStruct};\n"
      );
    }
  }

  std::string TypeFile(const std::string &type) {
    // TODO: Ensure snake case? Does it even work with dot delimited strings?
    std::string file_path = type;
    std::replace(file_path.begin(), file_path.end(), '.', '/');
    file_path.append(".rs");
    return file_path;
  }

  // Construct file to save code into.
  // `type` is a namespaced type name, e.g. `foo.bar.Table` will be stored in a
  // file like `foo/bar/table_generated.rs`
  void SaveCode(const std::string &type, const std::string& code) {
    // TODO: Ensure snake case? Does it even work with dot delimited strings?
    std::string file_path = TypeFile(type);
    size_t last_slash = file_path.find_last_of("/");
    if (last_slash != std::string::npos) {
      EnsureDirExists(file_path.substr(0, last_slash));
    }
    SaveFile(file_path.c_str(), code, false);
  }

  // StartScope.

  const r::Enum& GetEnum(int32_t index) {
    FLATBUFFERS_ASSERT(index >= 0);
    return *schema_->enums()->Get(index);
  }
  const r::Object& GetObject(int32_t index) {
    FLATBUFFERS_ASSERT(index >= 0);
    return *schema_->objects()->Get(index);
  }

  CaseManager case_;
  std::unordered_set<std::string> keywords_;
  const std::string flatc_version_;
  const bool serde_ = true;  // TODO!
  r::Schema const* schema_ = nullptr;
};
}  // namespace

std::unique_ptr<BfbsGenerator> NewRustBfbsGenerator(
  const std::string &flatc_version) {
  return std::unique_ptr<RustBfbsGenerator>(
    new RustBfbsGenerator(flatc_version));
}

}  // namespace flatbuffers
