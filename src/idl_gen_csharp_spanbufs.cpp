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

#include "idl_gen_csharp_spanbufs.h"

#include <unordered_set>

#include "flatbuffers/code_generators.h"
#include "flatbuffers/flatbuffers.h"
#include "flatbuffers/idl.h"
#include "flatbuffers/util.h"

namespace flatbuffers {

static TypedFloatConstantGenerator CSharpFloatGen("Double.", "Single.", "NaN",
                                                  "PositiveInfinity",
                                                  "NegativeInfinity");
static CommentConfig comment_config = {
  nullptr,
  "///",
  nullptr,
};

namespace csharpspanbufs {

// Generation mode determines which buffer type variant to generate
enum class GenerationMode {
  // ByteBuffer code, heap allocated, growable, Span based accessors
  ByteBuffer,
  // Span code, ref structs with span buffers
  SpanBuffer
};

class CSharpSpanBufsGenerator : public BaseGenerator {
  struct FieldArrayLength {
    std::string name;
    int length;
  };

  GenerationMode gen_mode_;

 public:
  CSharpSpanBufsGenerator(const Parser &parser, const std::string &path,
                          const std::string &file_name,
                          GenerationMode mode = GenerationMode::ByteBuffer)
      : BaseGenerator(parser, path, file_name,
                      parser.opts.cs_global_alias ? "global::" : "", ".", "cs"),
        gen_mode_(mode),
        cur_name_space_(nullptr) {
    // clang-format off

    // List of keywords retrieved from here:
    // https://docs.microsoft.com/en-us/dotnet/csharp/language-reference/keywords/

    // One per line to ease comparisons to that list are easier

    static const char *const keywords[] = {
      "abstract",
      "as",
      "base",
      "bool",
      "break",
      "byte",
      "case",
      "catch",
      "char",
      "checked",
      "class",
      "const",
      "continue",
      "decimal",
      "default",
      "delegate",
      "do",
      "double",
      "else",
      "enum",
      "event",
      "explicit",
      "extern",
      "false",
      "finally",
      "fixed",
      "float",
      "for",
      "foreach",
      "goto",
      "if",
      "implicit",
      "in",
      "int",
      "interface",
      "internal",
      "is",
      "lock",
      "long",
      "namespace",
      "new",
      "null",
      "object",
      "operator",
      "out",
      "override",
      "params",
      "private",
      "protected",
      "public",
      "readonly",
      "ref",
      "return",
      "sbyte",
      "sealed",
      "short",
      "sizeof",
      "stackalloc",
      "static",
      "string",
      "struct",
      "switch",
      "this",
      "throw",
      "true",
      "try",
      "typeof",
      "uint",
      "ulong",
      "unchecked",
      "unsafe",
      "ushort",
      "using",
      "virtual",
      "void",
      "volatile",
      "while",
      nullptr,
      // clang-format on
    };

    for (auto kw = keywords; *kw; kw++) keywords_.insert(*kw);
  }

  CSharpSpanBufsGenerator &operator=(const CSharpSpanBufsGenerator &);

  bool generateForCurrentMode() {
    std::string one_file_code;
    cur_name_space_ = parser_.current_namespace_;

    if (gen_mode_ == GenerationMode::ByteBuffer) {
      for (auto it = parser_.enums_.vec.begin(); it != parser_.enums_.vec.end();
           ++it) {
        std::string enumcode;
        auto &enum_def = **it;
        if (!parser_.opts.one_file)
          cur_name_space_ = enum_def.defined_namespace;
        GenEnum(enum_def, &enumcode, parser_.opts);
        if (parser_.opts.one_file) {
          one_file_code += enumcode;
        } else {
          if (!SaveType(enum_def.name, *enum_def.defined_namespace, enumcode,
                        false, parser_.opts))
            return false;
        }
      }
    }

    for (auto it = parser_.structs_.vec.begin();
         it != parser_.structs_.vec.end(); ++it) {
      std::string declcode;
      auto &struct_def = **it;
      if (!parser_.opts.one_file)
        cur_name_space_ = struct_def.defined_namespace;
      GenStruct(struct_def, &declcode, parser_.opts);
      // Verifier is only generated for ByteBuffer mode, SpanMode version will
      // simply call it.
      if (gen_mode_ == GenerationMode::ByteBuffer) {
        GenStructVerifier(struct_def, &declcode);
      }
      if (parser_.opts.one_file) {
        one_file_code += declcode;
      } else {
        if (!SaveType(struct_def.name, *struct_def.defined_namespace, declcode,
                      true, parser_.opts))
          return false;
      }
    }

    if (parser_.opts.one_file) {
      return SaveType(file_name_, *parser_.current_namespace_, one_file_code,
                      true, parser_.opts);
    }
    return true;
  }

  bool generate() {
    gen_mode_ = GenerationMode::ByteBuffer;
    if (!generateForCurrentMode()) return false;

    gen_mode_ = GenerationMode::SpanBuffer;
    if (!generateForCurrentMode()) return false;

    return true;
  }

 private:
  std::unordered_set<std::string> keywords_;

  std::string BufferTypeName() const {
    return gen_mode_ == GenerationMode::SpanBuffer ? "ByteSpanBuffer"
                                                   : "ByteBuffer";
  }

  std::string BuilderTypeName() const {
    return gen_mode_ == GenerationMode::SpanBuffer ? "FlatSpanBufferBuilder"
                                                   : "FlatBufferBuilder";
  }

  // Returns the builder parameter declaration with "ref" for SpanBuffer mode
  // SpanBuffer uses ref struct which requires passing by ref for mutations
  std::string BuilderParam() const {
    return gen_mode_ == GenerationMode::SpanBuffer
               ? "ref FlatSpanBufferBuilder builder"
               : "FlatBufferBuilder builder";
  }

  // Returns the builder argument for method calls - "ref builder" for
  // SpanBuffer mode
  std::string BuilderArg() const {
    return gen_mode_ == GenerationMode::SpanBuffer ? "ref builder" : "builder";
  }

  // Returns "scoped " prefix for Span parameters in SpanBuffer mode
  // This is needed because ref struct builder + Span parameter combination
  // requires scoped to prevent reference escape warnings in C#
  std::string ScopedPrefix() const {
    return gen_mode_ == GenerationMode::SpanBuffer ? "scoped " : "";
  }

  std::string TableBaseTypeName() const {
    return gen_mode_ == GenerationMode::SpanBuffer ? "TableSpan" : "Table";
  }

  std::string StructBaseTypeName() const {
    return gen_mode_ == GenerationMode::SpanBuffer ? "StructSpan" : "Struct";
  }

  std::string FlatbufferObjectInterface() const {
    return gen_mode_ == GenerationMode::SpanBuffer ? "IFlatbufferSpanObject"
                                                   : "IFlatbufferObject";
  }

  const std::string spanbuf_namespace = "StackBuffer";
  // Returns the namespace suffix for the current generation mode
  std::string NamespaceSuffix() const {
    return gen_mode_ == GenerationMode::SpanBuffer ? spanbuf_namespace : "";
  }

  // Returns true if generating span-based code
  bool IsSpanMode() const { return gen_mode_ == GenerationMode::SpanBuffer; }

  // Returns the fully qualified namespaced name for a struct definition
  // using the base namespace.
  // Used for Object API types which are shared between modes.
  std::string BaseNamespacedName(const StructDef &def) const {
    std::string ns = FullNamespace(".", *def.defined_namespace);
    if (ns.empty()) { return Name(def); }
    return ns + "." + Name(def);
  }

  // Returns the fully qualified namespaced name for a struct definition
  // in the SpanBuf namespace. Used for generating SpanBuf-specific Pack
  // methods.
  std::string SpanBufNamespacedName(const StructDef &def) const {
    std::string ns = FullNamespace(".", *def.defined_namespace);
    if (ns.empty()) { return spanbuf_namespace + "." + Name(def); }
    return ns + "." + spanbuf_namespace + "." + Name(def);
  }

  // Returns the fully qualified namespaced name for an enum definition
  // using the base namespace. Enums are always in the base namespace.
  std::string BaseNamespacedEnumName(const EnumDef &def) const {
    return WrapInNameSpace(def.defined_namespace, Name(def));
  }

  std::string EscapeKeyword(const std::string &name) const {
    return keywords_.find(name) == keywords_.end() ? name : "@" + name;
  }

  std::string Name(const FieldDef &field) const {
    std::string name = ConvertCase(field.name, Case::kUpperCamel);
    return EscapeKeyword(name);
  }

  std::string Name(const Definition &def) const {
    return EscapeKeyword(def.name);
  }

  // Returns the fully qualified namespaced name for a definition.
  // For structs/tables in SpanBuffer mode, includes the .SpanBuf suffix in the
  // namespace. For enums, always uses the base namespace (enums are shared).
  std::string NamespacedName(const Definition &def) const {
    std::string ns = FullNamespace(".", *def.defined_namespace);
    if (IsSpanMode()) {
      ns = ns.empty() ? NamespaceSuffix() : ns + "." + NamespaceSuffix();
    }
    if (ns.empty()) { return Name(def); }
    return ns + "." + Name(def);
  }

  std::string NamespacedEnumName(const EnumDef &def) const {
    return WrapInNameSpace(def.defined_namespace, Name(def));
  }

  std::string Name(const EnumVal &ev) const { return EscapeKeyword(ev.name); }

  // Save out the generated code for a single class while adding
  // declaration boilerplate.
  bool SaveType(const std::string &defname, const Namespace &ns,
                const std::string &classcode, bool needs_includes,
                const IDLOptions &options) const {
    if (!classcode.length()) return true;

    std::string code =
        "// <auto-generated>\n"
        "//  " +
        std::string(FlatBuffersGeneratedWarning()) +
        "\n"
        "// </auto-generated>\n\n";

    std::string namespace_name = FullNamespace(".", ns);
    std::string namespace_suffix = NamespaceSuffix();
    std::string complete_namespace = "";
    if (!namespace_name.empty() && !namespace_suffix.empty()) {
      complete_namespace =
          "namespace " + namespace_name + "." + namespace_suffix + "\n{\n\n";
    } else if (!namespace_name.empty()) {
      complete_namespace = "namespace " + namespace_name + "\n{\n\n";
    } else if (!namespace_suffix.empty()) {
      complete_namespace = "namespace " + namespace_suffix + "\n{\n\n";
    }
    code += complete_namespace;
    if (needs_includes) {
      code += "using global::System;\n";
      code += "using global::System.Buffers;\n";
      code += "using global::System.Collections.Generic;\n";
      code += "using global::System.Runtime.InteropServices;\n";
      code += "using global::Google.FlatSpanBuffers;\n";
      code += "using global::Google.FlatSpanBuffers.Operations;\n";
      code += "using global::Google.FlatSpanBuffers.Utils;\n";
      code += "using global::Google.FlatSpanBuffers.Vectors;\n";
    }
    code += classcode;
    if (!complete_namespace.empty()) { code += "\n}\n"; }

    // For span mode, use a separate directory to avoid file conflicts
    std::string dir = NamespaceDir(ns);
    if (IsSpanMode()) {
      dir += "StackBuffer/";
      EnsureDirExists(dir);
    }
    auto filename = dir + defname;
    if (options.one_file) { filename += options.filename_suffix; }
    filename +=
        options.filename_extension.empty() ? ".cs" : options.filename_extension;
    return SaveFile(filename.c_str(), code, false);
  }

  const Namespace *CurrentNameSpace() const { return cur_name_space_; }

  std::string GenTypeBasic(const Type &type, bool enableLangOverrides) const {
    // clang-format off
    static const char * const csharp_typename[] = {
      #define FLATBUFFERS_TD(ENUM, IDLTYPE, CTYPE, JTYPE, GTYPE, NTYPE, ...) \
        #NTYPE,
        FLATBUFFERS_GEN_TYPES(FLATBUFFERS_TD)
      #undef FLATBUFFERS_TD
    };
    // clang-format on

    if (enableLangOverrides) {
      // Enums use the base namespace, they are shared across modes
      if (IsEnum(type)) return NamespacedEnumName(*type.enum_def);
      if (type.base_type == BASE_TYPE_STRUCT) {
        return "Offset<" + NamespacedName(*type.struct_def) + ">";
      }
    }

    return csharp_typename[type.base_type];
  }

  inline std::string GenTypeBasic(const Type &type) const {
    return GenTypeBasic(type, true);
  }

  std::string GenTypePointer(const Type &type) const {
    switch (type.base_type) {
      case BASE_TYPE_STRING: return "string";
      case BASE_TYPE_VECTOR: return GenTypeGet(type.VectorType());
      case BASE_TYPE_STRUCT: return NamespacedName(*type.struct_def);
      case BASE_TYPE_UNION: return "TTable";
      default: return "Table";
    }
  }

  std::string GenTypeGet(const Type &type) const {
    return IsScalar(type.base_type)
               ? GenTypeBasic(type)
               : (IsArray(type) ? GenTypeGet(type.VectorType())
                                : GenTypePointer(type));
  }

  std::string GenOffsetType(const StructDef &struct_def) const {
    return "Offset<" + NamespacedName(struct_def) + ">";
  }

  std::string GenOffsetConstruct(const StructDef &struct_def,
                                 const std::string &variable_name) const {
    return "new Offset<" + NamespacedName(struct_def) + ">(" + variable_name +
           ")";
  }

  // Casts necessary to correctly read serialized data
  std::string DestinationCast(const Type &type) const {
    if (IsSeries(type)) {
      return DestinationCast(type.VectorType());
    } else {
      // Enums use the base namespace, they are shared across modes
      if (IsEnum(type)) return "(" + NamespacedEnumName(*type.enum_def) + ")";
    }
    return "";
  }

  // Cast statements for mutator method parameters.
  // In Java, parameters representing unsigned numbers need to be cast down to
  // their respective type. For example, a long holding an unsigned int value
  // would be cast down to int before being put onto the buffer. In C#, one cast
  // directly cast an Enum to its underlying type, which is essential before
  // putting it onto the buffer.
  std::string SourceCast(const Type &type,
                         const bool isOptional = false) const {
    if (IsSeries(type)) {
      return SourceCast(type.VectorType());
    } else {
      if (IsEnum(type))
        return "(" + GenTypeBasic(type, false) + (isOptional ? "?" : "") + ")";
    }
    return "";
  }

  std::string SourceCastBasic(const Type &type, const bool isOptional) const {
    return IsScalar(type.base_type) ? SourceCast(type, isOptional) : "";
  }

  std::string GenEnumDefaultValue(const FieldDef &field) const {
    auto &value = field.value;
    FLATBUFFERS_ASSERT(value.type.enum_def);
    auto &enum_def = *value.type.enum_def;
    auto enum_val = enum_def.FindByValue(value.constant);
    // Enums use the base namespace, they are shared across modes
    return enum_val ? (NamespacedEnumName(enum_def) + "." + Name(*enum_val))
                    : value.constant;
  }

  std::string GenDefaultValue(const FieldDef &field,
                              bool enableLangOverrides) const {
    // If it is an optional scalar field, the default is null
    if (field.IsScalarOptional()) { return "null"; }

    auto &value = field.value;
    if (enableLangOverrides) {
      // handles both enum case and vector of enum case
      if (value.type.enum_def != nullptr &&
          value.type.base_type != BASE_TYPE_UNION) {
        return GenEnumDefaultValue(field);
      }
    }

    auto longSuffix = "";
    switch (value.type.base_type) {
      case BASE_TYPE_BOOL: return value.constant == "0" ? "false" : "true";
      case BASE_TYPE_ULONG: return value.constant;
      case BASE_TYPE_UINT:
      case BASE_TYPE_LONG: return value.constant + longSuffix;
      default:
        if (IsFloat(value.type.base_type))
          return CSharpFloatGen.GenFloatConstant(field);
        else
          return value.constant;
    }
  }

  std::string GenDefaultValue(const FieldDef &field) const {
    return GenDefaultValue(field, true);
  }

  std::string GenDefaultValueBasic(const FieldDef &field,
                                   bool enableLangOverrides) const {
    auto &value = field.value;
    if (!IsScalar(value.type.base_type)) {
      if (enableLangOverrides) {
        switch (value.type.base_type) {
          case BASE_TYPE_STRING: return "default(StringOffset)";
          case BASE_TYPE_STRUCT:
            return "default(Offset<" + NamespacedName(*value.type.struct_def) +
                   ">)";
          case BASE_TYPE_VECTOR: return "default(VectorOffset)";
          default: break;
        }
      }
      return "0";
    }
    return GenDefaultValue(field, enableLangOverrides);
  }

