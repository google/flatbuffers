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

#include "lua_generator.h"

#include <memory>

#include "base_generator.h"
#include "flatbuffers/generator.h"
#include "flatbuffers/reflection_generated.h"
#include "flatbuffers/verifier.h"

namespace flatbuffers {
namespace {

class LuaGenerator : public BaseGenerator {
 public:
  GeneratorStatus generate(const reflection::Schema *schema) override {
    std::string filename = GetFileName(schema);
    return GeneratorStatus::FAILED;
  }
};
}  // namespace

std::unique_ptr<Generator> NewLuaGenerator() {
  return std::unique_ptr<LuaGenerator>(new LuaGenerator());
}

}  // namespace flatbuffers