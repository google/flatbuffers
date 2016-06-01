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

#include "flatbuffers/flatbuffers.h"
#include "flatbuffers/idl.h"
#include "flatbuffers/util.h"

namespace flatbuffers {

static std::string GenType(const Type &type) {
  switch (type.base_type) {
    case BASE_TYPE_STRUCT:
      return type.struct_def->defined_namespace->GetFullyQualifiedName(
               type.struct_def->name);
    case BASE_TYPE_UNION:
      return type.enum_def->defined_namespace->GetFullyQualifiedName(
               type.enum_def->name);
    case BASE_TYPE_VECTOR:
        return "[" + GenType(type.VectorType()) + "]";
    default:
        return kTypeNames[type.base_type];
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
  // Proto namespaces may clash with table names, so we have to prefix all:
  for (auto it = parser.namespaces_.begin(); it != parser.namespaces_.end();
       ++it) {
    for (auto comp = (*it)->components.begin(); comp != (*it)->components.end();
         ++comp) {
      (*comp) = "_" + (*comp);
    }
  }

  std::string schema;
  schema += "// Generated from " + file_name + ".proto\n\n";
  if (parser.opts.include_dependence_headers) {
    #ifdef FBS_GEN_INCLUDES  // TODO: currently all in one file.
    int num_includes = 0;
    for (auto it = parser.included_files_.begin();
         it != parser.included_files_.end(); ++it) {
      auto basename = flatbuffers::StripPath(
                        flatbuffers::StripExtension(it->first));
      if (basename != file_name) {
        schema += "include \"" + basename + ".fbs\";\n";
        num_includes++;
      }
    }
    if (num_includes) schema += "\n";
    #endif
  }
  // Generate code for all the enum declarations.
  const Namespace *last_namespace = nullptr;
  for (auto enum_def_it = parser.enums_.vec.begin();
           enum_def_it != parser.enums_.vec.end(); ++enum_def_it) {
    EnumDef &enum_def = **enum_def_it;
    GenNameSpace(*enum_def.defined_namespace, &schema, &last_namespace);
    GenComment(enum_def.doc_comment, &schema, nullptr);
    schema += "enum " + enum_def.name + " : ";
    schema += GenType(enum_def.underlying_type) + " {\n";
    for (auto it = enum_def.vals.vec.begin();
         it != enum_def.vals.vec.end(); ++it) {
      auto &ev = **it;
      GenComment(ev.doc_comment, &schema, nullptr, "  ");
      schema += "  " + ev.name + " = " + NumToString(ev.value) + ",\n";
    }
    schema += "}\n\n";
  }
  // Generate code for all structs/tables.
  for (auto it = parser.structs_.vec.begin();
           it != parser.structs_.vec.end(); ++it) {
    StructDef &struct_def = **it;
    GenNameSpace(*struct_def.defined_namespace, &schema, &last_namespace);
    GenComment(struct_def.doc_comment, &schema, nullptr);
    schema += "table " + struct_def.name + " {\n";
    for (auto field_it = struct_def.fields.vec.begin();
             field_it != struct_def.fields.vec.end(); ++field_it) {
      auto &field = **field_it;
      GenComment(field.doc_comment, &schema, nullptr, "  ");
      schema += "  " + field.name + ":" + GenType(field.value.type);
      if (field.value.constant != "0") schema += " = " + field.value.constant;
      if (field.required) schema += " (required)";
      schema += ";\n";
    }
    schema += "}\n\n";
  }
  return schema;
}

bool GenerateFBS(const Parser &parser,
                 const std::string &path,
                 const std::string &file_name) {
  return SaveFile((path + file_name + ".fbs").c_str(),
                  GenerateFBS(parser, file_name), false);
}

}  // namespace flatbuffers

