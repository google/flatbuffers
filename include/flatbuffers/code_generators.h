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

/** This file defines some classes, that code generators should extend to gain
   common functionalities :

   BaseGenerator is the base class for all (binary, textual, strongly typed,
   dynamically typed, whatever) generators.
   It is really abstract, general and flexible and doesn't do much appart from
   holding the parser, a path and a filename being processed.
   Still, it brings a common structure (normalization) among generators

   The many advantages of object based generators will come later from   :

   A) the CodeWriter class (semi-automatic indentation (python), semi-automatic
   (C++) namespace scopes, export to different files (classic, top level
   methods),
   simpler code to generate string from things (comments (vector of strings),
   indentation commands (TAB BAT), newlines (NL), numbers, const char* or
   std::string)

   B) the Generator subclass (heritance for supporting different versions of a
   language, avoid field name clash, write text/binary to file, types
   management,
   common computations and sctructures :  enum analysis (function, array, map,
   ordered list + binarysearch),...)

*/
namespace flatbuffers {

class BaseGenerator {
public:
  BaseGenerator(const Parser &parser_, const std::string &path_,
                const std::string &file_name_)
      : parser(parser_), path(path_), file_name(file_name_){};
  virtual bool generate() = 0;

protected:
  virtual ~BaseGenerator(){};

  const Parser &parser;
  const std::string &path;
  const std::string &file_name;
};

} // namespace flatbuffers

#endif // FLATBUFFERS_CODE_GENERATORS_H_
