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

#include "flatbuffers/code_generators.h"
#include "flatbuffers/flatbuffers.h"
#include "flatbuffers/idl.h"
#include "flatbuffers/util.h"

namespace flatbuffers {

static std::string GeneratedFileName(const std::string &path,
                                     const std::string &file_name) {
  return path + file_name + "_generated.rs";
}

// Convert a camelCaseIdentifier or CamelCaseIdentifier to a
// snake_case_indentifier.
std::string MakeSnakeCase(const std::string &in) {
  std::string s;
  for (size_t i = 0; i < in.length(); i++) {
    if (i == 0) {
      s += static_cast<char>(tolower(in[0]));
    } else if (in[i] == '_') {
      s += '_';
    } else if (!(islower(in[i]) || isdigit(in[i]))) {
      // Prevent duplicate underscores for Upper_Snake_Case strings
      // and UPPERCASE strings.
      if (islower(in[i - 1]) || isdigit(in[i - 1])) { s += '_'; }
      s += static_cast<char>(tolower(in[i]));
    } else {
      s += in[i];
    }
  }
  return s;
}

namespace rust {

class ModuleTree {
 public:
  ModuleTree *find(Namespace *ns) {
    ModuleTree *cur_node = this;
    for (auto ns_it = ns->components.begin(); ns_it != ns->components.end();
         ++ns_it) {
      cur_node = &cur_node->children_[*ns_it];
    }
    cur_node->ns_ = ns;
    return cur_node;
  }

  void insert_enum(EnumDef *value) {
    if (!value->generated) {
      find(value->defined_namespace)->enums_.push_back(value);
    }
  }

  void insert_struct(StructDef *value) {
    if (!value->generated) {
      find(value->defined_namespace)->structs_.push_back(value);
    }
  }

  std::map<std::string, ModuleTree> children_;
  std::vector<StructDef *> structs_;
  std::vector<EnumDef *> enums_;
  Namespace *ns_;
};

class RustGenerator : public BaseGenerator {
 public:
  RustGenerator(const Parser &parser, const std::string &path,
                const std::string &file_name)
      : BaseGenerator(parser, path, file_name, "", "::"),
        code_("    "),
        cur_name_space_(nullptr) {
    const char *keywords[] = {
      // list taken from:
      // https://doc.rust-lang.org/book/second-edition/appendix-01-keywords.html
      //
      // we write keywords one per line so that we can easily compare them
      // with
      // changes to that webpage in the future.

      // currently-used keywords
      "as", "break", "const", "continue", "crate", "dyn", "else", "enum",
      "extern", "false", "fn", "for", "if", "impl", "in", "let", "loop",
      "match", "mod", "move", "mut", "pub", "ref", "return", "Self", "self",
      "static", "struct", "super", "trait", "true", "type", "unsafe", "use",
      "where", "while",

      // future possible keywords
      "abstract", "async", "await", "become", "box", "do", "final", "macro",
      "override", "priv", "try", "typeof", "unsized", "virtual", "yield",

      // other rust terms we should not use
      "std", "usize", "isize", "u8", "i8", "u16", "i16", "u32", "i32", "u64",
      "i64", "u128", "i128", "f32", "f64", nullptr
    };
    for (auto kw = keywords; *kw; kw++) keywords_.insert(*kw);
  }

  // Iterate through all definitions we haven't generated code for (enums,
  // structs, and tables) and output them to a single file.
  bool generate() {
    code_.Clear();
    code_ += "// " + std::string(FlatBuffersGeneratedWarning()) + "\n\n";

    ModuleTree module_tree;

    for (auto it = parser_.enums_.vec.begin(); it != parser_.enums_.vec.end();
         ++it) {
      module_tree.insert_enum(*it);
    }

    for (auto it = parser_.structs_.vec.begin();
         it != parser_.structs_.vec.end(); ++it) {
      module_tree.insert_struct(*it);
    }
    GenModule(module_tree);

    const auto file_path = GeneratedFileName(path_, file_name_);
    const auto final_code = code_.ToString();
    return SaveFile(file_path.c_str(), final_code, false);
  }

 private:
  CodeWriter code_;

  std::set<std::string> keywords_;

  // This tracks the current namespace so we can insert namespace declarations.
  const Namespace *cur_name_space_;
  const Namespace *CurrentNameSpace() const { return cur_name_space_; }

  std::string EscapeKeyword(const std::string &name) const {
    return keywords_.find(name) == keywords_.end() ? name : name + "_";
  }

  std::string CamelName(const Definition &def) const {
    return EscapeKeyword(MakeCamel(def.name, true));
  }

  std::string CamelName(const EnumVal &ev) const {
    return EscapeKeyword(MakeCamel(ev.name, true));
  }

  std::string SnakeName(const Definition &def) const {
    return EscapeKeyword(MakeSnakeCase(def.name));
  }

  std::string SnakeName(const EnumVal &ev) const {
    return EscapeKeyword(MakeSnakeCase(ev.name));
  }

  std::string WrapInNameSpace(const Definition &def) const {
    return WrapInNameSpace(def.defined_namespace, CamelName(def));
  }
  std::string WrapInNameSpace(const Namespace *ns,
                              const std::string &name) const {
    if (CurrentNameSpace() == ns) return name;
    std::string prefix = GetRelativeNamespaceTraversal(CurrentNameSpace(), ns);
    return prefix + name;
  }

  // Determine the namespace traversal needed from the Rust crate root.
  // This may be useful in the future for referring to included files, but is
  // currently unused.
  std::string GetAbsoluteNamespaceTraversal(const Namespace *dst) const {
    std::stringstream stream;

    stream << "::";
    for (auto d = dst->components.begin(); d != dst->components.end(); ++d) {
      stream << MakeSnakeCase(*d) + "::";
    }
    return stream.str();
  }

  // Determine the relative namespace traversal needed to reference one
  // namespace from another namespace. This is useful because it does not
  // force the user to have a particular file layout. (If we output absolute
  // namespace paths, that may require users to organize their Rust crates in
  // a particular way.)
  std::string GetRelativeNamespaceTraversal(const Namespace *src,
                                            const Namespace *dst) const {
    // calculate the path needed to reference dst from src.
    // example: f(A::B::C, A::B::C) -> (none)
    // example: f(A::B::C, A::B)    -> super::
    // example: f(A::B::C, A::B::D) -> super::D
    // example: f(A::B::C, A)       -> super::super::
    // example: f(A::B::C, D)       -> super::super::super::D
    // example: f(A::B::C, D::E)    -> super::super::super::D::E
    // example: f(A, D::E)          -> super::D::E
    // does not include leaf object (typically a struct type).

    size_t i = 0;
    std::stringstream stream;

    auto s = src->components.begin();
    auto d = dst->components.begin();
    for (;;) {
      if (s == src->components.end()) { break; }
      if (d == dst->components.end()) { break; }
      if (*s != *d) { break; }
      ++s;
      ++d;
      ++i;
    }

    for (; s != src->components.end(); ++s) { stream << "super::"; }
    for (; d != dst->components.end(); ++d) {
      stream << MakeSnakeCase(*d) + "::";
    }
    return stream.str();
  }

