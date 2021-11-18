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

#include <cstdint>
#include <memory>
#include <string>
#include <unordered_set>

#include "base_generator.h"
#include "flatbuffers/base.h"
#include "flatbuffers/generator.h"
#include "flatbuffers/reflection_generated.h"
#include "flatbuffers/verifier.h"

namespace flatbuffers {
namespace {

class LuaGenerator : public BaseGenerator {
 public:
  explicit LuaGenerator()
      : BaseGenerator(), spaces_per_indent_(4), current_block_() {
    static const char *const keywords[] = {
      "and",      "break",  "do",   "else", "elseif", "end",  "false", "for",
      "function", "goto",   "if",   "in",   "local",  "nil",  "not",   "or",
      "repeat",   "return", "then", "true", "until",  "while"
    };
    keywords_.insert(std::begin(keywords), std::end(keywords));
  }

  GeneratorStatus generate(const reflection::Schema *schema) override {
    std::string filename = GetFileName(schema);

    if (!generate_enums(schema->enums())) { return GeneratorStatus::FAILED; }

    return GeneratorStatus::OK;
  }

 protected:
  bool generate_enums(
      const flatbuffers::Vector<flatbuffers::Offset<reflection::Enum>> *enums) {
    for (auto it = enums->cbegin(); it != enums->cend(); ++it) {
      auto enum_def = *it;
      start_code_block();
      const std::string enum_name = generate_enum(enum_def);
      emit_code_block(enum_name);
    }
    return true;
  }

  std::string generate_enum(const reflection::Enum *enum_def) {
    std::string ns;
    const std::string enum_name =
        normalize_name(denamespace(enum_def->name(), ns));

    generate_comment(enum_def->documentation());
    append("local " + enum_name + " = {\n");
    indent();
    for (auto it = enum_def->values()->cbegin();
         it != enum_def->values()->cend(); ++it) {
      const auto enum_val = *it;
      generate_comment(enum_val->documentation());
      append(normalize_name(enum_val->name()) + " = " +
             std::to_string(enum_val->value()) + ",\n");
    }
    dedent();
    append("}\n");
    return enum_name;
  }

  void generate_comment(
      const flatbuffers::Vector<flatbuffers::Offset<flatbuffers::String>>
          *comments) {
    if (comments->size() == 0) { return; }
  }

 private:
  std::string normalize_name(const std::string name) const {
    return keywords_.find(name) == keywords_.end() ? name : "_" + name;
  }

  std::string normalize_name(const flatbuffers::String *name) const {
    return normalize_name(name->str());
  }

  std::string denamespace(const flatbuffers::String *name, std::string &ns) {
    const size_t pos = name->str().find_last_of('.');
    if (pos == std::string::npos) {
      ns = "";
      return name->str();
    }
    ns = name->str().substr(0, pos);
    return name->str().substr(pos + 1);
  }

  void start_code_block() { current_block_.clear(); }

  std::string code_block() { return current_block_; }

  void append(std::string to_append) {
    current_block_ += indentation() + to_append;
  }

  std::string indentation() const {
    return std::string(spaces_per_indent_ * indent_level_, ' ');
  }

  void emit_code_block(const std::string &enum_name) {
    std::string code;
    code += std::string("-- test\n");
    code += code_block();
    code += "\n";
    code += "return " + enum_name;

    const std::string output_name = enum_name + ".lua";
    // TODO(derekbailey): figure out a save file without depending on util.h
    SaveFile(output_name.c_str(), code, false);
  }

  std::unordered_set<std::string> keywords_;
  const int32_t spaces_per_indent_;
  std::string current_block_;
};
}  // namespace

std::unique_ptr<Generator> NewLuaGenerator() {
  return std::unique_ptr<LuaGenerator>(new LuaGenerator());
}

}  // namespace flatbuffers