  std::string GenDefaultValueBasic(const FieldDef &field) const {
    return GenDefaultValueBasic(field, true);
  }

  void GenEnum(EnumDef &enum_def, std::string *code_ptr,
               const IDLOptions &opts) const {
    std::string &code = *code_ptr;
    if (enum_def.generated) return;

    // Generate enum definitions of the form:
    // public static (final) int name = value;
    // In Java, we use ints rather than the Enum feature, because we want them
    // to map directly to how they're used in C/C++ and file formats.
    // That, and Java Enums are expensive, and not universally liked.
    GenComment(enum_def.doc_comment, code_ptr, &comment_config);

    if (opts.cs_gen_json_serializer && opts.generate_object_based_api) {
      code +=
          "[System.Text.Json.Serialization.JsonConverter(typeof(System.Text."
          "Json.Serialization."
          "JsonStringEnumConverter))]\n";
    }
    // In C# this indicates enumeration values can be treated as bit flags.
    if (enum_def.attributes.Lookup("bit_flags")) {
      code += "[System.FlagsAttribute]\n";
    }
    if (enum_def.attributes.Lookup("private")) {
      code += "internal ";
    } else {
      code += "public ";
    }
    code += "enum " + Name(enum_def);
    code += " : " + GenTypeBasic(enum_def.underlying_type, false);
    code += "\n{\n";
    for (auto it = enum_def.Vals().begin(); it != enum_def.Vals().end(); ++it) {
      auto &ev = **it;
      GenComment(ev.doc_comment, code_ptr, &comment_config, "  ");
      code += "  ";
      code += Name(ev) + " = ";
      code += enum_def.ToString(ev);
      code += ",\n";
    }
    // Close the class
    code += "};\n\n";

    if (opts.generate_object_based_api) {
      GenEnum_ObjectAPI(enum_def, code_ptr, opts);
    }

    if (enum_def.is_union) { code += GenUnionVerify(enum_def.underlying_type); }
  }

  bool HasUnionStringValue(const EnumDef &enum_def) const {
    if (!enum_def.is_union) return false;
    for (auto it = enum_def.Vals().begin(); it != enum_def.Vals().end(); ++it) {
      auto &val = **it;
      if (IsString(val.union_type)) { return true; }
    }
    return false;
  }

  // Returns the function name that is able to read a value of the given type.
  std::string GenGetter(const Type &type) const {
    switch (type.base_type) {
      case BASE_TYPE_STRING: return "__p.__string";
      case BASE_TYPE_STRUCT: return "__p.__struct";
      case BASE_TYPE_UNION: return "__p.__union";
      case BASE_TYPE_VECTOR: return GenGetter(type.VectorType());
      case BASE_TYPE_ARRAY: return GenGetter(type.VectorType());
      default: {
        std::string getter = "__p.bb.Get<" + GenTypeBasic(type, false) + ">";
        return getter;
      }
    }
  }

  std::string GetObjectConstructor(flatbuffers::StructDef &struct_def,
                                   const std::string &data_buffer,
                                   const std::string &offset) const {
    // Use the generated type directly, to properly handle default values that
    // might not be written to the buffer.
    return "new " + Name(struct_def) + "().__assign(" + offset + ", " +
           data_buffer + ")";
  }

  // Returns the function name that is able to read a value of the given type.
  std::string GenGetterForLookupByKey(flatbuffers::StructDef &struct_def,
                                      flatbuffers::FieldDef *key_field,
                                      const std::string &data_buffer,
                                      const std::string &offset) const {
    // Use the generated type directly, to properly handle default values that
    // might not be written to the buffer.
    auto name = Name(*key_field);
    if (name == struct_def.name) { name += "_"; }
    return GetObjectConstructor(struct_def, data_buffer, offset) + "." + name;
  }

  // Direct mutation is only allowed for scalar fields.
  // Hence a setter method will only be generated for such fields.
  std::string GenSetter(const Type &type) const {
    if (IsScalar(type.base_type)) {
      std::string basicType = GenTypeBasic(type, false);
      if (basicType == "byte" || type.base_type == BASE_TYPE_BOOL) {
        return "__p.bb.PutByte";
      }
      return "__p.bb.Put<" + basicType + ">";
    } else {
      return "";
    }
  }

  // Returns the method name for use with add/put calls.
  std::string GenMethod(const Type &type) const {
    return IsScalar(type.base_type) ? ("<" + GenTypeBasic(type, false) + ">")
                                    : (IsStruct(type) ? "Struct" : "Offset");
  }

  // Recursively generate arguments for a constructor, to deal with nested
  // structs.
  void GenStructArgs(const StructDef &struct_def, std::string *code_ptr,
                     const char *nameprefix, size_t array_count = 0) const {
    std::string &code = *code_ptr;
    for (auto it = struct_def.fields.vec.begin();
         it != struct_def.fields.vec.end(); ++it) {
      auto &field = **it;
      const auto &field_type = field.value.type;
      const auto array_field = IsArray(field_type);
      const auto &type = array_field ? field_type.VectorType() : field_type;
      const auto array_cnt = array_field ? (array_count + 1) : array_count;
      if (IsStruct(type)) {
        // Generate arguments for a struct inside a struct. To ensure names
        // don't clash, and to make it obvious these arguments are constructing
        // a nested struct, prefix the name with the field name.
        GenStructArgs(*field_type.struct_def, code_ptr,
                      (nameprefix + (EscapeKeyword(field.name) + "_")).c_str(),
                      array_cnt);
      } else {
        code += ", ";
        code += GenTypeBasic(type);
        if (field.IsScalarOptional()) { code += "?"; }
        if (array_cnt > 0) {
          code += "[";
          for (size_t i = 1; i < array_cnt; i++) code += ",";
          code += "]";
        }
        code += " ";
        code += nameprefix;
        code += Name(field);
      }
    }
  }

  // Recusively generate struct construction statements of the form:
  // builder.putType(name);
  // and insert manual padding.
  void GenStructBody(const StructDef &struct_def, std::string *code_ptr,
                     const char *nameprefix, size_t index = 0,
                     bool in_array = false) const {
    std::string &code = *code_ptr;
    std::string indent((index + 1) * 2, ' ');
    code += indent + "  builder.Prep(";
    code += NumToString(struct_def.minalign) + ", ";
    code += NumToString(struct_def.bytesize) + ");\n";
    for (auto it = struct_def.fields.vec.rbegin();
         it != struct_def.fields.vec.rend(); ++it) {
      auto &field = **it;
      const auto &field_type = field.value.type;
      if (field.padding) {
        code += indent + "  builder.Pad(";
        code += NumToString(field.padding) + ");\n";
      }
      if (IsStruct(field_type)) {
        GenStructBody(*field_type.struct_def, code_ptr,
                      (nameprefix + (field.name + "_")).c_str(), index,
                      in_array);
      } else {
        const auto &type =
            IsArray(field_type) ? field_type.VectorType() : field_type;
        const auto index_var = "_idx" + NumToString(index);
        if (IsArray(field_type)) {
          code += indent + "  for (int " + index_var + " = ";
          code += NumToString(field_type.fixed_length);
          code += "; " + index_var + " > 0; " + index_var + "--) {\n";
          in_array = true;
        }
        if (IsStruct(type)) {
          GenStructBody(*field_type.struct_def, code_ptr,
                        (nameprefix + (field.name + "_")).c_str(), index + 1,
                        in_array);
        } else {
          code += IsArray(field_type) ? "  " : "";
          code += indent + "  builder.Put";
          code += GenMethod(type) + "(";
          code += SourceCast(type);
          auto argname = nameprefix + Name(field);
          code += argname;
          size_t array_cnt = index + (IsArray(field_type) ? 1 : 0);
          if (array_cnt > 0) {
            code += "[";
            for (size_t i = 0; in_array && i < array_cnt; i++) {
              code += "_idx" + NumToString(i) + "-1";
              if (i != (array_cnt - 1)) code += ",";
            }
            code += "]";
          }
          code += ");\n";
        }
        if (IsArray(field_type)) { code += indent + "  }\n"; }
      }
    }
  }
  // JT: Not referenced? Can remove?
  // std::string GenOffsetGetter(flatbuffers::FieldDef *key_field,
  //                             const char *num = nullptr) const {
  //   std::string key_offset =
  //       "Table.__offset(" + NumToString(key_field->value.offset) + ", ";
  //   if (num) {
  //     key_offset += num;
  //     key_offset += ".Value, builder.DataBuffer)";
  //   } else {
  //     key_offset += "bb.Length";
  //     key_offset += " - tableOffset, bb)";
  //   }
  //   return key_offset;
  // }

  std::string GenKeyGetter(flatbuffers::StructDef &struct_def,
                           flatbuffers::FieldDef *key_field) const {
    auto key_type = key_field->value.type;
    auto field_offset = key_field->value.offset;

    if (key_type.base_type == BASE_TYPE_STRING) {
      // Use VectorOffsetComparer for strings with byte element type
      return "new SortedVectorUtils.VectorOffsetComparer<" + struct_def.name +
             ", " + BufferTypeName() + ", byte>(builder.DataBuffer, " +
             NumToString(field_offset) + ")";
    } else if (IsScalar(key_type.base_type)) {
      // Use ScalarOffsetComparer with the actual scalar type
      std::string scalar_type = GenTypeBasic(key_type, false);
      return "new SortedVectorUtils.ScalarOffsetComparer<" + struct_def.name +
             ", " + BufferTypeName() + ", " + scalar_type +
             ">(builder.DataBuffer, " + NumToString(field_offset) + ")";
    } else {
      // Invalid key type - FlatBuffers rule:
      // key field must be string or scalar type
      FLATBUFFERS_ASSERT(false && "Key field must be of string or scalar type");
      return "";
    }
  }

  // Get the value of a table verification function start
  void GetStartOfTableVerifier(const StructDef &struct_def,
                               std::string *code_ptr) {
    std::string &code = *code_ptr;
    code += "\n";
    code += "static public class " + struct_def.name + "Verify\n";
    code += "{\n";
    code += "  static public bool Verify";
    code += "(ref Google.FlatSpanBuffers.Verifier verifier, uint tablePos)\n";
    code += "  {\n";
    code += "    return verifier.VerifyTableStart(tablePos)\n";
  }

  // Get the value of a table verification function end
  void GetEndOfTableVerifier(std::string *code_ptr) {
    std::string &code = *code_ptr;
    code += "      && verifier.VerifyTableEnd(tablePos);\n";
    code += "  }\n";
    code += "}\n";
  }

  std::string GetNestedFlatBufferName(const FieldDef &field) {
    std::string name;
    if (field.nested_flatbuffer) {
      name = NamespacedName(*field.nested_flatbuffer);
    } else {
      name = "";
    }
    return name;
  }

  // Generate the code to call the appropriate Verify function(s) for a field.
  void GenVerifyCall(CodeWriter &code_, const FieldDef &field,
                     const char *prefix) {
    code_.SetValue("PRE", prefix);
    code_.SetValue("NAME", ConvertCase(field.name, Case::kUpperCamel));
    code_.SetValue("REQUIRED", field.IsRequired() ? "Required" : "");
    code_.SetValue("REQUIRED_FLAG", field.IsRequired() ? "true" : "false");
    code_.SetValue("TYPE", GenTypeGet(field.value.type));
    code_.SetValue("INLINESIZE", NumToString(InlineSize(field.value.type)));
    code_.SetValue("OFFSET", NumToString(field.value.offset));

    if (IsScalar(field.value.type.base_type) || IsStruct(field.value.type)) {
      code_.SetValue("ALIGN", NumToString(InlineAlignment(field.value.type)));
      code_ +=
          "{{PRE}}      && verifier.VerifyField(tablePos, "
          "{{OFFSET}} /*{{NAME}}*/, {{INLINESIZE}} /*{{TYPE}}*/, {{ALIGN}}, "
          "{{REQUIRED_FLAG}})";
    } else {
      // TODO - probably code below should go to this 'else' - code_ +=
      // "{{PRE}}VerifyOffset{{REQUIRED}}(verifier, {{OFFSET}})\\";
    }

    switch (field.value.type.base_type) {
      case BASE_TYPE_UNION: {
        auto union_name = NamespacedName(*field.value.type.enum_def);
        code_.SetValue("ENUM_NAME1", field.value.type.enum_def->name);
        code_.SetValue("ENUM_NAME", union_name);
        code_.SetValue("SUFFIX", UnionTypeFieldSuffix());
        // Caution: This construction assumes, that UNION type id element has
        // been created just before union data and its offset precedes union.
        // Such assumption is common in flatbuffer implementation
        code_.SetValue("TYPE_ID_OFFSET",
                       NumToString(field.value.offset - sizeof(voffset_t)));
        code_ +=
            "{{PRE}}      && verifier.VerifyUnion(tablePos, "
            "{{TYPE_ID_OFFSET}}, "
            "{{OFFSET}} /*{{NAME}}*/, {{ENUM_NAME}}Verify.Verify, "
            "{{REQUIRED_FLAG}})";
        break;
      }
      case BASE_TYPE_STRUCT: {
        if (!field.value.type.struct_def->fixed) {
          code_ +=
              "{{PRE}}      && verifier.VerifyTable(tablePos, "
              "{{OFFSET}} /*{{NAME}}*/, {{TYPE}}Verify.Verify, "
              "{{REQUIRED_FLAG}})";
        }
        break;
      }
      case BASE_TYPE_STRING: {
        code_ +=
            "{{PRE}}      && verifier.VerifyString(tablePos, "
            "{{OFFSET}} /*{{NAME}}*/, {{REQUIRED_FLAG}})";
        break;
      }
      case BASE_TYPE_VECTOR: {
        switch (field.value.type.element) {
          case BASE_TYPE_STRING: {
            code_ +=
                "{{PRE}}      && verifier.VerifyVectorOfStrings(tablePos, "
                "{{OFFSET}} /*{{NAME}}*/, {{REQUIRED_FLAG}})";
            break;
          }
          case BASE_TYPE_STRUCT: {
            if (!field.value.type.struct_def->fixed) {
              code_ +=
                  "{{PRE}}      && verifier.VerifyVectorOfTables(tablePos, "
                  "{{OFFSET}} /*{{NAME}}*/, {{TYPE}}Verify.Verify, "
                  "{{REQUIRED_FLAG}})";
            } else {
              code_.SetValue(
                  "VECTOR_ELEM_INLINESIZE",
                  NumToString(InlineSize(field.value.type.VectorType())));
              code_ +=
                  "{{PRE}}      && "
                  "verifier.VerifyVectorOfData(tablePos, "
                  "{{OFFSET}} /*{{NAME}}*/, {{VECTOR_ELEM_INLINESIZE}} "
                  "/*{{TYPE}}*/, {{REQUIRED_FLAG}})";
            }
            break;
          }
          case BASE_TYPE_UNION: {
            // Vectors of unions are not yet supported for go
            break;
          }
          default:
            // Generate verifier for vector of data.
            // It may be either nested flatbuffer of just vector of bytes
            auto nfn = GetNestedFlatBufferName(field);
            if (!nfn.empty()) {
              code_.SetValue("CPP_NAME", nfn);
              // FIXME: file_identifier.
              code_ +=
                  "{{PRE}}      && verifier.VerifyNestedBuffer(tablePos, "
                  "{{OFFSET}} /*{{NAME}}*/, {{CPP_NAME}}Verify.Verify, "
                  "{{REQUIRED_FLAG}})";
            } else if (field.flexbuffer) {
              code_ +=
                  "{{PRE}}      && verifier.VerifyNestedBuffer(tablePos, "
                  "{{OFFSET}} /*{{NAME}}*/, null, {{REQUIRED_FLAG}})";
            } else {
              code_.SetValue(
                  "VECTOR_ELEM_INLINESIZE",
                  NumToString(InlineSize(field.value.type.VectorType())));
              code_ +=
                  "{{PRE}}      && verifier.VerifyVectorOfData(tablePos, "
                  "{{OFFSET}} /*{{NAME}}*/, {{VECTOR_ELEM_INLINESIZE}} "
                  "/*{{TYPE}}*/, {{REQUIRED_FLAG}})";
            }
            break;
        }

        break;
      }
      default: {
        break;
      }
    }
  }