  // Generate a comment from the schema.
  void GenComment(const std::vector<std::string> &comments) {
    for (auto comment_it = comments.begin(); comment_it != comments.end();
         comment_it++) {
      code_ += "///" + *comment_it;
    }
  }

  // Return a Rust type from the table in idl.h.
  std::string GetTypeBasic(const Type &type) const {
    FLATBUFFERS_ASSERT(IsScalar(type.base_type) && "incorrect type given");

    // clang-format off
    static const char * const ctypename[] = {
    #define FLATBUFFERS_TD(ENUM, IDLTYPE, CTYPE, JTYPE, GTYPE, NTYPE, PTYPE, \
                           RTYPE, ...) \
      #RTYPE,
      FLATBUFFERS_GEN_TYPES(FLATBUFFERS_TD)
    #undef FLATBUFFERS_TD
    };
    // clang-format on

    return ctypename[type.base_type];
  }

  // Return a Rust type for builders of structs
  std::string GetTypeBuilder(const Type &type) const {
    if (IsStruct(type)) {
      return WrapInNameSpace(*type.struct_def);
    } else if (IsTable(type)) {
      return "Offset<" + WrapInNameSpace(*type.struct_def) + ">";
    } else if (IsUnion(type)) {
      return WrapInNameSpace(*type.enum_def);
    } else if (IsEnum(type)) {
      return WrapInNameSpace(*type.enum_def);
    } else if (IsVector(type)) {
      return "Offset<[" + GetTypeBuilder(type.VectorType()) + "]>";
    } else if (IsString(type)) {
      return "Offset<str>";
    } else {
      FLATBUFFERS_ASSERT(IsScalar(type.base_type) && "Unsupported type");
      return GetTypeBasic(type);
    }
  }

  // Return a Rust type for struct accessors
  std::string GetTypeAccessor(const Type &type) const {
    if (IsStruct(type) || IsTable(type)) {
      return WrapInNameSpace(*type.struct_def) + "View<'a>";
    } else if (IsUnion(type)) {
      return WrapInNameSpace(*type.enum_def) + "View<'a>";
    } else if (IsEnum(type)) {
      return WrapInNameSpace(*type.enum_def);
    } else if (IsVector(type)) {
      return "Vector<'a, " + GetTypeAccessor(type.VectorType()) + ">";
    } else if (IsString(type)) {
      return "Str<'a>";
    } else {
      FLATBUFFERS_ASSERT(IsScalar(type.base_type) && "Unsupported type");
      return GetTypeBasic(type);
    }
  }

  std::string GetDefaultBuilderValue(const FieldDef &field) {
    const auto &type = field.value.type;
    if (IsStruct(type) || IsTable(type) || IsUnion(type) || IsVector(type) ||
        IsString(type)) {
      return "None";
    } else if (IsEnum(type)) {
      auto ev = type.enum_def->FindByValue(field.value.constant);
      assert(ev);
      return WrapInNameSpace(type.enum_def->defined_namespace,
                             CamelName(*type.enum_def) + "::" + CamelName(*ev));
    } else if (type.base_type == BASE_TYPE_BOOL) {
      return field.value.constant == "0" ? "false" : "true";
    } else {
      FLATBUFFERS_ASSERT(IsScalar(type.base_type) && "Unsupported type");
      return field.value.constant;
    }
  }

  bool TableFieldReturnsOption(const Type &type) {
    if (IsEnum(type)) {
      return false;
    } else if (IsUnion(type)) {
      return true;
    } else if (IsScalar(type.base_type)) {
      return false;
    } else {
      return true;
    }
  }

  void GenModule(ModuleTree &tree) {
    cur_name_space_ = tree.ns_;

    if (tree.enums_.size() > 0 || tree.structs_.size() > 0) {
      code_ +=
          "#![allow(unused_imports, irrefutable_let_patterns, dead_code, "
          "unused_mut)]";
      code_ += "use flatbuffers::{";
      code_ +=
          "    deserialize::{FromStructField, FromTableField, "
          "FromTableFieldUnion, Str, Table, Vector},";
      code_ +=
          "    errors::{InvalidFlatbuffer, OutOfBufferSpace, "
          "TryFromEnumError},";
      code_ += "    serialize::{";
      code_ +=
          "        builder::FlatbufferWriter, FlatbufferPrimitive, "
          "FlatbufferTable, Offset, RawOffset,";
      code_ += "    },";
      code_ += "};";
      code_ += "use core::{";
      code_ += "    convert::{TryFrom, TryInto},";
      code_ += "    fmt, ptr,";
      code_ += "};";
    }

    for (auto child_it = tree.children_.begin();
         child_it != tree.children_.end(); ++child_it) {
      code_ += "pub mod " + MakeSnakeCase(child_it->first) + " {";
      code_.IncrementIdentLevel();
      GenModule(child_it->second);
      code_.DecrementIdentLevel();
      code_ += "}";
    }

    for (auto enum_it = tree.enums_.begin(); enum_it != tree.enums_.end();
         ++enum_it) {
      EnumDef &value = **enum_it;
      code_ += "";
      if (value.is_union) {
        GenUnion(value);
      } else {
        GenEnum(value);
      }
    }

    for (auto struct_it = tree.structs_.begin();
         struct_it != tree.structs_.end(); ++struct_it) {
      StructDef &value = **struct_it;
      code_ += "";
      if (value.fixed) {
        GenStruct(value);
      } else {
        GenTable(value);
      }
    }
  }

