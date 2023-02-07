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

// A FileWriter interface to write data to file by default or
// save only file names
class FileWriter {
 public:
  virtual ~FileWriter() = default;

  enum Mode {
    DEFAULT = 0,
    SAVE_FILE_NAMES_ONLY = 1
  };

  virtual bool SaveFile();

  virtual std::string Path() const = 0;

  virtual std::string FileName() const = 0;
  
 protected:
  FileWriter(const std::string &path, const std::string &file_name);

 private:
  // Copying is not supported.
  FileWriter(const FileWriter &) = delete;
  FileWriter &operator=(const FileWriter &) = delete;
  
  std::string path_;
  std::string file_name_;
};

}  // namespace flatbuffers

#endif  // FLATBUFFERS_FILE_WRITER_H_
