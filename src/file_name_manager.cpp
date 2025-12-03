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

#include <iostream>
#include <tuple>

#include "flatbuffers/file_manager.h"

namespace flatbuffers {

bool FileNameSaver::SaveFile(const char* name, const char* buf, size_t len,
                             bool binary) {
  (void)buf;
  (void)len;
  (void)binary;

  std::ignore = file_names_.insert(name);

  // we want to simulate always successful save
  return true;
}

const char* FileNameSaver::AttemptSave(const char* name, const char* buf,
                                       size_t len, bool binary) {
  return SaveFile(name, buf, len, binary) ? nullptr
                                          : "Printing filename failed";
}

void FileNameSaver::Finish() {
  for (const auto& file_name : file_names_) {
    // Just print the file names to standard output.
    // No actual file is created.
    std::cout << file_name << "\n";
  }
}

}  // namespace flatbuffers
