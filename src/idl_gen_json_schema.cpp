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

#include "flatbuffers/code_generators.h"
#include "flatbuffers/idl.h"
#include "flatbuffers/util.h"
#include <iostream>

namespace flatbuffers {

static std::string GeneratedFileName(const std::string &path,
                                     const std::string &file_name) {
  return path + file_name + ".schema.json";
}

namespace jsons {

std::string GenNativeType(BaseType type) {
  switch (type) {
    case BASE_TYPE_BOOL:
      return "boolean";
    case BASE_TYPE_CHAR:
    case BASE_TYPE_UCHAR:
    case BASE_TYPE_SHORT:
    case BASE_TYPE_USHORT:
    case BASE_TYPE_INT:
    case BASE_TYPE_UINT:
    case BASE_TYPE_LONG:
    case BASE_TYPE_ULONG:
    case BASE_TYPE_FLOAT:
    case BASE_TYPE_DOUBLE:
      return "number";
    case BASE_TYPE_STRING:
      return "string";
  default: 
    std::cerr << "GenNativeType: Invalid base type " << type << "\n";
    return "\n";
  }
}

template <class T> std::string GenFullName(const T *enum_def) {
  std::string fullName;
  std::vector<std::string> nameSpaces = enum_def->defined_namespace->components;
  for (int nsindex = 0; nsindex < nameSpaces.size(); nsindex++) {
    std::string ns = nameSpaces[nsindex];
    fullName.append(ns + "_");
  }
  fullName.append(enum_def->name);
  return fullName;
}

template <class T> std::string GenTypeRef(const T *enum_def) {
  return "\"$ref\" : \"#/definitions/" + GenFullName(enum_def) + "\"";
}

std::string GenType(const std::string &name) {
  return "\"type\" : \"" + name + "\"";
}

std::string GenType(const Type &type) {
  switch (type.base_type) {
    case BASE_TYPE_VECTOR: {
      std::string typeline;
      typeline.append("\"type\" : \"array\", ");
      if (type.element == BASE_TYPE_STRUCT) {
        typeline.append("\"items\" : { " + GenTypeRef(type.struct_def) + " }");
      } else {
        typeline.append("\"items\" : { " +
                        GenType(GenNativeType(type.element)) +
                        " }");
      }
      return typeline;
    }
    case BASE_TYPE_STRUCT: {
      return GenTypeRef(type.struct_def);
    }
    case BASE_TYPE_UNION: {
      std::string unionTypes("\"anyOf\": [");
      for (int index = 0; index < type.enum_def->vals.vec.size(); ++index) {
        EnumVal* &ut = type.enum_def->vals.vec[index];
        if (ut->union_type.base_type == BASE_TYPE_NONE) {
          continue;
        }
        if (ut->union_type.base_type == BASE_TYPE_STRUCT) {
          unionTypes.append("{ " + GenTypeRef(ut->union_type.struct_def) + " }");
        }
        if (&ut != &type.enum_def->vals.vec.back()) {
          unionTypes.append(",");
        }
      }
      unionTypes.append("]");
      return unionTypes;
    }
    case BASE_TYPE_UTYPE:
      return GenTypeRef(type.enum_def);
  }
  if (type.base_type == BASE_TYPE_CHAR && type.enum_def != nullptr) {
    // it is a reference to an enum type
    return GenTypeRef(type.enum_def);
  }
  return GenType(GenNativeType(type.base_type));
}

class JsonSchemaGenerator : public BaseGenerator {
 private:
  CodeWriter code_;

 public:
  JsonSchemaGenerator(const Parser &parser, const std::string &path,
                      const std::string &file_name)
      : BaseGenerator(parser, path, file_name, "", "") {}

  explicit JsonSchemaGenerator(const BaseGenerator &base_generator)
      : BaseGenerator(base_generator) {}

  bool generate() override {
    code_.Clear();
    code_ += "{";
    code_ += "\"$schema\": \"http://json-schema.org/draft-04/schema#\",";
    code_ += "\"definitions\": {";
    for (int index = 0; index < parser_.enums_.vec.size(); ++index) {
      EnumDef *e = parser_.enums_.vec[index];
      code_ += "  \"" + GenFullName(e) + "\" : {";
      code_ += "    " + GenType("string") + ",";
      std::string enumdef("    \"enum\": [");

      for (int valindex = 0; valindex < e->vals.vec.size(); ++valindex) {
        EnumVal *enumval = e->vals.vec[valindex];
        enumdef.append("\"" + enumval->name + "\"");
        if (&enumval != &e->vals.vec.back()) {
          enumdef.append(", ");
        }
      }
      enumdef.append("]");
      code_ += enumdef;
      code_ += "  },";  // close type
    }

    for (int structindex = 0; structindex < parser_.structs_.vec.size(); structindex++) {
      StructDef * const &s = parser_.structs_.vec[structindex];
      code_ += "";
      code_ += "\"" + GenFullName(s) + "\" : {";
      code_ += "  " + GenType("object") + ",";
      std::string comment;
      for (const auto &commentLine : s->doc_comment) {
        comment.append(commentLine);
      }
      code_ += "  \"description\" : \"" + comment + "\",";
      code_ += "  \"properties\" : {";

      for (int propindex = 0; propindex < s->fields.vec.size(); propindex++) {
        FieldDef* &prop = s->fields.vec[propindex];
        std::string typeLine("    \"" + prop->name + "\" : { " + GenType(prop->value.type) + " }");
        if (&prop != &s->fields.vec.back()) {
          typeLine.append(",");
        }          
        code_ += typeLine;
      }
      std::vector<FieldDef*> props = s->fields.vec;
      std::vector<FieldDef *> requiredProperties;
      std::copy_if(props.begin(), props.end(),
                   back_inserter(requiredProperties),
                   [](FieldDef *prop) { return prop->required; });
      if (requiredProperties.size() > 0) {
        code_ += "  },";  // close properties
        std::string requiredString("\"required\" : [ ");
        for (int reqindex = 0; reqindex < requiredProperties.size(); reqindex++) {
          FieldDef*const &reqProp = requiredProperties[reqindex];
          requiredString.append("\"" + reqProp->name + "\"");
          if (&reqProp != &requiredProperties.back()) {
            requiredString.append(", ");
          }
        }
        requiredString.append("]");
        code_ += requiredString;
      } else {
        code_ += "  }";  // close properties
      }

      std::string closeType("}");
      if (&s != &parser_.structs_.vec.back()) {
        closeType.append(",");
      }

      code_ += closeType;  // close type
    }
    code_ += "},";  // close definitions

    // mark root type
    code_ += "\"$ref\" : \"#/definitions/" +
             GenFullName(parser_.root_struct_def_) + "\"";

    code_ += "}";  // close schema root
    const std::string file_path = GeneratedFileName(path_, file_name_);
    const std::string final_code = code_.ToString();
    return SaveFile(file_path.c_str(), final_code, false);
  }
};
}  // namespace jsons

bool GenerateJsonSchema(const Parser &parser, const std::string &path,
                        const std::string &file_name) {
  jsons::JsonSchemaGenerator generator(parser, path, file_name);
  return generator.generate();
}
}  // namespace flatbuffers
