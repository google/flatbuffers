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

#include "bfbs_gen_lua.h"

#include <cstdint>
#include <map>
#include <memory>
#include <string>
#include <unordered_set>
#include <vector>

// Ensure no includes to flatc internals. bfbs_gen.h and generator.h are OK.
#include "bfbs_gen.h"
#include "flatbuffers/bfbs_generator.h"

// The intermediate representation schema.
#include "flatbuffers/reflection_generated.h"

namespace flatbuffers {
namespace {

// To reduce typing
namespace r = ::reflection;

class LuaBfbsGenerator : public BaseBfbsGenerator {
 public:
  explicit LuaBfbsGenerator(const std::string &flatc_version)
      : BaseBfbsGenerator(),
        indent_level_(0),
        characters_per_indent_(2),
        indent_char_(' '),
        keywords_(),
        requires_(),
        current_block_(),
        current_obj_(nullptr),
        current_enum_(nullptr),
        flatc_version_(flatc_version) {
    static const char *const keywords[] = {
      "and",      "break",  "do",   "else", "elseif", "end",  "false", "for",
      "function", "goto",   "if",   "in",   "local",  "nil",  "not",   "or",
      "repeat",   "return", "then", "true", "until",  "while"
    };
    keywords_.insert(std::begin(keywords), std::end(keywords));
  }

  GeneratorStatus generate(const r::Schema *schema) FLATBUFFERS_OVERRIDE {
    if (!generate_enums(schema->enums())) { return FAILED; }
    if (!generate_objects(schema->objects(), schema->root_table())) {
      return FAILED;
    }
    return OK;
  }

 protected:
  bool generate_enums(
      const flatbuffers::Vector<flatbuffers::Offset<r::Enum>> *enums) {
    for (auto it = enums->cbegin(); it != enums->cend(); ++it) {
      auto enum_def = *it;
      start_code_block(enum_def);
      std::string ns;
      const std::string enum_name = generate_enum(enum_def, ns);
      emit_code_block(enum_name, ns, enum_def->declaration_file()->str());
    }
    return true;
  }

