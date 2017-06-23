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

namespace jsons {
  class JsonSchemaGenerator : public BaseGenerator {
  public:
    JsonSchemaGenerator(const Parser& parser, const std::string& path, const std::string& file_name)
      : BaseGenerator(parser, path, file_name, "", "") {
    }

    explicit JsonSchemaGenerator(const BaseGenerator& base_generator)
      : BaseGenerator(base_generator) {
    }

    bool generate() override{
      return true;
    }
  };
}  // namespace jsons

bool GenerateJsonSchema(const Parser &parser, const std::string &path, const std::string &file_name) {
  jsons::JsonSchemaGenerator generator(parser, path, file_name);
  return generator.generate();
}
}  // namespace flatbuffers
