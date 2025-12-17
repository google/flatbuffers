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

#include "idl_gen_json_schema.h"

#include <algorithm>
#include <iostream>
#include <limits>

#include "flatbuffers/code_generators.h"
#include "flatbuffers/idl.h"
#include "flatbuffers/util.h"

namespace flatbuffers {

namespace jsons {

namespace {

template <class T>
static std::string GenFullName(const T* enum_def) {
  std::string full_name;
  const auto& name_spaces = enum_def->defined_namespace->components;
  for (auto ns = name_spaces.cbegin(); ns != name_spaces.cend(); ++ns) {
    full_name.append(*ns + "_");
  }
  full_name.append(enum_def->name);
  return full_name;
}

template <class T>
static std::string GenTypeRef(const T* enum_def) {
  return "\"$ref\" : \"#/definitions/" + GenFullName(enum_def) + "\"";
}

static std::string GenType(const std::string& name) {
  return "\"type\" : \"" + name + "\"";
}

static std::string GenType(BaseType type) {
  switch (type) {
    case BASE_TYPE_BOOL:
      return "\"type\" : \"boolean\"";
    case BASE_TYPE_CHAR:
      return "\"type\" : \"integer\", \"minimum\" : " +
             NumToString(std::numeric_limits<int8_t>::min()) +
             ", \"maximum\" : " +
             NumToString(std::numeric_limits<int8_t>::max());
    case BASE_TYPE_UCHAR:
      return "\"type\" : \"integer\", \"minimum\" : 0, \"maximum\" :" +
             NumToString(std::numeric_limits<uint8_t>::max());
    case BASE_TYPE_SHORT:
      return "\"type\" : \"integer\", \"minimum\" : " +
             NumToString(std::numeric_limits<int16_t>::min()) +
             ", \"maximum\" : " +
             NumToString(std::numeric_limits<int16_t>::max());
    case BASE_TYPE_USHORT:
      return "\"type\" : \"integer\", \"minimum\" : 0, \"maximum\" : " +
             NumToString(std::numeric_limits<uint16_t>::max());
    case BASE_TYPE_INT:
      return "\"type\" : \"integer\", \"minimum\" : " +
             NumToString(std::numeric_limits<int32_t>::min()) +
             ", \"maximum\" : " +
             NumToString(std::numeric_limits<int32_t>::max());
    case BASE_TYPE_UINT:
      return "\"type\" : \"integer\", \"minimum\" : 0, \"maximum\" : " +
             NumToString(std::numeric_limits<uint32_t>::max());
    case BASE_TYPE_LONG:
      return "\"type\" : \"integer\", \"minimum\" : " +
             NumToString(std::numeric_limits<int64_t>::min()) +
             ", \"maximum\" : " +
             NumToString(std::numeric_limits<int64_t>::max());
    case BASE_TYPE_ULONG:
      return "\"type\" : \"integer\", \"minimum\" : 0, \"maximum\" : " +
             NumToString(std::numeric_limits<uint64_t>::max());
    case BASE_TYPE_FLOAT:
    case BASE_TYPE_DOUBLE:
      return "\"type\" : \"number\"";
    case BASE_TYPE_STRING:
      return "\"type\" : \"string\"";
    default:
      return "";
  }
}

static std::string GenBaseType(const Type& type) {
  if (type.struct_def != nullptr) {
    return GenTypeRef(type.struct_def);
  }
  if (type.enum_def != nullptr) {
    return GenTypeRef(type.enum_def);
  }
  return GenType(type.base_type);
}

static std::string GenArrayType(const Type& type) {
  std::string element_type;
  if (type.struct_def != nullptr) {
    element_type = GenTypeRef(type.struct_def);
  } else if (type.enum_def != nullptr) {
    element_type = GenTypeRef(type.enum_def);
  } else {
    element_type = GenType(type.element);
  }

  return "\"type\" : \"array\", \"items\" : {" + element_type + "}";
}

static std::string GenType(const Type& type) {
  switch (type.base_type) {
    case BASE_TYPE_ARRAY:
      FLATBUFFERS_FALLTHROUGH();  // fall thru
    case BASE_TYPE_VECTOR: {
      return GenArrayType(type);
    }
    case BASE_TYPE_STRUCT: {
      return GenTypeRef(type.struct_def);
    }
    case BASE_TYPE_UNION: {
      std::string union_type_string("\"anyOf\": [");
      const auto& union_types = type.enum_def->Vals();
      for (auto ut = union_types.cbegin(); ut < union_types.cend(); ++ut) {
        const auto& union_type = *ut;
        if (union_type->union_type.base_type == BASE_TYPE_NONE) {
          continue;
        }
        if (union_type->union_type.base_type == BASE_TYPE_STRUCT) {
          union_type_string.append(
              "{ " + GenTypeRef(union_type->union_type.struct_def) + " }");
        }
        if (union_type != *type.enum_def->Vals().rbegin()) {
          union_type_string.append(",");
        }
      }
      union_type_string.append("]");
      return union_type_string;
    }
    case BASE_TYPE_UTYPE:
      return GenTypeRef(type.enum_def);
    default: {
      return GenBaseType(type);
    }
  }
}

}  // namespace

class JsonSchemaGenerator : public BaseGenerator {
 private:
  std::string code_;

