/*
 * Copyright 2025 Google Inc. All rights reserved.
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

// TODO:
// * comment everything
// * squash commits
// * fix vector of unions
// * handle windows paths
// * test optional scalars (should work?)
// * print default vectors (this is just empty vs null) and strings (which have
// non null defaults)

// Future work:
// * nested_flatbuffer - call Parse<name> if I can get the data out of the
// reflection
// * bitfields - well known attributes are not passed down to us
// * flexbuffers

#include "bfbs_gen_wireshark.h"

#include <map>
#include <unordered_set>
#include <utility>

// Ensure no includes to flatc internals. bfbs_gen.h and generator.h are OK.
#include "bfbs_gen.h"
#include "bfbs_namer.h"

// The intermediate representation schema.
#include "flatbuffers/code_generator.h"
#include "flatbuffers/reflection.h"
#include "flatbuffers/reflection_generated.h"

namespace flatbuffers {
namespace {
// To reduce typing
namespace r = ::reflection;

std::set<std::string> LuaKeywords() {
  return { "and",   "break", "do",       "else", "elseif", "end",
           "false", "for",   "function", "goto", "if",     "in",
           "local", "nil",   "not",      "or",   "repeat", "return",
           "then",  "true",  "until",    "while" };
}

Namer::Config WiresharkDefaultConfig() {
  return { /*types=*/Case::kUpperCamel,
           /*constants=*/Case::kUnknown,
           /*methods=*/Case::kUpperCamel,
           /*functions=*/Case::kUpperCamel,
           /*fields=*/Case::kUpperCamel,
           /*variables=*/Case::kLowerCamel,
           /*variants=*/Case::kKeep,
           /*enum_variant_seperator=*/"",
           /*escape_keywords=*/Namer::Config::Escape::AfterConvertingCase,
           /*namespaces=*/Case::kKeep,
           /*namespace_seperator=*/"__",
           /*object_prefix=*/"",
           /*object_suffix=*/"",
           /*keyword_prefix=*/"",
           /*keyword_suffix=*/"_",
           /*filenames=*/Case::kKeep,
           /*directories=*/Case::kKeep,
           /*output_path=*/"",
           /*filename_suffix=*/"",
           /*filename_extension=*/".lua" };
}

// Returns the parser function name (defined in 00_wireshark_numbers.lua) for a
// given scalar type
std::string GetScalarParserFunction(r::BaseType type) {
  switch (type) {
    case r::Bool: return "Parse_Bool";
    case r::UType:
    case r::Byte:
    case r::UByte: return "Parse_Uint8";
    case r::Short: return "Parse_Int16";
    case r::UShort: return "Parse_Uint16";
    case r::Int: return "Parse_Int32";
    case r::UInt: return "Parse_Uint32";
    case r::Long: return "Parse_Int64";
    case r::ULong: return "Parse_Uint64";
    case r::Float: return "Parse_Float32";
    case r::Double: return "Parse_Float64";
    default: FLATBUFFERS_ASSERT(false);
  }
}

// Returns the FlatBuffers defined Proto Field type for a given scalar type
std::string GetScalarParserProtoField(r::BaseType type) {
  switch (type) {
    case r::Bool: return "fb_bool";
    case r::UType:
    case r::Byte:
    case r::UByte: return "fb_uint8";
    case r::Short: return "fb_int16";
    case r::UShort: return "fb_uint16";
    case r::Int: return "fb_int32";
    case r::UInt: return "fb_uint32";
    case r::Long: return "fb_int64";
    case r::ULong: return "fb_uint64";
    case r::Float: return "fb_float32";
    case r::Double: return "fb_float64";
    default: FLATBUFFERS_ASSERT(false);
  }
}

// helper function to convert named elements to underscore format for use as lua
// variables
void to_underscore(std::string &str, char match = '.') {
  std::replace(str.begin(), str.end(), match, '_');
}

