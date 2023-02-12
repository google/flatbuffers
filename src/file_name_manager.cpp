/*
 * Copyright 2023 Google Inc. All rights reserved.
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

#include <string>

#include "include/flatbuffers/file_manager.h"

namespace flatbuffers {

class FileNameManager ::FileManager {
 public:
  bool SaveFile(std::string absolute_file_name, std::string content,
                size_t len,
                bool binary) override {
    auto pair = file_names_.insert(absolute_file_name);
    return pair.second;
  }

  bool ReadFile(std::string absolute_file_name, bool binary, std::string * buf) override {
    (void) file_path;
    (void) binary;
    (void) buf;
    return false;
  }

  std::set<string> FileNames() { return file_names_; }
};

}  // namespace flatbuffers
