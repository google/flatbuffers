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

static bool HasFieldWithId(const std::vector<FieldDef *> &fields) {
  static const std::string ID = "id";

  for (const auto *field : fields) {
    const auto *id_attribute = field->attributes.Lookup(ID);
    if (id_attribute != nullptr && !id_attribute->constant.empty()) {
      return true;
    }
  }
  return false;
}

static bool HasNonPositiveFieldId(const std::vector<FieldDef *> &fields) {
  static const std::string ID = "id";

  for (const auto *field : fields) {
    const auto *id_attribute = field->attributes.Lookup(ID);
    if (id_attribute != nullptr && !id_attribute->constant.empty()) {
      voffset_t proto_id = 0;
      bool done = StringToNumber(id_attribute->constant.c_str(), &proto_id);
      if (!done) { return true; }
    }
  }
  return false;
}

static bool HasFieldIdFromReservedIds(
    const std::vector<FieldDef *> &fields,
    const std::vector<voffset_t> &reserved_ids) {
  static const std::string ID = "id";

  for (const auto *field : fields) {
    const auto *id_attribute = field->attributes.Lookup(ID);
    if (id_attribute != nullptr && !id_attribute->constant.empty()) {
      voffset_t proto_id = 0;
      bool done = StringToNumber(id_attribute->constant.c_str(), &proto_id);
      if (!done) { return true; }
      auto id_it =
          std::find(std::begin(reserved_ids), std::end(reserved_ids), proto_id);
      if (id_it != reserved_ids.end()) { return true; }
    }
  }
  return false;
}

static bool HasTwiceUsedId(const std::vector<FieldDef *> &fields) {
  static const std::string ID = "id";
  std::vector<voffset_t> used_proto_ids;
  for (const auto *field : fields) {
    const auto *id_attribute = field->attributes.Lookup(ID);
    if (id_attribute != nullptr && !id_attribute->constant.empty()) {
      voffset_t proto_id = 0;
      bool done = StringToNumber(id_attribute->constant.c_str(), &proto_id);
      if (done) { used_proto_ids.push_back(proto_id); }
    }
  }
  std::sort(std::begin(used_proto_ids), std::end(used_proto_ids));
  for (auto it = std::next(std::begin(used_proto_ids));
       it != std::end(used_proto_ids); it++) {
    if (*it == *std::prev(it)) { return true; }
  }

  return false;
}

static bool HasGapInProtoId(const std::vector<FieldDef *> &fields) {
  static const std::string ID = "id";
  std::vector<voffset_t> used_proto_ids;
  for (const auto *field : fields) {
    const auto *id_attribute = field->attributes.Lookup(ID);
    if (id_attribute != nullptr && !id_attribute->constant.empty()) {
      voffset_t proto_id = 0;
      bool done = StringToNumber(id_attribute->constant.c_str(), &proto_id);
      if (done) { used_proto_ids.push_back(proto_id); }
    }
  }
  std::sort(std::begin(used_proto_ids), std::end(used_proto_ids));
  for (auto it = std::next(std::begin(used_proto_ids));
       it != std::end(used_proto_ids); it++) {
    if (*it != *std::prev(it) + 1) { return true; }
  }

  return false;
}

struct ProtobufToFbsIdMap {
  using FieldName = std::string;
  using FieldID = voffset_t;
  using FieldNameToIdMap = std::unordered_map<FieldName, FieldID>;

  FieldNameToIdMap field_to_id;
  bool successful = false;
};

static ProtobufToFbsIdMap MapProtoIdsToFieldsId(
    const StructDef &struct_def, IDLOptions::ProtoIdGapAction gap_action) {
  const auto &fields = struct_def.fields.vec;

  if (!HasFieldWithId(fields)) { return { {}, true }; }

  if (HasNonPositiveFieldId(fields)) {
    fprintf(stderr, "Non positive\n");
    return { {}, false };
  }

  if (HasTwiceUsedId(fields)) {
    fprintf(stderr, "Twice\n");
    return { {}, false };
  }

  if (HasFieldIdFromReservedIds(fields, struct_def.reserved_ids)) {
    fprintf(stderr, "reserved\n");
    return { {}, false };
  }

  if (gap_action != IDLOptions::ProtoIdGapAction::NO_OP) {
    if (HasGapInProtoId(fields)) {
      if (gap_action == IDLOptions::ProtoIdGapAction::WARNING) {
        printf("Gap warning\n");
      } else {
        fprintf(stderr, "Gap error\n");
        return { {}, false };
      }
    }
  }

  static constexpr int UNION_ID = -1;
  std::vector<std::pair<int, std::string>> proto_ids;

  for (const auto *field : fields) {
    const auto *id_attribute = field->attributes.Lookup("id");
    if (id_attribute != nullptr) {
      // When we have union but do not use union flag to keep them
      if (id_attribute->constant.empty() &&
          field->value.type.base_type == BASE_TYPE_UNION) {
        proto_ids.emplace_back(UNION_ID, field->name);
      } else {
        voffset_t proto_id = 0;
        StringToNumber(id_attribute->constant.c_str(), &proto_id);
        proto_ids.emplace_back(proto_id, field->name);
      }
    } else {
      fprintf(stderr, "Fuck\n");
      return { {}, false };
    }
  }

  std::sort(
      std::begin(proto_ids), std::end(proto_ids),
      [](const auto &rhs, const auto &lhs) { return rhs.first < lhs.first; });
  std::unordered_map<std::string, voffset_t> proto_fbs_ids;

  voffset_t id = 0;
  for (const auto &element : proto_ids) {
    if (element.first == UNION_ID) { id++; }
    proto_fbs_ids.emplace(element.second, id++);
  }

  return { proto_fbs_ids, true };
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
        MapProtoIdsToFieldsId(struct_def, parser.opts.proto_id_gap_action);
    if (!proto_fbs_ids.successful) { return {}; }

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
          auto it = proto_fbs_ids.field_to_id.find(field.name);
          if (it != proto_fbs_ids.field_to_id.end()) {
            attributes.push_back("id: " + NumToString(it->second));
          }  // If not found it means we do not have any ids
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
  const std::string fbs = GenerateFBS(parser, file_name);
  if (fbs.empty()) return false;
  return SaveFile((path + file_name + ".fbs").c_str(), fbs, false);
}

}  // namespace flatbuffers