  // Generate table constructors, conditioned on its members' types.
  void GenTableVerifier(const StructDef &struct_def, std::string *code_ptr) {
    CodeWriter code_;

    GetStartOfTableVerifier(struct_def, code_ptr);

    // Generate struct fields accessors
    for (auto it = struct_def.fields.vec.begin();
         it != struct_def.fields.vec.end(); ++it) {
      auto &field = **it;
      if (field.deprecated) continue;

      GenVerifyCall(code_, field, "");
    }

    *code_ptr += code_.ToString();

    GetEndOfTableVerifier(code_ptr);
  }

  // Generate struct or table methods.
  void GenStructVerifier(const StructDef &struct_def, std::string *code_ptr) {
    if (struct_def.generated) return;

    // cur_name_space_ = struct_def.defined_namespace;

    // Generate verifiers
    if (struct_def.fixed) {
      // Fixed size structures do not require table members
      // verification - instead structure size is verified using VerifyField
    } else {
      // Create table verification function
      GenTableVerifier(struct_def, code_ptr);
    }
  }

  void GenStruct(StructDef &struct_def, std::string *code_ptr,
                 const IDLOptions &opts) const {
    if (struct_def.generated) return;
    std::string &code = *code_ptr;

    // Generate a struct accessor class, with methods of the form:
    // public type name() { return bb.getType(i + offset); }
    // or for tables of the form:
    // public type name() {
    //   int o = __offset(offset); return o != 0 ? bb.getType(o + i) : default;
    // }
    GenComment(struct_def.doc_comment, code_ptr, &comment_config);
    if (struct_def.attributes.Lookup("private")) {
      code += "internal ";
    } else {
      code += "public ";
    }
    if (struct_def.attributes.Lookup("csharp_partial")) {
      // generate a partial class for this C# struct/table
      code += "partial ";
    }
    // For span mode, use ref struct for compatibility with stackalloc
    if (IsSpanMode()) { code += "ref "; }
    code += "struct " + struct_def.name;
    code += " : " + FlatbufferObjectInterface();
    code += "\n{\n";
    code += "  private ";
    code += struct_def.fixed ? StructBaseTypeName() : TableBaseTypeName();
    code += " __p;\n";

    code += "  public " + BufferTypeName() +
            " ByteBuffer { get { return __p.bb; } }\n";

    if (!struct_def.fixed) {
      // Generate version check method.
      // Force compile time error if not using the same version runtime.
      code += "  public static void ValidateVersion() {";
      code += " FlatBufferConstants.";
      code += "FLATBUFFERS_25_2_10(); ";
      code += "}\n";

      // Generate a special accessor for the table that when used as the root
      // of a FlatBuffer
      std::string method_name = "GetRootAs" + struct_def.name;
      std::string method_signature =
          "  public static " + struct_def.name + " " + method_name;

      // create convenience method that doesn't require an existing object
      code += method_signature + "(" + BufferTypeName() + " _bb) ";
      code += "{ return " + method_name + "(_bb, new " + struct_def.name +
              "()); }\n";

      // create method that allows object reuse
      code += method_signature + "(" + BufferTypeName() + " _bb, " +
              struct_def.name + " obj) { ";
      code += "return (obj.__assign(_bb.Get<int>(_bb.Position";
      code += ") + _bb.Position";
      code += ", _bb)); }\n";
      if (parser_.root_struct_def_ == &struct_def) {
        if (parser_.file_identifier_.length()) {
          // Check if a buffer has the identifier.
          code += "  public static ";
          code += "bool " + struct_def.name;
          code += "BufferHasIdentifier(" + BufferTypeName() + " _bb) { return ";
          code += TableBaseTypeName() + ".__has_identifier(_bb, \"";
          code += parser_.file_identifier_;
          code += "\"); }\n";
        }

        // Generate the Verify method that checks if a ByteBuffer is save to
        // access
        code += "  public static ";
        code += "bool Verify" + struct_def.name + "(" + BufferTypeName() +
                " _bb) {";
        code += "Google.FlatSpanBuffers.Verifier verifier = new ";
        code += "Google.FlatSpanBuffers.Verifier(_bb); ";
        // For SpanBuf mode, we reference the base namespace's Verify class
        // since verifiers operate on the buffer
        std::string verify_class_ns =
            FullNamespace(".", *struct_def.defined_namespace);
        if (!verify_class_ns.empty()) verify_class_ns += ".";
        code += "return verifier.VerifyBuffer(\"";
        code += parser_.file_identifier_;
        code += "\", false, " + verify_class_ns + struct_def.name +
                "Verify.Verify);";
        code += " }\n";
      }
    }

    // Generate the __init method that sets the field in a pre-existing
    // accessor object. This is to allow object reuse.
    code += "  public void __init(int _i, " + BufferTypeName() + " _bb) ";
    code += "{ ";
    code += "__p = new ";
    code += struct_def.fixed ? StructBaseTypeName() : TableBaseTypeName();
    code += "(_i, _bb); ";
    code += "}\n";
    code += "  public " + struct_def.name + " __assign(int _i, " +
            BufferTypeName() + " _bb) ";
    code += "{ __init(_i, _bb); return this; }\n\n";
    for (auto it = struct_def.fields.vec.begin();
         it != struct_def.fields.vec.end(); ++it) {
      auto &field = **it;
      if (field.deprecated) continue;
      GenComment(field.doc_comment, code_ptr, &comment_config, "  ");
      std::string type_name = GenTypeGet(field.value.type);
      std::string type_name_dest = GenTypeGet(field.value.type);
      std::string conditional_cast = "";
      std::string optional = "";
      // In ByteBuffer mode, struct/table fields use C# nullable syntax Type?.
      // In SpanBuffer mode, ref structs cannot use C# nullable syntax, so we
      // use RefStructNullable<T> wrapper instead.
      bool use_nullable = !IsSpanMode() && !field.IsRequired();
      bool use_ref_struct_nullable =
          IsSpanMode() && !struct_def.fixed &&
          field.value.type.base_type == BASE_TYPE_STRUCT && !field.IsRequired();
      if (!struct_def.fixed && !field.IsRequired() &&
          (field.value.type.base_type == BASE_TYPE_STRUCT ||
           field.value.type.base_type == BASE_TYPE_UNION ||
           (IsVector(field.value.type) &&
            (field.value.type.element == BASE_TYPE_STRUCT ||
             field.value.type.element == BASE_TYPE_UNION)))) {
        if (use_nullable) {
          optional = "?";
          conditional_cast = "(" + type_name_dest + optional + ")";
        }
      }
      if (field.IsScalarOptional()) { optional = "?"; }
      std::string dest_mask = "";
      std::string dest_cast = DestinationCast(field.value.type);
      std::string src_cast = SourceCast(field.value.type);
      std::string field_name_camel = Name(field);
      if (field_name_camel == struct_def.name) { field_name_camel += "_"; }
      std::string return_type =
          use_ref_struct_nullable ? "RefStructNullable<" + type_name_dest + ">"
                                  : type_name_dest + optional;
      std::string method_start =
          "  public " + return_type + " " + field_name_camel;
      std::string obj = "(new " + type_name + "())";

      // Most field accessors need to retrieve and test the field offset first,
      // this is the prefix code for that:
      auto offset_prefix =
          IsArray(field.value.type)
              ? " { return "
              : (" { int o = __p.__offset(" + NumToString(field.value.offset) +
                 "); return o != 0 ? ");
      // Generate the accessors that don't do object reuse.
      if (field.value.type.base_type == BASE_TYPE_STRUCT) {
      } else if (IsVector(field.value.type) &&
                 field.value.type.element == BASE_TYPE_STRUCT) {
      } else if (field.value.type.base_type == BASE_TYPE_UNION ||
                 (IsVector(field.value.type) &&
                  field.value.type.VectorType().base_type == BASE_TYPE_UNION)) {
        method_start += "<TTable>";
        type_name = type_name_dest;
      }
      std::string getter = dest_cast + GenGetter(field.value.type);

      // True when field uses span-based accessor
      bool gen_span_getter =
          !struct_def.fixed &&
          ((IsSpanMode() &&
            (field.value.type.base_type == BASE_TYPE_UNION ||
             (IsVector(field.value.type) && !IsArray(field.value.type) &&
              (field.value.type.element == BASE_TYPE_STRUCT ||
               field.value.type.element == BASE_TYPE_UNION ||
               field.value.type.element == BASE_TYPE_STRING ||
               IsScalar(field.value.type.element))))) ||
           (!IsSpanMode() && IsVector(field.value.type) &&
            !IsArray(field.value.type) &&
            (field.value.type.element == BASE_TYPE_STRUCT ||
             field.value.type.element == BASE_TYPE_UNION ||
             field.value.type.element == BASE_TYPE_STRING ||
             IsScalar(field.value.type.element))));

      if (!gen_span_getter) { code += method_start; }
      std::string default_cast = "";
      // only create default casts for c# scalars or vectors of scalars
      if ((IsScalar(field.value.type.base_type) ||
           (IsVector(field.value.type) &&
            IsScalar(field.value.type.element)))) {
        // For scalars, default value will be returned by GetDefaultValue().
        // If the scalar is an enum, GetDefaultValue() returns an actual c# enum
        // that doesn't need to be casted. However, default values for enum
        // elements of vectors are integer literals ("0") and are still casted
        // for clarity.
        // If the scalar is optional and enum, we still need the cast.
        if ((field.value.type.enum_def == nullptr ||
             IsVector(field.value.type)) ||
            (IsEnum(field.value.type) && field.IsScalarOptional())) {
          default_cast = "(" + type_name_dest + optional + ")";
        }
      }
      std::string member_suffix = "; ";
      if (IsScalar(field.value.type.base_type)) {
        code += " { get";
        member_suffix += "} ";
        if (struct_def.fixed) {
          code += " { return " + getter;
          code += "(__p.bb_pos + ";
          code += NumToString(field.value.offset) + ")";
          code += dest_mask;
        } else {
          code += offset_prefix + getter;
          code += "(o + __p.bb_pos)" + dest_mask;
          code += " : " + default_cast;
          code += GenDefaultValue(field);
        }
      } else {
        switch (field.value.type.base_type) {
          case BASE_TYPE_STRUCT:
            // Generate property getter for struct fields
            if (!gen_span_getter) {
              code += " { get";
              member_suffix += "} ";
              if (struct_def.fixed) {
                code += " { return " + obj + ".__assign(" + "__p.";
                code += "bb_pos + " + NumToString(field.value.offset) + ", ";
                code += "__p.bb)";
              } else {
                code += offset_prefix + conditional_cast;
                if (use_ref_struct_nullable) {
                  code += "new RefStructNullable<";
                  code += GenTypeGet(field.value.type) + ">(";
                  code += obj + ".__assign(";
                  code += field.value.type.struct_def->fixed
                              ? "o + __p.bb_pos"
                              : "__p.__indirect(o + __p.bb_pos)";
                  code += ", __p.bb)) : default";
                } else if (IsSpanMode()) {
                  code += obj + ".__assign(";
                  code += field.value.type.struct_def->fixed
                              ? "o + __p.bb_pos"
                              : "__p.__indirect(o + __p.bb_pos)";
                  code += ", __p.bb) : default";
                } else {
                  code += obj + ".__assign(";
                  code += field.value.type.struct_def->fixed
                              ? "o + __p.bb_pos"
                              : "__p.__indirect(o + __p.bb_pos)";
                  code += ", __p.bb) : null";
                }
              }
            }
            break;
          case BASE_TYPE_STRING:
            code += " { get";
            member_suffix += "} ";
            code += offset_prefix + getter + "(o + " + "__p.";
            code += "bb_pos) : null";
            break;
          case BASE_TYPE_ARRAY: FLATBUFFERS_FALLTHROUGH();  // fall thru
          case BASE_TYPE_VECTOR: {
            auto vectortype = field.value.type.VectorType();
            // Skip vector element getters for struct/table/string/union
            // vectors, these will use Vector accessors Scalar vectors will use
            // Spans.
            bool skip_vector_getter =
                !struct_def.fixed && !IsArray(field.value.type) &&
                (vectortype.base_type == BASE_TYPE_STRUCT ||
                 vectortype.base_type == BASE_TYPE_STRING ||
                 vectortype.base_type == BASE_TYPE_UNION ||
                 IsScalar(vectortype.base_type));

            if (!skip_vector_getter) {
              if (vectortype.base_type == BASE_TYPE_UNION) {
                conditional_cast = IsSpanMode() ? "" : "(TTable?)";
                getter += "<TTable>";
              }
              code += "(";
              if (vectortype.base_type == BASE_TYPE_STRUCT) {
                getter = obj + ".__assign";
              }
              code += "int j)";
              const auto body = offset_prefix + conditional_cast + getter + "(";
              if (vectortype.base_type == BASE_TYPE_UNION) {
                code +=
                    " where TTable : struct, " + FlatbufferObjectInterface();
                if (IsSpanMode()) { code += ", allows ref struct"; }
                code += body;
              } else {
                code += body;
              }
              std::string index = "__p.";
              if (IsArray(field.value.type)) {
                index += "bb_pos + " + NumToString(field.value.offset) + " + ";
              } else {
                index += "__vector(o) + ";
              }
              index += "j * " + NumToString(InlineSize(vectortype));
              if (vectortype.base_type == BASE_TYPE_STRUCT) {
                code += vectortype.struct_def->fixed
                            ? index
                            : "__p.__indirect(" + index + ")";
                code += ", __p.bb";
              } else {
                code += index;
              }
              code += ")" + dest_mask;
              if (!IsArray(field.value.type)) {
                code += " : ";
                if (field.value.type.element == BASE_TYPE_BOOL) {
                  code += "false";
                } else if (IsScalar(field.value.type.element)) {
                  code += default_cast + "0";
                } else {
                  // For struct/table vector elements, use default in SpanBuffer
                  // mode
                  code += IsSpanMode() ? "default" : "null";
                }
              }
              if (vectortype.base_type == BASE_TYPE_UNION &&
                  HasUnionStringValue(*vectortype.enum_def)) {
                code += member_suffix;
                code += "}\n";
                code += "  public string " + Name(field) + "AsString(int j)";
                code += offset_prefix + GenGetter(Type(BASE_TYPE_STRING));
                code += "(" + index + ") : null";
              }
            }
            break;
          }
          case BASE_TYPE_UNION:
            if (!gen_span_getter) {
              if (IsSpanMode()) {
                code += "() where TTable : struct, ";
                code += FlatbufferObjectInterface() + ", allows ref struct";
                code += offset_prefix + getter;
                code += "<TTable>(o + __p.bb_pos) : default";
              } else {
                code += "() where TTable : struct, IFlatbufferObject";
                code += offset_prefix + "(TTable?)" + getter;
                code += "<TTable>(o + __p.bb_pos) : null";
              }
              if (HasUnionStringValue(*field.value.type.enum_def)) {
                code += member_suffix;
                code += "}\n";
                code += "  public string " + Name(field) + "AsString()";
                code += offset_prefix + GenGetter(Type(BASE_TYPE_STRING));
                code += "(o + __p.bb_pos) : null";
              }
              // As<> accesors for Unions
              // Loop through all the possible union types and generate an As
              // accessor that casts to the correct type.
              for (auto uit = field.value.type.enum_def->Vals().begin();
                   uit != field.value.type.enum_def->Vals().end(); ++uit) {
                auto val = *uit;
                if (val->union_type.base_type == BASE_TYPE_NONE) { continue; }
                auto union_field_type_name = GenTypeGet(val->union_type);
                code += member_suffix + "}\n";
                if (val->union_type.base_type == BASE_TYPE_STRUCT &&
                    val->union_type.struct_def->attributes.Lookup("private")) {
                  code += "  internal ";
                } else {
                  code += "  public ";
                }
                code += union_field_type_name + " ";
                code += field_name_camel + "As" + val->name + "() { return ";
                code += field_name_camel;
                if (IsString(val->union_type)) {
                  code += "AsString()";
                } else {
                  code += "<" + union_field_type_name + ">()" +
                          (IsSpanMode() ? "" : ".Value");
                }
              }
            }
            break;
          default: FLATBUFFERS_ASSERT(0);
        }
      }

      // Output member_suffix and closing brace if we generated a property
      // getter
      if (!gen_span_getter) {
        code += member_suffix;
        code += "}\n";
      }

      if (IsSpanMode() && !struct_def.fixed) {
        if (field.value.type.base_type == BASE_TYPE_UNION) {
          std::string union_field_name = Name(field);
          if (union_field_name == struct_def.name) { union_field_name += "_"; }
          auto offset_str = NumToString(field.value.offset);
          auto getter = GenGetter(field.value.type);

          if (field.IsRequired()) {
            code += "  public TTable " + union_field_name + "<TTable>()";
            code += " where TTable : struct, " + FlatbufferObjectInterface();
            code += ", allows ref struct";
            code += " { int o = __p.__offset(" + offset_str + ");";
            code += " return " + getter + "<TTable>(o + __p.bb_pos); }\n";
          } else {
            code += "  public RefStructNullable<TTable> ";
            code += union_field_name + "<TTable>()";
            code += " where TTable : struct, " + FlatbufferObjectInterface();
            code += ", allows ref struct";
            code += " { int o = __p.__offset(" + offset_str + ");";
            code += " return o != 0 ? new RefStructNullable<TTable>(";
            code += getter + "<TTable>(o + __p.bb_pos)) : default; }\n";
          }
          if (HasUnionStringValue(*field.value.type.enum_def)) {
            code += "  public string " + union_field_name + "AsString()";
            code += " { int o = __p.__offset(" + offset_str + ");";
            code += " return o != 0 ? " + GenGetter(Type(BASE_TYPE_STRING));
            code += "(o + __p.bb_pos) : null; }\n";
          }
          for (auto uit = field.value.type.enum_def->Vals().begin();
               uit != field.value.type.enum_def->Vals().end(); ++uit) {
            auto val = *uit;
            if (val->union_type.base_type == BASE_TYPE_NONE) { continue; }
            auto union_type_name = GenTypeGet(val->union_type);
            // Use SpanBuf variant type for struct types
            if (val->union_type.base_type == BASE_TYPE_STRUCT) {
              union_type_name = NamespacedName(*val->union_type.struct_def);
            }
            if (val->union_type.base_type == BASE_TYPE_STRUCT &&
                val->union_type.struct_def->attributes.Lookup("private")) {
              code += "  internal ";
            } else {
              code += "  public ";
            }

            std::string as_method = union_field_name + "As" + val->name + "()";
            if (IsString(val->union_type)) {
              code += "string " + as_method + " { return ";
              code += union_field_name + "AsString(); }\n";
            } else {
              // Struct/table union members:
              // for required, call generic directly; for optional, use .Value
              std::string accessor =
                  union_field_name + "<" + union_type_name + ">()";
              if (!field.IsRequired()) { accessor += ".Value"; }
              code += union_type_name + " " + as_method;
              code += " { return " + accessor + "; }\n";
            }
          }
        }
      }

      if (IsVector(field.value.type)) {
        auto vectortype = field.value.type.VectorType();
        // Skip generating separate Length property when the field provides
        // a Length property in its return type.
        bool skip_length_property = !IsArray(field.value.type) &&
                                    (vectortype.base_type == BASE_TYPE_STRUCT ||
                                     vectortype.base_type == BASE_TYPE_STRING ||
                                     vectortype.base_type == BASE_TYPE_UNION ||
                                     IsScalar(vectortype.base_type));

        if (!skip_length_property) {
          auto camel_name = Name(field);
          if (camel_name == struct_def.name) { camel_name += "_"; }
          code += "  public int " + camel_name;
          code += "Length";
          code += " { get";
          code += offset_prefix;
          code += "__p.__vector_len(o) : 0; ";
          code += "} ";
          code += "}\n";
        }

        // Generate struct/table vector wrappers
        if (!IsArray(field.value.type) &&
            vectortype.base_type == BASE_TYPE_STRUCT) {
          std::string elem_type_name = GenTypeGet(vectortype);
          std::string field_name = Name(field);
          if (field_name == struct_def.name) { field_name += "_"; }
          std::string element_size = NumToString(InlineSize(vectortype));
          auto offset_val = NumToString(field.value.offset);

          bool is_fixed_struct = vectortype.struct_def->fixed;

          if (IsSpanMode()) {
            std::string wrapper_type =
                is_fixed_struct ? "StructVectorSpan" : "TableVectorSpan";
            std::string full_wrapper_type =
                wrapper_type + "<" + elem_type_name + ">";

            if (field.IsRequired()) {
              code += "  public " + full_wrapper_type + " " + field_name;
              code += " { get { int o = __p.__offset(" + offset_val + "); ";
              code += "return new " + full_wrapper_type;
              code += "(__p, o, " + element_size + "); } }\n";
            } else {
              code += "  public RefStructNullable<" + full_wrapper_type;
              code += "> " + field_name;
              code += " { get { int o = __p.__offset(" + offset_val + "); ";
              code += "return o != 0 ? new RefStructNullable<";
              code += full_wrapper_type + ">(new " + full_wrapper_type;
              code += "(__p, o, " + element_size + ")) : default; } }\n";
            }
          } else {
            // Use non-ref struct wrappers with C# nullable
            std::string wrapper_type =
                is_fixed_struct ? "StructVector" : "TableVector";
            std::string full_wrapper_type =
                wrapper_type + "<" + elem_type_name + ">";

            if (field.IsRequired()) {
              code += "  public " + full_wrapper_type + " " + field_name;
              code += " { get { int o = __p.__offset(" + offset_val + "); ";
              code += "return new " + full_wrapper_type;
              code += "(__p, o, " + element_size + "); } }\n";
            } else {
              code += "  public " + full_wrapper_type + "? " + field_name;
              code += " { get { int o = __p.__offset(" + offset_val + "); ";
              code += "return o != 0 ? new " + full_wrapper_type;
              code += "(__p, o, " + element_size + ") : null; } }\n";
            }
          }
        }

        // Generate StringVector/StringVectorSpan property for string vectors
        if (!IsArray(field.value.type) &&
            vectortype.base_type == BASE_TYPE_STRING) {
          std::string field_name = Name(field);
          if (field_name == struct_def.name) { field_name += "_"; }
          auto offset_val = NumToString(field.value.offset);

          if (IsSpanMode()) {
            if (field.IsRequired()) {
              code += "  public StringVectorSpan " + field_name;
              code += " { get { int o = __p.__offset(" + offset_val + "); ";
              code += "return new StringVectorSpan(__p, o); } }\n";
            } else {
              code += "  public RefStructNullable<StringVectorSpan> ";
              code += field_name;
              code += " { get { int o = __p.__offset(" + offset_val + "); ";
              code += "return o != 0 ? ";
              code += "new RefStructNullable<StringVectorSpan>";
              code += "(new StringVectorSpan(__p, o)) : default; } }\n";
            }
          } else {
            if (field.IsRequired()) {
              code += "  public StringVector " + field_name;
              code += " { get { int o = __p.__offset(" + offset_val + "); ";
              code += "return new StringVector(__p, o); } }\n";
            } else {
              code += "  public StringVector? " + field_name;
              code += " { get { int o = __p.__offset(" + offset_val + "); ";
              code += "return o != 0 ? new StringVector(__p, o) : null; } }\n";
            }
          }
        }

        // Generate UnionVector/UnionVectorSpan property for union vectors
        if (!IsArray(field.value.type) &&
            vectortype.base_type == BASE_TYPE_UNION) {
          std::string field_name = Name(field);
          if (field_name == struct_def.name) { field_name += "_"; }
          auto value_offset_val = NumToString(field.value.offset);
          // The type vector is always the field right before the value vector
          // (offset - 2)
          auto type_offset_val = NumToString(field.value.offset - 2);
          std::string element_size = NumToString(InlineSize(vectortype));

          if (IsSpanMode()) {
            std::string wrapper_type = "UnionVectorSpan";

            if (field.IsRequired()) {
              code += "  public " + wrapper_type + " " + field_name;
              code +=
                  " { get { int o = __p.__offset(" + value_offset_val + "); ";
              code += "return new " + wrapper_type;
              code += "(__p, o, " + element_size + "); } }\n";
            } else {
              code += "  public RefStructNullable<" + wrapper_type + "> ";
              code += field_name;
              code +=
                  " { get { int o = __p.__offset(" + value_offset_val + "); ";
              code += "return o != 0 ? new RefStructNullable<";
              code += wrapper_type + ">(new " + wrapper_type;
              code += "(__p, o, " + element_size + ")) : default; } }\n";
            }
          } else {
            // Use C# nullable for non-spanmode code
            std::string wrapper_type = "UnionVector";

            if (field.IsRequired()) {
              code += "  public " + wrapper_type + " " + field_name;
              code +=
                  " { get { int o = __p.__offset(" + value_offset_val + "); ";
              code += "return new " + wrapper_type;
              code += "(__p, o, " + element_size + "); } }\n";
            } else {
              code += "  public " + wrapper_type + "? " + field_name;
              code +=
                  " { get { int o = __p.__offset(" + value_offset_val + "); ";
              code += "return o != 0 ? new " + wrapper_type;
              code += "(__p, o, " + element_size + ") : null; } }\n";
            }
          }
        }
        // See if we should generate a by-key accessor.
        if (field.value.type.element == BASE_TYPE_STRUCT &&
            !field.value.type.struct_def->fixed) {
          auto &sd = *field.value.type.struct_def;
          auto &fields = sd.fields.vec;
          for (auto kit = fields.begin(); kit != fields.end(); ++kit) {
            auto &key_field = **kit;
            if (key_field.key) {
              auto qualified_name = NamespacedName(sd);
              code += "  public bool TryGet" + Name(field) + "ByKey(";
              code += GenTypeGet(key_field.value.type) + " key, out " +
                      qualified_name + " value) { int o = __p.__offset(" +
                      NumToString(field.value.offset) +
                      "); if (o != 0) { return " + qualified_name +
                      ".TryGetByKey(__p.__vector(o), key, __p.bb, out value); "
                      "} value = default; return false; }\n";
              break;
            }
          }
        }
      }
      // Generate a ByteBuffer accessor for strings & vectors of scalars.
      if ((IsVector(field.value.type) &&
           IsScalar(field.value.type.VectorType().base_type)) ||
          IsString(field.value.type)) {
        bool is_string_field = IsString(field.value.type);
        auto offset_val = NumToString(field.value.offset);
        if (!is_string_field) {
          std::string accessor_name = Name(field);
          std::string element_type =
              GenTypeBasic(field.value.type.VectorType());
          std::string span_type = "ReadOnlySpan<" + element_type + ">";

          if (field.IsRequired()) {
            // Required field - return span directly without nullable wrapper
            code += "  public " + span_type + " " + accessor_name;
            code += " { get { return __p.__vector_as_span<" + element_type;
            code += ">(" + offset_val + "); } }\n";
          } else {
            // Optional field - wrap in RefStructNullable for null handling
            std::string nullable_type = "RefStructNullable<" + span_type + ">";
            code += "  public " + nullable_type + " " + accessor_name;
            code += " { get { int o = __p.__offset(" + offset_val + "); ";
            code += "return o != 0 ? new " + nullable_type;
            code += "(__p.__vector_as_span<" + element_type;
            code += ">(" + offset_val + ")) : default; } }\n";
          }
        } else {
          // Generate as method for strings:
          // public ReadOnlySpan<byte> GetNameBytes() { return ...; }
          std::string accessor_name = "Get" + Name(field) + "Bytes";
          auto vec_type = GenTypeBasic(field.value.type.VectorType());
          code += "  public ReadOnlySpan<" + vec_type + "> ";
          code += accessor_name + "() { return ";
          code += "__p.__vector_as_span<" + vec_type + ">(";
          code += offset_val;
          code += "); }\n";
        }

        if (parser_.opts.mutable_buffer) {
          auto vec_type = GenTypeBasic(field.value.type.VectorType());
          code += "  public RefStructNullable<Span<" + vec_type + ">> Mutable";
          code += Name(field);
          code += " { get { int o = __p.__offset(";
          code += offset_val;
          code += "); return o != 0 ? new RefStructNullable<Span<";
          code += vec_type;
          code += ">>(";
          code += "__p.__vector_as_span<" + vec_type + ">(";
          code += offset_val;
          code += ")) : default; } }\n";
        }
      }

      // Generate Length property and ByteBuffer accessor for arrays in structs.
      if (IsArray(field.value.type) && struct_def.fixed &&
          IsScalar(field.value.type.VectorType().base_type)) {
        auto camel_name = Name(field);
        if (camel_name == struct_def.name) { camel_name += "_"; }

        // Generate Length constant
        code += "  public const int " + camel_name;
        code += "Length = ";
        code += NumToString(field.value.type.fixed_length);
        code += ";\n";

        code += "  public ReadOnlySpan<" +
                GenTypeBasic(field.value.type.VectorType()) + "> Get";
        code += camel_name;
        code += "Bytes() { return ";

        // For byte arrays, we can return the span directly
        if (field.value.type.VectorType().base_type == BASE_TYPE_UCHAR) {
          code += "__p.bb.ToReadOnlySpan(__p.bb_pos + ";
          code += NumToString(field.value.offset);
          code += ", ";
          code += NumToString(field.value.type.fixed_length);
          code += ")";
        } else {
          // For other types, we need to cast the byte span
          code += "__p.bb.GetReadOnlySpan<" +
                  GenTypeBasic(field.value.type.VectorType());
          code += ">(__p.bb_pos + ";
          code += NumToString(field.value.offset);
          code += ", ";
          code += NumToString(field.value.type.fixed_length);
          code += ")";
        }
        code += "; }\n";

        if (parser_.opts.mutable_buffer) {
          code += "  public Span<" +
                  GenTypeBasic(field.value.type.VectorType()) + "> Mutable";
          code += camel_name;
          code += " { get { return ";

          if (field.value.type.VectorType().base_type == BASE_TYPE_UCHAR) {
            code += "__p.bb.ToSpan(__p.bb_pos + ";
            code += NumToString(field.value.offset);
            code += ", ";
            code += NumToString(field.value.type.fixed_length);
            code += ")";
          } else {
            code +=
                "__p.bb.GetSpan<" + GenTypeBasic(field.value.type.VectorType());
            code += ">(__p.bb_pos + ";
            code += NumToString(field.value.offset);
            code += ", ";
            code += NumToString(field.value.type.fixed_length);
            code += ")";
          }
          code += "; } }\n";
        }
      }

      // generate object accessors if is nested_flatbuffer
      if (field.nested_flatbuffer) {
        auto nested_type_name = NamespacedName(*field.nested_flatbuffer);
        auto nested_method_name =
            Name(field) + "As" + field.nested_flatbuffer->name;
        auto get_nested_method_name = nested_method_name;
        get_nested_method_name = "Get" + nested_method_name;
        if (IsSpanMode()) {
          obj = "(new " + nested_type_name + "())";
          code += "  public " + nested_type_name + " ";
          code += get_nested_method_name + "(";
          code += ") { int o = __p.__offset(";
          code += NumToString(field.value.offset) + "); ";
          code += "return o != 0 ? " + obj + ".__assign(";
          code += "__p.";
          code += "__indirect(__p.__vector(o)), ";
          code += "__p.bb) : default; }\n";
        } else {
          conditional_cast = "(" + nested_type_name + "?)";
          obj = "(new " + nested_type_name + "())";
          code += "  public " + nested_type_name + "? ";
          code += get_nested_method_name + "(";
          code += ") { int o = __p.__offset(";
          code += NumToString(field.value.offset) + "); ";
          code += "return o != 0 ? " + conditional_cast + obj + ".__assign(";
          code += "__p.";
          code += "__indirect(__p.__vector(o)), ";
          code += "__p.bb) : null; }\n";
        }
      }
      // Generate mutators for scalar fields or vectors of scalars.
      if (parser_.opts.mutable_buffer) {
        auto is_series = (IsSeries(field.value.type));
        const auto &underlying_type =
            is_series ? field.value.type.VectorType() : field.value.type;
        // Boolean parameters have to be explicitly converted to byte
        // representation.
        auto setter_parameter =
            underlying_type.base_type == BASE_TYPE_BOOL
                ? "(byte)(" + EscapeKeyword(field.name) + " ? 1 : 0)"
                : EscapeKeyword(field.name);
        auto mutator_prefix = "Mutate";
        // A vector mutator also needs the index of the vector element it
        // should mutate.
        auto mutator_params = (is_series ? "(int j, " : "(") +
                              GenTypeGet(underlying_type) + " " +
                              EscapeKeyword(field.name) + ") { ";
        auto setter_index =
            is_series
                ? "__p." +
                      (IsArray(field.value.type)
                           ? "bb_pos + " + NumToString(field.value.offset)
                           : "__vector(o)") +
                      +" + j * " + NumToString(InlineSize(underlying_type))
                : (struct_def.fixed
                       ? "__p.bb_pos + " + NumToString(field.value.offset)
                       : "o + __p.bb_pos");
        if (IsScalar(underlying_type.base_type) &&
            !IsUnion(field.value.type)) {
          code += "  public ";
          code += struct_def.fixed ? "void " : "bool ";
          code += mutator_prefix + Name(field);
          code += mutator_params;
          if (struct_def.fixed) {
            code += GenSetter(underlying_type) + "(" + setter_index + ", ";
            code += src_cast + setter_parameter + "); }\n";
          } else {
            code += "int o = __p.__offset(";
            code += NumToString(field.value.offset) + ");";
            code += " if (o != 0) { " + GenSetter(underlying_type);
            code += "(" + setter_index + ", " + src_cast + setter_parameter +
                    "); return true; } else { return false; } }\n";
          }
        }
      }
      if (parser_.opts.java_primitive_has_method &&
          IsScalar(field.value.type.base_type) && !struct_def.fixed) {
        auto vt_offset_constant =
            "  public static final int VT_" +
            ConvertCase(field.name, Case::kScreamingSnake) + " = " +
            NumToString(field.value.offset) + ";";

        code += vt_offset_constant;
        code += "\n";
      }
    }
    code += "\n";
    auto struct_has_create = false;
    std::set<flatbuffers::FieldDef *> field_has_create_set;
    flatbuffers::FieldDef *key_field = nullptr;
    if (struct_def.fixed) {
      struct_has_create = true;
      // create a struct constructor function
      code += "  public static " + GenOffsetType(struct_def) + " ";
      code += "Create";
      code += struct_def.name + "(" + BuilderParam();
      GenStructArgs(struct_def, code_ptr, "");
      code += ") {\n";
      GenStructBody(struct_def, code_ptr, "");
      code += "    return ";
      code += GenOffsetConstruct(struct_def, "builder.Offset");
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
      // JVM specifications restrict default constructor params to be < 255.
      // Longs and doubles take up 2 units, so we set the limit to be < 127.
      // In SpanBuffer mode, skip Object API generation, so don't use struct
      // fields pattern
      bool use_object_api_for_create =
          opts.generate_object_based_api && !IsSpanMode();
      if ((has_no_struct_fields || use_object_api_for_create) && num_fields &&
          num_fields < 127) {
        struct_has_create = true;
        // Generate a table constructor of the form:
        // public static int createName(FlatBufferBuilder builder, args...)
        code += "  public static " + GenOffsetType(struct_def) + " ";
        code += "Create" + struct_def.name;
        code += "(" + BuilderParam();
        for (auto it = struct_def.fields.vec.begin();
             it != struct_def.fields.vec.end(); ++it) {
          auto &field = **it;
          if (field.deprecated) continue;
          code += ",\n      ";
          if (IsStruct(field.value.type) && use_object_api_for_create) {
            code += WrapInNameSpace(
                field.value.type.struct_def->defined_namespace,
                GenTypeName_ObjectAPI(field.value.type.struct_def->name, opts));
            code += " ";
            code += EscapeKeyword(field.name);
            code += " = null";
          } else {
            code += GenTypeBasic(field.value.type);
            if (field.IsScalarOptional()) { code += "?"; }
            code += " ";
            code += EscapeKeyword(field.name);
            if (!IsScalar(field.value.type.base_type)) code += "Offset";

            code += " = ";
            code += GenDefaultValueBasic(field);
          }
        }
        code += ") {\n    builder.";
        code += "StartTable(";
        code += NumToString(struct_def.fields.vec.size()) + ");\n";
        for (size_t size = struct_def.sortbysize ? sizeof(largest_scalar_t) : 1;
             size; size /= 2) {
          for (auto it = struct_def.fields.vec.rbegin();
               it != struct_def.fields.vec.rend(); ++it) {
            auto &field = **it;
            if (!field.deprecated &&
                (!struct_def.sortbysize ||
                 size == SizeOf(field.value.type.base_type))) {
              code += "    " + struct_def.name + ".";
              code += "Add";
              code += Name(field) + "(" + BuilderArg() + ", ";
              if (IsStruct(field.value.type) && use_object_api_for_create) {
                code += GenTypePointer(field.value.type) + ".Pack(" +
                        BuilderArg() + ", " + EscapeKeyword(field.name) + ")";
              } else {
                code += EscapeKeyword(field.name);
                if (!IsScalar(field.value.type.base_type)) code += "Offset";
              }

              code += ");\n";
            }
          }
        }
        code += "    return " + struct_def.name + ".";
        code += "End" + struct_def.name;
        code += "(" + BuilderArg() + ");\n  }\n\n";
      }
      // Generate a set of static methods that allow table construction,
      // of the form:
      // public static void addName(FlatBufferBuilder builder, short name)
      // { builder.addShort(id, name, default); }
      // Unlike the Create function, these always work.
      code += "  public static void Start";
      code += struct_def.name;
      code += "(" + BuilderParam() + ") { builder.";
      code += "StartTable(";
      code += NumToString(struct_def.fields.vec.size()) + "); }\n";
      for (auto it = struct_def.fields.vec.begin();
           it != struct_def.fields.vec.end(); ++it) {
        auto &field = **it;
        if (field.deprecated) continue;
        if (field.key) key_field = &field;
        code += "  public static void Add";
        code += Name(field);
        code += "(" + BuilderParam() + ", ";
        code += GenTypeBasic(field.value.type);
        auto argname = ConvertCase(field.name, Case::kLowerCamel);
        if (!IsScalar(field.value.type.base_type)) argname += "Offset";
        if (field.IsScalarOptional()) { code += "?"; }
        code += " " + EscapeKeyword(argname) + ") { ";

        if (IsStruct(field.value.type)) {
          // For true structs (fixed size), use generic AddStruct method
          code += "builder.AddStruct(";
          code += NumToString(it - struct_def.fields.vec.begin()) + ", ";
          code += EscapeKeyword(argname);
          if (!field.IsScalarOptional()) {
            code += ", ";
            code += GenDefaultValue(field, false);
          }
        } else if (!IsScalar(field.value.type.base_type) &&
                   field.value.type.base_type != BASE_TYPE_UNION) {
          // Use generic AddOffset for offset types (string, vector, table)
          code += "builder.AddOffset(";
          code += NumToString(it - struct_def.fields.vec.begin()) + ", ";
          code += EscapeKeyword(argname);
          if (!field.IsScalarOptional()) {
            code += ", ";
            code += GenDefaultValue(field, false);
          }
        } else {
          // Use regular Add method for scalars and unions (unions store int
          // values)
          code += "builder.Add" + GenMethod(field.value.type) + "(";
          code += NumToString(it - struct_def.fields.vec.begin()) + ", ";
          code += SourceCastBasic(field.value.type, field.IsScalarOptional());
          code += EscapeKeyword(argname);
          if (!field.IsScalarOptional()) {
            code += ", ";
            code += GenDefaultValue(field, false);
          }
        }
        code += "); }\n";
        if (IsVector(field.value.type)) {
          auto vector_type = field.value.type.VectorType();
          auto alignment = InlineAlignment(vector_type);
          auto elem_size = InlineSize(vector_type);
          if (!IsStruct(vector_type)) {
            field_has_create_set.insert(&field);
            std::string vector_element_type;
            // This is the key part - determine the array type
            if (vector_type.base_type == BASE_TYPE_STRING) {
              vector_element_type = "StringOffset";
            } else if (vector_type.base_type == BASE_TYPE_UNION) {
              // For union vectors, use int[] for the data vector
              vector_element_type = "int";
            } else {
              vector_element_type = GenTypeBasic(vector_type);
            }

            code += "  public static VectorOffset ";
            code += "Create";
            code += Name(field);
            code += "VectorBlock(" + BuilderParam() + ", ";
            code += ScopedPrefix() + "Span<" + vector_element_type + "> data) ";
            code += "{ builder.StartVector(";
            code += NumToString(elem_size);
            code += ", data.Length, ";
            code += NumToString(alignment);
            code += "); ";

            // For offset types, use AddOffsetSpan; for scalar types, use
            // AddSpan
            if (vector_type.base_type == BASE_TYPE_UNION) {
              code += "builder.AddOffsetSpan(data); ";
            } else if (vector_type.base_type == BASE_TYPE_STRING ||
                       vector_type.struct_def) {
              code +=
                  "builder.AddOffsetSpan<" + vector_element_type + ">(data); ";
            } else {
              // For scalar types, add elements directly
              code += "builder.AddSpan<" + vector_element_type + ">(data); ";
            }

            code += "return builder.EndVector(); }\n";

            code += "  public static VectorOffset ";
            code += "Create";
            code += Name(field);
            code += "Vector(" + BuilderParam() + ", ";
            code += ScopedPrefix() + "Span<" + vector_element_type + "> data) ";
            code += "{ return Create";
            code += Name(field);
            code += "VectorBlock(" + BuilderArg() + ", data); }\n";
          }
          // Generate a method to start a vector, data to be added manually
          // after.
          code += "  public static void Start";
          code += Name(field);
          code += "Vector(" + BuilderParam() + ", int numElems) ";
          code += "{ builder.StartVector(";
          code += NumToString(elem_size);
          code += ", numElems, " + NumToString(alignment);
          code += "); }\n";
        }
      }
      code += "  public static " + GenOffsetType(struct_def) + " ";
      code += "End" + struct_def.name;
      code += "(" + BuilderParam() + ") {\n    int o = builder.";
      code += "EndTable();\n";
      for (auto it = struct_def.fields.vec.begin();
           it != struct_def.fields.vec.end(); ++it) {
        auto &field = **it;
        if (!field.deprecated && field.IsRequired()) {
          code += "    builder.Required(o, ";
          code += NumToString(field.value.offset);
          code += ");  // " + field.name + "\n";
        }
      }
      code += "    return " + GenOffsetConstruct(struct_def, "o") + ";\n  }\n";
      if (parser_.root_struct_def_ == &struct_def) {
        std::string size_prefix[] = { "", "SizePrefixed" };
        for (int i = 0; i < 2; ++i) {
          code += "  public static void ";
          code += "Finish" + size_prefix[i] + struct_def.name;
          code += "Buffer(" + BuilderParam() + ", " + GenOffsetType(struct_def);
          code += " offset) {";
          code += " builder.Finish" + size_prefix[i] + "(offset";
          code += ".Value";

          if (parser_.file_identifier_.length())
            code += ", \"" + parser_.file_identifier_ + "\"";
          code += "); }\n";
        }
      }
    }
    // Only generate key compare function for table,
    // because `key_field` is not set for struct
    if (struct_def.has_key && !struct_def.fixed) {
      FLATBUFFERS_ASSERT(key_field);
      auto name = Name(*key_field);
      if (name == struct_def.name) { name += "_"; }
      code += "\n  public static VectorOffset ";
      code += "CreateSortedVectorOf" + struct_def.name;
      code += "(" + BuilderParam() + ", ";
      code +=
          ScopedPrefix() + "Span<Offset<" + struct_def.name + ">> offsets) {\n";
      code +=
          "    var comparer = " + GenKeyGetter(struct_def, key_field) + ";\n";
      code += "    RefStructSorters.Sort(offsets, ref comparer);\n";
      code += "    return builder.CreateVectorOfTables<" + struct_def.name +
              ">(offsets);\n";
      code += "  }\n";

      code += "\n  public static bool TryGetByKey(";
      code += "int vectorLocation, ";
      code += GenTypeGet(key_field->value.type);
      code += " key, " + BufferTypeName() + " bb, out " + struct_def.name +
              " value) {\n";
      code += "    return SortedVectorUtils.TryGetByKey";

      if (key_field->value.type.base_type == BASE_TYPE_STRING) {
        code += "<" + struct_def.name + ", " + BufferTypeName() + ">";
        code += "(vectorLocation, key, ref bb, " +
                NumToString(key_field->value.offset) + ", out value);\n";
      } else {
        code += "<" + struct_def.name + ", " +
                GenTypeBasic(key_field->value.type, false) + ", " +
                BufferTypeName() + ">";
        code += "(vectorLocation, key, ref bb, " +
                NumToString(key_field->value.offset) + ", " +
                GenDefaultValueBasic(*key_field) + ", out value);\n";
      }

      code += "  }\n";
    }

    if (opts.generate_object_based_api) {
      GenPackUnPack_ObjectAPI(struct_def, code_ptr, opts, struct_has_create,
                              field_has_create_set);
    }
    code += "}\n\n";

    // Only generate the *T class in ByteBuffer mode, it's shared between both
    // modes
    if (opts.generate_object_based_api && !IsSpanMode()) {
      GenStruct_ObjectAPI(struct_def, code_ptr, opts);
    }
  }

