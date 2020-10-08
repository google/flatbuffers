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

#include <iostream>

#include "flatbuffers/code_generators.h"
#include "flatbuffers/idl.h"
#include "flatbuffers/util.h"

namespace flatbuffers {

namespace jsons {

std::string GenNativeType(BaseType type) {
  switch (type) {
    case BASE_TYPE_BOOL: return "boolean";
    case BASE_TYPE_CHAR:
    case BASE_TYPE_UCHAR:
    case BASE_TYPE_SHORT:
    case BASE_TYPE_USHORT:
    case BASE_TYPE_INT:
    case BASE_TYPE_UINT:
    case BASE_TYPE_LONG:
    case BASE_TYPE_ULONG:
    case BASE_TYPE_FLOAT:
    case BASE_TYPE_DOUBLE: return "number";
    case BASE_TYPE_STRING: return "string";
    case BASE_TYPE_ARRAY: return "array";
    default: return "";
  }
}

template<class T> std::string GenFullName(const T *enum_def) {
  std::string full_name;
  const auto &name_spaces = enum_def->defined_namespace->components;
  for (auto ns = name_spaces.cbegin(); ns != name_spaces.cend(); ++ns) {
    full_name.append(*ns + "_");
  }
  full_name.append(enum_def->name);
  return full_name;
}

template<class T> std::string GenTypeRef(const T *enum_def) {
  return "\"$ref\" : \"#/definitions/" + GenFullName(enum_def) + "\"";
}

std::string GenType(const std::string &name) {
  return "\"type\" : \"" + name + "\"";
}

std::string GenType(const Type &type) {
  if (type.enum_def != nullptr && !type.enum_def->is_union) {
    // it is a reference to an enum type
    return GenTypeRef(type.enum_def);
  }
  switch (type.base_type) {
    case BASE_TYPE_ARRAY: FLATBUFFERS_FALLTHROUGH();  // fall thru
    case BASE_TYPE_VECTOR: {
      std::string typeline;
      typeline.append("\"type\" : \"array\", \"items\" : { ");
      if (type.element == BASE_TYPE_STRUCT) {
        typeline.append(GenTypeRef(type.struct_def));
      } else {
        typeline.append(GenType(GenNativeType(type.element)));
      }
      typeline.append(" }");
      return typeline;
    }
    case BASE_TYPE_STRUCT: {
      return GenTypeRef(type.struct_def);
    }
    case BASE_TYPE_UNION: {
      std::string union_type_string("\"anyOf\": [");
      const auto &union_types = type.enum_def->Vals();
      for (auto ut = union_types.cbegin(); ut < union_types.cend(); ++ut) {
        auto &union_type = *ut;
        if (union_type->union_type.base_type == BASE_TYPE_NONE) { continue; }
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
    case BASE_TYPE_UTYPE: return GenTypeRef(type.enum_def);
    default: return GenType(GenNativeType(type.base_type));
  }
}

class JsonSchemaGenerator : public BaseGenerator {
 private:
  std::string code_;

 public:
  JsonSchemaGenerator(const Parser &parser, const std::string &path,
                      const std::string &file_name)
      : BaseGenerator(parser, path, file_name, "", "", "json") {}

  explicit JsonSchemaGenerator(const BaseGenerator &base_generator)
      : BaseGenerator(base_generator) {}

  std::string GeneratedFileName(const std::string &path,
                                const std::string &file_name,
                                const IDLOptions &options /* unused */) const {
    (void)options;
    return path + file_name + ".schema.json";
  }

  // If indentation is less than 0, that indicates we don't want any newlines
  // either.
  const std::string NewLine() {
    return parser_.opts.indent_step >= 0 ? "\n" : "";
  }

  const std::string Indent(int indent) {
    std::string indentation = "";
    return indentation.append(indent * std::max(parser_.opts.indent_step, 0), ' ');
  }

  bool generate() {
    code_ = "";
    if (parser_.root_struct_def_ == nullptr) { return false; }
    code_ += "{" + NewLine();
    code_ += Indent(1) +
             "\"$schema\": \"http://json-schema.org/draft-04/schema#\"," +
             NewLine();
    code_ += Indent(1) + "\"definitions\": {" + NewLine();
    for (auto e = parser_.enums_.vec.cbegin(); e != parser_.enums_.vec.cend();
         ++e) {
      code_ += Indent(2) + "\"" + GenFullName(*e) + "\" : {" + NewLine();
      code_ += Indent(3) + GenType("string") + "," + NewLine();
      std::string enumdef(Indent(3) + "\"enum\": [");
      for (auto enum_value = (*e)->Vals().begin();
           enum_value != (*e)->Vals().end(); ++enum_value) {
        enumdef.append("\"" + (*enum_value)->name + "\"");
        if (*enum_value != (*e)->Vals().back()) { enumdef.append(", "); }
      }
      enumdef.append("]");
      code_ += enumdef + NewLine();
      code_ += Indent(2) + "}," + NewLine();  // close type
    }
    for (auto s = parser_.structs_.vec.cbegin();
         s != parser_.structs_.vec.cend(); ++s) {
      const auto &structure = *s;
      code_ += Indent(2) + "\"" + GenFullName(structure) + "\" : {" + NewLine();
      code_ += Indent(3) + GenType("object") + "," + NewLine();
      std::string comment;
      const auto &comment_lines = structure->doc_comment;
      for (auto comment_line = comment_lines.cbegin();
           comment_line != comment_lines.cend(); ++comment_line) {
        comment.append(*comment_line);
      }
      if (comment.size() > 0) {
        std::string description;
        if (!EscapeString(comment.c_str(), comment.length(), &description, true,
                          true)) {
          return false;
        }
        code_ +=
            Indent(3) + "\"description\" : \"" + description + "\"," + NewLine();
      }
      code_ += Indent(3) + "\"properties\" : {" + NewLine();

      const auto &properties = structure->fields.vec;
      for (auto prop = properties.cbegin(); prop != properties.cend(); ++prop) {
        const auto &property = *prop;
        std::string arrayInfo = "";
        if (IsArray(property->value.type)) {
          arrayInfo = "," + NewLine() + Indent(8) + "\"minItems\": " +
                      NumToString(property->value.type.fixed_length) + 
                      "," + NewLine() + Indent(8) + "\"maxItems\": " +
                      NumToString(property->value.type.fixed_length);
        }
        std::string deprecated_info = "";
        if (property->deprecated) {
          deprecated_info = "," + NewLine() + Indent(8) + "\"deprecated\" : true,";
        }
        std::string typeLine = Indent(4) + "\"" + property->name + "\"";
        typeLine += " : {" + NewLine() + Indent(8);
        typeLine += GenType(property->value.type);
        typeLine += arrayInfo;
        typeLine += deprecated_info;
        typeLine += NewLine() + Indent(7) + "}";
        if (property != properties.back()) { typeLine.append(","); }
        code_ += typeLine + NewLine();
      }
      code_ += Indent(3) + "}," + NewLine();  // close properties

      std::vector<FieldDef *> requiredProperties;
      std::copy_if(properties.begin(), properties.end(),
                   back_inserter(requiredProperties),
                   [](FieldDef const *prop) { return prop->required; });
      if (requiredProperties.size() > 0) {
        std::string required_string(Indent(3) + "\"required\" : [");
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
      std::string closeType(Indent(2) + "}");
      if (*s != parser_.structs_.vec.back()) { closeType.append(","); }
      code_ += closeType + NewLine();  // close type
    }
    code_ += Indent(1) + "}," + NewLine();  // close definitions

    // mark root type
    code_ += Indent(1) + "\"$ref\" : \"#/definitions/" +
             GenFullName(parser_.root_struct_def_) + "\"" + NewLine();

    code_ += "}" + NewLine();  // close schema root
    return true;
  }

  bool save() {
    const std::string file_path =
        GeneratedFileName(path_, file_name_, parser_.opts);
    return SaveFile(file_path.c_str(), code_, false);
  }

  const std::string getJson()
  { 
    return code_;
  }
};
}  // namespace jsons

bool GenerateJsonSchema(const Parser &parser, const std::string &path,
                        const std::string &file_name) {
  jsons::JsonSchemaGenerator generator(parser, path, file_name);
  if (!generator.generate()) { return false; }
  return generator.save();
}

bool GenerateJsonSchema(const Parser &parser, std::string *json) {
  std::string path;
  std::string file_name;
  jsons::JsonSchemaGenerator generator(parser, path, file_name);
  if (!generator.generate()) { return false; }
  *json = generator.getJson();
  return true;
}
}  // namespace flatbuffers
