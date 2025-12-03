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

#include <cstddef>
#include <set>
#include <string>

namespace flatbuffers {

// A File interface to write data to file by default or
// save only file names
class FileSaver {
 public:
  FileSaver() = default;
  virtual ~FileSaver() = default;

  virtual bool SaveFile(const char* name, const char* buf, size_t len,
                        bool binary) = 0;

  virtual const char* AttemptSave(const char* name, const char* buf, size_t len,
                               bool binary) = 0;

  bool SaveFile(const char* name, const std::string& buf, bool binary) {
    return SaveFile(name, buf.c_str(), buf.size(), binary);
  }

  const char* AttemptSave(const char* name, const std::string& buf, bool binary) {
    return AttemptSave(name, buf.c_str(), buf.size(), binary);
  }

  virtual void Finish() {}

 private:
  // Copying is not supported.
  FileSaver(const FileSaver&) = delete;
  FileSaver& operator=(const FileSaver&) = delete;
  // Rule of 5
  FileSaver(FileSaver&&) = default;
  FileSaver& operator=(FileSaver&&) = default;
};

class RealFileSaver final : public FileSaver {
 public:
  bool SaveFile(const char* name, const char* buf, size_t len,
                bool binary) final;

  const char* AttemptSave(const char* name, const char* buf, size_t len,
                          bool binary) final;
 private:
  std::string error_msg;
};

class FileNameSaver final : public FileSaver {
 public:
  bool SaveFile(const char* name, const char* buf, size_t len,
                bool binary) final;

  const char* AttemptSave(const char* name, const char* buf, size_t len,
                          bool binary) final;

  void Finish() final;

 private:
  std::set<std::string> file_names_{};
};

}  // namespace flatbuffers

#endif  // FLATBUFFERS_FILE_MANAGER_H_