  bool EmitXFlatbuffersMetadata() const {
    return parser_.opts.jsonschema_include_xflatbuffers;
  }

  std::string JsonString(const std::string& value) const {
    std::string escaped;
    if (EscapeString(value.c_str(), value.length(), &escaped,
                     parser_.opts.allow_non_utf8, parser_.opts.natural_utf8)) {
      return escaped;
    }
    return "\"\"";
  }

  static std::string JoinStrings(const std::vector<std::string>& parts,
                                 const std::string& separator) {
    std::string output;
    for (size_t i = 0; i < parts.size(); ++i) {
      output += parts[i];
      if (i + 1 < parts.size()) output += separator;
    }
    return output;
  }

  static std::string BaseTypeNameForMetadata(BaseType type) {
    // Prefer schema-surface type names when available.
    if (type == BASE_TYPE_UTYPE) return "utype";

    const auto* name = TypeName(type);
    if (name && *name) return name;

    switch (type) {
      case BASE_TYPE_VECTOR: return "vector";
      case BASE_TYPE_VECTOR64: return "vector64";
      case BASE_TYPE_ARRAY: return "array";
      case BASE_TYPE_STRUCT: return "struct";
      case BASE_TYPE_UNION: return "union";
      default: return "";
    }
  }

  std::string NamespaceArrayForMetadata(const Namespace* ns) const {
    if (ns == nullptr || ns->components.empty()) return "[]";

    std::vector<std::string> components;
    components.reserve(ns->components.size());
    for (const auto& component : ns->components) {
      components.push_back(JsonString(component));
    }
    return "[" + JoinStrings(components, ", ") + "]";
  }

