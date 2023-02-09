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

#ifndef FLATBUFFERS_FILE_WRITER_H_
#define FLATBUFFERS_FILE_WRITER_H_

#include <string>

namespace flatbuffers {

// A File interface to write data to file by default or
// save only file names
class File {
 public:
  virtual ~File() = default;

  virtual bool SaveFile(std::string file_path, std::string content);

  virtual bool ReadFile(std::string file_path, bool binary, std::string *buf);

  virtual std::set<string> FileNames() const = 0;
  
 protected:
  File();

 private:
  // Copying is not supported.
  File(const File &) = delete;
  File &operator=(const File &) = delete;
  
  std::set<string> files_;
};

}  // namespace flatbuffers

#endif  // FLATBUFFERS_FILE_WRITER_H_
