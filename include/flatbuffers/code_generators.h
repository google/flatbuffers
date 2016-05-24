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

#ifndef FLATBUFFERS_CODE_GENERATORS_H_
#define FLATBUFFERS_CODE_GENERATORS_H_

namespace flatbuffers {

class BaseGenerator {
 public:
  BaseGenerator(const Parser &parser, const std::string &path,
                const std::string &file_name)
      : parser_(parser), path_(path), file_name_(file_name){};
  virtual bool generate() = 0;

  bool isEverythingGenerated() {
    for (auto it = parser_.enums_.vec.begin(); it != parser_.enums_.vec.end();
         ++it) {
      if (!(*it)->generated) return false;
    }
    for (auto it = parser_.structs_.vec.begin();
         it != parser_.structs_.vec.end(); ++it) {
      if (!(*it)->generated) return false;
    }
    return true;
  }

 protected:
  virtual ~BaseGenerator(){};

  const Parser &parser_;
  const std::string &path_;
  const std::string &file_name_;
};

}  // namespace flatbuffers

#endif  // FLATBUFFERS_CODE_GENERATORS_H_