  std::string TypeObjectForMetadata(const Type& type) const {
    std::vector<std::string> parts;

    auto add_kv = [&](const std::string& key, const std::string& value) {
      parts.push_back("\"" + key + "\" : " + value);
    };

    if (type.base_type == BASE_TYPE_STRUCT && type.struct_def != nullptr) {
      add_kv("base", JsonString(type.struct_def->fixed ? "struct" : "table"));
      add_kv("ref", JsonString(GenFullName(type.struct_def)));
    } else if (type.base_type == BASE_TYPE_UNION && type.enum_def != nullptr) {
      add_kv("base", JsonString("union"));
      add_kv("ref", JsonString(GenFullName(type.enum_def)));
    } else if (type.base_type == BASE_TYPE_UTYPE && type.enum_def != nullptr) {
      add_kv("base", JsonString("utype"));
      add_kv("ref", JsonString(GenFullName(type.enum_def)));
    } else if (type.base_type == BASE_TYPE_VECTOR ||
               type.base_type == BASE_TYPE_VECTOR64 ||
               type.base_type == BASE_TYPE_ARRAY) {
      add_kv("base", JsonString(type.base_type == BASE_TYPE_ARRAY ? "array"
                                                                  : "vector"));
      if (type.base_type == BASE_TYPE_VECTOR64) add_kv("vector64", "true");
      if (type.base_type == BASE_TYPE_ARRAY) {
        add_kv("fixed_length", NumToString(type.fixed_length));
      }
      add_kv("element", TypeObjectForMetadata(type.VectorType()));
    } else if (type.enum_def != nullptr) {
      add_kv("base", JsonString("enum"));
      add_kv("ref", JsonString(GenFullName(type.enum_def)));
      add_kv("scalar", JsonString(BaseTypeNameForMetadata(type.base_type)));
    } else {
      add_kv("base", JsonString(BaseTypeNameForMetadata(type.base_type)));
    }

    return "{ " + JoinStrings(parts, ", ") + " }";
  }

  std::string FieldObjectForMetadata(const FieldDef& field) const {
    std::vector<std::string> parts;

    auto add_kv = [&](const std::string& key, const std::string& value) {
      parts.push_back("\"" + key + "\" : " + value);
    };

    add_kv("type", TypeObjectForMetadata(field.value.type));

    const auto* id = field.attributes.Lookup("id");
    if (id != nullptr) {
      int64_t id_value = 0;
      if (StringToNumber(id->constant.c_str(), &id_value)) {
        add_kv("id", NumToString(id_value));
      } else {
        add_kv("id", JsonString(id->constant));
      }
    }

    const char* presence = "";
    switch (field.presence) {
      case FieldDef::kRequired: presence = "required"; break;
      case FieldDef::kOptional: presence = "optional"; break;
      case FieldDef::kDefault: presence = "default"; break;
    }
    add_kv("presence", JsonString(presence));

    if (field.deprecated) add_kv("deprecated", "true");
    if (field.key) add_kv("key", "true");
    if (field.shared) add_kv("shared", "true");
    if (field.native_inline) add_kv("native_inline", "true");
    if (field.flexbuffer) add_kv("flexbuffer", "true");
    if (field.offset64) add_kv("offset64", "true");
    if (field.nested_flatbuffer != nullptr) {
      add_kv("nested_flatbuffer",
             JsonString(GenFullName(field.nested_flatbuffer)));
    }

    if (field.sibling_union_field != nullptr) {
      if (field.value.type.base_type == BASE_TYPE_UNION) {
        add_kv("union_type_field", JsonString(field.sibling_union_field->name));
      } else if (field.value.type.base_type == BASE_TYPE_UTYPE) {
        add_kv("union_value_field",
               JsonString(field.sibling_union_field->name));
      }
    }

    return "{ " + JoinStrings(parts, ", ") + " }";
  }

  std::string EnumObjectForMetadata(const EnumDef& enum_def) const {
    std::vector<std::string> parts;

    auto add_kv = [&](const std::string& key, const std::string& value) {
      parts.push_back("\"" + key + "\" : " + value);
    };

    add_kv("kind", JsonString(enum_def.is_union ? "union" : "enum"));
    add_kv("name", JsonString(enum_def.name));
    add_kv("namespace", NamespaceArrayForMetadata(enum_def.defined_namespace));
    add_kv("underlying_type",
           JsonString(BaseTypeNameForMetadata(enum_def.underlying_type.base_type)));

    std::vector<std::string> values;
    values.reserve(enum_def.Vals().size());
    for (const auto* enum_val : enum_def.Vals()) {
      std::vector<std::string> value_parts;
      value_parts.push_back("\"name\" : " + JsonString(enum_val->name));
      value_parts.push_back("\"value\" : " +
                            JsonString(enum_def.ToString(*enum_val)));
      if (enum_def.is_union && enum_val->union_type.base_type != BASE_TYPE_NONE) {
        if (enum_val->union_type.struct_def != nullptr) {
          value_parts.push_back("\"union_type\" : " +
                                JsonString(GenFullName(enum_val->union_type.struct_def)));
        }
      }
      values.push_back("{ " + JoinStrings(value_parts, ", ") + " }");
    }
    add_kv("values", "[ " + JoinStrings(values, ", ") + " ]");

    return "{ " + JoinStrings(parts, ", ") + " }";
  }