  void GenEnum(EnumDef &enum_def) {
    code_.SetValue("ENUM_NAME", CamelName(enum_def));
    code_.SetValue("ENUM_NAME_ORIG", enum_def.name);
    code_.SetValue("BASE_TYPE", GetTypeBasic(enum_def.underlying_type));
    code_.SetValue("SIZE",
                   NumToString(SizeOf(enum_def.underlying_type.base_type)));

    code_ += "/// Enum for the flatbuffer `{{ENUM_NAME_ORIG}}` enum";
    code_ += "///";
    GenComment(enum_def.doc_comment);
    code_ +=
        "#[derive(Copy, Clone, Debug, PartialEq, Eq, PartialOrd, Ord, Hash)]";
    code_ += "pub enum {{ENUM_NAME}} {";
    code_.IncrementIdentLevel();
    for (auto it = enum_def.Vals().begin(); it != enum_def.Vals().end(); ++it) {
      const auto &ev = **it;

      GenComment(ev.doc_comment);
      code_.SetValue("KEY", CamelName(ev));
      code_.SetValue("VALUE", enum_def.ToString(ev));
      code_ += "{{KEY}} = {{VALUE}},";
    }
    code_.DecrementIdentLevel();
    code_ += "}";
    code_ += "";
    code_ += "impl From<{{ENUM_NAME}}> for {{BASE_TYPE}} {";
    code_ += "    #[inline]";
    code_ += "    fn from(value: {{ENUM_NAME}}) -> {{BASE_TYPE}} {";
    code_ += "        value as {{BASE_TYPE}}";
    code_ += "    }";
    code_ += "}";
    code_ += "";
    code_ += "impl TryFrom<{{BASE_TYPE}}> for {{ENUM_NAME}} {";
    code_ += "    type Error = TryFromEnumError;";
    code_ += "    #[inline]";
    code_ +=
        "    fn try_from(value: {{BASE_TYPE}}) -> Result<{{ENUM_NAME}}, "
        "TryFromEnumError> {";
    code_ += "        match value {";
    for (auto it = enum_def.Vals().begin(); it != enum_def.Vals().end(); ++it) {
      const auto &ev = **it;

      code_.SetValue("KEY", CamelName(ev));
      code_.SetValue("VALUE", enum_def.ToString(ev));
      code_ += "            {{VALUE}} => Ok({{ENUM_NAME}}::{{KEY}}),";
    }
    code_ += "            _ => Err(TryFromEnumError),";
    code_ += "        }";
    code_ += "    }";
    code_ += "}";
    code_ += "";
    code_ += "#[doc(hidden)]";
    code_ += "impl<'a> FromTableField<'a> for {{ENUM_NAME}} {";
    code_ += "    const INLINE_SIZE: usize = {{SIZE}};";
    code_ += "";
    code_ += "    #[inline]";
    code_ +=
        "    fn from_field(buf: &'a [u8], offset: usize) -> "
        "Result<{{ENUM_NAME}}, InvalidFlatbuffer> {";
    code_ += "        {{BASE_TYPE}}::from_field(buf, offset)?";
    code_ += "            .try_into()";
    code_ += "            .or(Err(InvalidFlatbuffer))";
    code_ += "    }";
    code_ += "}";
    code_ += "";
    code_ += "#[doc(hidden)]";
    code_ += "unsafe impl<'a> FromStructField<'a> for {{ENUM_NAME}} {";
    code_ += "    type Input = [u8; {{SIZE}}];";
    code_ += "";
    code_ += "    #[inline]";
    code_ +=
        "    fn from_struct_field(buf: &'a Self::Input) -> "
        "Result<{{ENUM_NAME}}, InvalidFlatbuffer> {";
    code_ += "        {{BASE_TYPE}}::from_struct_field(buf)?";
    code_ += "            .try_into()";
    code_ += "            .or(Err(InvalidFlatbuffer))";
    code_ += "    }";
    code_ += "}";
    code_ += "";
    code_ += "unsafe impl FlatbufferPrimitive for {{ENUM_NAME}} {";
    code_ += "    const SIZE: usize = {{SIZE}};";
    code_ += "    const ALIGNMENT: usize = {{SIZE}};";
    code_ += "";
    code_ += "    #[inline]";
    code_ +=
        "    unsafe fn serialize(&self, buffer: *mut u8, offset: RawOffset) "
        "{";
    code_ +=
        "        FlatbufferPrimitive::serialize(&(*self as {{BASE_TYPE}}), "
        "buffer, offset)";
    code_ += "    }";
    code_ += "}";
  }

