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

#include <csignal>
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

// To reduce typing
namespace r = ::reflection;

class LuaGenerator : public BaseGenerator {
 public:
  explicit LuaGenerator() : BaseGenerator(2, ' '), current_block_() {
    static const char *const keywords[] = {
      "and",      "break",  "do",   "else", "elseif", "end",  "false", "for",
      "function", "goto",   "if",   "in",   "local",  "nil",  "not",   "or",
      "repeat",   "return", "then", "true", "until",  "while"
    };
    keywords_.insert(std::begin(keywords), std::end(keywords));
  }

  GeneratorStatus generate(const r::Schema *schema) override {
    std::string filename = GetFileName(schema);

    if (!generate_enums(schema->enums())) { return GeneratorStatus::FAILED; }
    if (!generate_objects(schema->objects(), schema->root_table())) {
      return GeneratorStatus::FAILED;
    }

    return GeneratorStatus::OK;
  }

 protected:
  bool generate_enums(
      const flatbuffers::Vector<flatbuffers::Offset<r::Enum>> *enums) {
    for (auto it = enums->cbegin(); it != enums->cend(); ++it) {
      auto enum_def = *it;
      start_code_block();
      const std::string enum_name = generate_enum(enum_def);
      emit_code_block(enum_name, /*include_imports=*/false);
    }
    return true;
  }

  std::string generate_enum(const r::Enum *enum_def) {
    std::string ns;
    const std::string enum_name =
        normalize_name(denamespace(enum_def->name(), ns));

    append_comments(enum_def->documentation());
    append_line("local " + enum_name + " = {");
    {
      // TODO(derekbailey): It would be cool if this would auto dedent on
      // leaving scope.
      indent();
      for (auto it = enum_def->values()->cbegin();
           it != enum_def->values()->cend(); ++it) {
        const auto enum_val = *it;
        append_comments(enum_val->documentation());
        append_line(normalize_name(enum_val->name()) + " = " +
                    std::to_string(enum_val->value()) + ",");
      }
      dedent();
    }
    append_line("}");
    return enum_name;
  }

  bool generate_objects(
      const flatbuffers::Vector<flatbuffers::Offset<r::Object>> *objects,
      const r::Object *root_object) {
    for (auto it = objects->cbegin(); it != objects->cend(); ++it) {
      auto object_def = *it;
      start_code_block();
      const std::string object_name =
          generate_object(object_def, object_def == root_object);
      emit_code_block(object_name, /*include_imports=*/true);
    }
    return true;
  }

  std::string generate_object(const r::Object *object_def,
                              bool is_root_object) {
    std::string ns;
    const std::string object_name =
        normalize_name(denamespace(object_def->name(), ns));

    append_comments(object_def->documentation());
    append_line("local " + object_name + " = {}");
    append_line("local mt = {}");
    append_line();
    append_line("function " + object_name + ".New()");
    {
      indent();
      append_line("local o = {}");
      append_line("setmetatable(o, {__index = mt})");
      append_line(("return o"));
      dedent();
    }
    append_line("end");
    if (is_root_object) { generate_root_object(object_def, object_name); }
    append_line();

    // Generates a init method that receives a pre-existing accessor object, so
    // that objects can be reused.
    append_line("function mt:Init(buf, pos)");
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
      generate_object_field(object_def, *it, object_name);
    }