  std::string StructObjectForMetadata(const StructDef& struct_def) const {
    std::vector<std::string> parts;

    auto add_kv = [&](const std::string& key, const std::string& value) {
      parts.push_back("\"" + key + "\" : " + value);
    };

    add_kv("kind", JsonString(struct_def.fixed ? "struct" : "table"));
    add_kv("name", JsonString(struct_def.name));
    add_kv("namespace",
           NamespaceArrayForMetadata(struct_def.defined_namespace));
    if (struct_def.has_key) add_kv("has_key", "true");
    if (struct_def.fixed) {
      add_kv("minalign", NumToString(struct_def.minalign));
      add_kv("bytesize", NumToString(struct_def.bytesize));
    }

    return "{ " + JoinStrings(parts, ", ") + " }";
  }

 public:
  JsonSchemaGenerator(const Parser& parser, const std::string& path,
                      const std::string& file_name)
      : BaseGenerator(parser, path, file_name, "", "", "json") {}

  explicit JsonSchemaGenerator(const BaseGenerator& base_generator)
      : BaseGenerator(base_generator) {}

  std::string GeneratedFileName(const std::string& path,
                                const std::string& file_name,
                                const IDLOptions& options /* unused */) const {
    (void)options;
    return path + file_name + ".schema.json";
  }

  // If indentation is less than 0, that indicates we don't want any newlines
  // either.
  std::string NewLine() const {
    return parser_.opts.indent_step >= 0 ? "\n" : "";
  }

  std::string Indent(int indent) const {
    const auto num_spaces = indent * std::max(parser_.opts.indent_step, 0);
    return std::string(num_spaces, ' ');
  }

  std::string PrepareDescription(
      const std::vector<std::string>& comment_lines) {
    std::string comment;
    for (auto line_iterator = comment_lines.cbegin();
         line_iterator != comment_lines.cend(); ++line_iterator) {
      const auto& comment_line = *line_iterator;

      // remove leading and trailing spaces from comment line
      const auto start = std::find_if(comment_line.begin(), comment_line.end(),
                                      [](char c) { return !isspace(c); });
      const auto end =
          std::find_if(comment_line.rbegin(), comment_line.rend(), [](char c) {
            return !isspace(c);
          }).base();
      if (start < end) {
        comment.append(start, end);
      } else {
        comment.append(comment_line);
      }

      if (line_iterator + 1 != comment_lines.cend()) comment.append("\n");
    }
    if (!comment.empty()) {
      std::string description;
      if (EscapeString(comment.c_str(), comment.length(), &description, true,
                       true)) {
        return description;
      }
      return "";
    }
    return "";
  }