  void GenUnion(EnumDef &union_def) {
    code_.SetValue("TAG_NAME", CamelName(union_def) + "Tag");
    code_.SetValue("VIEW_NAME", CamelName(union_def) + "View");
    code_.SetValue("SERIALIZE_NAME", CamelName(union_def));
    code_.SetValue("UNION_NAME_ORIG", union_def.name);
    code_.SetValue("BASE_TYPE", GetTypeBasic(union_def.underlying_type));
    code_.SetValue("SIZE",
                   NumToString(SizeOf(union_def.underlying_type.base_type)));

    code_ += "#[derive(Copy, Clone)]";
    code_ += "enum {{TAG_NAME}} {";
    code_.IncrementIdentLevel();
    for (auto it = union_def.Vals().begin(); it != union_def.Vals().end();
         ++it) {
      const auto &ev = **it;

      code_.SetValue("KEY", CamelName(ev));
      code_.SetValue("VALUE", union_def.ToString(ev));
      code_ += "{{KEY}} = {{VALUE}},";
    }
    code_.DecrementIdentLevel();
    code_ += "}";
    code_ += "";
    code_ += "impl<'a> FromTableField<'a> for {{TAG_NAME}} {";
    code_ += "    const INLINE_SIZE: usize = {{SIZE}};";
    code_ += "";
    code_ += "    #[inline]";
    code_ +=
        "    fn from_field(buf: &'a [u8], offset: usize) -> "
        "Result<{{TAG_NAME}}, InvalidFlatbuffer> {";
    code_ += "        match {{BASE_TYPE}}::from_field(buf, offset)? {";
    for (auto it = union_def.Vals().begin(); it != union_def.Vals().end();
         ++it) {
      const auto &ev = **it;

      if (ev.union_type.base_type == BASE_TYPE_NONE) { continue; }

      code_.SetValue("KEY", CamelName(ev));
      code_.SetValue("VALUE", union_def.ToString(ev));
      code_ += "            {{VALUE}} => Ok({{TAG_NAME}}::{{KEY}}),";
    }
    code_ += "            _ => Ok({{TAG_NAME}}::NONE),";
    code_ += "        }";
    code_ += "    }";
    code_ += "}";
    code_ += "";
    code_ += "unsafe impl FlatbufferPrimitive for {{TAG_NAME}} {";
    code_ += "    const SIZE: usize = {{SIZE}};";
    code_ += "    const ALIGNMENT: usize = {{SIZE}};";
    code_ += "";
    code_ += "    #[inline]";
    code_ +=
        "    unsafe fn serialize(&self, buffer: *mut u8, offset: RawOffset) "
        "{";
    code_ +=
        "        FlatbufferPrimitive::serialize(&(*self as {{BASE_TYPE}}), "
        "buffer, offset)";
    code_ += "    }";
    code_ += "}";
    code_ += "";
    code_ += "/// View of a flatbuffer `{{UNION_NAME_ORIG}}` object";
    code_ += "///";
    code_ +=
        "/// This enum is used for deserializing. For serializing see [`"
        "{{SERIALIZE_NAME}}`](enum.{{SERIALIZE_NAME}}.html).";
    code_ += "///";
    GenComment(union_def.doc_comment);
    code_ += "#[derive(Copy, Clone, Debug)]";
    code_ += "pub enum {{VIEW_NAME}}<'a> {";
    code_.IncrementIdentLevel();
    for (auto it = union_def.Vals().begin(); it != union_def.Vals().end();
         ++it) {
      const auto &ev = **it;

      if (ev.union_type.base_type == BASE_TYPE_NONE) { continue; }

      auto table_type =
          WrapInNameSpace(ev.union_type.struct_def->defined_namespace,
                          ev.union_type.struct_def->name) +
          "View<'a>";
      code_.SetValue("NAME", CamelName(ev));
      code_.SetValue("TABLE_TYPE", table_type);
      GenComment(ev.doc_comment);
      code_ += "{{NAME}}({{TABLE_TYPE}}),";
    }
    code_.DecrementIdentLevel();
    code_ += "}";
    code_ += "";
    code_ += "#[doc(hidden)]";
    code_ += "impl<'a> FromTableFieldUnion<'a> for {{VIEW_NAME}}<'a> {";
    code_ += "    #[inline]";
    code_ += "    fn from_field_union(";
    code_ += "        buf: &'a [u8],";
    code_ += "        tag_offset: usize,";
    code_ += "        value_offset: usize,";
    code_ += "    ) -> Result<Option<Self>, InvalidFlatbuffer> {";
    code_ += "        match {{TAG_NAME}}::from_field(buf, tag_offset)? {";
    for (auto it = union_def.Vals().begin(); it != union_def.Vals().end();
         ++it) {
      const auto &ev = **it;
      code_.SetValue("NAME", CamelName(ev));

      if (ev.union_type.base_type == BASE_TYPE_NONE) {
        code_ += "            {{TAG_NAME}}::{{NAME}} => Ok(None),";
      } else {
        code_ +=
            "            {{TAG_NAME}}::{{NAME}} => "
            "Ok(Some({{VIEW_NAME}}::{{NAME}}(FromTableField::from_field(";
        code_ += "                buf,";
        code_ += "                value_offset,";
        code_ += "            )?))),";
      }
    }
    code_ += "        }";
    code_ += "    }";
    code_ += "}";
    code_ += "";
    code_ += "impl<'a> {{VIEW_NAME}}<'a> {";
    for (auto it = union_def.Vals().begin(); it != union_def.Vals().end();
         ++it) {
      const auto &ev = **it;

      if (ev.union_type.base_type == BASE_TYPE_NONE) { continue; }

      auto table_type =
          WrapInNameSpace(ev.union_type.struct_def->defined_namespace,
                          ev.union_type.struct_def->name) +
          "View<'a>";
      code_.SetValue("NAME", CamelName(ev));
      code_.SetValue("FUNCTION_NAME", "as_" + SnakeName(ev));
      code_.SetValue("TABLE_TYPE", table_type);

      code_ += "    #[inline]";
      code_ += "    pub fn {{FUNCTION_NAME}}(self) -> Option<{{TABLE_TYPE}}> {";
      code_ += "        if let {{VIEW_NAME}}::{{NAME}}(inner) = self {";
      code_ += "            Some(inner)";
      code_ += "        } else {";
      code_ += "            None";
      code_ += "        }";
      code_ += "    }";
      code_ += "";
    }

    code_ += "}";
    code_ += "";
    code_ += "/// Builder for a flatbuffer `{{UNION_NAME_ORIG}}` object";
    code_ += "///";
    code_ +=
        "/// This enum is used for serializing. For deserializing see [`"
        "{{VIEW_NAME}}`](enum.{{VIEW_NAME}}.html).";
    code_ += "///";
    GenComment(union_def.doc_comment);
    code_ += "#[derive(Copy, Clone, Debug, PartialEq)]";
    code_ += "pub enum {{SERIALIZE_NAME}} {";
    code_.IncrementIdentLevel();
    for (auto it = union_def.Vals().begin(); it != union_def.Vals().end();
         ++it) {
      const auto &ev = **it;

      if (ev.union_type.base_type == BASE_TYPE_NONE) { continue; }

      auto table_type =
          "Offset<" +
          WrapInNameSpace(ev.union_type.struct_def->defined_namespace,
                          ev.union_type.struct_def->name) +
          ">";
      code_.SetValue("NAME", CamelName(ev));
      code_.SetValue("TABLE_TYPE", table_type);
      GenComment(ev.doc_comment);
      code_ += "{{NAME}}({{TABLE_TYPE}}),";
    }
    code_.DecrementIdentLevel();
    code_ += "}";
    code_ += "";
    code_ += "impl {{SERIALIZE_NAME}} {";
    code_ += "    #[inline]";
    code_ += "    fn tag(&self) -> {{TAG_NAME}} {";
    code_ += "        match self {";
    for (auto it = union_def.Vals().begin(); it != union_def.Vals().end();
         ++it) {
      const auto &ev = **it;

      if (ev.union_type.base_type == BASE_TYPE_NONE) { continue; }
      code_.SetValue("NAME", CamelName(ev));

      code_ +=
          "            {{SERIALIZE_NAME}}::{{NAME}}(_) => "
          "{{TAG_NAME}}::{{NAME}},";
    }
    code_ += "        }";
    code_ += "    }";
    code_ += "";
    code_ += "    #[inline]";
    code_ += "    fn offset(&self) -> RawOffset {";
    code_ += "        match self {";
    for (auto it = union_def.Vals().begin(); it != union_def.Vals().end();
         ++it) {
      const auto &ev = **it;

      if (ev.union_type.base_type == BASE_TYPE_NONE) { continue; }
      code_.SetValue("NAME", CamelName(ev));

      code_ +=
          "            {{SERIALIZE_NAME}}::{{NAME}}(offset) => "
          "offset.raw_offset(),";
    }
    code_ += "        }";
    code_ += "    }";
    code_ += "}";
  }

