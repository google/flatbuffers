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

#include "flatbuffers/file_manager.h"

#include <fstream>
#include <set>
#include <string>

namespace flatbuffers {

bool RealFileSaver::SaveFile(const char* name, const char* buf, size_t len,
                             bool binary) {
  std::ofstream ofs(name, binary ? std::ofstream::binary : std::ofstream::out);
  if (!ofs.is_open()) return false;
  ofs.write(buf, len);
  return !ofs.bad();
}

const char* RealFileSaver::AttemptSave(const char* name, const char* buf,
                                       size_t len, bool binary) {
  if (!SaveFile(name, buf, len, binary)) {
    static std::string error_msg;
    error_msg = "Could not save file " + std::string(name ? name : "unknown");
    return error_msg.c_str();
  }
  return nullptr;
}
}  // namespace flatbuffers