  // NOTE: GenVectorAccessObject from the original idl_gen_csharp.cpp is not
  // used in either generator. Vector access is handled by StructVector/
  // TableVector wrappers and span-based accessors instead.

  std::string GenUnionVerify(const Type &union_type) const {
    if (union_type.enum_def) {
      const auto &enum_def = *union_type.enum_def;

      auto ret = "\n\nstatic public class " + enum_def.name + "Verify\n";
      ret += "{\n";
      ret +=
          "  static public bool Verify(ref Google.FlatSpanBuffers.Verifier "
          "verifier, "
          "byte typeId, uint tablePos)\n";
      ret += "  {\n";
      ret += "    bool result = true;\n";

      const auto union_enum_loop = [&]() {
        ret += "    switch((" + enum_def.name + ")typeId)\n";
        ret += "    {\n";

        for (auto it = enum_def.Vals().begin(); it != enum_def.Vals().end();
             ++it) {
          const auto &ev = **it;
          if (ev.IsZero()) { continue; }

          ret += "      case " + Name(enum_def) + "." + Name(ev) + ":\n";

          if (IsString(ev.union_type)) {
            ret += "       result = verifier.VerifyUnionString(tablePos);\n";
            ret += "        break;";
          } else if (ev.union_type.base_type == BASE_TYPE_STRUCT) {
            if (!ev.union_type.struct_def->fixed) {
              auto type = GenTypeGet(ev.union_type);
              ret += "        result = " + type +
                     "Verify.Verify(ref verifier, tablePos);\n";
            } else {
              ret += "        result = verifier.VerifyUnionData(tablePos, " +
                     NumToString(InlineSize(ev.union_type)) + ", " +
                     NumToString(InlineAlignment(ev.union_type)) + ");\n";
              ;
            }
            ret += "        break;";
          } else {
            FLATBUFFERS_ASSERT(false);
          }
          ret += "\n";
        }

        ret += "      default: result = true;\n";
        ret += "        break;\n";
        ret += "    }\n";
        ret += "    return result;\n";
      };

      union_enum_loop();
      ret += "  }\n";
      ret += "}\n";
      ret += "\n";

      return ret;
    }
    FLATBUFFERS_ASSERT(0);
    return "";
  }