  void GenStruct(StructDef &struct_def) {
    code_.SetValue("ALIGN", NumToString(struct_def.minalign));
    code_.SetValue("SIZE", NumToString(struct_def.bytesize));
    code_.SetValue("VIEW_NAME", CamelName(struct_def) + "View");
    code_.SetValue("SERIALIZE_NAME", CamelName(struct_def));
    code_.SetValue("STRUCT_NAME_ORIG", struct_def.name);

    code_ += "/// View of a flatbuffer `{{STRUCT_NAME_ORIG}}` object";
    code_ += "///";
    code_ +=
        "/// This struct is used for deserializing. For serializing see "
        "[`{{SERIALIZE_NAME}}`](struct.{{SERIALIZE_NAME}}.html).";
    code_ += "///";
    GenComment(struct_def.doc_comment);
    code_ += "#[derive(Copy, Clone)]";
    code_ += "pub struct {{VIEW_NAME}}<'a> {";
    code_ += "    slice: &'a [u8; {{SIZE}}],";
    code_ += "}";
    code_ += "";
    code_ += "#[doc(hidden)]";
    code_ += "impl<'a> FromTableField<'a> for {{VIEW_NAME}}<'a> {";
    code_ += "    const INLINE_SIZE: usize = {{SIZE}};";
    code_ += "";
    code_ += "    #[inline]";
    code_ +=
        "    fn from_field(buf: &'a [u8], offset: usize) -> Result<Self, "
        "InvalidFlatbuffer> {";
    code_ +=
        "        let slice = buf.get(offset..offset + "
        "{{SIZE}}).ok_or(InvalidFlatbuffer)?;";
    code_ += "        Ok(Self {";
    code_ +=
        "            slice: unsafe { &*(slice.as_ptr() as *const [u8; "
        "{{SIZE}}]) },";
    code_ += "        })";
    code_ += "    }";
    code_ += "}";
    code_ += "";
    code_ += "#[doc(hidden)]";
    code_ += "unsafe impl<'a> FromStructField<'a> for {{VIEW_NAME}}<'a> {";
    code_ += "    type Input = [u8; {{SIZE}}];";
    code_ += "";
    code_ += "    #[inline]";
    code_ +=
        "    fn from_struct_field(slice: &'a Self::Input) -> Result<Self, "
        "InvalidFlatbuffer> {";
    code_ += "        Ok(Self { slice })";
    code_ += "    }";
    code_ += "}";
    code_ += "";
    code_ += "impl<'a> {{VIEW_NAME}}<'a> {";
    code_.IncrementIdentLevel();
    for (auto it = struct_def.fields.vec.begin();
         it != struct_def.fields.vec.end(); ++it) {
      const auto &field = **it;
      code_.SetValue("FIELD_NAME", SnakeName(field));
      code_.SetValue("FIELD_NAME_ORIG", field.name);
      code_.SetValue("FIELD_TYPE", GetTypeAccessor(field.value.type));
      code_.SetValue("OFFSET", NumToString(field.value.offset));
      code_ += "/// Getter for the `{{FIELD_NAME_ORIG}}` field.";
      code_ += "///";
      GenComment(field.doc_comment);
      code_ += "///";
      code_ += "/// # Panics";
      code_ += "///";
      code_ += "/// If the value cannot be deserialized.";
      code_ += "#[inline]";
      code_ += "pub fn {{FIELD_NAME}}(self) -> {{FIELD_TYPE}} {";
      code_ += "    self.try_get_{{FIELD_NAME}}().unwrap()";
      code_ += "}";
      code_ += "";
      code_ += "/// Getter for the `{{FIELD_NAME_ORIG}}` field.";
      code_ += "///";
      GenComment(field.doc_comment);
      code_ += "#[inline]";
      code_ +=
          "pub fn try_get_{{FIELD_NAME}}(self) -> "
          "Result<{{FIELD_TYPE}}, InvalidFlatbuffer> {";
      code_ +=
          "    type SliceType<'a> = <{{FIELD_TYPE}} as "
          "FromStructField<'a>>::Input;";
      code_ += "    const OFFSET: usize = {{OFFSET}};";
      code_ += "    const SIZE: usize = core::mem::size_of::<SliceType>();";
      code_ += "    let slice = self.slice.get(OFFSET..OFFSET+SIZE).unwrap();";
      code_ +=
          "    let slice: &'a SliceType<'a> = unsafe { &*(slice.as_ptr() as "
          "*const "
          "SliceType<'a>) };";
      code_ +=
          "    <{{FIELD_TYPE}} as "
          "FromStructField<'a>>::from_struct_field(slice)";
      code_ += "}";
      code_ += "";
    }

    code_.DecrementIdentLevel();
    code_ += "}";
    code_ += "";
    code_ += "impl<'a> fmt::Debug for {{VIEW_NAME}}<'a> {";
    code_ +=
        "    fn fmt(&self, formatter: &mut fmt::Formatter<'_>) -> "
        "fmt::Result "
        "{";
    code_ += "        formatter";
    code_ += "            .debug_struct(\"{{VIEW_NAME}}\")";
    for (auto it = struct_def.fields.vec.begin();
         it != struct_def.fields.vec.end(); ++it) {
      const auto &field = **it;
      code_.SetValue("FIELD_NAME", SnakeName(field));
      code_ +=
          "            .field(\"{{FIELD_NAME}}\", "
          "&self.try_get_{{FIELD_NAME}}())";
    }
    code_ += "            .finish()";
    code_ += "    }";
    code_ += "}";
    code_ += "";
    code_ += "impl<'a> PartialEq for {{VIEW_NAME}}<'a> {";
    code_ += "    #[inline]";
    code_ +=
        "    fn eq(&self, other: &Self) -> bool "
        "{";
    code_ += "        self.slice[..] == other.slice[..]";
    code_ += "    }";
    code_ += "}";
    code_ += "";
    code_ += "/// Builder for a flatbuffer `{{STRUCT_NAME_ORIG}}` object";
    code_ += "///";
    code_ +=
        "/// This struct is used for serializing. For deserializing see "
        "[`{{VIEW_NAME}}`](struct.{{VIEW_NAME}}.html).";
    code_ += "///";
    GenComment(struct_def.doc_comment);
    code_ += "#[derive(Copy, Clone, Debug, PartialEq)]";
    code_ += "pub struct {{SERIALIZE_NAME}} {";
    code_.IncrementIdentLevel();
    for (auto it = struct_def.fields.vec.begin();
         it != struct_def.fields.vec.end(); ++it) {
      const auto &field = **it;
      code_.SetValue("FIELD_NAME", SnakeName(field));
      code_.SetValue("FIELD_TYPE", GetTypeBuilder(field.value.type));
      GenComment(field.doc_comment);
      code_ += "pub {{FIELD_NAME}}: {{FIELD_TYPE}},";
    }
    code_.DecrementIdentLevel();
    code_ += "}";
    code_ += "";
    code_ += "unsafe impl FlatbufferPrimitive for {{SERIALIZE_NAME}} {";
    code_.IncrementIdentLevel();
    code_ += "const SIZE: usize = {{SIZE}};";
    code_ += "const ALIGNMENT: usize = {{ALIGN}};";
    code_ += "";
    code_ += "#[inline]";
    code_ += "unsafe fn serialize(&self, buffer: *mut u8, offset: RawOffset) {";
    code_.IncrementIdentLevel();
    for (auto it = struct_def.fields.vec.begin();
         it != struct_def.fields.vec.end(); ++it) {
      const auto &field = **it;
      code_.SetValue("FIELD_NAME", SnakeName(field));
      code_.SetValue("FIELD_TYPE", GetTypeBuilder(field.value.type));
      code_.SetValue("OFFSET", NumToString(field.value.offset));
      code_ +=
          "FlatbufferPrimitive::serialize(&self.{{FIELD_NAME}}, "
          "buffer.add({{OFFSET}}), "
          "offset + {{OFFSET}});";
      if (field.padding) {
        code_.SetValue("PADDING", NumToString(field.padding));
        code_ +=
            "ptr::write_bytes(buffer.add({{OFFSET}} + <{{FIELD_TYPE}} as "
            "FlatbufferPrimitive>::SIZE), 0, {{PADDING}});";
      }
    }
    code_.DecrementIdentLevel();
    code_ += "}";
    code_.DecrementIdentLevel();
    code_ += "}";
  }