class WiresharkBfbsGenerator : public BaseBfbsGenerator {
 public:
  explicit WiresharkBfbsGenerator(const std::string &flatc_version)
      : BaseBfbsGenerator(),
        schema_(nullptr),
        options_(),
        flatc_version_(flatc_version),
        namer_(WiresharkDefaultConfig(), LuaKeywords()) {}

  Status GenerateFromSchema(const r::Schema *schema,
                            const CodeGenOptions &options)
      FLATBUFFERS_OVERRIDE {
    schema_ = schema;
    options_ = options;

    // generates 01_<filename>_enums.lua
    // provides lookup tables for enum values to names
    if (!GenerateEnums()) { return ERROR; }
    // generates 01_<filename>_lookup.lua
    // provides lookup tables for member names and fields for each object
    if (!GenerateMemberLookupTables()) { return ERROR; }
    // generates 02_<filename>.lua
    // provides the actual parsing functions for each object in the schema
    // and dissectors for root objects
    if (!GenerateTableFiles()) { return ERROR; }

    return OK;
  }

  using BaseBfbsGenerator::GenerateCode;

  // all of these functions were copied from the lua bfbs generator
  Status GenerateCode(const Parser &, const std::string &,
                      const std::string &) override {
    return Status::NOT_IMPLEMENTED;
  }

  Status GenerateMakeRule(const Parser &, const std::string &,
                          const std::string &, std::string &) override {
    return Status::NOT_IMPLEMENTED;
  }

  Status GenerateGrpcCode(const Parser &, const std::string &,
                          const std::string &) override {
    return Status::NOT_IMPLEMENTED;
  }

  Status GenerateRootFile(const Parser &, const std::string &) override {
    return Status::NOT_IMPLEMENTED;
  }

  bool IsSchemaOnly() const override { return true; }

  bool SupportsBfbsGeneration() const override { return true; }

  bool SupportsRootFileGeneration() const override { return false; }

  IDLOptions::Language Language() const override {
    return IDLOptions::kWireshark;
  }

  std::string LanguageName() const override { return "Wireshark"; }

  uint64_t SupportedAdvancedFeatures() const FLATBUFFERS_OVERRIDE {
    return r::AdvancedArrayFeatures | r::AdvancedUnionFeatures |
           r::OptionalScalars | r::DefaultVectorsAndStrings;
  }

 private:
  // convenience struct for holding all the common strings used in generation
  struct names {
    std::string name_space;
    std::string object_name;
    std::string full_name;
    std::string full_name_underscore;
    std::string declaration_file;
    std::string declaration_file_underscore;

    // Constructor to initialize all members
    names(const std::string &ns, const std::string &obj_name,
          const std::string &full, const std::string &full_underscore,
          const std::string &decl_file, const std::string &decl_file_underscore)
        : name_space(ns),
          object_name(obj_name),
          full_name(full),
          full_name_underscore(full_underscore),
          declaration_file(decl_file),
          declaration_file_underscore(decl_file_underscore) {}

    template<typename T>
    names(const T &object)
        : names(std::string(),                     // name_space
                std::string(),                     // object_name
                object.name()->str(),              // full_name
                object.name()->str(),              // full_name_underscore
                object.declaration_file()->str(),  // declaration_file
                object.declaration_file()->str()  // declaration_file_underscore
          ) {
      static_assert(std::is_base_of<r::Object, T>::value ||
                        std::is_base_of<r::Enum, T>::value,
                    "T must be a reflection object or enum type");

      to_underscore(full_name_underscore);

      const bool is_absolute_path =
          declaration_file.rfind("//", 0) == std::string::npos;

      declaration_file_underscore =
          declaration_file_underscore.substr(is_absolute_path ? 1 : 2);

      to_underscore(declaration_file_underscore, '/');
      to_underscore(declaration_file_underscore);
    }
  };

