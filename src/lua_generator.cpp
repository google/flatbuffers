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
#include "flatbuffers/generator.h"
#include "flatbuffers/idl.h"
#include "flatbuffers/reflection_generated.h"

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
    if (!generate_objects(schema->objects(), schema->root_table())) {
      return GeneratorStatus::FAILED;
    }

    return GeneratorStatus::OK;
  }

 protected:
  bool generate_enums(
      const flatbuffers::Vector<flatbuffers::Offset<reflection::Enum>> *enums) {
    for (auto it = enums->cbegin(); it != enums->cend(); ++it) {
      auto enum_def = *it;
      start_code_block();
      const std::string enum_name = generate_enum(enum_def);
      emit_code_block(enum_name, /*include_imports=*/false);
    }
    return true;
  }

  std::string generate_enum(const reflection::Enum *enum_def) {
    std::string ns;
    const std::string enum_name =
        normalize_name(denamespace(enum_def->name(), ns));

    generate_comment(enum_def->documentation());
    append_line("local " + enum_name + " = {");
    {
      // TODO(derekbailey): It would be cool if this would auto dedent on
      // leaving scope.
      indent();
      for (auto it = enum_def->values()->cbegin();
           it != enum_def->values()->cend(); ++it) {
        const auto enum_val = *it;
        generate_comment(enum_val->documentation());
        append_line(normalize_name(enum_val->name()) + " = " +
                    std::to_string(enum_val->value()) + ",");
      }
      dedent();
    }
    append_line("}");
    return enum_name;
  }

  bool generate_objects(
      const flatbuffers::Vector<flatbuffers::Offset<reflection::Object>>
          *objects,
      const reflection::Object *root_object) {
    for (auto it = objects->cbegin(); it != objects->cend(); ++it) {
      auto object_def = *it;
      start_code_block();
      const std::string object_name =
          generate_object(object_def, object_def == root_object);
      emit_code_block(object_name, /*include_imports=*/true);
    }
    return true;
  }

  std::string generate_object(const reflection::Object *object_def,
                              bool is_root_object) {
    std::string ns;
    const std::string object_name =
        normalize_name(denamespace(object_def->name(), ns));
    const std::string metatable_name = object_name + "_mt";

    generate_comment(object_def->documentation());
    append_line("local " + object_name + " = {}");
    append_line("local " + metatable_name + " = {}");
    append_line();
    append_line("function " + object_name + ".New()");
    {
      indent();
      append_line("local o = {}");
      append_line("setmetatable(o, {__index = " + metatable_name + "})");
      append_line(("return o"));
      dedent();
    }
    append_line("end");
    if (is_root_object) { generate_root_object(object_def, object_name); }
    append_line();

    // Generates a init method that receives a pre-existing accessor object, so
    // that objects can be reused.
    append_line("function " + metatable_name + ":Init(buf, pos");
    {
      indent();
      append_line("self.view = flatbuffers.view.New(buf, pos)");
      dedent();
    }
    append_line("end");
    append_line();

    // Create all the field accessors.
    for (auto it = object_def->fields()->cbegin();
         it != object_def->fields()->cend(); ++it) {
      generate_object_field(object_def, *it, object_name, metatable_name);
    }

    return object_name;
  }

  void generate_root_object(const reflection::Object *object_def,
                            const std::string &object_name) {
    append_line();
    append_line("function " + object_name + ".GetRootAs" + object_name +
                "(buf, offset)");
    {
      indent();
      append_line("if type(buf) == \"string\" then");
      {
        indent();
        append_line("buf = flatbuffers.binaryArray.New(buf)");
        dedent();
      }
      append_line("end");
      append_line();
      append_line("local n = flatbuffers.N.UOffsetT:Unpack(buf, offset)");
      append_line("local o = " + object_name + ".New()");
      append_line("o:Init(buf, n + offset)");
      append_line("return o");
      dedent();
    }
    append_line("end");
  }

  void generate_object_field(const reflection::Object *object_def,
                             const reflection::Field *field_def,
                             const std::string &object_name,
                             const std::string &metatable_name) {
    if (field_def->deprecated()) { return; }

    const std::string field_name = normalize_name(field_def->name());
    // TODO(derekbailey): remove dependency on makeCamel
    const std::string field_name_camel_case = MakeCamel(field_name);

    generate_comment(field_def->documentation());
    if (IsScalar(field_def->type()->base_type())) {
      if (object_def->is_struct()) {
        append_line("function " + metatable_name + ":" + field_name_camel_case +
                    "()");
        {
          indent();

          append_line("return " + generate_getter(field_def->type()) +
                      "self.view.pos + " + std::to_string(field_def->offset()) +
                      ")");
          dedent();
        }
      }
    }
  }

 private:
  void generate_comment(
      const flatbuffers::Vector<flatbuffers::Offset<flatbuffers::String>>
          *comments) {
    if (comments->size() == 0) { return; }
  }

  std::string generate_getter(const reflection::Type *type,
                              bool element_type = false) {
    switch (element_type ? type->element() : type->base_type()) {
      case reflection::BaseType::String: return "self.view:String(";
      case reflection::BaseType::Union: return "self.view:Union(";
      case reflection::BaseType::Vector: return generate_getter(type, true);
      default:
        return "self.view:Get(flatbuffers.N." +
               MakeCamel(generate_type(type, element_type)) + ", ";
    }
  }

  std::string generate_type(const reflection::Type *type,
                            bool element_type = false) {
    const reflection::BaseType base_type =
        element_type ? type->element() : type->base_type();
    if (IsScalar(base_type)) { return reflection::EnumNameBaseType(base_type); }
    switch (base_type) {
      case reflection::BaseType::String: return "string";
      case reflection::BaseType::Vector: return generate_getter(type, true);
      // TODO(derekbailey): this index refers to the index in the
      // schema::objects case reflection::BaseType::Obj: return type->;
      default: return "*flatbuffers.Table";
    }
  }

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

  void append_line() { current_block_ += indentation() + "\n"; }

  void append_line(std::string to_append) { append(to_append + "\n"); }

  void append(std::string to_append) {
    current_block_ += indentation() + to_append;
  }

  std::string indentation() const {
    return std::string(spaces_per_indent_ * indent_level_, ' ');
  }

  void emit_code_block(const std::string &name, bool include_imports) {
    std::string code;
    if (include_imports) {
      code += "local flatbuffers = require('flatbuffers')\n\n";
    }
    code += code_block();
    code += "\n";
    code += "return " + name;

    const std::string output_name = name + ".lua";
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