  std::string generate_enum(const r::Enum *enum_def, std::string &ns) {
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
                    NumToString(enum_val->value()) + ",");
      }
      dedent();
    }
    append_line("}");
    append_line();
    return enum_name;
  }

  bool generate_objects(
      const flatbuffers::Vector<flatbuffers::Offset<r::Object>> *objects,
      const r::Object *root_object) {
    for (auto it = objects->cbegin(); it != objects->cend(); ++it) {
      auto object_def = *it;
      start_code_block(object_def);
      std::string ns;
      // Register the main flatbuffers module.
      register_requires("flatbuffers", "flatbuffers");
      const std::string object_name =
          generate_object(object_def, ns, object_def == root_object);
      emit_code_block(object_name, ns, object_def->declaration_file()->str());
    }
    return true;
  }

  std::string generate_object(const r::Object *object_def, std::string &ns,
                              bool is_root_object) {
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
    if (is_root_object) { generate_root_object(object_name); }
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

    // TODO(derekbailey): The reflection IR sorts by field.name instead of
    // field.id. For now we create a mapping to "sort" by id instead.
    const std::vector<uint32_t> field_to_id_map = map_by_field_id(object_def);
    for (size_t i = 0; i < field_to_id_map.size(); ++i) {
      generate_object_field(object_def,
                            object_def->fields()->Get(field_to_id_map[i]));
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
                    NumToString(object_def->fields()->size()) + ")");
        dedent();
      }
      append_line("end");
      append_line();

      for (size_t i = 0; i < field_to_id_map.size(); ++i) {
        auto field = object_def->fields()->Get(field_to_id_map[i]);
        if (field->deprecated()) { continue; }

        const std::string field_name = normalize_name(field->name());

        append_line("function " + object_name + ".Add" +
                    make_camel_case(field_name) + "(builder, " +
                    make_camel_case(field_name, false) + ")");
        {
          indent();
          append_line("builder:Prepend" + generate_method(field) + "Slot(" +
                      NumToString(i) + ", " +
                      make_camel_case(field_name, false) + ", " +
                      default_value(field) + ")");
          dedent();
        }
        append_line("end");
        append_line();

        if (IsVector(field->type()->base_type())) {
          append_line("function " + object_name + ".Start" +
                      make_camel_case(field_name) +
                      "Vector(builder, numElems)");
          {
            indent();
            const int32_t element_size = field->type()->element_size();
            int32_t alignment = 0;
            if (IsStruct(field->type(), /*use_element=*/true)) {
              alignment =
                  get_object_by_index(field->type()->index())->minalign();
            } else {
              alignment = element_size;
            }

            append_line("return builder:StartVector(" +
                        NumToString(element_size) + ", numElems, " +
                        NumToString(alignment) + ")");
            dedent();
          }
          append_line("end");
          append_line();
        }
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

  void generate_root_object(const std::string &object_name) {
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
                             const r::Field *field_def) {
    // Skip writing deprecated fields altogether.
    if (field_def->deprecated()) { return true; }

    const std::string field_name = normalize_name(field_def->name());
    const std::string field_name_camel_case = make_camel_case(field_name);
    const r::BaseType base_type = field_def->type()->base_type();

    // Generate some fixed strings so we don't repeat outselves later.
    const std::string getter_signature =
        "function mt:" + field_name_camel_case + "()";
    const std::string offset_prefix =
        "local o = self.view:Offset(" + NumToString(field_def->offset()) + ")";
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
                      "self.view.pos + " + NumToString(field_def->offset()) +
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
        case r::String: {
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
        case r::Obj: {
          if (object_def->is_struct()) {
            append_line("function mt:" + field_name_camel_case + "(obj)");
            {
              indent();
              append_line("obj:Init(self.view.bytes, self.view.pos + " +
                          NumToString(field_def->offset()) + ")");
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

                const r::Object *field_object = get_object(field_def->type());
                if (!field_object) {
                  // TODO(derekbailey): this is an error condition. we should
                  // report it better.
                  return false;
                }
                append_line(
                    "local x = " +
                    std::string(field_object->is_struct()
                                    ? "self.view.pos + o"
                                    : "self.view:Indirect(self.view.pos + o)"));
                const std::string require_name = register_requires(field_def);
                append_line("local obj = " + require_name + ".New()");
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
        case r::Union: {
          append_line(getter_signature);
          {
            indent();
            append_line(offset_prefix);
            append_line(offset_prefix_2);
            {
              indent();
              append_line(
                  "local obj = "
                  "flatbuffers.view.New(flatbuffers.binaryArray.New("
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
        case r::Array:
        case r::Vector: {
          const r::BaseType vector_base_type = field_def->type()->element();
          int32_t element_size = field_def->type()->element_size();
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
                  append_line("x = x + ((j-1) * " + NumToString(element_size) +
                              ")");
                  if (IsTable(field_def->type(), /*use_element=*/true)) {
                    append_line("x = self.view:Indirect(x)");
                  } else {
                    // Vector of structs are inline, so we need to query the
                    // size of the struct.
                    const reflection::Object *obj =
                        get_object_by_index(field_def->type()->index());
                    element_size = obj->bytesize();
                  }

                  // Include the referenced type, thus we need to make sure
                  // we set `use_element` to true.
                  const std::string require_name =
                      register_requires(field_def, /*use_element=*/true);
                  append_line("local obj = " + require_name + ".New()");
                  append_line("obj:Init(self.view.bytes, x)");
                  append_line("return obj");
                } else {
                  append_line("local a = self.view:Vector(o)");
                  append_line("return " + generate_getter(field_def->type()) +
                              "a + ((j-1) * " + NumToString(element_size) +
                              "))");
                }
                dedent();
              }
              append_line("end");
              // Only generate a default value for those types that are
              // supported.
              if (!IsStructOrTable(vector_base_type)) {
                append_line(
                    "return " +
                    std::string(vector_base_type == r::String ? "''" : "0"));
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
                            NumToString(field_def->offset()) +
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
        default: {
          return false;
        }
      }
    }
    return true;
  }

 private:
  void indent() { indent_level_++; }
  void dedent() { indent_level_--; }

  std::string indentation() const {
    return std::string(characters_per_indent_ * indent_level_, indent_char_);
  }

  void append_comments(
      const flatbuffers::Vector<flatbuffers::Offset<flatbuffers::String>>
          *comments) {
    if (!comments) { return; }
    for (auto it = comments->cbegin(); it != comments->cend(); ++it) {
      append_line("--" + it->str());
    }
  }

  std::string generate_struct_builder_args(const r::Object *object,
                                           std::string prefix = "") {
    // Structs need to be order by field.id, but the IR orders them by
    // field.name. So we first have to sort by field.id.
    const std::vector<uint32_t> field_to_id_map = map_by_field_id(object);

    std::string signature;
    for (size_t i = 0; i < field_to_id_map.size(); ++i) {
      auto field = object->fields()->Get(field_to_id_map[i]);
      if (IsStructOrTable(field->type()->base_type())) {
        const r::Object *field_object = get_object(field->type());
        signature += generate_struct_builder_args(
            field_object, prefix + normalize_name(field->name()) + "_");
      } else {
        signature += ", " + prefix +
                     make_camel_case(normalize_name(field->name()), false);
      }
    }
    return signature;
  }

  void append_struct_builder_body(const r::Object *object,
                                  std::string prefix = "") {
    // Structs need to be order by field.id, but the IR orders them by
    // field.name. So we first have to sort by field.id.
    const std::vector<uint32_t> field_to_id_map = map_by_field_id(object);

    append_line("builder:Prep(" + NumToString(object->minalign()) + ", " +
                NumToString(object->bytesize()) + ")");

    // We need to reverse the order we iterate over, since we build the buffer
    // backwards.
    for (int i = static_cast<int>(field_to_id_map.size()) - 1; i >= 0; --i) {
      auto field = object->fields()->Get(field_to_id_map[i]);
      const int32_t num_padding_bytes = field->padding();
      if (num_padding_bytes) {
        append_line("builder:Pad(" + NumToString(num_padding_bytes) + ")");
      }
      if (IsStructOrTable(field->type()->base_type())) {
        const r::Object *field_object = get_object(field->type());
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
      case r::String: return "self.view:String(";
      case r::Union: return "self.view:Union(";
      case r::Vector: return generate_getter(type, true);
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
      case r::String: return "string";
      case r::Vector: return generate_getter(type, true);
      case r::Obj: {
        const r::Object *obj = get_object(type);
        return normalize_name(denamespace(obj->name()));
      };
      default: return "*flatbuffers.Table";
    }
  }

  std::string generate_type(const r::BaseType base_type) {
    // Need to override the default naming to match the Lua runtime libraries.
    // TODO(derekbailey): make overloads in the runtime libraries to avoid this.
    switch (base_type) {
      case r::None: return "uint8";
      case r::UType: return "uint8";
      case r::Byte: return "int8";
      case r::UByte: return "uint8";
      case r::Short: return "int16";
      case r::UShort: return "uint16";
      case r::Int: return "int32";
      case r::UInt: return "uint32";
      case r::Long: return "int64";
      case r::ULong: return "uint64";
      case r::Float: return "Float32";
      case r::Double: return "Float64";
      default: return r::EnumNameBaseType(base_type);
    }
  }

  std::string default_value(const r::Field *field) {
    const r::BaseType base_type = field->type()->base_type();
    if (IsFloatingPoint(base_type)) {
      return NumToString(field->default_real());
    }
    if (IsBool(base_type)) {
      return field->default_integer() ? "true" : "false";
    }
    if (IsScalar(base_type)) { return NumToString((field->default_integer())); }
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

  void start_code_block(const reflection::Enum *enum_def) {
    current_enum_ = enum_def;
    current_obj_ = nullptr;
    start_code_block();
  }

  void start_code_block(const reflection::Object *object_def) {
    current_obj_ = object_def;
    current_enum_ = nullptr;
    start_code_block();
  }

  void start_code_block() {
    current_block_.clear();
    requires_.clear();
  }

  std::string code_block() { return current_block_; }

  void append_line() { current_block_ += indentation() + "\n"; }

  void append_line(std::string to_append) { append(to_append + "\n"); }

  void append(std::string to_append) {
    current_block_ += indentation() + to_append;
  }

  std::string register_requires(const r::Field *field,
                                bool use_element = false) {
    std::string type_name;

    const r::BaseType type =
        use_element ? field->type()->element() : field->type()->base_type();

    if (IsStructOrTable(type)) {
      const r::Object *object = get_object_by_index(field->type()->index());
      if (object == current_obj_) { return denamespace(object->name()); }
      type_name = object->name()->str();
    } else {
      const r::Enum *enum_def = get_enum_by_index(field->type()->index());
      if (enum_def == current_enum_) { return denamespace(enum_def->name()); }
      type_name = enum_def->name()->str();
    }

    // Prefix with double __ to avoid name clashing, since these are defined
    // at the top of the file and have lexical scoping. Replace '.' with '_' so
    // it can be a legal identifier.
    std::string name = "__" + type_name;
    std::replace(name.begin(), name.end(), '.', '_');

    return register_requires(name, type_name);
  }

  std::string register_requires(const std::string &local_name,
                                const std::string &requires_name) {
    requires_[local_name] = requires_name;
    return local_name;
  }

  void emit_code_block(const std::string &name, const std::string &ns,
                       const std::string &declaring_file) {
    const std::string root_type = schema_->root_table()->name()->str();
    const std::string root_file =
        schema_->root_table()->declaration_file()->str();
    const std::string full_qualified_name = ns.empty() ? name : ns + "." + name;

    std::string code = "--[[ " + full_qualified_name + "\n\n";
    code +=
        "  Automatically generated by the FlatBuffers compiler, do not "
        "modify.\n";
    code += "  Or modify. I'm a message, not a cop.\n";
    code += "\n";
    code += "  flatc version: " + flatc_version_ + "\n";
    code += "\n";
    code += "  Declared by  : " + declaring_file + "\n";
    code += "  Rooting type : " + root_type + " (" + root_file + ")\n";
    code += "\n--]]\n\n";

    if (!requires_.empty()) {
      for (auto it = requires_.cbegin(); it != requires_.cend(); ++it) {
        code += "local " + it->first + " = require('" + it->second + "')\n";
      }
      code += "\n";
    }

    code += code_block();
    code += "return " + name;

    // Namespaces are '.' deliminted, so replace it with the path separator.
    std::string path = ns;

    if (path.empty()) {
      path = ".";
    } else {
      std::replace(path.begin(), path.end(), '.', '/');
    }

    // TODO(derekbailey): figure out a save file without depending on util.h
    EnsureDirExists(path);
    const std::string file_name = path + "/" + name + ".lua";
    SaveFile(file_name.c_str(), code, false);
  }

  int8_t indent_level_;
  const int8_t characters_per_indent_;
  const char indent_char_;
  std::unordered_set<std::string> keywords_;
  std::map<std::string, std::string> requires_;
  std::string current_block_;
  const r::Object *current_obj_;
  const r::Enum *current_enum_;
  const std::string flatc_version_;
};
}  // namespace

std::unique_ptr<BfbsGenerator> NewLuaBfbsGenerator(
    const std::string &flatc_version) {
  return std::unique_ptr<LuaBfbsGenerator>(new LuaBfbsGenerator(flatc_version));
}

}  // namespace flatbuffers