  // iterate over every enum in the schema and generate a lookup table for each.
  // this allows wireshark to display the enum names instead of just the integer
  // values
  bool GenerateEnums() {
    std::map<std::string, std::string> files;

    ForAllEnums(schema_->enums(), [&](const r::Enum *enum_def) {
      // get all strings up front
      const names object_names(*enum_def);

      const std::string enum_lookup_table_name =
          "ENUM_" + object_names.full_name_underscore;

      // create or access current file contents
      std::string &code = files[object_names.declaration_file_underscore];

      // -- begin actual code generation

      // add header if not present
      if (code.empty()) {
        code += "--[[\n";
        code +=
            "  Automatically generated by the FlatBuffers compiler, do not "
            "modify.\n";
        code += "  This file contains Lua tables for enums in the schema.\n";
        code += "--]]\n\n";
      }

      // create table for enum values
      code += "-- Enum: " + object_names.full_name + "\n";
      code += "" + enum_lookup_table_name + " = {\n";

      // add each enum value entry
      ForAllEnumValues(enum_def, [&](const r::EnumVal *enum_val) {
        const std::string name = enum_val->name()->str();
        const std::string value = NumToString(enum_val->value());

        code += "  [" + value + "] = \"" + name + "\",\n";
      });

      code += "}\n\n";
    });

    // write files to disk
    for (const auto &file : files) {
      const std::string file_name =
          options_.output_path + "01_" + file.first + "_enums.lua";
      SaveFile(file_name.c_str(), file.second, false);
    }

    return true;
  }

  // iterate over every object in the schema and generate lookup tables for
  // member names and fields. These are used in the parse functions so that each
  // field has a dedicated ProtoField definition so that users can take full
  // advantage of wireshark filtering. Two tables are created to enforce
  // ordering.
  bool GenerateMemberLookupTables() {
    std::map<std::string, std::string> files;

    // loop over every object in the schema
    ForAllObjects(schema_->objects(), [&](const r::Object *object) {
      // get all strings up front
      const names object_names(*object);
      const std::string member_name_table =
          object_names.full_name_underscore + "_member_names";
      const std::string memeber_field_table =
          object_names.full_name_underscore + "_member_fields";

      // -- begin actual code generation

      // create or access current file contents
      std::string &code = files[object_names.declaration_file_underscore];

      // add header if not present
      if (code.empty()) {
        code += "--[[\n";
        code +=
            "  Automatically generated by the FlatBuffers compiler, do not "
            "modify.\n";
        code += "  This file contains Lua tables for enums in the schema.\n";
        code += "--]]\n\n";
      }

      code += "-- Object: " + object_names.full_name + "\n";

      // create a table of field names for each object
      code += member_name_table + " = {\n";

      // add each field to the table
      ForAllFields(object, /*reverse=*/false, [&](const r::Field *field) {
        code += "  [" + NumToString(field->id()) + "] = \"" +
                namer_.Variable(field->name()->str()) + "\",\n";
      });

      code += "}\n\n";

      // create a lookup table of ProtoFields
      code += memeber_field_table + " = {\n";

      // add the "self" entry - this uses an integer key so that it won't ever
      // get confused with the fields themselves
      code += "  [1] = ProtoField.bytes(\"" + object_names.full_name +
              "\", \"" + object_names.full_name + "\", base.NONE, \"" +
              object_names.full_name + "\"),\n";

      try {
        // generate the ProtoField definitions for each field in the object
        ForAllFields(object, /*reverse=*/false, [&](const r::Field *field) {
          code += CreateProtoFieldDefinition(field,
                                             object_names.full_name_underscore);
        });

        code += "}\n\n";
      } catch (const std::runtime_error &) { return; }
    });

    // write lookup files to disk
    for (const auto &file : files) {
      const std::string file_name =
          options_.output_path + "01_" + file.first + "_lookup.lua";
      SaveFile(file_name.c_str(), file.second, false);
    }

    return true;
  }

