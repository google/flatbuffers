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
#include "include/flatbuffers/file.h"

namespace flatbuffers {

class DefaultFile :: File {
 public:
  bool SaveFile(std::string file_path, std::string buf, bool binary) override {
          std::ofstream ofs(name, binary ? std::ofstream::binary : std::ofstream::out);
          if (!ofs.is_open()) return false;
          ofs.write(buf.c_str(), buf.size());
          return !ofs.bad();
  }

  std::string ReadFile(std::string file_path) override {


  }

  std::set<string> FileNames() { return file_names_; }
};

}  // namespace flatbuffers

#endif  // FLATBUFFERS_FILE_WRITER_H_