  void GenEnum_ObjectAPI(EnumDef &enum_def, std::string *code_ptr,
                         const IDLOptions &opts) const {
    auto &code = *code_ptr;
    if (enum_def.generated) return;
    if (!enum_def.is_union) return;
    auto union_name = enum_def.name + "Union";
    auto class_member = std::string("Value");
    if (class_member == enum_def.name) { class_member += "_"; };
    // Add JsonConverter attribute to the class so it's used when in collections
    if (opts.cs_gen_json_serializer) {
      code += "[System.Text.Json.Serialization.JsonConverter(typeof(" +
              union_name + "_JsonConverter))]\n";
    }
    if (enum_def.attributes.Lookup("private")) {
      code += "internal ";
    } else {
      code += "public ";
    }
    code += "class " + union_name + " {\n";
    // Type
    code += "  public " + enum_def.name + " Type { get; set; }\n";
    // Value
    code += "  public object " + class_member + " { get; set; }\n";
    code += "\n";
    // Constructor
    code += "  public " + union_name + "() {\n";
    code += "    this.Type = " + enum_def.name + "." +
            enum_def.Vals()[0]->name + ";\n";
    code += "    this." + class_member + " = null;\n";
    code += "  }\n\n";
    // As<T>
    code += "  public T As<T>() where T : class { return this." + class_member +
            " as T; }\n";
    // As, From
    for (auto it = enum_def.Vals().begin(); it != enum_def.Vals().end(); ++it) {
      auto &ev = **it;
      if (ev.union_type.base_type == BASE_TYPE_NONE) continue;
      auto type_name = GenTypeGet_ObjectAPI(ev.union_type, opts);
      std::string accessibility =
          (ev.union_type.base_type == BASE_TYPE_STRUCT &&
           ev.union_type.struct_def->attributes.Lookup("private"))
              ? "internal"
              : "public";
      // As
      code += "  " + accessibility + " " + type_name + " As" + ev.name +
              "() { return this.As<" + type_name + ">(); }\n";
      // From
      auto lower_ev_name = ev.name;
      std::transform(lower_ev_name.begin(), lower_ev_name.end(),
                     lower_ev_name.begin(), CharToLower);
      code += "  " + accessibility + " static " + union_name + " From" +
              ev.name + "(" + type_name + " _" + lower_ev_name +
              ") { return new " + union_name + "{ Type = " + Name(enum_def) +
              "." + Name(ev) + ", " + class_member + " = _" + lower_ev_name +
              " }; }\n";
    }
    code += "\n";
    // Pack()
    code +=
        "  public static int Pack(Google.FlatSpanBuffers.FlatBufferBuilder "
        "builder, " +
        union_name + " _o) {\n";
    code += "    switch (_o.Type) {\n";
    for (auto it = enum_def.Vals().begin(); it != enum_def.Vals().end(); ++it) {
      auto &ev = **it;
      if (ev.union_type.base_type == BASE_TYPE_NONE) {
        code += "      default: return 0;\n";
      } else {
        code += "      case " + Name(enum_def) + "." + Name(ev) + ": return ";
        if (IsString(ev.union_type)) {
          code += "builder.CreateString(_o.As" + ev.name + "()).Value;\n";
        } else {
          code += BaseNamespacedName(*ev.union_type.struct_def) +
                  ".Pack(builder, _o.As" + ev.name + "()).Value;\n";
        }
      }
    }
    code += "    }\n";
    code += "  }\n";
    // Pack() for SpanBuf mode - uses ref parameter for ref struct builder
    code +=
        "  public static int Pack(ref "
        "Google.FlatSpanBuffers.FlatSpanBufferBuilder "
        "builder, " +
        union_name + " _o) {\n";
    code += "    switch (_o.Type) {\n";
    for (auto it = enum_def.Vals().begin(); it != enum_def.Vals().end(); ++it) {
      auto &ev = **it;
      if (ev.union_type.base_type == BASE_TYPE_NONE) {
        code += "      default: return 0;\n";
      } else {
        code += "      case " + Name(enum_def) + "." + Name(ev) + ": return ";
        if (IsString(ev.union_type)) {
          code += "builder.CreateString(_o.As" + ev.name + "()).Value;\n";
        } else {
          code += SpanBufNamespacedName(*ev.union_type.struct_def) +
                  ".Pack(ref builder, _o.As" + ev.name + "()).Value;\n";
        }
      }
    }
    code += "    }\n";
    code += "  }\n";
    code += "}\n\n";

    // JsonConverter
    if (opts.cs_gen_json_serializer) {
      if (enum_def.attributes.Lookup("private")) {
        code += "internal ";
      } else {
        code += "public ";
      }
      code += "class " + union_name +
              "_JsonConverter : System.Text.Json.Serialization.JsonConverter<" +
              union_name + "> {\n";

      // Read method - deserializes {"Type":"EnumName","Value":{...}} format
      code +=
          "  public override " + union_name +
          " Read(ref System.Text.Json.Utf8JsonReader reader, System.Type "
          "typeToConvert, System.Text.Json.JsonSerializerOptions options) {\n";
      code +=
          "    if (reader.TokenType == System.Text.Json.JsonTokenType.Null) { "
          "reader.Read(); return null; }\n";
      code +=
          "    if (reader.TokenType != "
          "System.Text.Json.JsonTokenType.StartObject) throw new "
          "System.Text.Json.JsonException(\"Expected start of object for union "
          "type\");\n";
      code += "    " + Name(enum_def) + " unionType = " + Name(enum_def) +
              ".NONE;\n";
      code += "    " + union_name + " result = new " + union_name + "();\n";
      code += "    while (reader.Read()) {\n";
      code +=
          "      if (reader.TokenType == "
          "System.Text.Json.JsonTokenType.EndObject) break;\n";
      code +=
          "      if (reader.TokenType != "
          "System.Text.Json.JsonTokenType.PropertyName) continue;\n";
      code += "      string propName = reader.GetString();\n";
      code += "      reader.Read();\n";
      code += "      if (propName == \"Type\") {\n";
      code += "        string typeName = reader.GetString();\n";
      code += "        if (System.Enum.TryParse<" + Name(enum_def) +
              ">(typeName, out var parsed)) unionType = parsed;\n";
      code += "      } else if (propName == \"Value\") {\n";
      code += "        result.Type = unionType;\n";
      code += "        switch (unionType) {\n";
      for (auto it = enum_def.Vals().begin(); it != enum_def.Vals().end();
           ++it) {
        auto &ev = **it;
        if (ev.union_type.base_type == BASE_TYPE_NONE) {
          code += "          default: break;\n";
        } else if (IsString(ev.union_type)) {
          code += "          case " + Name(enum_def) + "." + Name(ev) +
                  ": result." + class_member +
                  " = reader.GetString(); break;\n";
        } else {
          auto type_name = GenTypeGet_ObjectAPI(ev.union_type, opts);
          code += "          case " + Name(enum_def) + "." + Name(ev) +
                  ": result." + class_member +
                  " = System.Text.Json.JsonSerializer.Deserialize<" +
                  type_name + ">(ref reader, options); break;\n";
        }
      }
      code += "        }\n";
      code += "      }\n";
      code += "    }\n";
      code += "    return result;\n";
      code += "  }\n";

      // ReadWithTypeContext - kept for compatibility
      code +=
          "  public " + union_name +
          " ReadWithTypeContext(ref System.Text.Json.Utf8JsonReader reader, " +
          union_name +
          " _o, System.Text.Json.JsonSerializerOptions options) {\n";
      code += "    if (_o == null) return null;\n";
      code += "    switch (_o.Type) {\n";
      for (auto it = enum_def.Vals().begin(); it != enum_def.Vals().end();
           ++it) {
        auto &ev = **it;
        if (ev.union_type.base_type == BASE_TYPE_NONE) {
          code += "      default: break;\n";
        } else {
          auto type_name = GenTypeGet_ObjectAPI(ev.union_type, opts);
          code += "      case " + Name(enum_def) + "." + Name(ev) + ": _o." +
                  class_member +
                  " = System.Text.Json.JsonSerializer.Deserialize<" +
                  type_name + ">(ref reader, options); break;\n";
        }
      }
      code += "    }\n";
      code += "    return _o;\n";
      code += "  }\n";

      code +=
          "  public override void Write(System.Text.Json.Utf8JsonWriter "
          "writer, " +
          union_name +
          " value, System.Text.Json.JsonSerializerOptions options) {\n";
      code += "    if (value == null || value." + class_member +
              " == null) { writer.WriteNullValue(); return; }\n";
      code += "    writer.WriteStartObject();\n";
      code += "    writer.WriteString(\"Type\", value.Type.ToString());\n";
      code += "    writer.WritePropertyName(\"Value\");\n";
      code += "    System.Text.Json.JsonSerializer.Serialize(writer, value." +
              class_member + ", value." + class_member +
              ".GetType(), options);\n";
      code += "    writer.WriteEndObject();\n";
      code += "  }\n";
      code += "}\n\n";
    }
  }