  // iterate over every object and create the parse functions for for every
  // field of every object. Then create the root table/struct parse function,
  // and create wireshark dissectors for root objects.
  bool GenerateTableFiles() {
    std::map<std::string, std::string> files;

    // loop over every object in the schema
    ForAllObjects(schema_->objects(), [&](const r::Object *object) {
      // get all strings up front
      const names object_names(*object);
      const std::string member_name_table =
          object_names.full_name_underscore + "_union_type_enum_values";
      const std::string member_lookup_table =
          object_names.full_name_underscore + "_member_fields";
      const std::string member_names_table =
          object_names.full_name_underscore + "_member_names";
      const std::string member_list_table =
          object_names.full_name_underscore + "_member_list";
      const std::string proto_name =
          "PROTO_" + object_names.full_name_underscore;
      const std::string parse_object_function_name =
          "Parse_" + object_names.full_name_underscore;

      // create or access current file contents
      auto &code = files[object_names.declaration_file_underscore];

      // -- begin actual code generation

      // header
      if (code.empty()) {
        code += "--[[\n";
        code +=
            "  Automatically generated by the FlatBuffers compiler, do not "
            "modify.\n";
        code += "  This file contains Lua tables for enums in the schema.\n";
        code += "--]]\n\n";
      }

      code += "-- Object: " + object_names.full_name + "\n";

      // create a lookup table for union types in this object
      code += "local " + member_name_table + " = {}\n\n";

      // create parser functions for each field
      ForAllFields(object, /*reverse=*/false, [&](const r::Field *field) {
        const std::string field_function_name =
            "Parse_" + namer_.Variable(field->name()->str());

        code += "local function " + field_function_name +
                "(buffer, offset, tree)\n";
        code += "  local field_name = " + member_names_table + "[" +
                NumToString(field->id()) + "]\n";
        code += CreateParserDefinition(field, object, member_lookup_table);
        code += "end\n\n";
      });

      // create the lookup table that matches field names to parse functions
      code += "local " + member_list_table + " = {\n";

      ForAllFields(object, false, [&](const r::Field *field) {
        const std::string field_function_name =
            "Parse_" + namer_.Variable(field->name()->str());

        if (object->is_struct()) {
          code += "  { " + NumToString(field->offset()) + ", " +
                  member_names_table + "[" + NumToString(field->id()) + "], " +
                  field_function_name + " },\n";
        } else {
          code += "  { " + member_names_table + "[" + NumToString(field->id()) +
                  "], " + field_function_name + " },\n";
        }
      });

      code += "}\n\n";

      const bool is_root_node = object == schema_->root_table();

      // collect pointers to every dependency of the root node so we can add the
      // ProtoFields to it
      const auto deps = CollectDependencies(object, schema_);

      // root nodes get a Proto definition and dissector function
      if (is_root_node) {
        // create the proto definition
        code += proto_name + " = Proto(\"" + object_names.full_name +
                "\", \"flatbuffers: " + object_names.full_name + "\")\n";
        code += proto_name + ".fields = Construct_Fields(\n";
        code += "  fb_basic_fields";

        // add all types to the ProtoField definition so wireshark lookup works
        for (const auto *dep : deps) {
          std::string dep_full_name = dep->name()->str();
          to_underscore(dep_full_name);
          code += ",\n  " + dep_full_name + "_member_fields";
        }

        code += "\n)\n\n";

        // register the protocol with wireshark
        code += "Register_Proto(" + proto_name + ")\n\n";

        // add the dissector function for the protocol
        code += proto_name + ".dissector = function(buffer, info, tree)\n";
        code += "  info.cols.protocol = " + proto_name + ".description\n";
        code +=
            "  return " + parse_object_function_name + "(buffer, 0, tree)\n";
        code += "end\n\n";
      }

      // create the parse function for this object
      code +=
          "function " + parse_object_function_name + "(buffer, offset, tree";

      // if the object is a root node, we have some more code to add.
      if (is_root_node) {
        code += ", verbose)\n";
        code += "  local file_name = \"flatbuffer: " +
                object_names.declaration_file + "\"\n\n";
        code +=
            "  assert(buffer.reported_len, \"Please pass the full Tvb into "
            "the parser, not a TvbRange (or anything else!)\")\n\n";
        code += "  FB_VERBOSE = verbose and verbose or false\n\n";
        code += "  local subtree = tree:add(" + proto_name +
                ", buffer(offset), buffer(offset):raw(), file_name)\n\n";

        code +=
            "  offset = Parse_Root_Offset(buffer, offset, subtree).value\n\n";

        // parse file_ident if present in the schema
        if (const std::string ident = schema_->file_ident()->str();
            !ident.empty()) {
          code += "  local file_ident_buffer = buffer(offset + 4, 4)\n";
          code += "  local file_ident_string = file_ident_buffer:string()\n";
          code +=
              "  local ident_match = file_ident_string == \"" + ident + "\"\n";
          code +=
              "  subtree:add(fb_file_ident, file_ident_buffer, "
              "file_ident_string)";
          code +=
              ":append_text(\" [\" .. (ident_match and "
              "\"MATCH\" or \"MISMATCH\") .. \"]\")\n\n";
        }

      } else {
        code += ")\n";
        code += "  local subtree = tree\n";
      }

      // call the derived parse function
      if (object->is_struct()) {
        code += "  return Parse_Struct(buffer, offset, subtree, " +
                member_lookup_table + "[1], " +
                NumToString(object->bytesize()) + ", " + member_names_table +
                ", " + member_list_table + ")\n";
      } else {
        code += "  return Parse_Table(buffer, offset, subtree, " +
                member_lookup_table + "[1], " + member_names_table + ", " +
                member_list_table + ")\n";
      }
      code += "end\n\n";
    });

    // write all files to disk
    for (const auto &file : files) {
      const std::string file_name =
          options_.output_path + "02_" + file.first + ".lua";
      SaveFile(file_name.c_str(), file.second, false);
    }

    return true;
  }

