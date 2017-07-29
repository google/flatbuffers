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
	const char * outExt = ".schema.json";

  static std::string GeneratedFileName(const std::string &path,
                                       const std::string &file_name) {
    return path + file_name + outExt;
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
        return "";
      }
    }

  class JsonSchemaGenerator : public BaseGenerator {
	private:
		CodeWriter code_;

		template <class T> std::string GenFullName(const T *enum_def) {
			std::string full_name;
			const auto &name_spaces = enum_def->defined_namespace->components;
			for (auto ns = name_spaces.cbegin(); ns != name_spaces.cend(); ++ns) {
				full_name.append(*ns + "_");
			}
			full_name.append(enum_def->name);
			return full_name;
		}

		template <class T> std::string GenTypeRef(const T *enum_def) {
			  std::string ret = "\"$ref\" : \"";
			  auto basename = flatbuffers::StripPath(flatbuffers::StripExtension(enum_def->file));

		  if (basename != this->file_name_)
		  {
			std::transform(basename.begin(), basename.end(), basename.begin(), ::tolower);
			// relative path?
			ret += basename + outExt;
		  }
        
		  ret += "#/definitions/" + GenFullName(enum_def) + "\"";

		  return ret;
		}

		std::string GenType(const std::string &name) {
		  return "\"type\" : \"" + name + "\"";
		}

		std::string GenType(const Type &type) {
		  if (type.base_type == BASE_TYPE_CHAR && type.enum_def != nullptr) {
			// it is a reference to an enum type
			return GenTypeRef(type.enum_def);
		  }
		  switch (type.base_type) {
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
			const auto &union_types = type.enum_def->vals.vec;
			for (auto ut = union_types.cbegin(); ut < union_types.cend(); ++ut) {
			  auto &union_type = *ut;
			  if (union_type->union_type.base_type == BASE_TYPE_NONE) {
				continue;
			  }
			  if (union_type->union_type.base_type == BASE_TYPE_STRUCT) {
				union_type_string.append("{ " + GenTypeRef(union_type->union_type.struct_def) + " }");
			  }
			  if (union_type != *type.enum_def->vals.vec.rbegin()) {
				union_type_string.append(",");
			  }
			}
			union_type_string.append("]");
			return union_type_string;
		  }
		  case BASE_TYPE_UTYPE:
			return GenTypeRef(type.enum_def);
		  default:
			return GenType(GenNativeType(type.base_type));
		}
	  }

    public:
      JsonSchemaGenerator(const Parser &parser, const std::string &path,
                          const std::string &file_name)
        : BaseGenerator(parser, path, file_name, "", "") {}

      explicit JsonSchemaGenerator(const BaseGenerator &base_generator)
        : BaseGenerator(base_generator) {}     

      bool generate() {
        code_.Clear();
        code_ += "{";
        code_ += "  \"$schema\": \"http://json-schema.org/draft-04/schema#\",";
        code_ += "  \"$id\": \"file://" + this->file_name_ + outExt + "\",";
        code_ += "  \"definitions\": {";
        for (auto e = parser_.enums_.vec.cbegin();
             e != parser_.enums_.vec.cend();
             ++e) {
		  const auto &enumObj = *e;
          if (!parser_.opts.include_dependence_headers)
          {
            const auto basename =
              flatbuffers::StripPath(flatbuffers::StripExtension(enumObj->file));
            if (basename != this->file_name_)
              continue;
          }
			
          code_ += "";
          code_ += "    \"" + GenFullName(enumObj) + "\" : {";
          code_ += "      " + GenType("string") + ",";
          std::string enumdef("      \"enum\": [");
          for (auto enum_value = enumObj->vals.vec.begin();
               enum_value != enumObj->vals.vec.end();
               ++enum_value) {
            enumdef.append("\"" + (*enum_value)->name + "\"");
            if (*enum_value != enumObj->vals.vec.back()) {
              enumdef.append(", ");
            }
          }
          enumdef.append("]");
          code_ += enumdef;
          code_ += "    },";  // close type
        }
        for (auto s = parser_.structs_.vec.cbegin(); 
             s != parser_.structs_.vec.cend();
             ++s) {
		  const auto &structure = *s;
          if (!parser_.opts.include_dependence_headers)
          {
            const auto basename =
              flatbuffers::StripPath(flatbuffers::StripExtension(structure->file));
            if (basename != this->file_name_)
              continue;
          }

          code_ += "";
          code_ += "    \"" + GenFullName(structure) + "\" : {";
          code_ += "      " + GenType("object") + ",";
          std::string comment;
          const auto &comment_lines = structure->doc_comment;
          for (auto comment_line = comment_lines.cbegin();
              comment_line != comment_lines.cend();
              ++comment_line) {
            comment.append(*comment_line);
          }
          if (comment.size() > 0) {
            code_ += "      \"description\" : \"" + comment + "\",";
          }
          code_ += "      \"properties\" : {";

          const auto &properties = structure->fields.vec;
          for (auto prop = properties.cbegin(); prop != properties.cend(); ++prop) {
            const auto &property = *prop;

            std::string typeLine("        \"" + property->name + "\" : { " + GenType(property->value.type));
            if (property->value.type.base_type == BASE_TYPE_VECTOR)
            {
              auto reqLength =
                property->attributes.Lookup("len");
              if (reqLength)
              {
                auto len = reqLength->constant;
                typeLine += "        , \"minItems\" : " + len + ", \"maxItems\" : " + len;
              }
            }
			      typeLine += " }";

            if (property != properties.back()) {
              typeLine.append(",");
            }          
            code_ += typeLine;
          }

          std::vector<FieldDef *> requiredProperties;
          if (structure->fixed) {
            // all fields are required for fixed structs
            requiredProperties.resize(properties.size());
            std::copy(properties.begin(), properties.end(), requiredProperties.begin());
          } else {
            std::copy_if(
              properties.begin(), properties.end(),
              std::back_inserter(requiredProperties),
              [](flatbuffers::FieldDef *prop)
              {
                if(prop->required)
                  return true; 
                if (prop->attributes.Lookup("schemaRequired"))
                  return true;

                return false;
              }
            );
          }
          if (requiredProperties.size() > 0) {
			      code_ += "      },";  // close properties
            std::string required_string("      \"required\" : [ ");
            for (auto req_prop = requiredProperties.cbegin();
                req_prop != requiredProperties.cend();
                ++req_prop) {
              required_string.append("\"" + (*req_prop)->name + "\"");
              if (*req_prop != requiredProperties.back()) {
                required_string.append(", ");
              }
            }
            required_string.append("]");
            code_ += required_string;
          } else {
            code_ += "      }";  // close properties
          }

          std::string closeType("    }");
          if (*s != parser_.structs_.vec.back()) {
            closeType.append(",");
          }
          code_ += closeType;  // close type
        }
        code_ += "  }";  // close definitions

        if (parser_.root_struct_def_ && flatbuffers::StripPath(flatbuffers::StripExtension(parser_.root_struct_def_->file)) == file_name_)
        {
          code_ += ",";
          // mark root type
          code_ += "  \"$ref\" : \"#/definitions/" +
                   GenFullName(parser_.root_struct_def_) + "\"";
        }

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
