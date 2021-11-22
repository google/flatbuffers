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

  GeneratorStatus GenerateFromSchema(const r::Schema *schema)
      FLATBUFFERS_OVERRIDE {
    if (!GenerateEnums(schema->enums())) { return FAILED; }
    if (!GenerateObjects(schema->objects(), schema->root_table())) {
      return FAILED;
    }
    return OK;
  }

  uint64_t SupportedAdvancedFeatures() const FLATBUFFERS_OVERRIDE {
    return 0xF;
  }

 protected:
  bool GenerateEnums(
      const flatbuffers::Vector<flatbuffers::Offset<r::Enum>> *enums) {
    for (auto it = enums->cbegin(); it != enums->cend(); ++it) {
      auto enum_def = *it;
      StartCodeBlock(enum_def);
      std::string ns;
      const std::string enum_name = GenerateEnum(enum_def, ns);
      EmitCodeBlock(enum_name, ns, enum_def->declaration_file()->str());
    }
    return true;
  }

  std::string GenerateEnum(const r::Enum *enum_def, std::string &ns) {
    const std::string enum_name =
        NormalizeName(Denamespace(enum_def->name(), ns));

    AppendComments(enum_def->documentation());
    AppendLine("local " + enum_name + " = {");
    {
      // TODO(derekbailey): It would be cool if this would auto Dedent on
      // leaving scope.
      Indent();
      for (auto it = enum_def->values()->cbegin();
           it != enum_def->values()->cend(); ++it) {
        const auto enum_val = *it;
        AppendComments(enum_val->documentation());
        AppendLine(NormalizeName(enum_val->name()) + " = " +
                   NumToString(enum_val->value()) + ",");
      }
      Dedent();
    }
    AppendLine("}");
    AppendLine();
    return enum_name;
  }

  bool GenerateObjects(
      const flatbuffers::Vector<flatbuffers::Offset<r::Object>> *objects,
      const r::Object *root_object) {
    for (auto it = objects->cbegin(); it != objects->cend(); ++it) {
      auto object_def = *it;
      StartCodeBlock(object_def);
      std::string ns;
      // Register the main flatbuffers module.
      RegisterRequires("flatbuffers", "flatbuffers");
      const std::string object_name =
          GenerateObject(object_def, ns, object_def == root_object);
      EmitCodeBlock(object_name, ns, object_def->declaration_file()->str());
    }
    return true;
  }

  std::string GenerateObject(const r::Object *object_def, std::string &ns,
                             bool is_root_object) {
    const std::string object_name =
        NormalizeName(Denamespace(object_def->name(), ns));

    AppendComments(object_def->documentation());
    AppendLine("local " + object_name + " = {}");
    AppendLine("local mt = {}");
    AppendLine();
    AppendLine("function " + object_name + ".New()");
    {
      Indent();
      AppendLine("local o = {}");
      AppendLine("setmetatable(o, {__index = mt})");
      AppendLine(("return o"));
      Dedent();
    }
    AppendLine("end");
    if (is_root_object) { GenerateRootObject(object_name); }
    AppendLine();

    // Generates a init method that receives a pre-existing accessor object, so
    // that objects can be reused.
    AppendLine("function mt:Init(buf, pos)");
    {
      Indent();
      AppendLine("self.view = flatbuffers.view.New(buf, pos)");
      Dedent();
    }
    AppendLine("end");
    AppendLine();

    // Create all the field accessors.

    // TODO(derekbailey): The reflection IR sorts by field.name instead of
    // field.id. For now we create a mapping to "sort" by id instead.
    const std::vector<uint32_t> field_to_id_map = FieldIdToIndex(object_def);
    for (size_t i = 0; i < field_to_id_map.size(); ++i) {
      GenerateObjectField(object_def,
                          object_def->fields()->Get(field_to_id_map[i]));
    }

    // Create all the builders
    if (object_def->is_struct()) {
      AppendLine("function " + object_name + ".Create" + object_name +
                 "(builder" + GenerateStructBuilderArgs(object_def) + ")");
      {
        Indent();
        AppendStructBuilderBody(object_def);
        AppendLine("return builder:Offset()");
        Dedent();
      }
      AppendLine("end");
      AppendLine();
    } else {
      // Table builders
      AppendLine("function " + object_name + ".Start(builder)");
      {
        Indent();
        AppendLine("builder:StartObject(" +
                   NumToString(object_def->fields()->size()) + ")");
        Dedent();
      }
      AppendLine("end");
      AppendLine();

      for (size_t i = 0; i < field_to_id_map.size(); ++i) {
        auto field = object_def->fields()->Get(field_to_id_map[i]);
        if (field->deprecated()) { continue; }

        const std::string field_name = NormalizeName(field->name());

        AppendLine("function " + object_name + ".Add" +
                   MakeCamelCase(field_name) + "(builder, " +
                   MakeCamelCase(field_name, false) + ")");
        {
          Indent();
          AppendLine("builder:Prepend" + GenerateMethod(field) + "Slot(" +
                     NumToString(i) + ", " + MakeCamelCase(field_name, false) +
                     ", " + DefaultValue(field) + ")");
          Dedent();
        }
        AppendLine("end");
        AppendLine();

        if (IsVector(field->type()->base_type())) {
          AppendLine("function " + object_name + ".Start" +
                     MakeCamelCase(field_name) + "Vector(builder, numElems)");
          {
            Indent();
            const int32_t element_size = field->type()->element_size();
            int32_t alignment = 0;
            if (IsStruct(field->type(), /*use_element=*/true)) {
              alignment = GetObjectByIndex(field->type()->index())->minalign();
            } else {
              alignment = element_size;
            }

            AppendLine("return builder:StartVector(" +
                       NumToString(element_size) + ", numElems, " +
                       NumToString(alignment) + ")");
            Dedent();
          }
          AppendLine("end");
          AppendLine();
        }
      }

      AppendLine("function " + object_name + ".End(builder)");
      {
        Indent();
        AppendLine("return builder:EndObject()");
        Dedent();
      }
      AppendLine("end");
      AppendLine();
    }

    return object_name;
  }

  void GenerateRootObject(const std::string &object_name) {
    AppendLine();
    AppendLine("function " + object_name + ".GetRootAs" + object_name +
               "(buf, offset)");
    {
      Indent();
      AppendLine("if type(buf) == \"string\" then");
      {
        Indent();
        AppendLine("buf = flatbuffers.binaryArray.New(buf)");
        Dedent();
      }
      AppendLine("end");
      AppendLine();
      AppendLine("local n = flatbuffers.N.UOffsetT:Unpack(buf, offset)");
      AppendLine("local o = " + object_name + ".New()");
      AppendLine("o:Init(buf, n + offset)");
      AppendLine("return o");
      Dedent();
    }
    AppendLine("end");
  }

  bool GenerateObjectField(const r::Object *object_def,
                           const r::Field *field_def) {
    // Skip writing deprecated fields altogether.
    if (field_def->deprecated()) { return true; }

    const std::string field_name = NormalizeName(field_def->name());
    const std::string field_name_camel_case = MakeCamelCase(field_name);
    const r::BaseType base_type = field_def->type()->base_type();

    // Generate some fixed strings so we don't repeat outselves later.
    const std::string getter_signature =
        "function mt:" + field_name_camel_case + "()";
    const std::string offset_prefix =
        "local o = self.view:Offset(" + NumToString(field_def->offset()) + ")";
    const std::string offset_prefix_2 = "if o ~= 0 then";

    AppendComments(field_def->documentation());
    if (IsScalar(base_type)) {
      AppendLine(getter_signature);
      {
        Indent();
        if (object_def->is_struct()) {
          // TODO(derekbailey): it would be nice to modify the view:Get to just
          // pass in the offset and not have to add it its own self.view.pos.
          AppendLine("return " + GenerateGetter(field_def->type()) +
                     "self.view.pos + " + NumToString(field_def->offset()) +
                     ")");
        } else {
          // Table accessors
          AppendLine(offset_prefix);
          AppendLine(offset_prefix_2);
          {
            Indent();
            std::string getter =
                GenerateGetter(field_def->type()) + "self.view.pos + o)";
            if (IsBool(base_type)) { getter = "(" + getter + " ~=0)"; }
            AppendLine("return " + getter);
            Dedent();
          }
          AppendLine("end");
          AppendLine("return " + DefaultValue(field_def));
        }
        Dedent();
      }
      AppendLine("end");
      AppendLine();
    } else {
      switch (base_type) {
        case r::String: {
          AppendLine(getter_signature);
          {
            Indent();
            AppendLine(offset_prefix);
            AppendLine(offset_prefix_2);
            {
              Indent();
              AppendLine("return " + GenerateGetter(field_def->type()) +
                         "self.view.pos + o)");
              Dedent();
            }
            AppendLine("end");
            Dedent();
          }
          AppendLine("end");
          AppendLine();
          break;
        }
        case r::Obj: {
          if (object_def->is_struct()) {
            AppendLine("function mt:" + field_name_camel_case + "(obj)");
            {
              Indent();
              AppendLine("obj:Init(self.view.bytes, self.view.pos + " +
                         NumToString(field_def->offset()) + ")");
              AppendLine("return obj");
              Dedent();
            }
          } else {
            AppendLine(getter_signature);
            {
              Indent();
              AppendLine(offset_prefix);
              AppendLine(offset_prefix_2);
              {
                Indent();

                const r::Object *field_object = GetObject(field_def->type());
                if (!field_object) {
                  // TODO(derekbailey): this is an error condition. we should
                  // report it better.
                  return false;
                }
                AppendLine(
                    "local x = " +
                    std::string(field_object->is_struct()
                                    ? "self.view.pos + o"
                                    : "self.view:Indirect(self.view.pos + o)"));
                const std::string require_name = RegisterRequires(field_def);
                AppendLine("local obj = " + require_name + ".New()");
                AppendLine("obj:Init(self.view.bytes, x)");
                AppendLine("return obj");
                Dedent();
              }
              AppendLine("end");
              Dedent();
            }
          }
          AppendLine("end");
          AppendLine();
          break;
        }
        case r::Union: {
          AppendLine(getter_signature);
          {
            Indent();
            AppendLine(offset_prefix);
            AppendLine(offset_prefix_2);
            {
              Indent();
              AppendLine(
                  "local obj = "
                  "flatbuffers.view.New(flatbuffers.binaryArray.New("
                  "0), 0)");
              AppendLine(GenerateGetter(field_def->type()) + "obj, o)");
              AppendLine("return obj");
              Dedent();
            }
            AppendLine("end");
            Dedent();
          }
          AppendLine("end");
          AppendLine();
          break;
        }
        case r::Array:
        case r::Vector: {
          const r::BaseType vector_base_type = field_def->type()->element();
          int32_t element_size = field_def->type()->element_size();
          {
            AppendLine("function mt:" + field_name_camel_case + "(j)");
            {
              Indent();
              AppendLine(offset_prefix);
              AppendLine(offset_prefix_2);
              {
                Indent();
                if (IsStructOrTable(vector_base_type)) {
                  AppendLine("local x = self.view:Vector(o)");
                  AppendLine("x = x + ((j-1) * " + NumToString(element_size) +
                             ")");
                  if (IsTable(field_def->type(), /*use_element=*/true)) {
                    AppendLine("x = self.view:Indirect(x)");
                  } else {
                    // Vector of structs are inline, so we need to query the
                    // size of the struct.
                    const reflection::Object *obj =
                        GetObjectByIndex(field_def->type()->index());
                    element_size = obj->bytesize();
                  }

                  // Include the referenced type, thus we need to make sure
                  // we set `use_element` to true.
                  const std::string require_name =
                      RegisterRequires(field_def, /*use_element=*/true);
                  AppendLine("local obj = " + require_name + ".New()");
                  AppendLine("obj:Init(self.view.bytes, x)");
                  AppendLine("return obj");
                } else {
                  AppendLine("local a = self.view:Vector(o)");
                  AppendLine("return " + GenerateGetter(field_def->type()) +
                             "a + ((j-1) * " + NumToString(element_size) +
                             "))");
                }
                Dedent();
              }
              AppendLine("end");
              // Only generate a default value for those types that are
              // supported.
              if (!IsStructOrTable(vector_base_type)) {
                AppendLine("return " + std::string(vector_base_type == r::String
                                                       ? "''"
                                                       : "0"));
              }
              Dedent();
            }
            AppendLine("end");
            AppendLine();

            // If the vector is composed of single byte values, we generate a
            // helper function to get it as a byte string in Lua.
            if (IsSingleByte(vector_base_type)) {
              AppendLine("function mt:" + field_name_camel_case +
                         "AsString(start, stop)");
              {
                Indent();
                AppendLine("return self.view:VectorAsString(" +
                           NumToString(field_def->offset()) + ", start, stop)");
                Dedent();
              }
              AppendLine("end");
              AppendLine();
            }
          }

          {
            // We also make a new accessor to query just the length of the
            // vector.
            AppendLine("function mt:" + field_name_camel_case + "Length()");
            {
              Indent();
              AppendLine(offset_prefix);
              AppendLine(offset_prefix_2);
              {
                Indent();
                AppendLine("return self.view:VectorLen(o)");
                Dedent();
              }
              AppendLine("end");
              AppendLine("return 0");
              Dedent();
            }
            AppendLine("end");
            AppendLine();
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
  void Indent() { indent_level_++; }
  void Dedent() { indent_level_--; }

  std::string Indentation() const {
    return std::string(characters_per_indent_ * indent_level_, indent_char_);
  }

  void AppendComments(
      const flatbuffers::Vector<flatbuffers::Offset<flatbuffers::String>>
          *comments) {
    if (!comments) { return; }
    for (auto it = comments->cbegin(); it != comments->cend(); ++it) {
      AppendLine("--" + it->str());
    }
  }

  std::string GenerateStructBuilderArgs(const r::Object *object,
                                        std::string prefix = "") {
    // Structs need to be order by field.id, but the IR orders them by
    // field.name. So we first have to sort by field.id.
    const std::vector<uint32_t> field_to_id_map = FieldIdToIndex(object);

    std::string signature;
    for (size_t i = 0; i < field_to_id_map.size(); ++i) {
      auto field = object->fields()->Get(field_to_id_map[i]);
      if (IsStructOrTable(field->type()->base_type())) {
        const r::Object *field_object = GetObject(field->type());
        signature += GenerateStructBuilderArgs(
            field_object, prefix + NormalizeName(field->name()) + "_");
      } else {
        signature +=
            ", " + prefix + MakeCamelCase(NormalizeName(field->name()), false);
      }
    }
    return signature;
  }

  void AppendStructBuilderBody(const r::Object *object,
                               std::string prefix = "") {
    // Structs need to be order by field.id, but the IR orders them by
    // field.name. So we first have to sort by field.id.
    const std::vector<uint32_t> field_to_id_map = FieldIdToIndex(object);

    AppendLine("builder:Prep(" + NumToString(object->minalign()) + ", " +
               NumToString(object->bytesize()) + ")");

    // We need to reverse the order we iterate over, since we build the buffer
    // backwards.
    for (int i = static_cast<int>(field_to_id_map.size()) - 1; i >= 0; --i) {
      auto field = object->fields()->Get(field_to_id_map[i]);
      const int32_t num_padding_bytes = field->padding();
      if (num_padding_bytes) {
        AppendLine("builder:Pad(" + NumToString(num_padding_bytes) + ")");
      }
      if (IsStructOrTable(field->type()->base_type())) {
        const r::Object *field_object = GetObject(field->type());
        AppendStructBuilderBody(field_object,
                                prefix + NormalizeName(field->name()) + "_");
      } else {
        AppendLine("builder:Prepend" + GenerateMethod(field) + "(" + prefix +
                   MakeCamelCase(NormalizeName(field->name()), false) + ")");
      }
    }
  }

  std::string GenerateMethod(const r::Field *field) {
    const r::BaseType base_type = field->type()->base_type();
    if (IsScalar(base_type)) { return MakeCamelCase(GenerateType(base_type)); }
    if (IsStructOrTable(base_type)) { return "Struct"; }
    return "UOffsetTRelative";
  }

  std::string GenerateGetter(const r::Type *type, bool element_type = false) {
    switch (element_type ? type->element() : type->base_type()) {
      case r::String: return "self.view:String(";
      case r::Union: return "self.view:Union(";
      case r::Vector: return GenerateGetter(type, true);
      default:
        return "self.view:Get(flatbuffers.N." +
               MakeCamelCase(GenerateType(type, element_type)) + ", ";
    }
  }

  std::string GenerateType(const r::Type *type, bool element_type = false) {
    const r::BaseType base_type =
        element_type ? type->element() : type->base_type();
    if (IsScalar(base_type)) { return GenerateType(base_type); }
    switch (base_type) {
      case r::String: return "string";
      case r::Vector: return GenerateGetter(type, true);
      case r::Obj: {
        const r::Object *obj = GetObject(type);
        return NormalizeName(Denamespace(obj->name()));
      };
      default: return "*flatbuffers.Table";
    }
  }

  std::string GenerateType(const r::BaseType base_type) {
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

  std::string DefaultValue(const r::Field *field) {
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

  std::string NormalizeName(const std::string name) const {
    return keywords_.find(name) == keywords_.end() ? name : "_" + name;
  }

  std::string NormalizeName(const flatbuffers::String *name) const {
    return NormalizeName(name->str());
  }

  std::string Denamespace(const flatbuffers::String *name) {
    std::string ns;
    return Denamespace(name, ns);
  }

  std::string Denamespace(const flatbuffers::String *name, std::string &ns) {
    const size_t pos = name->str().find_last_of('.');
    if (pos == std::string::npos) {
      ns = "";
      return name->str();
    }
    ns = name->str().substr(0, pos);
    return name->str().substr(pos + 1);
  }

  void StartCodeBlock(const reflection::Enum *enum_def) {
    current_enum_ = enum_def;
    current_obj_ = nullptr;
    StartCodeBlock();
  }

  void StartCodeBlock(const reflection::Object *object_def) {
    current_obj_ = object_def;
    current_enum_ = nullptr;
    StartCodeBlock();
  }

  void StartCodeBlock() {
    current_block_.clear();
    requires_.clear();
  }

  std::string CodeBlock() { return current_block_; }

  void AppendLine() { current_block_ += Indentation() + "\n"; }

  void AppendLine(std::string to_append) { Append(to_append + "\n"); }

  void Append(std::string to_append) {
    current_block_ += Indentation() + to_append;
  }

  std::string RegisterRequires(const r::Field *field,
                               bool use_element = false) {
    std::string type_name;

    const r::BaseType type =
        use_element ? field->type()->element() : field->type()->base_type();

    if (IsStructOrTable(type)) {
      const r::Object *object = GetObjectByIndex(field->type()->index());
      if (object == current_obj_) { return Denamespace(object->name()); }
      type_name = object->name()->str();
    } else {
      const r::Enum *enum_def = GetEnumByIndex(field->type()->index());
      if (enum_def == current_enum_) { return Denamespace(enum_def->name()); }
      type_name = enum_def->name()->str();
    }

    // Prefix with double __ to avoid name clashing, since these are defined
    // at the top of the file and have lexical scoping. Replace '.' with '_' so
    // it can be a legal identifier.
    std::string name = "__" + type_name;
    std::replace(name.begin(), name.end(), '.', '_');

    return RegisterRequires(name, type_name);
  }

  std::string RegisterRequires(const std::string &local_name,
                               const std::string &requires_name) {
    requires_[local_name] = requires_name;
    return local_name;
  }

  void EmitCodeBlock(const std::string &name, const std::string &ns,
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

    code += CodeBlock();
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