    // Create all the builders
    if (object_def->is_struct()) {
      append_line("function " + object_name + ".Create" + object_name +
                  "(builder" + generate_struct_builder_args(object_def) + ")");
      {
        indent();
        append_struct_builder_body(object_def);
        append_line("return builder:Offset()");
        dedent();
      }
      append_line("end");
      append_line();
    } else {
      // Table builders
      append_line("function " + object_name + ".Start(builder)");
      {
        indent();
        append_line("builder:StartObject(" +
                    std::to_string(object_def->fields()->size()) + ")");
        dedent();
      }
      append_line("end");
      append_line();
      for (auto it = object_def->fields()->cbegin();
           it != object_def->fields()->cend(); ++it) {
        auto field = *it;
        if (field->deprecated()) { continue; }
        auto offset = it - object_def->fields()->cbegin();

        const std::string field_name = normalize_name(field->name());

        append_line("function " + object_name + ".Add" +
                    make_camel_case(field_name) + "(builder, " +
                    make_camel_case(field_name, false) + ")");
        {
          indent();
          append_line("builder:Prepend" + generate_method(field) + "Slot(" +
                      std::to_string(offset) + ", " +
                      make_camel_case(field_name, false) + ", " +
                      default_value(field) + ")");
          dedent();
        }
        append_line("end");
        append_line();
      }

      append_line("function " + object_name + ".End(builder)");
      {
        indent();
        append_line("return builder:EndObject()");
        dedent();
      }
      append_line("end");
      append_line();
    }

