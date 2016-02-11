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

// independent from idl_parser, since this code is not needed for most clients

#include <string>

#include "flatbuffers/flatbuffers.h"
#include "flatbuffers/idl.h"
#include "flatbuffers/util.h"
#include "flatbuffers/idl_gen_base.h"

#ifdef _WIN32
#include <direct.h>
#define PATH_SEPARATOR "\\"
#define mkdir(n, m) _mkdir(n)
#else
#include <sys/stat.h>
#define PATH_SEPARATOR "/"
#endif

namespace flatbuffers {
  namespace kotlin {
  	  // definitions have name, index and namespace
  	  // structDef has name, index, namespace, fields (that have type)
  	  // enumdef is_union
  	  // enumval have name, val (structDef if union)
    static const char *  wireTypes[] = {"Byte", // NONE
   "Byte", // UTYPE
   "Boolean", // BOOL  Boolean(user) Byte (wire) Array<Boolean> Array<byte>
   "Byte", // CHAR
   "Byte", // UCHAR
   "Short", // SHORT
   "Short", // USHORT   Int(user)    Short (wire)
   "Int", // INT
   "Int", // UINT
   "Long", // LONG
   "Long", // ULONG
   "Float", // FLOAT
   "Double", // DOUBLE
   /** pointer types */
   "Int", // STRING
   "Int", // VECTOR      
   "Int", // STRUCT      Name (user) Table
   "Int" // UNION        Name(user) or Underlying <->  TTable Int (offset)
};

    static const char *  userTypes[] = {"Byte", // NONE
   "Byte", // UTYPE
   "Boolean", // BOOL  Boolean(user) Byte (wire) Array<Boolean> Array<byte>
   "Byte", // CHAR
   "Int", // UCHAR
   "Short", // SHORT
   "Int", // USHORT   Int(user)    Short (wire)
   "Int", // INT
   "Long", // UINT
   "Long", // LONG
   "Long", // ULONG
   "Float", // FLOAT
   "Double", // DOUBLE
};


 extern bool SaveType(const Parser &parser, const Definition &def, const std::string &classcode, const std::string &path, bool needs_imports);
 extern void GenStruct(const StructDef &struct_def, std::string *code_ptr, StructDef * /*root_struct_def*/, const Parser &parser);
 extern void endEnumDeclaration(const EnumDef &enum_def,std::string *code_ptr, const Parser & parser);

    enum Of {container, element };
 
    class KotlinGenerator : public BaseGenerator {
    public:
      KotlinGenerator(const Parser &parsera,const std::string &patha,const std::string & file_namea) : BaseGenerator(parsera, patha, file_namea) {};
    protected:
    

      bool generateStruct(const StructDef &struct_def) {
      	      std::string declcode;
      	      kotlin::GenStruct(struct_def, &declcode, parser.root_struct_def_, parser);
      	      return kotlin::SaveType(parser, struct_def, declcode, path, true);
      }
      
      void enumImports(const EnumDef &/*enum_def*/, std::string &code) {
      	      code += "import java.nio.*;\nimport com.google.flatbuffers.kotlin.*;\n\n";
      }
      
      void unionImports(const EnumDef &/*enum_def*/, std::string &code) {
      	  code += "import java.nio.*;\nimport com.google.flatbuffers.kotlin.*;\n\n";
          //code += "import com.google.flatbuffers.kotlin.Table\n\n";
      }
      
      void enumDeclaration(const EnumDef &enum_def, std::string & code) {
      	      code += "public enum class " + enum_def.name + "(val value: " + userScalarType(enum_def.underlying_type.base_type) + ") {\n";
              enumMembersDeclaration(enum_def, code);
        code += ";\n";
        endEnumDeclaration(enum_def, &code, parser);
      }
      
      void enumMembersDeclaration(const EnumDef &enum_def, std::string & code) {
      	      for (auto it = enum_def.vals.vec.begin(); it != enum_def.vals.vec.end();++it) {
        	auto &ev = **it;
        	GenComment(ev.doc_comment, &code, nullptr, "\t");
        	if (it != enum_def.vals.vec.begin()) code += ",\n";
        	enumElement(ev, code);
               }
      }
      
      void enumElement(const EnumVal ev, std::string &code) {
            code += "\t" +  ev.name + "(" + NumToString(ev.value) + ")";
      }
      
      /*void endEnumDeclaration(const EnumDef &enum_def,std::string &code) {
      	      staticEnumMethods(const EnumDef &enum_def,std::string &code)
      }*/
      
      /** scalar type for the getters/setters on the wire facing api
          this is usually a language type that can hold the schema type 
          and that ressembles it the most,
          except that booleans are serialized as byte (in the spec)
      */
      const char * wireScalarType(const BaseType &base_type) {
      	      return wireTypes[base_type];
      }
      
      /** scalar type for the getters/setters on the user facing api 
          this is the most convenient language type that can hold the wire type
          except that booleans are serialized as byte (see the flatbuffers spec)
      */
      const char * userScalarType(const BaseType &base_type) {
      	      return userTypes[base_type];
      }
      
      bool generateEnum(const EnumDef &enum_def) {
        if (enum_def.generated) return true;
        std::string code;
        atFileStart(code);
        startNamespace(code);
        //imports(code);
        enumImports(enum_def, code);
        GenComment(enum_def.doc_comment, &code, nullptr);
        enumDeclaration(enum_def, code);
        return SaveFile((enum_def.name + ".kt").c_str(), code, false);
      }
      
      bool generateUnion(const EnumDef &enum_def) {
        if (enum_def.generated) return true;
        std::string code;
        atFileStart(code);
        startNamespace(code);
        //imports(code);
        unionImports(enum_def, code);
        GenComment(enum_def.doc_comment, &code, nullptr);
        enumDeclaration(enum_def, code);
        return SaveFile((enum_def.name + ".kt").c_str(), code, false);
      }
      
    };	  
  } // namespace kotlin
  
  bool GenerateKotlin2(const Parser &parser,const std::string &path,const std::string & file_name) {
     kotlin::KotlinGenerator * generator = new kotlin::KotlinGenerator(parser, path, file_name);
     return generator->generate();
  }
}  // namespace flatbuffers
	