  bool generate() {
    code_ = "";
    if (parser_.root_struct_def_ == nullptr) {
      std::cerr << "Error: Binary schema not generated, no root struct found\n";
      return false;
    }
    code_ += "{" + NewLine();
    code_ += Indent(1) +
             "\"$schema\": \"https://json-schema.org/draft/2019-09/schema\"," +
             NewLine();
    if (EmitXFlatbuffersMetadata()) {
      std::vector<std::pair<std::string, std::string>> xfb_keys;
      xfb_keys.emplace_back("root_type",
                            JsonString(GenFullName(parser_.root_struct_def_)));
      if (!parser_.file_identifier_.empty()) {
        xfb_keys.emplace_back("file_identifier",
                              JsonString(parser_.file_identifier_));
      }
      if (!parser_.file_extension_.empty()) {
        xfb_keys.emplace_back("file_extension",
                              JsonString(parser_.file_extension_));
      }

      code_ += Indent(1) + "\"x-flatbuffers\" : {" + NewLine();
      for (size_t i = 0; i < xfb_keys.size(); ++i) {
        code_ += Indent(2) + "\"" + xfb_keys[i].first + "\" : " +
                 xfb_keys[i].second;
        if (i + 1 < xfb_keys.size()) code_ += ",";
        code_ += NewLine();
      }
      code_ += Indent(1) + "}," + NewLine();
    }
    code_ += Indent(1) + "\"definitions\": {" + NewLine();
    for (auto e = parser_.enums_.vec.cbegin(); e != parser_.enums_.vec.cend();
         ++e) {
      code_ += Indent(2) + "\"" + GenFullName(*e) + "\" : {" + NewLine();
      code_ += Indent(3) + GenType("string") + "," + NewLine();
      if (EmitXFlatbuffersMetadata()) {
        code_ += Indent(3) + "\"x-flatbuffers\" : " +
                 EnumObjectForMetadata(**e) + "," + NewLine();
      }
      auto enumdef(Indent(3) + "\"enum\": [");
      for (auto enum_value = (*e)->Vals().begin();
           enum_value != (*e)->Vals().end(); ++enum_value) {
        enumdef.append("\"" + (*enum_value)->name + "\"");
        if (*enum_value != (*e)->Vals().back()) {
          enumdef.append(", ");
        }
      }
      enumdef.append("]");
      code_ += enumdef + NewLine();
      code_ += Indent(2) + "}," + NewLine();  // close type
    }
    for (auto s = parser_.structs_.vec.cbegin();
         s != parser_.structs_.vec.cend(); ++s) {
      const auto& structure = *s;
      code_ += Indent(2) + "\"" + GenFullName(structure) + "\" : {" + NewLine();
      code_ += Indent(3) + GenType("object") + "," + NewLine();
      const auto& comment_lines = structure->doc_comment;
      auto comment = PrepareDescription(comment_lines);
      if (comment != "") {
        code_ += Indent(3) + "\"description\" : " + comment + "," + NewLine();
      }
      if (EmitXFlatbuffersMetadata()) {
        code_ += Indent(3) + "\"x-flatbuffers\" : " +
                 StructObjectForMetadata(*structure) + "," + NewLine();
      }

      code_ += Indent(3) + "\"properties\" : {" + NewLine();

      const auto& properties = structure->fields.vec;
      for (auto prop = properties.cbegin(); prop != properties.cend(); ++prop) {
        const auto& property = *prop;
        std::string arrayInfo = "";
        if (IsArray(property->value.type)) {
          arrayInfo = "," + NewLine() + Indent(8) + "\"minItems\": " +
                      NumToString(property->value.type.fixed_length) + "," +
                      NewLine() + Indent(8) + "\"maxItems\": " +
                      NumToString(property->value.type.fixed_length);
        }
        std::string deprecated_info = "";
        if (property->deprecated) {
          deprecated_info =
              "," + NewLine() + Indent(8) + "\"deprecated\" : true";
        }
        std::string flatbuffers_info = "";
        if (EmitXFlatbuffersMetadata()) {
          flatbuffers_info =
              "," + NewLine() + Indent(8) + "\"x-flatbuffers\" : " +
              FieldObjectForMetadata(*property);
        }
        std::string typeLine = Indent(4) + "\"" + property->name + "\"";
        typeLine += " : {" + NewLine() + Indent(8);
        typeLine += GenType(property->value.type);
        typeLine += arrayInfo;
        typeLine += deprecated_info;
        typeLine += flatbuffers_info;
        auto description = PrepareDescription(property->doc_comment);
        if (description != "") {
          typeLine +=
              "," + NewLine() + Indent(8) + "\"description\" : " + description;
        }

        typeLine += NewLine() + Indent(7) + "}";
        if (property != properties.back()) {
          typeLine.append(",");
        }
        code_ += typeLine + NewLine();
      }
      code_ += Indent(3) + "}," + NewLine();  // close properties

      std::vector<FieldDef*> requiredProperties;
      std::copy_if(properties.begin(), properties.end(),
                   back_inserter(requiredProperties),
                   [](FieldDef const* prop) { return prop->IsRequired(); });
      if (!requiredProperties.empty()) {
        auto required_string(Indent(3) + "\"required\" : [");
        for (auto req_prop = requiredProperties.cbegin();
             req_prop != requiredProperties.cend(); ++req_prop) {
          required_string.append("\"" + (*req_prop)->name + "\"");
          if (*req_prop != requiredProperties.back()) {
            required_string.append(", ");
          }
        }
        required_string.append("],");
        code_ += required_string + NewLine();
      }
      code_ += Indent(3) + "\"additionalProperties\" : false" + NewLine();
      auto closeType(Indent(2) + "}");
      if (*s != parser_.structs_.vec.back()) {
        closeType.append(",");
      }
      code_ += closeType + NewLine();  // close type
    }
    code_ += Indent(1) + "}," + NewLine();  // close definitions

    // mark root type
    code_ += Indent(1) + "\"$ref\" : \"#/definitions/" +
             GenFullName(parser_.root_struct_def_) + "\"" + NewLine();

    code_ += "}" + NewLine();  // close schema root
    return true;
  }