  // helper function which creates a ProtoField definition for a given field in
  // the schema.
  std::string CreateProtoFieldDefinition(const r::Field *field,
                                         const std::string &object_full_name) {
    std::string code;

    // collect basic information
    std::string field_name = field->name()->str();
    const std::string field_name_full = object_full_name + "_" + field_name;
    const r::Type *type = field->type();
    r::BaseType base_type = type->base_type();
    std::string enum_lookup_table = "nil";
    const int32_t index = type->index();

    if (field->deprecated()) { field_name += " [DEPRECATED]"; }

    // enum protofield definitions have to generate their lookup table name.
    // union enums have to grab their base types
    if (IsInteger(base_type) && index >= 0) {
      // enum types always have a valid index
      std::string enum_name_underscore =
          schema_->enums()->Get(index)->name()->str();
      to_underscore(enum_name_underscore);

      // set the lookup table string
      enum_lookup_table = "ENUM_" + enum_name_underscore;

      if (base_type == r::UType) {
        // union enums have the base_type UType, we need the actual underlying
        // type to get the appropriate width
        base_type =
            schema_->enums()->Get(index)->underlying_type()->base_type();
      }
    }

    // generate the key for the table and the prefix of the value
    code += "  [" + object_full_name + "_member_names[" +
            NumToString(field->id()) + "]] = ";
    code += "ProtoField.";

    // create the ProtoField type for this field
    switch (base_type) {
      case r::Bool:
        code += "bool(\"" + field_name_full + "\", \"" + field_name +
                "\", nil, nil, \"" + field_name;
        break;
      case r::UType:
      case r::Byte:
      case r::UByte:
        code += "uint8(\"" + field_name_full + "\", \"" + field_name +
                "\", base.DEC, " + enum_lookup_table + ", nil, \"" + field_name;
        break;
      case r::Short:
        code += "int16(\"" + field_name_full + "\", \"" + field_name +
                "\", base.DEC, " + enum_lookup_table + ", nil, \"" + field_name;
        break;
      case r::UShort:
        code += "uint16(\"" + field_name_full + "\", \"" + field_name +
                "\", base.DEC, " + enum_lookup_table + ", nil, \"" + field_name;
        break;
      case r::Int:
        code += "int32(\"" + field_name_full + "\", \"" + field_name +
                "\", base.DEC, " + enum_lookup_table + ", nil, \"" + field_name;
        break;
      case r::UInt:
        code += "uint32(\"" + field_name_full + "\", \"" + field_name +
                "\", base.DEC, " + enum_lookup_table + ", nil, \"" + field_name;
        break;
      case r::Long:
        code += "int64(\"" + field_name_full + "\", \"" + field_name +
                "\", base.DEC, " + enum_lookup_table + ", nil, \"" + field_name;
        break;
      case r::ULong:
        code += "uint64(\"" + field_name_full + "\", \"" + field_name +
                "\", base.DEC, " + enum_lookup_table + ", nil, \"" + field_name;
        break;
      case r::Double:
        code += "double(\"" + field_name_full + "\", \"" + field_name +
                "\", base.DEC,  \"" + field_name;
        break;
      case r::Float:
        code += "float(\"" + field_name_full + "\", \"" + field_name +
                "\", base.DEC,  \"" + field_name;
        break;
      case r::String:
        code += "string(\"" + field_name_full + "\", \"" + field_name +
                "\", base.UNICODE, \"" + field_name;
        break;
      case r::Obj:
      case r::Union:
        code += "bytes(\"" + field_name_full + "\", \"" + field_name +
                "\", base.NONE, \"" + field_name;
        break;
      case r::Vector:
      case r::Array:
      case r::Vector64:
        code += "bytes(\"" + field_name_full + "\", \"" + field_name +
                "\", base.NONE, \"" + field_name;
        break;
      default: throw std::runtime_error("Unsupported field type");
    }

    code += "\"),\n";

    return code;
  }

