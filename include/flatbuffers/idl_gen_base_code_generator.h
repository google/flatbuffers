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

#ifndef FLATBUFFERS_IDL_GEN_BASE_CODE_GENERATOR_H_
#define FLATBUFFERS_IDL_GEN_BASE_CODE_GENERATOR_H_

#include <set>
#include "flatbuffers/util.h"

// This file defines a base abstract class, 
// that code generators can extend to gain common functionnalities.

namespace flatbuffers {
  extern std::string MakeCamel(const std::string &in, bool first);

  /** all generators should implement this 
      
      A. migration path (for a Language code generator) : 
      
      1) write a LanguageGenerator that is a concrete implementation of BaseGenerator and implement the 
            generate() 
         method with the body of the old
              bool GenerateLanguage(const Parser &parser,const std::string &path,const std::string & file_name)
      
      2) write a new   
           bool GenerateLanguage(const Parser &parser,const std::string &path,const std::string & file_name)
         method by replacing the old body with : 
            LanguageGenerator * generator = LanguageGenerator(parser, path, file_name);
            return generator->generate();
      
      Look at the Kotlin Generator, for an example of how it is done
  
      B. migration path (for flatc) 
      
      Once all code generators use a Base Generator 
      (this can be done easily, without the active intervention of the code generator maintainers),
      use a switch to make flatc call the relevant 
          LanguageGenerator * generator = LanguageGenerator(parser, path, file_name);
          return generator->generate();
      code for each generator
      
  */
  class BaseGenerator {
    public:
      BaseGenerator(const Parser & parser_,const std::string & path_,const std::string & file_name_) : parser(parser_), path(path_), file_name(file_name_) {};
      virtual bool generate() = 0;
    protected:
      virtual ~BaseGenerator() {};
      
      const Parser & parser;
      const std::string & path;
      const std::string & file_name;
  };

  /** maybee we should split the utilities among a few orthogonal interfaces (file & path, types, safeNames and safeTypes) ? */
  class StronglyTypedGenerator : public BaseGenerator {
    public:
      StronglyTypedGenerator(const Parser &parser_,const std::string &path_,const std::string & file_name_, const std::set<std::string> keywords_) : BaseGenerator(parser_, path_, file_name_), keywords(keywords_){
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
      }
      
      virtual bool generate() {
   	EnsureDirExists(namespace_dir);
        for (auto it = parser.enums_.vec.begin();it != parser.enums_.vec.end() ; ++it) {
          auto &enum_def = **it;
          if (!enum_def.vals.vec.size()) continue;  	
          if (enum_def.is_union) {
          	  if (!generateUnion(enum_def)) return false;
          } else {
          	  if (!generateEnum(enum_def)) return false;
          }
        }

        for (auto it = parser.structs_.vec.begin(); it != parser.structs_.vec.end(); ++it) {
          auto &struct_def = **it;
          if (struct_def.fixed) {
            if (!generateStruct(struct_def)) return false;
          } else {
            if (!generateTable(struct_def)) return false;
          }
        }       
        return true;
   }
   protected:
   virtual ~StronglyTypedGenerator() {};
    
   virtual bool generateEnum(const EnumDef &enum_def) = 0;
   virtual bool generateUnion(const EnumDef &enum_def) = 0;
   virtual bool generateStruct(const StructDef &struct_def) = 0;
   virtual bool generateTable(const StructDef &struct_def) = 0;
   
   virtual std::string wireScalarType(const BaseType &base_type) = 0;
   virtual std::string userScalarType(const BaseType &base_type) = 0;
   //virtual const char * writeToWire(BaseType & baseType) = 0;
   //virtual const char * readFromWire(BaseType & baseType) = 0;
   
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
   
   /** util methods */
   bool hasNestedStruct(const StructDef &struct_def) {
     for (auto it = struct_def.fields.vec.begin();it != struct_def.fields.vec.end();++it) {
      auto &field = **it;
      if (IsStruct(field.value.type) && field.value.type.struct_def->fixed) return true;
     }
     return false;
   }
   
  std::string lowerFirst(const std::string &in) {
    std::string s;
    for (size_t i = 0; i < in.length(); i++) {
      if (!i) s += static_cast<char>(tolower(in[0])); else s += in[i];
    }
    return s;
  }

   std::string safeName(const Definition & definition) {
     return sanitize(definition.name, false);
   }
      
   std::string SafeName(const Definition & definition) {
     return sanitize(definition.name, true);
   }
   
   bool saveFile(const std::string name, const std::string ext, const std::string code) {
     return SaveFile((namespace_dir + name + ext).c_str(), code, false);
   }
   
   std::string sanitize(const std::string name, const bool isFirstLetterUpper) {
	// transforms name with _ inside into camelCase
	std::string camelName = MakeCamel(name, isFirstLetterUpper);
	// if there is a trailing "_", add one
	if (camelName.size() >= 1 && camelName.compare(camelName.size() - 1, 1, "_") == 0) return camelName + "_";
	// if this is a reserved word, add a trailing "_"
	if (keywords.find(camelName) != keywords.end()) return camelName + "_"; else return camelName;
   }
   
   static bool isAnArithmeticProgression(const EnumDef &enum_def) {
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
   
   std::string multiplyBySizeOf(const Type &type) {
      int a = InlineSize(type);
      if (a == 1) return ""; else return " * " + NumToString(a);
   }

   // Ensure that a type is prefixed with its namespace whenever it is used
   // outside of its namespace.
   std::string wrapInNameSpace(const Namespace *ns, const std::string &name) {
     if (parser.namespaces_.back() == ns) return name;
     std::string qualified_name;
     for (auto it = ns->components.begin(); it != ns->components.end(); ++it) qualified_name += *it + ".";
     return qualified_name + name;
   }
   
   std::string wrapInNameSpace(const Definition &def) {
     return wrapInNameSpace(def.defined_namespace, def.name);
   }
    
   const std::set<std::string> keywords; 
    std::string namespace_name; // TODO make const
    std::string namespace_dir;
 };
}  // namespace flatbuffers

#endif  // FLATBUFFERS_IDL_GEN_BASE_CODE_GENERATOR_H_
