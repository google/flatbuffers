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

#ifndef FLATBUFFERS_FILE_MANAGER_H_
#define FLATBUFFERS_FILE_MANAGER_H_

#include <string>

namespace flatbuffers {

// A File interface to write data to file by default or
// save only file names
class FileManager {
 public:
  virtual ~FileManager() = default;

  virtual bool SaveFile(std::string absolute_file_path, std::string content, size_t len, bool binary) = 0;

  virtual bool ReadFile(std::string absolute_file_path, bool binary, std::string *buf) = 0;

  virtual std::set<std::string> FileNames() const = 0;

 protected:
  FileManager();

 private:
  // Copying is not supported.
  FileManager(const FileManager &) = delete;
  FileManager &operator=(const FileManager &) = delete;

  std::set<std::string> file_names_;
};

}  // namespace flatbuffers

#endif  // FLATBUFFERS_FILE_MANAGER_H_