  bool save() const {
    const auto file_path = GeneratedFileName(path_, file_name_, parser_.opts);
    return parser_.opts.file_saver->SaveFile(file_path.c_str(), code_, false);
  }

  const std::string getJson() { return code_; }
};
}  // namespace jsons

static bool GenerateJsonSchema(const Parser& parser, const std::string& path,
                               const std::string& file_name) {
  jsons::JsonSchemaGenerator generator(parser, path, file_name);
  if (!generator.generate()) {
    return false;
  }
  return generator.save();
}

namespace {

class JsonSchemaCodeGenerator : public CodeGenerator {
 public:
  Status GenerateCode(const Parser& parser, const std::string& path,
                      const std::string& filename) override {
    if (!GenerateJsonSchema(parser, path, filename)) {
      return Status::ERROR;
    }
    return Status::OK;
  }

  Status GenerateCode(const uint8_t*, int64_t, const CodeGenOptions&) override {
    return Status::NOT_IMPLEMENTED;
  }

  Status GenerateMakeRule(const Parser& parser, const std::string& path,
                          const std::string& filename,
                          std::string& output) override {
    (void)parser;
    (void)path;
    (void)filename;
    (void)output;
    return Status::NOT_IMPLEMENTED;
  }

  Status GenerateGrpcCode(const Parser& parser, const std::string& path,
                          const std::string& filename) override {
    (void)parser;
    (void)path;
    (void)filename;
    return Status::NOT_IMPLEMENTED;
  }

  Status GenerateRootFile(const Parser& parser,
                          const std::string& path) override {
    (void)parser;
    (void)path;
    return Status::NOT_IMPLEMENTED;
  }
  bool IsSchemaOnly() const override { return true; }

  bool SupportsBfbsGeneration() const override { return false; }

  bool SupportsRootFileGeneration() const override { return false; }

  IDLOptions::Language Language() const override {
    return IDLOptions::kJsonSchema;
  }

  std::string LanguageName() const override { return "JsonSchema"; }
};
}  // namespace

std::unique_ptr<CodeGenerator> NewJsonSchemaCodeGenerator() {
  return std::unique_ptr<JsonSchemaCodeGenerator>(
      new JsonSchemaCodeGenerator());
}
}  // namespace flatbuffers