  std::string GenTypeName_ObjectAPI(const std::string &name,
                                    const IDLOptions &opts) const {
    return opts.object_prefix + name + opts.object_suffix;
  }

  // Generate union unpacking code for both vector and non-vector unions.
  void GenUnionUnPack_ObjectAPI(const EnumDef &enum_def, std::string *code_ptr,
                                const std::string &camel_name,
                                const std::string &camel_name_short,
                                bool is_vector) const {
    auto &code = *code_ptr;
    std::string varialbe_name = "_o." + camel_name;
    std::string class_member = "Value";
    if (class_member == enum_def.name) class_member += "_";
    std::string indent = "    ";
    // Use base namespace for Object API types (they are shared between modes)
    auto enum_type_name = BaseNamespacedEnumName(enum_def);
    auto union_type_name = enum_type_name + "Union";

    if (is_vector) {
      varialbe_name = "_o_" + camel_name;
      indent = "        ";  // Inside if block and for loop
      // Cache the type element to avoid repeated span indexing
      code += indent + "var _" + camel_name_short + "_type = _" +
              camel_name_short + "_types_value[_j];\n";
      code +=
          indent + "var " + varialbe_name + " = _o." + camel_name + "[_j];\n";
      code += indent + "if (" + varialbe_name + " == null) {\n";
      code +=
          indent + "  " + varialbe_name + " = new " + union_type_name + "();\n";
      code += indent + "  _o." + camel_name + "[_j] = " + varialbe_name + ";\n";
      code += indent + "}\n";
      code += indent + "var _" + camel_name_short +
              "_old_type = " + varialbe_name + ".Type;\n";
      code +=
          indent + varialbe_name + ".Type = _" + camel_name_short + "_type;\n";
      code += indent + "switch (_" + camel_name_short + "_type) {\n";
    } else {
      // Check if we can reuse the existing union wrapper
      code += indent + "if (" + varialbe_name + " == null) {\n";
      code +=
          indent + "  " + varialbe_name + " = new " + union_type_name + "();\n";
      code += indent + "}\n";
      code += indent + "var _" + camel_name_short +
              "OldType = " + varialbe_name + ".Type;\n";
      code += indent + varialbe_name + ".Type = this." + camel_name_short +
              "Type;\n";
      code += indent + "switch (this." + camel_name_short + "Type) {\n";
    }

    for (auto eit = enum_def.Vals().begin(); eit != enum_def.Vals().end();
         ++eit) {
      auto &ev = **eit;
      if (ev.union_type.base_type == BASE_TYPE_NONE) {
        code += indent + "  default:\n";
        code +=
            indent + "    " + varialbe_name + "." + class_member + " = null;\n";
        code += indent + "    break;\n";
      } else {
        auto case_enum_value = enum_type_name + "." + Name(ev);
        code += indent + "  case " + case_enum_value + ":\n";
        if (IsString(ev.union_type)) {
          if (is_vector) {
            code += indent + "    " + varialbe_name + "." + class_member +
                    " = _" + camel_name_short + "_value.GetAsString(_j);\n";
          } else {
            code += indent + "    " + varialbe_name + "." + class_member +
                    " = this." + camel_name + "AsString();\n";
          }
        } else {
          // Try to reuse the inner object if the old type matches
          auto inner_type_name =
              BaseNamespacedName(*ev.union_type.struct_def) + "T";
          if (is_vector) {
            auto old_type_var = "_" + camel_name_short + "_old_type";
            code += indent + "    if (" + old_type_var +
                    " == " + case_enum_value + ") {\n";
            code += indent + "      _" + camel_name_short + "_value.GetAs<" +
                    GenTypeGet(ev.union_type) + ">(_j).UnPackTo((" +
                    inner_type_name + ")" + varialbe_name + "." + class_member +
                    ");\n";
            code += indent + "    } else {\n";
            code += indent + "      " + varialbe_name + "." + class_member +
                    " = _" + camel_name_short + "_value.GetAs<" +
                    GenTypeGet(ev.union_type) + ">(_j).UnPack();\n";
            code += indent + "    }\n";
          } else {
            auto old_type_var = "_" + camel_name_short + "OldType";
            code += indent + "    if (this." + camel_name + "<" +
                    GenTypeGet(ev.union_type) + ">().HasValue) {\n";
            code += indent + "      if (" + old_type_var +
                    " == " + case_enum_value + ") {\n";
            code += indent + "        this." + camel_name + "<" +
                    GenTypeGet(ev.union_type) + ">().Value.UnPackTo((" +
                    inner_type_name + ")" + varialbe_name + "." + class_member +
                    ");\n";
            code += indent + "      } else {\n";
            code += indent + "        " + varialbe_name + "." + class_member +
                    " = this." + camel_name + "<" + GenTypeGet(ev.union_type) +
                    ">().Value.UnPack();\n";
            code += indent + "      }\n";
            code += indent + "    } else {\n";
            code += indent + "      " + varialbe_name + "." + class_member +
                    " = null;\n";
            code += indent + "    }\n";
          }
        }
        code += indent + "    break;\n";
      }
    }
    code += indent + "}\n";
  }