    return object_name;
  }

  void generate_root_object(const r::Object *object_def,
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

  bool generate_object_field(const r::Object *object_def,
                             const r::Field *field_def,
                             const std::string &object_name) {
    // Skip writing deprecated fields altogether.
    if (field_def->deprecated()) { return true; }

    const std::string field_name = normalize_name(field_def->name());
    const std::string field_name_camel_case = make_camel_case(field_name);
    const r::BaseType base_type = field_def->type()->base_type();

    // Generate some fixed strings so we don't repeat outselves later.
    const std::string getter_signature =
        "function mt:" + field_name_camel_case + "()";
    const std::string offset_prefix = "local o = self.view:Offset(" +
                                      std::to_string(field_def->offset()) + ")";
    const std::string offset_prefix_2 = "if o ~= 0 then";

    append_comments(field_def->documentation());
    if (IsScalar(base_type)) {
      append_line(getter_signature);
      {
        indent();
        if (object_def->is_struct()) {
          // TODO(derekbailey): it would be nice to modify the view:Get to just
          // pass in the offset and not have to add it its own self.view.pos.
          append_line("return " + generate_getter(field_def->type()) +
                      "self.view.pos + " + std::to_string(field_def->offset()) +
                      ")");
        } else {
          // Table accessors
          append_line(offset_prefix);
          append_line(offset_prefix_2);
          {
            indent();
            std::string getter =
                generate_getter(field_def->type()) + "self.view.pos + o)";
            if (IsBool(base_type)) { getter = "(" + getter + " ~=0)"; }
            append_line("return " + getter);
            dedent();
          }
          append_line("end");
          append_line("return " + default_value(field_def));
        }
        dedent();
      }
      append_line("end");
      append_line();
    } else {
      switch (base_type) {
        case r::BaseType::String: {
          append_line(getter_signature);
          {
            indent();
            append_line(offset_prefix);
            append_line(offset_prefix_2);
            {
              indent();
              append_line("return " + generate_getter(field_def->type()) +
                          "self.view.pos + o)");
              dedent();
            }
            append_line("end");
            dedent();
          }
          append_line("end");
          append_line();
          break;
        }
        case r::BaseType::Obj: {
          if (object_def->is_struct()) {
            append_line("function mt:" + field_name_camel_case + "(obj)");
            {
              indent();
              append_line("obj:Init(self.view.bytes, self.view.pos +" +
                          std::to_string(field_def->offset()) + ")");
              append_line("return obj");
              dedent();
            }
          } else {
            append_line(getter_signature);
            {
              indent();
              append_line(offset_prefix);
              append_line(offset_prefix_2);
              {
                indent();

                const r::Object *field_object =
                    get_object_by_index(field_def->type()->index());
                if (!field_object) {
                  // TODO(derekbailey): this is an error condition. we should
                  // report it better.
                  return false;
                }

                append_line("local x = self.view.pos + o");
                if (!field_object->is_struct()) {
                  append_line("local x = self.view:Indirect(x)");
                }

                // TODO(derekbailey): handle the namespacing of the required
                // file.
                append_line("local obj = require(\"" + field_name +
                            "\").New()");
                append_line("obj:Init(self.view.bytes, x)");
                append_line("return obj");
                dedent();
              }
              append_line("end");
              dedent();
            }
          }
          append_line("end");
          append_line();
          break;
        }
        case r::BaseType::Union: {
          append_line(getter_signature);
          {
            indent();
            append_line(offset_prefix);
            append_line(offset_prefix_2);
            {
              indent();
              // TODO(derekbailey): fix the complicated require statement.
              append_line(
                  "local obj = "
                  "flatbuffers.view.New(require('flatbuffers.binaryarray').New("
                  "0), 0)");
              append_line(generate_getter(field_def->type()) + "obj, o)");
              append_line("return obj");
              dedent();
            }
            append_line("end");
            dedent();
          }
          append_line("end");
          append_line();
          break;
        }
        case r::BaseType::Vector: {
          const r::BaseType vector_base_type = field_def->type()->element();

          // TODO(derekbailey): the 1 needs to be the inline size of the
          // vector base type. This requires indirection to get the
          // object from the objects array.
          const int32_t inline_size = 1;

          {
            append_line("function mt:" + field_name_camel_case + "(j)");
            {
              indent();
              append_line(offset_prefix);
              append_line(offset_prefix_2);
              {
                indent();
                if (IsStructOrTable(vector_base_type)) {
                  append_line("local x = self.view:Vector(o)");
                  append_line("local x = x + ((j-i) * " +
                              std::to_string(inline_size) + ")");
                  const bool is_table = true;
                  if (is_table) { append_line("x = self.view:Indirect(x)"); }
                  append_line("local obj = require(\"" + field_name +
                              "\").New()");
                  append_line("obj:Init(self.view.bytes, x)");
                  append_line("return obj");
                } else {
                  append_line("local a = self.view:Vector(o)");
                  append_line("return " + generate_getter(field_def->type()) +
                              "a + ((j-1) * " + std::to_string(inline_size) +
                              "))");
                }
                dedent();
              }
              append_line("end");
              // Only generate a default value for those types that are
              // supported.
              if (!IsStructOrTable(vector_base_type)) {
                append_line("return " +
                            std::string(vector_base_type == r::BaseType::String
                                            ? "''"
                                            : "0"));
              }
              dedent();
            }
            append_line("end");
            append_line();

            // If the vector is composed of single byte values, we generate a
            // helper function to get it as a byte string in Lua.
            if (IsSingleByte(vector_base_type)) {
              append_line("function mt:" + field_name_camel_case +
                          "AsString(start, stop)");
              {
                indent();
                append_line("return self.view:VectorAsString(" +
                            std::to_string(field_def->offset()) +
                            ", start, stop)");
                dedent();
              }
              append_line("end");
              append_line();
            }
          }

          {
            // We also make a new accessor to query just the length of the
            // vector.
            append_line("function mt:" + field_name_camel_case + "Length()");
            {
              indent();
              append_line(offset_prefix);
              append_line(offset_prefix_2);
              {
                indent();
                append_line("return self.view:VectorLen(o)");
                dedent();
              }
              append_line("end");
              append_line("return 0");
              dedent();
            }
            append_line("end");
            append_line();
          }
          break;
        }
        default: return false;
      }
    }
    return true;
  }

 private:
  void append_comments(
      const flatbuffers::Vector<flatbuffers::Offset<flatbuffers::String>>
          *comments) {
    if (!comments) { return; }
    for (auto it = comments->cbegin(); it != comments->cend(); ++it) {
      auto comment = *it;
      // TODO(derekbailey): it would be nice to limit this to some max line
      // width if possible.
      append_line("-- " + comment->str());
    }
  }

  std::string generate_struct_builder_args(const r::Object *object,
                                           std::string prefix = "") {
    std::string signature;
    for (auto it = object->fields()->cbegin(); it != object->fields()->cend();
         ++it) {
      auto field = *it;
      if (IsStructOrTable(field->type()->base_type())) {
        const r::Object *field_object =
            get_object_by_index(field->type()->index());
        signature += generate_struct_builder_args(
            field_object, prefix + normalize_name(field->name()) + "_");
      } else {
        signature +=
            ", " + make_camel_case(normalize_name(field->name()), false);
      }
    }
    return signature;
  }

  void append_struct_builder_body(const r::Object *object,
                                  std::string prefix = "") {
    append_line("builder:Prep(" + std::to_string(object->minalign()) + ", " +
                std::to_string(object->bytesize()) + ")");
    for (auto it = object->fields()->crbegin(); it != object->fields()->crend();
         ++it) {
      auto field = *it;
      // TODO(derekbailey): is their padding to take care of here?
      if (IsStructOrTable(field->type()->base_type())) {
        const r::Object *field_object =
            get_object_by_index(field->type()->index());
        append_struct_builder_body(
            field_object, prefix + normalize_name(field->name()) + "_");
      } else {
        append_line("builder:Prepend" + generate_method(field) + "(" + prefix +
                    make_camel_case(normalize_name(field->name()), false) +
                    ")");
      }
    }
  }

  std::string generate_method(const r::Field *field) {
    const r::BaseType base_type = field->type()->base_type();
    if (IsScalar(base_type)) {
      return make_camel_case(generate_type(base_type));
    }
    if (IsStructOrTable(base_type)) { return "Struct"; }
    return "UOffsetTRelative";
  }

  std::string generate_getter(const r::Type *type, bool element_type = false) {
    switch (element_type ? type->element() : type->base_type()) {
      case r::BaseType::String: return "self.view:String(";
      case r::BaseType::Union: return "self.view:Union(";
      case r::BaseType::Vector: return generate_getter(type, true);
      default:
        return "self.view:Get(flatbuffers.N." +
               make_camel_case(generate_type(type, element_type)) + ", ";
    }
  }

  std::string generate_type(const r::Type *type, bool element_type = false) {
    const r::BaseType base_type =
        element_type ? type->element() : type->base_type();
    if (IsScalar(base_type)) { return generate_type(base_type); }
    switch (base_type) {
      case r::BaseType::String: return "string";
      case r::BaseType::Vector: return generate_getter(type, true);
      case r::BaseType::Obj: {
        const r::Object *obj = get_object_by_index(type->index());
        if (obj) { return normalize_name(denamespace(obj->name())); }
        // TODO(derekbailey): should error here.
        return "error";
      };
      default: return "*flatbuffers.Table";
    }
  }

  std::string generate_type(const r::BaseType base_type) {
    // Need to override the default naming to match the Lua runtime libraries.
    // TODO(derekbailey): make overloads in the runtime libraries to avoid this.
    switch (base_type) {
      case r::BaseType::None: return "uint8";
      case r::BaseType::UType: return "uint8";
      case r::BaseType::Byte: return "int8";
      case r::BaseType::UByte: return "uint8";
      case r::BaseType::Short: return "int16";
      case r::BaseType::UShort: return "uint16";
      case r::BaseType::Int: return "int32";
      case r::BaseType::UInt: return "uint32";
      case r::BaseType::Long: return "int64";
      case r::BaseType::ULong: return "uint64";
      case r::BaseType::Float: return "Float32";
      case r::BaseType::Double: return "Float64";
      default: return r::EnumNameBaseType(base_type);
    }
  }

  std::string default_value(const r::Field *field) {
    const r::BaseType base_type = field->type()->base_type();
    if (IsFloatingPoint(base_type)) {
      return std::to_string(field->default_real());
    }
    if (IsBool(base_type)) {
      return field->default_integer() ? "true" : "false";
    }
    if (IsScalar(base_type)) {
      return std::to_string((field->default_integer()));
    }
    // represents offsets
    return "0";
  }

  std::string normalize_name(const std::string name) const {
    return keywords_.find(name) == keywords_.end() ? name : "_" + name;
  }

  std::string normalize_name(const flatbuffers::String *name) const {
    return normalize_name(name->str());
  }

  std::string denamespace(const flatbuffers::String *name) {
    std::string ns;
    return denamespace(name, ns);
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
  std::string current_block_;
};
}  // namespace

std::unique_ptr<Generator> NewLuaGenerator() {
  return std::unique_ptr<LuaGenerator>(new LuaGenerator());
}

}  // namespace flatbuffers