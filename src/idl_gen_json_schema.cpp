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

#include "flatbuffers/idl.h"
#include "flatbuffers/util.h"
#include "flatbuffers/code_generators.h"


namespace flatbuffers {
  static std::string GeneratedFileName(const std::string &path,
                                       const std::string &file_name) {
    return path + file_name + ".schema.json";
  }
namespace jsons {
  // Return a C++ type from the table in idl.h
  std::string GenTypeBasic(const Type &type, bool user_facing_type) {
    static const char *ctypename[] = {
#define FLATBUFFERS_TD(ENUM, IDLTYPE, CTYPE, JTYPE, GTYPE, NTYPE, PTYPE) \
            #CTYPE,
      FLATBUFFERS_GEN_TYPES(FLATBUFFERS_TD)
#undef FLATBUFFERS_TD
    };
    if (user_facing_type) {
      if (type.base_type == BASE_TYPE_BOOL) return "bool";
    }
    return ctypename[type.base_type];
  }

  std::string GenTypeNative(const Type &type,
                            const FieldDef &field) {
    switch (type.base_type) {
    case BASE_TYPE_STRING:
    {
      return "string";
    }
    case BASE_TYPE_VECTOR:
    {
      const auto type_name = GenTypeNative(type.VectorType(), field);
      return "array";
    }
    case BASE_TYPE_STRUCT:
    {
      return "baseTypeStruct";
    }
    case BASE_TYPE_UNION:
    {
      return type.enum_def->name + "Union";
    }
    default:
    {
      return GenTypeBasic(type, true);
    }
    }
  }

  class JsonSchemaGenerator : public BaseGenerator {
  private:
    CodeWriter code_;
  public:
    JsonSchemaGenerator(const Parser& parser, const std::string& path, const std::string& file_name)
      : BaseGenerator(parser, path, file_name, "", "") {
    }

    explicit JsonSchemaGenerator(const BaseGenerator& base_generator)
      : BaseGenerator(base_generator) {
    }

    bool generate() override{
      code_.Clear();
      code_ += "{";
      code_ += "\"$schema\": \"http://json-schema.org/draft-04/schema#\",";
      code_ += "\"definitions\": {";

      for (auto &s : parser_.structs_.vec) {
        code_ += "";
        code_ += "\"" + s->name + "\" : {";
        code_ += "\"type\" : \"object\",";
        std::stringstream comment;
        for (auto commentLine : s->doc_comment) {
          comment << commentLine;
        }
        code_ += "\"description\" : \"" + comment.str() +"\",";
        code_ += "\"properties\" : {";

        for (auto const &prop : s->fields.vec) {
          std::string typeLine("  \"" + prop->name + "\" : { \"type\" : \"" + GenTypeBasic(prop->value.type, true) + "\" }");
          if (&prop != &s->fields.vec.back()) {
            code_ += typeLine + ",";
          }
          else {
            code_ += typeLine;
          }
        }
        code_ += "  }";  // close properties

        if (&s != &parser_.structs_.vec.back()) {
          code_ += "},";  // close type
        }
        else {
          code_ += "}";  // close type
        }
      }
      code_ += "},";  // close definitions

      // mark root type
      code_ += "\"$ref\" : \"#/definitions/" + parser_.root_struct_def_->name + "\"";
      code_ += "}";  // close schema root
      const auto file_path = GeneratedFileName(path_, file_name_);
      const auto final_code = code_.ToString();
      return SaveFile(file_path.c_str(), final_code, false);
    }
  };
}  // namespace jsons

bool GenerateJsonSchema(const Parser &parser, const std::string &path, const std::string &file_name) {
  jsons::JsonSchemaGenerator generator(parser, path, file_name);
  return generator.generate();
}
}  // namespace flatbuffers