  // generate the actual custom parse function for the given field
  std::string CreateParserDefinition(const r::Field *field,
                                     const r::Object *object,
                                     const std::string &field_lookup) {
    std::string code;

    // we have to regenerate the underscore name here.. :shrug:
    std::string full_object_name_underscore = object->name()->str();
    to_underscore(full_object_name_underscore);

    const r::Type *type = field->type();
    r::BaseType base_type = type->base_type();

    // when the base type is a union, we have to fish out the true underlying
    // type
    if (base_type == r::UType) {
      // UType should always have a valid index
      const int32_t index = type->index();
      base_type = schema_->enums()->Get(index)->underlying_type()->base_type();
    }

    // generate the actual parser definition based on type
    switch (base_type) {
      // scalar types
      case r::Bool:
        code += "  local parsed_data = Parse_Bool(buffer, offset, tree, " +
                field_lookup + "[field_name], " +
                (field->default_integer() ? "true" : "false") + ")\n";
        break;
      case r::UType:
      case r::Byte:
      case r::UByte:
        code += "  local parsed_data = Parse_Uint8(buffer, offset, tree, " +
                field_lookup + "[field_name], " +
                NumToString(field->default_integer()) + ")\n";
        break;
      case r::Short:
        code += "  local parsed_data = Parse_Int16(buffer, offset, tree, " +
                field_lookup + "[field_name], " +
                NumToString(field->default_integer()) + ")\n";
        break;
      case r::UShort:
        code += "  local parsed_data = Parse_Uint16(buffer, offset, tree, " +
                field_lookup + "[field_name], " +
                NumToString(field->default_integer()) + ")\n";
        break;
      case r::Int:
        code += "  local parsed_data = Parse_Int32(buffer, offset, tree, " +
                field_lookup + "[field_name], " +
                NumToString(field->default_integer()) + ")\n";
        break;
      case r::UInt:
        code += "  local parsed_data = Parse_Uint32(buffer, offset, tree, " +
                field_lookup + "[field_name], " +
                NumToString(field->default_integer()) + ")\n";
        break;
      case r::Long:
        code += "  local parsed_data = Parse_Int64(buffer, offset, tree, " +
                field_lookup + "[field_name], " +
                NumToString(field->default_integer()) + ")\n";
        break;
      case r::ULong:
        code += "  local parsed_data = Parse_Uint64(buffer, offset, tree, " +
                field_lookup + "[field_name], " +
                NumToString(field->default_integer()) + ")\n";
        break;
      case r::Float:
        code += "  local parsed_data = Parse_Float32(buffer, offset, tree, " +
                field_lookup + "[field_name], " +
                NumToString(field->default_real()) + ")\n";
        break;
      case r::Double:
        code += "  local parsed_data = Parse_Float64(buffer, offset, tree, " +
                field_lookup + "[field_name], " +
                NumToString(field->default_real()) + ")\n";
        break;
      // non scalar types
      case r::String:
        code +=
            "  local parsed_data = Parse_Offset_Field(buffer, offset, tree," +
            field_lookup + "[field_name], Parse_String)\n";
        break;
      case r::Vector64:
        // handle just like regular vectors, but the indirect size is 8 and the
        // function called is different.
      case r::Vector: {
        const auto element_base_type = type->element();
        bool is_indirect = false;
        std::string object_type;
        std::string object_parser;
        // handle scalar types differently from object types
        // inlined structs and offset types are handled the same here - just
        // call their parse function
        switch (element_base_type) {
          case r::Obj: {
            const auto sub_object = schema_->objects()->Get(type->index());
            std::string sub_object_name_underscore = sub_object->name()->str();
            to_underscore(sub_object_name_underscore);

            object_type = sub_object_name_underscore + "_member_fields[1]";
            object_parser = "Parse_" + sub_object_name_underscore;

            if (!sub_object->is_struct()) { is_indirect = true; }
          } break;
          case r::Union: {
            break;
          }
          default:
            // scalar types
            object_type = GetScalarParserProtoField(element_base_type);
            object_parser = GetScalarParserFunction(element_base_type);
            break;
        }

        // derived parse function name based on width
        const std::string parse_function =
            "Parse_Vector" +
            std::string((type->base_type() == r::Vector64) ? "64" : "");

        // ParseVector calls into ParseArray, so we need all the arguments to
        // ParseArray
        code += "  local parsed_data = " + parse_function +
                "(buffer, offset, tree, " +
                std::string(is_indirect ? "true" : "false") + ", " +
                field_lookup + "[field_name], " + object_type + ", " +
                NumToString(type->element_size()) + ", " + object_parser +
                ")\n";
      } break;
      case r::Obj: {
        const r::Object *sub_object = schema_->objects()->Get(type->index());
        std::string sub_object_full_name = sub_object->name()->str();
        to_underscore(sub_object_full_name);

        code += "  local parsed_data = Parse_" + sub_object_full_name +
                "(buffer, offset, tree)\n";
        code += "  parsed_data.tree:prepend_text(field_name .. \": \")\n";
      } break;
      case r::Union: {
        // we could probably just infer that union type is stored at id - 1, but
        // just in case, I'll go hunting for it.
        uint32_t union_type_index;
        const std::string union_type_name = field->name()->str() + "_type";

        // this currently loops over all fields with no break. could be more
        // optimal
        ForAllFields(object, false, [&](const r::Field *obj_field) {
          if (obj_field->name()->str() == union_type_name) {
            union_type_index = obj_field->id();
          }
        });

        // create a local lookup table of union value to parser function
        code += "  local union_functions = {\n";

        // add parser functions for each enum value in the union
        ForAllEnumValues(schema_->enums()->Get(type->index()),
                         [&](const r::EnumVal *enum_val) {
                           // for each enum value, get the target object and its
                           // parser function
                           int32_t union_type_index = enum_val->value();

                           if (union_type_index == 0) {
                             // skip the NONE entry, as it doesn't have a parser
                             return;
                           }

                           const r::Object *enum_target_object =
                               schema_->objects()->Get(union_type_index);

                           std::string enum_target_name_underscore =
                               enum_target_object->name()->str();
                           to_underscore(enum_target_name_underscore);

                           code += "    [" + NumToString(union_type_index) +
                                   "] = " + "Parse_" +
                                   enum_target_name_underscore + ",\n";
                         });

        code += "  }\n\n";

        // parse the union type based on the lookup table
        code += "  local parsed_data = Parse_Union(buffer, offset, tree, " +
                field_lookup + "[field_name], union_functions[" +
                full_object_name_underscore + "_union_type_enum_values[" +
                NumToString(union_type_index) + "]])\n";
      } break;
      case r::Array: {
        const auto element_base_type = type->element();
        std::string object_type;
        std::string object_parser;

        switch (element_base_type) {
          case r::Obj: {
            const auto sub_object = schema_->objects()->Get(type->index());

            std::string sub_object_name_underscore = sub_object->name()->str();
            to_underscore(sub_object_name_underscore);

            object_type = sub_object_name_underscore + "_member_fields[1]";
            object_parser = "Parse_" + sub_object_name_underscore;
            break;
          }
          default:
            object_type = GetScalarParserProtoField(element_base_type);
            object_parser = GetScalarParserFunction(element_base_type);
            break;
        }

        code += "  local parsed_data = Parse_Array(buffer, offset, tree, " +
                field_lookup + "[field_name], " + object_type + ", " +
                NumToString(type->element_size()) + ", " +
                NumToString(type->fixed_length()) + ", " + object_parser +
                ")\n";
      } break;
      case r::None:
      case r::MaxBaseType: FLATBUFFERS_ASSERT(false);
    }

    // if this field is a union lookup, it gets more code
    if (IsInteger(type->base_type()) && type->index() >= 0 &&
        schema_->enums()->Get(type->index())->is_union()) {
      // set the value of the union field in the local lookup table, so that we
      // can reference it when we come accross the union itself
      code += "  " + full_object_name_underscore + "_union_type_enum_values[" +
              NumToString(field->id()) + "] = parsed_data.value\n";
    }

    return code;
  }