  void GenPackUnPack_ObjectAPI(
      StructDef &struct_def, std::string *code_ptr, const IDLOptions &opts,
      bool struct_has_create,
      const std::set<FieldDef *> &field_has_create) const {
    auto &code = *code_ptr;
    auto struct_name = GenTypeName_ObjectAPI(struct_def.name, opts);
    // UnPack()
    code += "  public " + struct_name + " UnPack() {\n";
    code += "    var _o = new " + struct_name + "();\n";
    code += "    this.UnPackTo(_o);\n";
    code += "    return _o;\n";
    code += "  }\n";
    // UnPackTo()
    code += "  public void UnPackTo(" + struct_name + " _o) {\n";
    for (auto it = struct_def.fields.vec.begin();
         it != struct_def.fields.vec.end(); ++it) {
      auto &field = **it;
      if (field.deprecated) continue;
      auto camel_name = Name(field);
      if (camel_name == struct_def.name) { camel_name += "_"; }
      auto camel_name_short = Name(field);
      auto start = "    _o." + camel_name + " = ";
      switch (field.value.type.base_type) {
        case BASE_TYPE_STRUCT: {
          auto fixed = struct_def.fixed && field.value.type.struct_def->fixed;
          auto type_name = GenTypeGet_ObjectAPI(field.value.type, opts);
          if (fixed) {
            code += "    if (_o." + camel_name + " == null) _o." + camel_name +
                    " = new " + type_name + "();\n";
            code += "    this." + camel_name + ".UnPackTo(_o." + camel_name +
                    ");\n";
          } else {
            // Optional struct/table, reuse if it exists and has value
            code += "    if (this." + camel_name + ".HasValue) {\n";
            code += "      if (_o." + camel_name + " == null) _o." +
                    camel_name + " = new " + type_name + "();\n";
            code += "      this." + camel_name + ".Value.UnPackTo(_o." +
                    camel_name + ");\n";
            code += "    } else {\n";
            code += "      _o." + camel_name + " = null;\n";
            code += "    }\n";
          }
          break;
        }
        case BASE_TYPE_ARRAY: {
          auto type_name = GenTypeGet_ObjectAPI(field.value.type, opts);
          auto length_str = NumToString(field.value.type.fixed_length);
          auto unpack_method = field.value.type.struct_def == nullptr ? ""
                               : field.value.type.struct_def->fixed
                                   ? ".UnPack()"
                                   : "?.UnPack()";
          code += start + "new " + type_name.substr(0, type_name.length() - 1) +
                  length_str + "];\n";
          code += "    for (var _j = 0; _j < " + length_str + "; ++_j) { _o." +
                  camel_name + "[_j] = this." + camel_name + "(_j)" +
                  unpack_method + "; }\n";
          break;
        }
        case BASE_TYPE_VECTOR:
          if (field.value.type.element == BASE_TYPE_UNION) {
            auto type_name = GenTypeGet_ObjectAPI(field.value.type, opts);
            code += "    var _" + field.name + "_types = this." +
                    camel_name_short + "Type;\n";
            code += "    var _" + field.name + " = this." + camel_name + ";\n";
            code += "    var _" + field.name + "_len = _" + field.name +
                    ".HasValue ? _" + field.name + ".Value.Length : 0;\n";
            code += "    if (_o." + camel_name + " == null) {\n";
            code += "      _o." + camel_name + " = new " + type_name + "(_" +
                    field.name + "_len);\n";
            code += "    }\n";
            code += "    ObjectApiUtil.ResizeList(_o." + camel_name + ", _" +
                    field.name + "_len);\n";
            code += "    if (_" + field.name + ".HasValue) {\n";
            code += "      var _" + field.name + "_value = _" + field.name +
                    ".Value;\n";
            code += "      var _" + field.name + "_types_value = _" +
                    field.name + "_types.Value;\n";
            code += "      for (var _j = 0; _j < _" + field.name +
                    "_len; ++_j) {\n";
            GenUnionUnPack_ObjectAPI(*field.value.type.enum_def, code_ptr,
                                     camel_name, field.name, true);
            code += "      }\n";
            code += "    }\n";
          } else if (field.value.type.element != BASE_TYPE_UTYPE) {
            auto fixed = field.value.type.struct_def == nullptr;
            auto element_type = field.value.type.element;
            // Check if element is a primitive/blittable type that can use span
            // copy
            auto is_scalar_element =
                IsScalar(element_type) && element_type != BASE_TYPE_BOOL;
            auto is_enum_element = IsEnum(field.value.type.VectorType());

            bool use_property_access = (element_type == BASE_TYPE_STRUCT ||
                                        element_type == BASE_TYPE_STRING);

            if (fixed && (is_scalar_element || is_enum_element)) {
              // Use span-based copy for primitive/enum types
              auto type_name = GenTypeGet_ObjectAPI(field.value.type, opts);
              code += "    var _" + field.name + "_vec = this." + camel_name +
                      ";\n";
              code += "    var _" + field.name + "_len = _" + field.name +
                      "_vec.HasValue ? _" + field.name +
                      "_vec.Value.Length : 0;\n";
              code += "    if (_o." + camel_name + " == null) {\n";
              code += "      _o." + camel_name + " = new " + type_name + "(_" +
                      field.name + "_len);\n";
              code += "    }\n";
              code += "    ObjectApiUtil.ResizeList(_o." + camel_name + ", _" +
                      field.name + "_len);\n";
              code += "    if (_" + field.name + "_vec.HasValue) { _" +
                      field.name +
                      "_vec.Value.CopyTo(CollectionsMarshal.AsSpan(_o." +
                      camel_name + ")); }\n";
            } else if (fixed && use_property_access) {
              // Non-blittable primitives, reuse list if exists
              auto type_name = GenTypeGet_ObjectAPI(field.value.type, opts);
              code += "    var _" + field.name + "_vec = this." + camel_name +
                      ";\n";
              code += "    var _" + field.name + "_len = _" + field.name +
                      "_vec.HasValue ? _" + field.name +
                      "_vec.Value.Length : 0;\n";
              code += "    if (_o." + camel_name + " == null) {\n";
              code += "      _o." + camel_name + " = new " + type_name + "(_" +
                      field.name + "_len);\n";
              code += "    } else {\n";
              code += "      _o." + camel_name + ".Clear();\n";
              code += "      if (_o." + camel_name + ".Capacity < _" +
                      field.name + "_len) {\n";
              code += "        _o." + camel_name + ".Capacity = _" +
                      field.name + "_len;\n";
              code += "      }\n";
              code += "    }\n";
              code += "    if (_" + field.name + "_vec.HasValue) {\n";
              code += "      var _" + field.name + "_value = _" + field.name +
                      "_vec.Value;\n";
              code += "      for (var _j = 0; _j < _" + field.name +
                      "_len; ++_j) { _o." + camel_name + ".Add(_" + field.name +
                      "_value[_j]); }\n";
              code += "    }\n";
            } else if (fixed) {
              auto type_name = GenTypeGet_ObjectAPI(field.value.type, opts);
              code += "    var _" + field.name + "_vec = this." + camel_name +
                      ";\n";
              code += "    var _" + field.name + "_len = _" + field.name +
                      "_vec.HasValue ? _" + field.name +
                      "_vec.Value.Length : 0;\n";
              code += "    if (_o." + camel_name + " == null) {\n";
              code += "      _o." + camel_name + " = new " + type_name + "(_" +
                      field.name + "_len);\n";
              code += "    } else {\n";
              code += "      _o." + camel_name + ".Clear();\n";
              code += "      if (_o." + camel_name + ".Capacity < _" +
                      field.name + "_len) {\n";
              code += "        _o." + camel_name + ".Capacity = _" +
                      field.name + "_len;\n";
              code += "      }\n";
              code += "    }\n";
              code += "    if (_" + field.name + "_vec.HasValue) {\n";
              code += "      for (var _j = 0; _j < _" + field.name +
                      "_len; ++_j) { _o." + camel_name + ".Add(_" + field.name +
                      "_vec.Value[_j]); }\n";
              code += "    }\n";
            } else {
              // Structs/tables - reuse list and inner objects where possible
              auto type_name = GenTypeGet_ObjectAPI(field.value.type, opts);
              auto element_type_name = GenTypeName_ObjectAPI(
                  field.value.type.struct_def->name, opts);
              if (use_property_access) {
                code += "    var _" + field.name + "_vec = this." + camel_name +
                        ";\n";
                code += "    var _" + field.name + "_len = _" + field.name +
                        "_vec.HasValue ? _" + field.name +
                        "_vec.Value.Length : 0;\n";
                code += "    if (_o." + camel_name + " == null) {\n";
                code += "      _o." + camel_name + " = new " + type_name +
                        "(_" + field.name + "_len);\n";
                code += "    }\n";
                code += "    ObjectApiUtil.ResizeList(_o." + camel_name +
                        ", _" + field.name + "_len);\n";
                code += "    if (_" + field.name + "_vec.HasValue) {\n";
                code += "      var _" + field.name + "_value = _" + field.name +
                        "_vec.Value;\n";
                code += "      for (var _j = 0; _j < _" + field.name +
                        "_len; ++_j) {\n";
                code += "        var _src = _" + field.name + "_value[_j];\n";
                code += "        if (_o." + camel_name + "[_j] == null) { _o." +
                        camel_name + "[_j] = new " + element_type_name +
                        "(); }\n";
                code += "        _src.UnPackTo(_o." + camel_name + "[_j]);\n";
                code += "      }\n";
                code += "    }\n";
              } else {
                code += "    var _" + field.name + "_len = this." + camel_name +
                        "Length;\n";
                code += "    if (_o." + camel_name + " == null) {\n";
                code += "      _o." + camel_name + " = new " + type_name +
                        "(_" + field.name + "_len);\n";
                code += "    }\n";
                code += "    ObjectApiUtil.ResizeList(_o." + camel_name +
                        ", _" + field.name + "_len);\n";
                code += "    for (var _j = 0; _j < _" + field.name +
                        "_len; ++_j) {\n";
                code += "      var _src = this." + camel_name + "(_j);\n";
                code += "      if (_src.HasValue) {\n";
                code += "        if (_o." + camel_name + "[_j] == null) { _o." +
                        camel_name + "[_j] = new " + element_type_name +
                        "(); }\n";
                code +=
                    "        _src.Value.UnPackTo(_o." + camel_name + "[_j]);\n";
                code += "      } else {\n";
                code += "        _o." + camel_name + "[_j] = null;\n";
                code += "      }\n";
                code += "    }\n";
              }
            }
          }
          break;
        case BASE_TYPE_UTYPE: break;
        case BASE_TYPE_UNION: {
          GenUnionUnPack_ObjectAPI(*field.value.type.enum_def, code_ptr,
                                   camel_name, camel_name_short, false);
          break;
        }
        default: {
          code += start + "this." + camel_name + ";\n";
          break;
        }
      }
    }
    code += "  }\n";
    // Pack()
    code += "  public static " + GenOffsetType(struct_def) + " Pack(" +
            BuilderParam() + ", " + struct_name + " _o) {\n";
    code += "    if (_o == null) return default(" + GenOffsetType(struct_def) +
            ");\n";
    for (auto it = struct_def.fields.vec.begin();
         it != struct_def.fields.vec.end(); ++it) {
      auto &field = **it;
      if (field.deprecated) continue;
      auto camel_name = Name(field);
      if (camel_name == struct_def.name) { camel_name += "_"; }
      auto camel_name_short = Name(field);
      // pre
      switch (field.value.type.base_type) {
        case BASE_TYPE_STRUCT: {
          if (!field.value.type.struct_def->fixed) {
            code += "    var _" + field.name + " = _o." + camel_name +
                    " == null ? default(" +
                    GenOffsetType(*field.value.type.struct_def) +
                    ") : " + GenTypeGet(field.value.type) + ".Pack(" +
                    BuilderArg() + ", _o." + camel_name + ");\n";
          } else if (struct_def.fixed && struct_has_create) {
            std::vector<FieldArrayLength> array_lengths;
            FieldArrayLength tmp_array_length = {
              field.name,
              field.value.type.fixed_length,
            };
            array_lengths.push_back(tmp_array_length);
            GenStructPackDecl_ObjectAPI(*field.value.type.struct_def, code_ptr,
                                        array_lengths);
          }
          break;
        }
        case BASE_TYPE_STRING: {
          std::string create_string =
              field.shared ? "CreateSharedString" : "CreateString";
          code += "    var _" + field.name + " = _o." + camel_name +
                  " == null ? default(StringOffset) : "
                  "builder." +
                  create_string + "(_o." + camel_name + ");\n";
          break;
        }
        case BASE_TYPE_VECTOR: {
          if (field_has_create.find(&field) != field_has_create.end()) {
            auto property_name = camel_name;
            auto gen_for_loop = true;
            std::string array_name = "__" + field.name;
            std::string array_type = "";
            std::string to_array = "";
            std::string element_size = "4";  // default for offsets and ints
            switch (field.value.type.element) {
              case BASE_TYPE_STRING: {
                std::string create_string =
                    field.shared ? "CreateSharedString" : "CreateString";
                array_type = "StringOffset";
                to_array += "builder." + create_string + "(_o." +
                            property_name + "[_j])";
                element_size = "4";
                break;
              }
              case BASE_TYPE_STRUCT:
                array_type = "Offset<" + GenTypeGet(field.value.type) + ">";
                to_array = GenTypeGet(field.value.type) + ".Pack(" +
                           BuilderArg() + ", _o." + property_name + "[_j])";
                element_size = "4";
                break;
              case BASE_TYPE_UTYPE:
                property_name = camel_name.substr(0, camel_name.size() - 4);
                // Union enum types are always in base namespace
                array_type = BaseNamespacedEnumName(*field.value.type.enum_def);
                to_array = "_o." + property_name + "[_j].Type";
                // Enum size is typically 1 byte (sbyte/byte)
                element_size = "1";
                break;
              case BASE_TYPE_UNION:
                array_type = "int";
                // Union wrapper types are always in base namespace
                to_array = BaseNamespacedEnumName(*field.value.type.enum_def) +
                           "Union.Pack(" + BuilderArg() + ",  _o." +
                           property_name + "[_j])";
                element_size = "4";
                break;
              default: gen_for_loop = false; break;
            }
            code += "    var _" + field.name + " = default(VectorOffset);\n";
            code += "    if (_o." + property_name + " != null) {\n";
            if (gen_for_loop) {
              // Use stackalloc for small arrays, ArrayPool for larger ones
              const int stackalloc_threshold_bytes = 256;
              std::string stackalloc_threshold = std::to_string(
                  stackalloc_threshold_bytes / std::stoi(element_size));
              std::string count_expr = "_o." + property_name + ".Count";
              code +=
                  "      var _" + field.name + "_len = " + count_expr + ";\n";
              code += "      " + array_type + "[] _" + field.name +
                      "_arr = null;\n";
              code += "      try {\n";
              code += "        Span<" + array_type + "> " + array_name +
                      " = _" + field.name + "_len <= " + stackalloc_threshold +
                      "\n";
              code += "          ? stackalloc " + array_type + "[_" +
                      field.name + "_len]\n";
              code += "          : (_" + field.name + "_arr = ArrayPool<" +
                      array_type + ">.Shared.Rent(_" + field.name +
                      "_len)).AsSpan(0, _" + field.name + "_len);\n";
              code += "        for (var _j = 0; _j < _" + field.name +
                      "_len; ++_j) { ";
              code += array_name + "[_j] = " + to_array + "; }\n";
              code += "        _" + field.name + " = Create" +
                      camel_name_short + "Vector(" + BuilderArg() + ", " +
                      array_name + ");\n";
              code += "      } finally {\n";
              code += "        if (_" + field.name +
                      "_arr != null) { ArrayPool<" + array_type +
                      ">.Shared.Return(_" + field.name + "_arr); }\n";
              code += "      }\n";
            } else {
              code += "      _" + field.name + " = Create" + camel_name_short +
                      "Vector(" + BuilderArg() +
                      ", CollectionsMarshal.AsSpan(_o." + property_name +
                      "));\n";
            }
            code += "    }\n";
          } else {
            auto pack_method =
                field.value.type.struct_def == nullptr
                    ? "builder.Add" + GenMethod(field.value.type.VectorType()) +
                          "(_o." + camel_name + "[_j]);"
                    : GenTypeGet(field.value.type) + ".Pack(" + BuilderArg() +
                          ", _o." + camel_name + "[_j]);";
            code += "    var _" + field.name + " = default(VectorOffset);\n";
            code += "    if (_o." + camel_name + " != null) {\n";
            code += "      Start" + camel_name_short + "Vector(" +
                    BuilderArg() + ", _o." + camel_name + ".Count);\n";
            code += "      for (var _j = _o." + camel_name +
                    ".Count - 1; _j >= 0; --_j) { " + pack_method + " }\n";
            code += "      _" + field.name + " = builder.EndVector();\n";
            code += "    }\n";
          }
          break;
        }
        case BASE_TYPE_ARRAY: {
          if (field.value.type.struct_def != nullptr) {
            std::vector<FieldArrayLength> array_lengths;
            FieldArrayLength tmp_array_length = {
              field.name,
              field.value.type.fixed_length,
            };
            array_lengths.push_back(tmp_array_length);
            GenStructPackDecl_ObjectAPI(*field.value.type.struct_def, code_ptr,
                                        array_lengths);
          } else {
            code += "    var _" + field.name + " = _o." + camel_name + ";\n";
          }
          break;
        }
        case BASE_TYPE_UNION: {
          // Union enums and wrappers are always in base namespace (shared
          // between modes)
          code += "    var _" + field.name + "_type = _o." + camel_name +
                  " == null ? " +
                  BaseNamespacedEnumName(*field.value.type.enum_def) +
                  ".NONE : " + "_o." + camel_name + ".Type;\n";
          code +=
              "    var _" + field.name + " = _o." + camel_name +
              " == null ? 0 : " + GenTypeGet_ObjectAPI(field.value.type, opts) +
              ".Pack(" + BuilderArg() + ", _o." + camel_name + ");\n";
          break;
        }
        default: break;
      }
    }
    if (struct_has_create) {
      // Create
      code += "    return Create" + struct_def.name + "(\n";
      code += "      " + BuilderArg();
      for (auto it = struct_def.fields.vec.begin();
           it != struct_def.fields.vec.end(); ++it) {
        auto &field = **it;
        if (field.deprecated) continue;
        auto camel_name = Name(field);
        if (camel_name == struct_def.name) { camel_name += "_"; }
        switch (field.value.type.base_type) {
          case BASE_TYPE_STRUCT: {
            if (struct_def.fixed) {
              GenStructPackCall_ObjectAPI(*field.value.type.struct_def,
                                          code_ptr,
                                          "      _" + field.name + "_");
            } else {
              code += ",\n";
              if (field.value.type.struct_def->fixed) {
                if (opts.generate_object_based_api)
                  code += "      _o." + camel_name;
                else
                  code += "      " + GenTypeGet(field.value.type) + ".Pack(" +
                          BuilderArg() + ", _o." + camel_name + ")";
              } else {
                code += "      _" + field.name;
              }
            }
            break;
          }
          case BASE_TYPE_ARRAY: {
            if (field.value.type.struct_def != nullptr) {
              GenStructPackCall_ObjectAPI(*field.value.type.struct_def,
                                          code_ptr,
                                          "      _" + field.name + "_");
            } else {
              code += ",\n";
              code += "      _" + field.name;
            }
            break;
          }
          case BASE_TYPE_UNION: FLATBUFFERS_FALLTHROUGH();   // fall thru
          case BASE_TYPE_UTYPE: FLATBUFFERS_FALLTHROUGH();   // fall thru
          case BASE_TYPE_STRING: FLATBUFFERS_FALLTHROUGH();  // fall thru
          case BASE_TYPE_VECTOR: {
            code += ",\n";
            code += "      _" + field.name;
            break;
          }
          default:  // scalar
            code += ",\n";
            code += "      _o." + camel_name;
            break;
        }
      }
      code += ");\n";
    } else {
      // Start, End
      code += "    Start" + struct_def.name + "(" + BuilderArg() + ");\n";
      for (auto it = struct_def.fields.vec.begin();
           it != struct_def.fields.vec.end(); ++it) {
        auto &field = **it;
        if (field.deprecated) continue;
        auto camel_name = Name(field);
        switch (field.value.type.base_type) {
          case BASE_TYPE_STRUCT: {
            if (field.value.type.struct_def->fixed) {
              code += "    Add" + camel_name + "(" + BuilderArg() + ", " +
                      GenTypeGet(field.value.type) + ".Pack(" + BuilderArg() +
                      ", _o." + camel_name + "));\n";
            } else {
              code += "    Add" + camel_name + "(" + BuilderArg() + ", _" +
                      field.name + ");\n";
            }
            break;
          }
          case BASE_TYPE_STRING: FLATBUFFERS_FALLTHROUGH();  // fall thru
          case BASE_TYPE_ARRAY: FLATBUFFERS_FALLTHROUGH();   // fall thru
          case BASE_TYPE_VECTOR: {
            code += "    Add" + camel_name + "(" + BuilderArg() + ", _" +
                    field.name + ");\n";
            break;
          }
          case BASE_TYPE_UTYPE: break;
          case BASE_TYPE_UNION: {
            code += "    Add" + camel_name + "Type(" + BuilderArg() + ", _" +
                    field.name + "_type);\n";
            code += "    Add" + camel_name + "(" + BuilderArg() + ", _" +
                    field.name + ");\n";
            break;
          }
          // scalar
          default: {
            code += "    Add" + camel_name + "(" + BuilderArg() + ", _o." +
                    camel_name + ");\n";
            break;
          }
        }
      }
      code += "    return End" + struct_def.name + "(" + BuilderArg() + ");\n";
    }
    code += "  }\n";
  }

