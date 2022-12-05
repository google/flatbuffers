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

static std::unordered_map<std::string, int> MapUnionId(
    const EnumDef &enum_def) {
  if (!enum_def.is_union) return {};

  std::unordered_map<std::string, int> map;
  std::vector<voffset_t> ids;
  voffset_t id = 0;
  for (const auto &it : enum_def.Vals()) {
    if (it->attributes.Lookup("id")) {
      if (StringToNumber(it->attributes.Lookup("id")->constant.c_str(), &id))
        ids.push_back(id);
      else
        fprintf(
            stderr,
            "Field id in field %s in Union %s is not a non-negative number\n",
            it->name.c_str(), enum_def.name.c_str());
    } else
      fprintf(stderr, "Field id in field %s in Union %s is missing\n",
              it->name.c_str(), enum_def.name.c_str());
  }

  std::sort(ids.begin(), ids.end());
  voffset_t generated_id = 0;
  for (auto id : ids) {
    auto result = map.emplace(NumToString(id), generated_id++);
    if (!result.second)
      fprintf(stderr, "Id %u is set twice in Union %s\n", id,
              enum_def.name.c_str());
  }

  return map;
}

static void MapStructId(StructDef &struct_def) {
  // Check if there is a field with id or not.
  bool has_id = false;
  for (const auto &field_it : struct_def.fields.vec) {
    if (field_it->attributes.Lookup("id")) {
      has_id = true;
      break;
    }
  }
  // Non of the fields have id
  if (!has_id) return;

  // Sort struct fields based on ids, consider 0 if the field does not have an
  // id or it is empty.
  std::sort(struct_def.fields.vec.begin(), struct_def.fields.vec.end(),
            [](const FieldDef *a, const FieldDef *b) {
              auto a_id = 0;
              if (a->attributes.Lookup("id") &&
                  !a->attributes.Lookup("id")->constant.empty())
                a_id = std::stoi(a->attributes.Lookup("id")->constant);

              auto b_id = 0;
              if (b->attributes.Lookup("id") &&
                  !b->attributes.Lookup("id")->constant.empty())
                b_id = std::stoi(b->attributes.Lookup("id")->constant);

              return a_id < b_id;
            });
  // Check for duplicate id usage
  for (auto it = std::next(struct_def.fields.vec.begin());
       it != struct_def.fields.vec.end(); it++) {
    if ((*(it - 1))->attributes.Lookup("id") && (*it)->attributes.Lookup("id"))
      if ((*(it - 1))->attributes.Lookup("id")->constant ==
          (*it)->attributes.Lookup("id")->constant)
        fprintf(stderr, "Id %s is set twice in struct %s\n",
                (*it)->attributes.Lookup("id")->constant.c_str(),
                struct_def.name.c_str());
  }

  voffset_t id = 0;
  for (auto field_it = struct_def.fields.vec.begin();
       field_it != struct_def.fields.vec.end(); ++field_it) {
    if (!(*field_it)->attributes.Lookup("id") &&
        (*field_it)->value.type.base_type != BASE_TYPE_UNION)
      fprintf(stderr, "Field id in field %s in struct : %s is missing\n",
              (*field_it)->name.c_str(), struct_def.name.c_str());

    if ((*field_it)->value.type.base_type == BASE_TYPE_UNION) id++;

    if ((*field_it)->attributes.Lookup("id"))
      (*field_it)->attributes.Lookup("id")->constant = NumToString(id++);
    else {
      auto val = new Value();
      val->constant = NumToString(id++);
      (*field_it)->attributes.Add("id", val);
    }
  }
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

    const auto map = MapUnionId(enum_def);
    for (auto it = enum_def.Vals().begin(); it != enum_def.Vals().end(); ++it) {
      auto &ev = **it;
      GenComment(ev.doc_comment, &schema, nullptr, "  ");
      if (enum_def.is_union) {
        schema += "  " + GenType(ev.union_type);
        const auto &id_str = ev.attributes.Lookup("id");
        if (id_str && !id_str->constant.empty())
          schema += " (id: " + NumToString(map.at(id_str->constant)) + ")";
        schema += ",\n";
      } else
        schema += "  " + ev.name + " = " + enum_def.ToString(ev) + ",\n";
    }
    schema += "}\n\n";
  }
  // Generate code for all structs/tables.
  for (auto it = parser.structs_.vec.begin(); it != parser.structs_.vec.end();
       ++it) {
    StructDef &struct_def = **it;
    MapStructId(struct_def);
    if (parser.opts.include_dependence_headers && struct_def.generated) {
      continue;
    }
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

        if (field.attributes.Lookup("id"))
          attributes.push_back("id: " +
                               field.attributes.Lookup("id")->constant);
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