  // helper function for collecting all directly included tables and structs of
  // a root table. this is used to fill in the wireshark protocol fields section
  std::vector<const r::Object *> CollectDependencies(const r::Object *object,
                                                     const r::Schema *schema) {
    std::vector<const r::Object *> dependencies;
    std::unordered_set<const r::Object *> visited;

    // recursive caller to collect and return all depdenent objects
    std::function<void(const r::Object *)> collect = [&](const r::Object *obj) {
      if (!obj || visited.count(obj)) { return; }
      visited.insert(obj);
      dependencies.push_back(obj);

      for (const auto field : *obj->fields()) {
        const auto type = field->type();
        if (type->base_type() == r::BaseType::Obj ||
            type->base_type() == r::BaseType::Union) {
          const auto *dep = schema->objects()->Get(type->index());
          collect(dep);
        } else if (type->base_type() == r::BaseType::Vector &&
                   type->element() == r::BaseType::Obj) {
          const auto *dep = schema->objects()->Get(type->index());
          collect(dep);
        }
      }
    };

    collect(object);
    return dependencies;
  }

  const r::Schema *schema_;
  CodeGenOptions options_;

  const std::string flatc_version_;
  const BfbsNamer namer_;
};

}  // namespace

std::unique_ptr<CodeGenerator> NewWiresharkBfbsGenerator(
    const std::string &flatc_version) {
  return std::unique_ptr<CodeGenerator>(
      new WiresharkBfbsGenerator(flatc_version));
}

}  // namespace flatbuffers