  void GenStructPackDecl_ObjectAPI(
      const StructDef &struct_def, std::string *code_ptr,
      std::vector<FieldArrayLength> &array_lengths) const {
    auto &code = *code_ptr;
    for (auto it = struct_def.fields.vec.begin();
         it != struct_def.fields.vec.end(); ++it) {
      auto &field = **it;
      auto is_array = IsArray(field.value.type);
      const auto &field_type =
          is_array ? field.value.type.VectorType() : field.value.type;
      FieldArrayLength tmp_array_length = {
        field.name,
        field_type.fixed_length,
      };
      array_lengths.push_back(tmp_array_length);
      if (field_type.struct_def != nullptr) {
        GenStructPackDecl_ObjectAPI(*field_type.struct_def, code_ptr,
                                    array_lengths);
      } else {
        std::vector<FieldArrayLength> array_only_lengths;
        for (size_t i = 0; i < array_lengths.size(); ++i) {
          if (array_lengths[i].length > 0) {
            array_only_lengths.push_back(array_lengths[i]);
          }
        }
        std::string name;
        for (size_t i = 0; i < array_lengths.size(); ++i) {
          name += "_" + array_lengths[i].name;
        }
        code += "    var " + name + " = ";
        if (array_only_lengths.size() > 0) {
          code += "new " + GenTypeBasic(field_type) + "[";
          for (size_t i = 0; i < array_only_lengths.size(); ++i) {
            if (i != 0) { code += ","; }
            code += NumToString(array_only_lengths[i].length);
          }
          code += "];\n";
          code += "    ";
          // initialize array
          for (size_t i = 0; i < array_only_lengths.size(); ++i) {
            auto idx = "idx" + NumToString(i);
            code += "for (var " + idx + " = 0; " + idx + " < " +
                    NumToString(array_only_lengths[i].length) + "; ++" + idx +
                    ") {";
          }
          for (size_t i = 0; i < array_only_lengths.size(); ++i) {
            auto idx = "idx" + NumToString(i);
            if (i == 0) {
              code += name + "[" + idx;
            } else {
              code += "," + idx;
            }
          }
          code += "] = _o";
          for (size_t i = 0, j = 0; i < array_lengths.size(); ++i) {
            code += "." + ConvertCase(array_lengths[i].name, Case::kUpperCamel);
            if (array_lengths[i].length <= 0) continue;
            code += "[idx" + NumToString(j++) + "]";
          }
          code += ";";
          for (size_t i = 0; i < array_only_lengths.size(); ++i) {
            code += "}";
          }
        } else {
          code += "_o";
          for (size_t i = 0; i < array_lengths.size(); ++i) {
            code += "." + ConvertCase(array_lengths[i].name, Case::kUpperCamel);
          }
          code += ";";
        }
        code += "\n";
      }
      array_lengths.pop_back();
    }
  }

  void GenStructPackCall_ObjectAPI(const StructDef &struct_def,
                                   std::string *code_ptr,
                                   std::string prefix) const {
    auto &code = *code_ptr;
    for (auto it = struct_def.fields.vec.begin();
         it != struct_def.fields.vec.end(); ++it) {
      auto &field = **it;
      const auto &field_type = field.value.type;
      if (field_type.struct_def != nullptr) {
        GenStructPackCall_ObjectAPI(*field_type.struct_def, code_ptr,
                                    prefix + field.name + "_");
      } else {
        code += ",\n";
        code += prefix + field.name;
      }
    }
  }

  std::string GenTypeGet_ObjectAPI(flatbuffers::Type type,
                                   const IDLOptions &opts) const {
    // Object API types always use base namespace
    // because *T classes and union wrappers are shared between modes
    switch (type.base_type) {
      case BASE_TYPE_STRUCT: {
        // For structs, return the *T type in base namespace
        return WrapInNameSpace(
            type.struct_def->defined_namespace,
            GenTypeName_ObjectAPI(type.struct_def->name, opts));
      }
      case BASE_TYPE_ARRAY: {
        if (type.struct_def != nullptr) {
          return WrapInNameSpace(
                     type.struct_def->defined_namespace,
                     GenTypeName_ObjectAPI(type.struct_def->name, opts)) +
                 "[]";
        }
        // Scalar array - use enum type for enums
        auto element_type = type.VectorType();
        if (IsEnum(element_type)) {
          return BaseNamespacedEnumName(*element_type.enum_def) + "[]";
        }
        return GenTypeBasic(element_type, false) + "[]";
      }
      case BASE_TYPE_VECTOR: {
        std::string element_type;
        if (type.struct_def != nullptr) {
          element_type = WrapInNameSpace(
              type.struct_def->defined_namespace,
              GenTypeName_ObjectAPI(type.struct_def->name, opts));
        } else if (type.element == BASE_TYPE_UNION) {
          element_type = BaseNamespacedEnumName(*type.enum_def) + "Union";
        } else if (type.element == BASE_TYPE_STRING) {
          element_type = "string";
        } else if (IsEnum(type.VectorType())) {
          // Use enum type for enum vectors
          element_type = BaseNamespacedEnumName(*type.VectorType().enum_def);
        } else {
          // Scalar element
          element_type = GenTypeBasic(type.VectorType(), false);
        }
        return "List<" + element_type + ">";
      }
      case BASE_TYPE_UNION: {
        return BaseNamespacedEnumName(*type.enum_def) + "Union";
      }
      case BASE_TYPE_STRING: {
        return "string";
      }
      default: {
        // Scalars use underlying type, enums use enum type with base namespace
        if (IsEnum(type)) { return BaseNamespacedEnumName(*type.enum_def); }
        return GenTypeBasic(type, false);
      }
    }
  }

  void GenStruct_ObjectAPI(StructDef &struct_def, std::string *code_ptr,
                           const IDLOptions &opts) const {
    auto &code = *code_ptr;
    if (struct_def.attributes.Lookup("private")) {
      code += "internal ";
    } else {
      code += "public ";
    }
    if (struct_def.attributes.Lookup("csharp_partial")) {
      // generate a partial class for this C# struct/table
      code += "partial ";
    }
    auto class_name = GenTypeName_ObjectAPI(struct_def.name, opts);
    code += "class " + class_name;
    code += "\n{\n";
    // Generate Properties
    for (auto it = struct_def.fields.vec.begin();
         it != struct_def.fields.vec.end(); ++it) {
      auto &field = **it;
      if (field.deprecated) continue;
      if (field.value.type.base_type == BASE_TYPE_UTYPE) continue;
      if (field.value.type.element == BASE_TYPE_UTYPE) continue;
      auto type_name = GenTypeGet_ObjectAPI(field.value.type, opts);
      if (field.IsScalarOptional()) type_name += "?";
      auto camel_name = Name(field);
      if (camel_name == struct_def.name) { camel_name += "_"; }
      if (opts.cs_gen_json_serializer) {
        if (IsUnion(field.value.type)) {
          auto utype_name = NamespacedName(*field.value.type.enum_def);
          code += "  [System.Text.Json.Serialization.JsonPropertyName(\"" +
                  field.name + "_type\")]\n";
          if (IsVector(field.value.type)) {
            code += "  private " + utype_name + "[] " + camel_name + "Type {\n";
            code += "    get {\n";
            code += "      if (this." + camel_name + " == null) return null;\n";
            code += "      var _o = new " + utype_name + "[this." + camel_name +
                    ".Count];\n";
            code +=
                "      for (var _j = 0; _j < _o.Length; ++_j) { _o[_j] = "
                "this." +
                camel_name + "[_j].Type; }\n";
            code += "      return _o;\n";
            code += "    }\n";
            code += "    set {\n";
            code += "      this." + camel_name + " = new List<" + utype_name +
                    "Union>();\n";
            code += "      for (var _j = 0; _j < value.Length; ++_j) {\n";
            code += "        var _o = new " + utype_name + "Union();\n";
            code += "        _o.Type = value[_j];\n";
            code += "        this." + camel_name + ".Add(_o);\n";
            code += "      }\n";
            code += "    }\n";
            code += "  }\n";
          } else {
            code += "  private " + utype_name + " " + camel_name + "Type {\n";
            code += "    get {\n";
            code += "      return this." + camel_name + " != null ? this." +
                    camel_name + ".Type : " + utype_name + ".NONE;\n";
            code += "    }\n";
            code += "    set {\n";
            code += "      this." + camel_name + " = new " + utype_name +
                    "Union();\n";
            code += "      this." + camel_name + ".Type = value;\n";
            code += "    }\n";
            code += "  }\n";
          }
        }
        code += "  [System.Text.Json.Serialization.JsonPropertyName(\"" +
                field.name + "\")]\n";
        // Only apply JsonConverter for non-vector unions
        // For vector unions, the list items will use their converter
        // automatically
        if (IsUnion(field.value.type) && !IsVector(field.value.type)) {
          code += "  [System.Text.Json.Serialization.JsonConverter(typeof(" +
                  type_name + "_JsonConverter))]\n";
        }
        if (field.attributes.Lookup("hash")) {
          code += "  [System.Text.Json.Serialization.JsonIgnore]\n";
        }
      }
      code += "  public " + type_name + " " + camel_name + " { get; set; }\n";
    }
    // Generate Constructor
    code += "\n";
    code += "  public " + class_name + "() {\n";
    for (auto it = struct_def.fields.vec.begin();
         it != struct_def.fields.vec.end(); ++it) {
      auto &field = **it;
      if (field.deprecated) continue;
      if (field.value.type.base_type == BASE_TYPE_UTYPE) continue;
      if (field.value.type.element == BASE_TYPE_UTYPE) continue;
      auto camel_name = Name(field);
      if (camel_name == struct_def.name) { camel_name += "_"; }
      code += "    this." + camel_name + " = ";
      auto type_name = GenTypeGet_ObjectAPI(field.value.type, opts);
      if (IsScalar(field.value.type.base_type)) {
        code += GenDefaultValue(field) + ";\n";
      } else {
        switch (field.value.type.base_type) {
          case BASE_TYPE_STRUCT: {
            if (IsStruct(field.value.type)) {
              code += "new " + type_name + "();\n";
            } else {
              code += "null;\n";
            }
            break;
          }
          case BASE_TYPE_ARRAY: {
            code += "new " + type_name.substr(0, type_name.length() - 1) +
                    NumToString(field.value.type.fixed_length) + "];\n";
            break;
          }
          default: {
            code += "null;\n";
            break;
          }
        }
      }
    }
    code += "  }\n";
    // Generate Serialization
    if (opts.cs_gen_json_serializer &&
        parser_.root_struct_def_ == &struct_def) {
      code += "\n";
      code +=
          "  private static readonly System.Text.Json.JsonSerializerOptions "
          "_jsonOptions = new System.Text.Json.JsonSerializerOptions { "
          "WriteIndented = true, NumberHandling = "
          "System.Text.Json.Serialization.JsonNumberHandling."
          "AllowNamedFloatingPointLiterals, Converters = { new "
          "System.Text.Json.Serialization.JsonStringEnumConverter() } };\n";
      code += "  public static " + class_name +
              " DeserializeFromJson(string jsonText) {\n";
      code += "    return System.Text.Json.JsonSerializer.Deserialize<" +
              class_name + ">(jsonText, _jsonOptions);\n";
      code += "  }\n";
      code += "  public string SerializeToJson() {\n";
      code +=
          "    return System.Text.Json.JsonSerializer.Serialize(this, "
          "_jsonOptions);\n";
      code += "  }\n";
    }
    if (parser_.root_struct_def_ == &struct_def) {
      code += "  public static " + class_name +
              " DeserializeFromBinary(byte[] fbBuffer) {\n";
      code += "    return " + struct_def.name + ".GetRootAs" + struct_def.name +
              "(new ByteBuffer(fbBuffer)).UnPack();\n";
      code += "  }\n";
      code += "  public byte[] SerializeToBinary() {\n";
      code += "    var fbb = new FlatBufferBuilder(0x10000);\n";
      code += "    " + struct_def.name + ".Finish" + struct_def.name +
              "Buffer(fbb, " + struct_def.name + ".Pack(fbb, this));\n";
      code += "    return fbb.DataBuffer.ToSizedSpan().ToArray();\n";
      code += "  }\n";
    }
    code += "}\n\n";
  }

  // This tracks the current namespace used to determine if a type need to be
  // prefixed by its namespace
  const Namespace *cur_name_space_;
};
}  // namespace csharpspanbufs

static bool GenerateCSharp(const Parser &parser, const std::string &path,
                           const std::string &file_name) {
  csharpspanbufs::CSharpSpanBufsGenerator generator(parser, path, file_name);
  return generator.generate();
}

namespace {

class CSharpSpanBufsCodeGenerator : public CodeGenerator {
 public:
  Status GenerateCode(const Parser &parser, const std::string &path,
                      const std::string &filename) override {
    if (!GenerateCSharp(parser, path, filename)) { return Status::ERROR; }
    return Status::OK;
  }

  Status GenerateCode(const uint8_t *, int64_t,
                      const CodeGenOptions &) override {
    return Status::NOT_IMPLEMENTED;
  }

  Status GenerateMakeRule(const Parser &parser, const std::string &path,
                          const std::string &filename,
                          std::string &output) override {
    output = JavaCSharpMakeRule(false, parser, path, filename);
    return Status::OK;
  }

  Status GenerateGrpcCode(const Parser &parser, const std::string &path,
                          const std::string &filename) override {
    (void)parser;
    (void)path;
    (void)filename;
    return Status::NOT_IMPLEMENTED;
  }

  Status GenerateRootFile(const Parser &parser,
                          const std::string &path) override {
    (void)parser;
    (void)path;
    return Status::NOT_IMPLEMENTED;
  }

  bool IsSchemaOnly() const override { return true; }

  bool SupportsBfbsGeneration() const override { return false; }

  bool SupportsRootFileGeneration() const override { return false; }

  IDLOptions::Language Language() const override {
    return IDLOptions::kCSharpSpanBufs;
  }

  std::string LanguageName() const override { return "CSharp"; }
};
}  // namespace

std::unique_ptr<CodeGenerator> NewCSharpSpanBufsCodeGenerator() {
  return std::unique_ptr<CSharpSpanBufsCodeGenerator>(
      new CSharpSpanBufsCodeGenerator());
}

}  // namespace flatbuffers
