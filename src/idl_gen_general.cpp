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

#include "flatbuffers/code_generators.h"
#include "flatbuffers/flatbuffers.h"
#include "flatbuffers/idl.h"
#include "flatbuffers/util.h"

#if defined(FLATBUFFERS_CPP98_STL)
#  include <cctype>
#endif  // defined(FLATBUFFERS_CPP98_STL)

namespace flatbuffers {
std::string GeneralMakeRule(const Parser &parser, const std::string &path,
                            const std::string &file_name) {
  FLATBUFFERS_ASSERT(parser.opts.lang <= IDLOptions::kMAX);

  std::string file_extension =
      (parser.opts.lang == IDLOptions::kJava) ? ".java" : ".cs";

  std::string make_rule;

  for (auto it = parser.enums_.vec.begin(); it != parser.enums_.vec.end();
       ++it) {
    auto &enum_def = **it;
    if (!make_rule.empty()) make_rule += " ";
    std::string directory =
        BaseGenerator::NamespaceDir(parser, path, *enum_def.defined_namespace);
    make_rule += directory + enum_def.name + file_extension;
  }

  for (auto it = parser.structs_.vec.begin(); it != parser.structs_.vec.end();
       ++it) {
    auto &struct_def = **it;
    if (!make_rule.empty()) make_rule += " ";
    std::string directory = BaseGenerator::NamespaceDir(
        parser, path, *struct_def.defined_namespace);
    make_rule += directory + struct_def.name + file_extension;
  }

  make_rule += ": ";
  auto included_files = parser.GetIncludedFilesRecursive(file_name);
  for (auto it = included_files.begin(); it != included_files.end(); ++it) {
    make_rule += " " + *it;
  }
  return make_rule;
}

std::string BinaryFileName(const Parser &parser, const std::string &path,
                           const std::string &file_name) {
  auto ext = parser.file_extension_.length() ? parser.file_extension_ : "bin";
  return path + file_name + "." + ext;
}

bool GenerateBinary(const Parser &parser, const std::string &path,
                    const std::string &file_name) {
  if (parser.opts.use_flexbuffers) {
    auto data_vec = parser.flex_builder_.GetBuffer();
    auto data_ptr = reinterpret_cast<char *>(data(data_vec));
    return !parser.flex_builder_.GetSize() ||
           flatbuffers::SaveFile(
               BinaryFileName(parser, path, file_name).c_str(), data_ptr,
               parser.flex_builder_.GetSize(), true);
  }
  return !parser.builder_.GetSize() ||
         flatbuffers::SaveFile(
             BinaryFileName(parser, path, file_name).c_str(),
             reinterpret_cast<char *>(parser.builder_.GetBufferPointer()),
             parser.builder_.GetSize(), true);
}

std::string BinaryMakeRule(const Parser &parser, const std::string &path,
                           const std::string &file_name) {
  if (!parser.builder_.GetSize()) return "";
  std::string filebase =
      flatbuffers::StripPath(flatbuffers::StripExtension(file_name));
  std::string make_rule =
      BinaryFileName(parser, path, filebase) + ": " + file_name;
  auto included_files =
      parser.GetIncludedFilesRecursive(parser.root_struct_def_->file);
  for (auto it = included_files.begin(); it != included_files.end(); ++it) {
    make_rule += " " + *it;
  }
  return make_rule;
}

}  // namespace flatbuffers