  void GenTable(StructDef &table_def) {
    code_.SetValue("VIEW_NAME", CamelName(table_def) + "View");
    code_.SetValue("SERIALIZE_NAME", CamelName(table_def));
    code_.SetValue("SNAKE_NAME", SnakeName(table_def));
    code_.SetValue("TABLE_NAME_ORIG", table_def.name);
    code_ += "/// View of a flatbuffer `{{TABLE_NAME_ORIG}}` object";
    code_ += "///";
    code_ +=
        "/// This struct is used for deserializing. For serializing see "
        "[`{{SERIALIZE_NAME}}`](struct.{{SERIALIZE_NAME}}.html).";
    code_ += "///";
    GenComment(table_def.doc_comment);
    code_ += "#[derive(Copy, Clone)]";
    code_ += "pub struct {{VIEW_NAME}}<'a> {";
    code_ += "    table: Table<'a>,";
    code_ += "}";
    code_ += "";
    code_ += "#[doc(hidden)]";
    code_ += "impl<'a> FromTableField<'a> for {{VIEW_NAME}}<'a> {";
    code_ += "    const INLINE_SIZE: usize = Table::INLINE_SIZE;";
    code_ += "";
    code_ += "    #[inline]";
    code_ +=
        "    fn from_field(buf: &'a [u8], offset: usize) -> Result<Self, "
        "InvalidFlatbuffer> {";
    code_ +=
        "        FromTableField::from_field(buf, offset).map(|table| "
        "Self { table })";
    code_ += "    }";
    code_ += "}";
    code_ += "";

    std::map<std::string, FieldDef *> union_keys;
    for (auto it = table_def.fields.vec.begin();
         it != table_def.fields.vec.end(); ++it) {
      const auto &field = **it;

      if (IsUnion(field.value.type) &&
          field.value.type.base_type != BASE_TYPE_UNION) {
        union_keys[field.name] = *it;
      }
    }

    code_ += "impl<'a> {{VIEW_NAME}}<'a> {";
    code_.IncrementIdentLevel();
    for (auto it = table_def.fields.vec.begin();
         it != table_def.fields.vec.end(); ++it) {
      const auto &field = **it;

      if (field.deprecated) { continue; }

      if (IsUnion(field.value.type) &&
          field.value.type.base_type != BASE_TYPE_UNION) {
        continue;
      }

      code_.SetValue("FIELD_NAME", SnakeName(field));
      code_.SetValue("FIELD_NAME_ORIG", field.name);
      code_.SetValue("FIELD_TYPE", GetTypeAccessor(field.value.type));
      code_.SetValue("OFFSET", NumToString(field.value.offset));

      code_ += "/// Getter for the `{{FIELD_NAME_ORIG}}` field.";
      code_ += "///";
      GenComment(field.doc_comment);
      code_ += "///";
      code_ += "/// # Panics";
      code_ += "///";
      if (field.required) {
        code_ += "/// If the value cannot be deserialized or is not present.";
        code_ += "#[inline]";
        code_ += "pub fn {{FIELD_NAME}}(&self) -> {{FIELD_TYPE}} {";
      } else if (TableFieldReturnsOption(field.value.type)) {
        code_ += "/// If the value cannot be deserialized.";
        code_ += "#[inline]";
        code_ += "pub fn {{FIELD_NAME}}(&self) -> Option<{{FIELD_TYPE}}> {";
      } else {
        code_ += "/// If the value cannot be deserialized.";
        code_ += "#[inline]";
        code_ += "pub fn {{FIELD_NAME}}(&self) -> {{FIELD_TYPE}} {";
      }
      code_ += "    self.try_get_{{FIELD_NAME}}().unwrap()";
      code_ += "}";
      code_ += "";

      code_ += "/// Getter for the `{{FIELD_NAME_ORIG}}` field.";
      code_ += "///";
      GenComment(field.doc_comment);
      code_ += "#[inline]";
      if (field.required) {
        FLATBUFFERS_ASSERT(TableFieldReturnsOption(field.value.type));
        code_ +=
            "pub fn try_get_{{FIELD_NAME}}(&self) -> Result<{{FIELD_TYPE}}, "
            "InvalidFlatbuffer> {";
        if (field.value.type.base_type == BASE_TYPE_UNION) {
          FieldDef *field_key = union_keys[field.name + "_type"];
          code_.SetValue("TYPE_OFFSET", NumToString(field_key->value.offset));
          code_ +=
              "    self.table.get_field_union({{TYPE_OFFSET}}, "
              "{{OFFSET}}).and_then(|value| "
              "value.ok_or(InvalidFlatbuffer))";
        } else {
          code_ +=
              "    self.table.get_field({{OFFSET}}).and_then(|value| "
              "value.ok_or(InvalidFlatbuffer))";
        }
      } else if (TableFieldReturnsOption(field.value.type)) {
        code_ +=
            "pub fn try_get_{{FIELD_NAME}}(&self) ->  "
            "Result<Option<{{FIELD_TYPE}}>, "
            "InvalidFlatbuffer> {";
        if (field.value.type.base_type == BASE_TYPE_UNION) {
          FieldDef *field_key = union_keys[field.name + "_type"];
          code_.SetValue("TYPE_OFFSET", NumToString(field_key->value.offset));
          code_ +=
              "    self.table.get_field_union({{TYPE_OFFSET}}, {{OFFSET}})";
        } else {
          code_ += "    self.table.get_field({{OFFSET}})";
        }
      } else {
        code_.SetValue("DEFAULT", GetDefaultBuilderValue(field));
        code_ +=
            "pub fn try_get_{{FIELD_NAME}}(&self) -> Result<{{FIELD_TYPE}}, "
            "InvalidFlatbuffer> {";
        code_ +=
            "    self.table.get_field({{OFFSET}}).map(|value| "
            "value.unwrap_or({{DEFAULT}}))";
      }
      code_ += "}";
      code_ += "";
    }
    code_ +=
        "/// Begin parsing a flatbuffer with a `Weapon` as the root object";
    code_ += "#[inline]";
    code_ +=
        "pub fn from_buffer_as_root(buffer: &'a [u8]) -> Result<Self, "
        "InvalidFlatbuffer> {";
    code_ += "    FromTableField::from_field(buffer, 0)";
    code_ += "}";
    code_.DecrementIdentLevel();
    code_ += "}";
    code_ += "";

    if (parser_.root_struct_def_ == &table_def) {
      code_ += "#[inline]";
      code_ +=
          "pub fn get_root_as_{{SNAKE_NAME}}<'a>(buffer: &'a [u8]) -> "
          "Result<{{VIEW_NAME}}<'a>, InvalidFlatbuffer> {";
      code_ += "    {{VIEW_NAME}}::from_buffer_as_root(buffer)";
      code_ += "}";
      code_ += "";
    }

    code_ += "impl<'a> fmt::Debug for {{VIEW_NAME}}<'a> {";
    code_.IncrementIdentLevel();
    code_ +=
        "fn fmt(&self, formatter: &mut fmt::Formatter<'_>) -> fmt::Result {";
    code_.IncrementIdentLevel();
    code_ += "formatter.debug_struct(\"{{VIEW_NAME}}\")";
    code_.IncrementIdentLevel();
    for (auto it = table_def.fields.vec.begin();
         it != table_def.fields.vec.end(); ++it) {
      const auto &field = **it;

      if (field.deprecated) { continue; }

      if (IsUnion(field.value.type) &&
          field.value.type.base_type != BASE_TYPE_UNION) {
        continue;
      }
      code_.SetValue("FIELD_NAME", SnakeName(field));
      code_ += ".field(\"{{FIELD_NAME}}\", &self.try_get_{{FIELD_NAME}}())";
    }

    code_ += ".finish()";
    code_.DecrementIdentLevel();
    code_.DecrementIdentLevel();
    code_ += "}";
    code_.DecrementIdentLevel();
    code_ += "}";
    code_ += "";
    code_ += "/// Builder for a flatbuffer `{{TABLE_NAME_ORIG}}` object";
    code_ += "///";
    code_ +=
        "/// This struct is used for serializing. For deserializing see "
        "[`{{VIEW_NAME}}`](struct.{{VIEW_NAME}}.html).";
    code_ += "///";
    GenComment(table_def.doc_comment);
    code_ += "#[derive(Copy, Clone, Debug, PartialEq)]";
    code_ += "pub struct {{SERIALIZE_NAME}} {";
    for (auto it = table_def.fields.vec.begin();
         it != table_def.fields.vec.end(); ++it) {
      const auto &field = **it;

      if (field.deprecated) { continue; }

      if (IsUnion(field.value.type) &&
          field.value.type.base_type != BASE_TYPE_UNION) {
        continue;
      }

      code_.SetValue("FIELD_NAME", SnakeName(field));
      code_.SetValue("FIELD_TYPE", GetTypeBuilder(field.value.type));
      if (TableFieldReturnsOption(field.value.type)) {
        code_ += "    pub {{FIELD_NAME}}: Option<{{FIELD_TYPE}}>,";
      } else {
        code_ += "    pub {{FIELD_NAME}}: {{FIELD_TYPE}},";
      }
    }
    code_ += "}";
    code_ += "";
    code_ += "impl Default for {{SERIALIZE_NAME}} {";
    code_ += "    #[inline]";
    code_ += "    fn default() -> {{SERIALIZE_NAME}} {";
    code_ += "        {{SERIALIZE_NAME}} {";
    for (auto it = table_def.fields.vec.begin();
         it != table_def.fields.vec.end(); ++it) {
      const auto &field = **it;

      if (field.deprecated) { continue; }

      if (IsUnion(field.value.type) &&
          field.value.type.base_type != BASE_TYPE_UNION) {
        continue;
      }
      code_.SetValue("FIELD_NAME", SnakeName(field));
      code_.SetValue("DEFAULT_VALUE", GetDefaultBuilderValue(field));
      code_ += "            {{FIELD_NAME}}: {{DEFAULT_VALUE}},";
    }
    code_ += "        }";
    code_ += "    }";
    code_ += "}";
    code_ += "";
    code_ += "unsafe impl FlatbufferTable for {{SERIALIZE_NAME}} {";
    code_.IncrementIdentLevel();
    code_ += "#[inline]";
    code_ += "fn validate_required(&self) {";
    code_.IncrementIdentLevel();

    voffset_t max_offset = 4;
    size_t max_align = 1;
    for (auto it = table_def.fields.vec.begin();
         it != table_def.fields.vec.end(); ++it) {
      const auto &field = **it;

      if (field.deprecated) { continue; }

      if (field.value.offset > max_offset) { max_offset = field.value.offset; }
      if (InlineAlignment(field.value.type) > max_align) {
        max_align = InlineAlignment(field.value.type);
      }

      if (IsUnion(field.value.type) &&
          field.value.type.base_type != BASE_TYPE_UNION) {
        continue;
      }

      if (field.required) {
        code_.SetValue("FIELD_NAME", SnakeName(field));
        code_ += "if self.{{FIELD_NAME}}.is_none() {";
        code_ +=
            "    panic!(\"Missing field '{{FIELD_NAME}}' while serializing "
            "{{SERIALIZE_NAME}}\");";
        code_ += "}";
      }
    }
    code_.DecrementIdentLevel();
    code_ += "}";
    code_ += "";
    code_ += "#[inline]";
    code_ += "fn serialize<F: FlatbufferWriter>(";
    code_ += "    &self,";
    code_ += "    flatbuffer: &mut F,";
    code_ += ") -> Result<RawOffset, OutOfBufferSpace> {";
    code_.IncrementIdentLevel();
    code_.SetValue("VTABLE_SIZE", NumToString(max_offset + 2));
    code_ += "let mut vtable = [0u8; {{VTABLE_SIZE}}];";
    code_ += "";
    code_ += "let mut size = 0;";
    code_ += "let mut alignment = 4;";
    code_ += "let mut vtable_len = 4;";
    code_ += "";

    for (size_t align = table_def.sortbysize ? max_align : 1; align;
         align /= 2) {
      for (auto it = table_def.fields.vec.begin();
           it != table_def.fields.vec.end(); ++it) {
        const auto &field = **it;

        if (field.deprecated) { continue; }

        if (table_def.sortbysize &&
            InlineAlignment(field.value.type) != align) {
          continue;
        }

        if (IsUnion(field.value.type)) {
          if (field.value.type.base_type != BASE_TYPE_UNION) {
            std::string name = SnakeName(field);
            name.resize(name.size() - 5);  // remove _type
            code_.SetValue("FIELD_NAME", name);
            code_.SetValue("FIELD_TYPE",
                           WrapInNameSpace(*field.value.type.enum_def) + "Tag");
          } else {
            code_.SetValue("FIELD_NAME", SnakeName(field));
            code_.SetValue("FIELD_TYPE", "Offset<()>");
          }
        } else {
          code_.SetValue("FIELD_NAME", SnakeName(field));
          code_.SetValue("FIELD_TYPE", GetTypeBuilder(field.value.type));
        }
        code_.SetValue("FIELD_NAME_ORIG", SnakeName(field));

        code_.SetValue("DEFAULT_VALUE", GetDefaultBuilderValue(field));
        code_.SetValue("OFFSET", NumToString(field.value.offset));
        if (!table_def.sortbysize) {
          code_ += "let mut {{FIELD_NAME_ORIG}}_pad = 0;";
        }
        code_ += "if self.{{FIELD_NAME}} != {{DEFAULT_VALUE}} {";
        code_ += "    vtable_len = vtable_len.max({{OFFSET}} + 2);";
        code_ +=
            "    const CUR_ALIGN: usize = <{{FIELD_TYPE}} as "
            "FlatbufferPrimitive>::ALIGNMENT;";
        code_ += "    alignment = alignment.max(CUR_ALIGN);";
        if (!table_def.sortbysize) {
          code_ +=
              "    let new_size = (size + CUR_ALIGN - 1) & !(CUR_ALIGN - "
              "1);";
          code_ += "    {{FIELD_NAME_ORIG}}_pad = new_size - size;";
          code_ += "    size = new_size;";
        }
        code_ += "    vtable[{{OFFSET}}..{{OFFSET}}+2].copy_from_slice(";
        code_ += "        &((size + 4) as u16).to_le_bytes());";
        code_ += "    size += <{{FIELD_TYPE}} as FlatbufferPrimitive>::SIZE;";
        code_ += "}";
      }
    }
    code_ += "flatbuffer.align_before_write(size, alignment - 1)?;";

    for (size_t align = 1; align <= (table_def.sortbysize ? max_align : 1);
         align *= 2) {
      for (auto it = table_def.fields.vec.rbegin();
           it != table_def.fields.vec.rend(); ++it) {
        const auto &field = **it;

        if (field.deprecated) { continue; }

        if (table_def.sortbysize &&
            InlineAlignment(field.value.type) != align) {
          continue;
        }

        code_.SetValue("FIELD_NAME_ORIG", SnakeName(field));
        if (IsUnion(field.value.type)) {
          if (field.value.type.base_type != BASE_TYPE_UNION) {
            std::string name = SnakeName(field);
            name.resize(name.size() - 5);  // remove _type
            code_.SetValue("FIELD_NAME", name);
            code_.SetValue("FIELD_TYPE",
                           WrapInNameSpace(*field.value.type.enum_def) + "Tag");
            code_ += "if let Some(ref value) = self.{{FIELD_NAME}} {";
            code_ += "    flatbuffer.write_primitive(&value.tag())?;";
          } else {
            code_.SetValue("FIELD_NAME", SnakeName(field));
            code_.SetValue("FIELD_TYPE", GetTypeBuilder(field.value.type));
            code_ += "if let Some(ref value) = self.{{FIELD_NAME}} {";
            code_ += "    flatbuffer.write_primitive(&value.offset())?;";
          }
        } else {
          code_.SetValue("FIELD_NAME", SnakeName(field));
          code_.SetValue("FIELD_TYPE", GetTypeBuilder(field.value.type));
          if (TableFieldReturnsOption(field.value.type)) {
            code_ += "if let Some(ref value) = self.{{FIELD_NAME}} {";
            code_ += "    flatbuffer.write_primitive(value)?;";
          } else {
            code_.SetValue("DEFAULT_VALUE", GetDefaultBuilderValue(field));
            code_ += "if self.{{FIELD_NAME}} != {{DEFAULT_VALUE}} {";
            code_ += "    flatbuffer.write_primitive(&self.{{FIELD_NAME}})?;";
          }
        }

        if (!table_def.sortbysize) {
          code_ += "    flatbuffer.write_zeros({{FIELD_NAME_ORIG}}_pad)?;";
        }
        code_ += "}";
      }
    }

    code_ +=
        "flatbuffer.write_vtable_and_offset(&mut vtable[..vtable_len], size + "
        "4)";
    code_.DecrementIdentLevel();
    code_ += "}";
    code_.DecrementIdentLevel();
    code_ += "}";
  }
};  // namespace rust
}  // namespace rust

bool GenerateRust(const Parser &parser, const std::string &path,
                  const std::string &file_name) {
  rust::RustGenerator generator(parser, path, file_name);
  return generator.generate();
}

std::string RustMakeRule(const Parser &parser, const std::string &path,
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

// TODO(rw): Generated code should import other generated files.
// TODO(rw): Generated code should refer to namespaces in included files in a
//           way that makes them referrable.
// TODO(maxburke): There should be test schemas added that use language
//           keywords as fields of structs, tables, unions, enums, to make sure
//           that internal code generated references escaped names correctly.
