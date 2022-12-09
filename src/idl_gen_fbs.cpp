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

// independent from idl_parser, since this code is not needed for most clients

#include <unordered_map>
#include <utility>
#include <vector>

#include "flatbuffers/code_generators.h"
#include "flatbuffers/flatbuffers.h"
#include "flatbuffers/flatc.h"
#include "flatbuffers/idl.h"
#include "flatbuffers/util.h"
namespace flatbuffers {

static std::string GenType(const Type &type, bool underlying = false) {
  switch (type.base_type) {
    case BASE_TYPE_STRUCT:
      return type.struct_def->defined_namespace->GetFullyQualifiedName(
          type.struct_def->name);
    case BASE_TYPE_VECTOR: return "[" + GenType(type.VectorType()) + "]";
    default:
      if (type.enum_def && !underlying) {
        return type.enum_def->defined_namespace->GetFullyQualifiedName(
            type.enum_def->name);
      } else {
        return kTypeNames[type.base_type];
      }
  }
}

static std::unordered_map<std::string, voffset_t> MapStructId(
    const StructDef &struct_def, IDLOptions::ProtoIdGapAction gap_action) {
  std::vector<std::pair<std::string, std::string> > ids;

  bool fields_with_id = false;
  bool possible_missing_id = false;

  for (const auto &field : struct_def.fields.vec) {
    if (field->attributes.Lookup("id")) {
      fields_with_id = true;
      if (field->attributes.Lookup("id")->constant.empty() &&
          field->value.type.base_type != BASE_TYPE_UNION)
        ids.push_back(std::make_pair("-1", field->name));
      else {
        ids.push_back(std::make_pair(field->attributes.Lookup("id")->constant,
                                     field->name));

        // Check for non positive id number
        if (!field->attributes.Lookup("id")->constant.empty()) {
          voffset_t id = 0;
          bool done = StringToNumber(
              field->attributes.Lookup("id")->constant.c_str(), &id);
          if (!done)
            fprintf(stderr,
                    "Field id in struct %s has a non positive number value\n",
                    struct_def.name.c_str());
        }
      }
    } else {
      if (field->value.type.base_type != BASE_TYPE_UNION)
        possible_missing_id = true;
      ids.push_back(std::make_pair(std::string(), field->name));
    }
  }

  // None of the fields has id
  if (!fields_with_id) return {};

  // Check for missing id
  if (possible_missing_id)
    fprintf(stderr, "Field id in struct : %s is missing\n",
            struct_def.name.c_str());

  // Sort ids, consider 0 if the field does not have an id.
  std::sort(ids.begin(), ids.end(), [](const auto &lhs, const auto &rhs) {
    auto a_id = lhs.first.empty() ? 0 : std::stoi(lhs.first);
    auto b_id = rhs.first.empty() ? 0 : std::stoi(rhs.first);
    return a_id < b_id;
  });

  for (auto it = std::next(ids.begin()); it != ids.end(); it++) {
    if (std::find(struct_def.reserved_ids.begin(),
                  struct_def.reserved_ids.end(),
                  stoi(it->first)) != struct_def.reserved_ids.end())
      fprintf(stderr,
              "Field %s with id %s in struct %s uses id from reserved ids\n",
              it->second.c_str(), it->first.c_str(), struct_def.name.c_str());

    // Check for twice use of ids
    if (!it->first.empty() && std::prev(it)->first == it->first)
      fprintf(stderr, "Fields %s and %s with use id %s in struct %s twice\n",
              it->second.c_str(), std::prev(it)->second.c_str(),
              it->first.c_str(), struct_def.name.c_str());

    // Check for gap between ids
    if (gap_action != IDLOptions::ProtoIdGapAction::NO_OP) {
      if (!it->first.empty() && !std::prev(it)->first.empty() &&
          it->first != "-1" && std::prev(it)->first != "-1")
        if (std::stoi(it->first) != std::stoi(std::prev(it)->first) + 1) {
          if (gap_action == IDLOptions::ProtoIdGapAction::ERROR)
            fprintf(stderr,
                    "Field %s with id %s  and field %s with id %s in struct"
                    "%s have gap\n",
                    it->second.c_str(), it->first.c_str(),
                    std::prev(it)->second.c_str(), std::prev(it)->first.c_str(),
                    struct_def.name.c_str());
          else if (gap_action == IDLOptions::ProtoIdGapAction::WARNING)
            printf(
                "Field %s with id %s  and field %s with id %s in struct %s "
                "have gap\n",
                it->second.c_str(), it->first.c_str(),
                std::prev(it)->second.c_str(), std::prev(it)->first.c_str(),
                struct_def.name.c_str());
        }
    }
  }

  std::unordered_map<std::string, voffset_t> proto_fbs_ids;

  voffset_t id = 0;
  for (const auto &element : ids) {
    if (element.first.empty()) id++;
    proto_fbs_ids.emplace(element.second, id++);
  }

  return proto_fbs_ids;
}

static void GenNameSpace(const Namespace &name_space, std::string *_schema,
                         const Namespace **last_namespace) {
  if (*last_namespace == &name_space) return;
  *last_namespace = &name_space;
  auto &schema = *_schema;
  schema += "namespace ";
  for (auto it = name_space.components.begin();
       it != name_space.components.end(); ++it) {
    if (it != name_space.components.begin()) schema += ".";
    schema += *it;
  }
  schema += ";\n\n";
}

// Generate a flatbuffer schema from the Parser's internal representation.
std::string GenerateFBS(const Parser &parser, const std::string &file_name) {
  // Proto namespaces may clash with table names, escape the ones that were
  // generated from a table:
  for (auto it = parser.namespaces_.begin(); it != parser.namespaces_.end();
       ++it) {
    auto &ns = **it;
    for (size_t i = 0; i < ns.from_table; i++) {
      ns.components[ns.components.size() - 1 - i] += "_";
    }

    if (parser.opts.proto_mode && !parser.opts.proto_namespace_suffix.empty()) {
      // Since we know that all these namespaces come from a .proto, and all are
      // being converted, we can simply apply this suffix to all of them.
      ns.components.insert(ns.components.end() - ns.from_table,
                           parser.opts.proto_namespace_suffix);
    }
  }

  std::string schema;
  schema += "// Generated from " + file_name + ".proto\n\n";
  if (parser.opts.include_dependence_headers) {
    // clang-format off
    int num_includes = 0;
    for (auto it = parser.included_files_.begin();
         it != parser.included_files_.end(); ++it) {
      if (it->second.empty())
        continue;
      std::string basename;
      if(parser.opts.keep_prefix) {
        basename = flatbuffers::StripExtension(it->second);
      } else {
        basename = flatbuffers::StripPath(
                flatbuffers::StripExtension(it->second));
      }
      schema += "include \"" + basename + ".fbs\";\n";
      num_includes++;
    }
    if (num_includes) schema += "\n";
    // clang-format on
  }

  // Generate code for all the enum declarations.
  const Namespace *last_namespace = nullptr;
  for (auto enum_def_it = parser.enums_.vec.begin();
       enum_def_it != parser.enums_.vec.end(); ++enum_def_it) {
    EnumDef &enum_def = **enum_def_it;
    if (parser.opts.include_dependence_headers && enum_def.generated) {
      continue;
    }
    GenNameSpace(*enum_def.defined_namespace, &schema, &last_namespace);
    GenComment(enum_def.doc_comment, &schema, nullptr);
    if (enum_def.is_union)
      schema += "union " + enum_def.name;
    else
      schema += "enum " + enum_def.name + " : ";

    schema += GenType(enum_def.underlying_type, true) + " {\n";

    for (auto it = enum_def.Vals().begin(); it != enum_def.Vals().end(); ++it) {
      auto &ev = **it;
      GenComment(ev.doc_comment, &schema, nullptr, "  ");
      if (enum_def.is_union)
        schema += "  " + GenType(ev.union_type) + ",\n";
      else
        schema += "  " + ev.name + " = " + enum_def.ToString(ev) + ",\n";
    }
    schema += "}\n\n";
  }
  // Generate code for all structs/tables.
  for (auto it = parser.structs_.vec.begin(); it != parser.structs_.vec.end();
       ++it) {
    StructDef &struct_def = **it;
    const auto proto_fbs_ids =
        MapStructId(struct_def, parser.opts.proto_id_gap_action);
    if (parser.opts.include_dependence_headers && struct_def.generated)
      continue;

    GenNameSpace(*struct_def.defined_namespace, &schema, &last_namespace);
    GenComment(struct_def.doc_comment, &schema, nullptr);
    schema += "table " + struct_def.name + " {\n";
    for (auto field_it = struct_def.fields.vec.begin();
         field_it != struct_def.fields.vec.end(); ++field_it) {
      auto &field = **field_it;
      if (field.value.type.base_type != BASE_TYPE_UTYPE) {
        GenComment(field.doc_comment, &schema, nullptr, "  ");
        schema += "  " + field.name + ":" + GenType(field.value.type);
        if (field.value.constant != "0") schema += " = " + field.value.constant;
        std::vector<std::string> attributes;
        if (field.IsRequired()) attributes.push_back("required");
        if (field.key) attributes.push_back("key");

        if (parser.opts.keep_proto_id) {
          auto it = proto_fbs_ids.find(field.name);
          if (it != proto_fbs_ids.end())
            attributes.push_back("id: " + NumToString(it->second));
        }

        if (!attributes.empty()) {
          schema += " (";
          for (const auto &attribute : attributes) {
            schema += attribute + ",";
          }
          schema.pop_back();
          schema += ")";
        }

        schema += ";\n";
      }
    }
    schema += "}\n\n";
  }
  return schema;
}

bool GenerateFBS(const Parser &parser, const std::string &path,
                 const std::string &file_name) {
  return SaveFile((path + file_name + ".fbs").c_str(),
                  GenerateFBS(parser, file_name), false);
}

}  // namespace flatbuffers
