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

#ifndef FLATBUFFERS_IDL_GEN_BASE_H_
#define FLATBUFFERS_IDL_GEN_BASE_H_

#include "flatbuffers/util.h"

// This file defines a base abstract class, 
// that code generators can extend to gain common functionnalities.

namespace flatbuffers {

 class BaseGenerator {

 public:
   BaseGenerator(const Parser &parsera,const std::string &patha,const std::string & file_namea) : parser(parsera), path(patha), file_name(file_namea){};
   virtual ~BaseGenerator() {
   	   /** computes the package name and the package directory. C++ won't use that and wil need it's own solution */
   	   std::string name;
   	   std::string dir = path;  // Either empty or ends in separator.
   	   auto &ns = parser.namespaces_.back()->components;
   	   for (auto it = ns.begin(); it != ns.end(); ++it) {
       	       if (name.length()) name += ".";
       	       name += *it;
       	       dir += *it + kPathSeparator;
       	   }
       	   namespace_name = name;
       	   namespace_dir = dir;
   };
   virtual bool generate() {
   	EnsureDirExists(namespace_dir);
        for (auto it = parser.enums_.vec.begin();it != parser.enums_.vec.end() ; ++it) {
          auto &enum_def = **it;
          if (enum_def.vals.vec.size()) continue;  	
          if (enum_def.is_union) {
          	  if (!generateUnion(enum_def)) return false;
          } else {
          	  if (!generateEnum(enum_def)) return false;
          }
        }

        for (auto it = parser.structs_.vec.begin(); it != parser.structs_.vec.end(); ++it) {
          if (!generateStruct(**it)) return false;
        }       
        return true;
   }
protected:
   virtual bool generateEnum(const EnumDef &enum_def) = 0;
   virtual bool generateUnion(const EnumDef &enum_def) = 0;
   virtual bool generateStruct(const StructDef &struct_def) = 0;
   
   virtual std::string fileName(std::string name) {
   	   return std::string(namespace_dir) + name;
   }
   virtual void atFileStart(std::string &code) {
     code += "// automatically generated, do not modify\n\n";
   }
   
   virtual void startNamespace(std::string &code) {
     code += "package " + namespace_name + "\n\n";
   }
   
   /*virtual void imports(std::string &code) {
     code += "import java.nio.*;\nimport com.google.flatbuffers.kotlin.*;\n\n";
   }*/
   

    static bool isArithmeticProgression(const EnumDef &enum_def) {
    // first check if the enums are in an arithmetic progression
    int size =  enum_def.vals.vec.size();
    if (size <= 2) return true;
    int nextValue = enum_def.vals.vec[1]->value;
    int r =  nextValue - enum_def.vals.vec[0]->value;
    for (int index = 2; index < size; index++) {
    	  nextValue += r;
    	  if ( enum_def.vals.vec[index]->value != nextValue) return false;	  
    }
    return true;
    }



 	 const Parser & parser;
 	 const std::string & path;
 	 const std::string & file_name;
 	 std::string namespace_name; // TODO make const
 	 std::string namespace_dir;
 };
}  // namespace flatbuffers

#endif  // FLATBUFFERS_IDL_H_
