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
 extern void toString(const StructDef &struct_def, std::string *code_ptr);
 extern void fieldAsByteBuffer(const FieldDef &field, std::string *code_ptr);
 extern void generateStructAccessor(const StructDef &struct_def, const FieldDef &field,std::string *code_ptr);
 extern void GenTableBuilders(const StructDef &struct_def, std::string *code_ptr, const Parser &parser);
 extern void generateStructBuilder(const StructDef &struct_def, std::string *code_ptr);
  extern void generateCreateTable(const StructDef &struct_def, std::string *code_ptr);
    class KotlinGenerator : public BaseGenerator {
    public:
      KotlinGenerator(const Parser &parsera,const std::string &patha,const std::string & file_namea) : BaseGenerator(parsera, patha, file_namea) {};
    protected:
    

      /*bool generateStruct(const StructDef &struct_def) {
      	      std::string declcode;
      	      kotlin::GenStruct(struct_def, &declcode, parser.root_struct_def_, parser);
      	      return kotlin::SaveType(parser, struct_def, declcode, path, true);
      }
      
      bool generateTable(const StructDef &struct_def) {
      	      std::string declcode;
      	      kotlin::GenStruct(struct_def, &declcode, parser.root_struct_def_, parser);
      	      return kotlin::SaveType(parser, struct_def, declcode, path, true);
      }*/
      
      /** struct generation */
      bool generateStruct(const StructDef &struct_def) {
        if (struct_def.generated) return true;

        std::string code;
        
        atFileStart(code);
        startNamespace(code);
        structImports(struct_def, code);  
        GenComment(struct_def.doc_comment, &code, nullptr);
        code += "public open class " + struct_def.name + "(byteBuffer : ByteBuffer = EMPTY_BYTEBUFFER) : Struct"; 
        code += "(byteBuffer.order(ByteOrder.LITTLE_ENDIAN), if (byteBuffer === EMPTY_BYTEBUFFER) 0 else byteBuffer.getInt(byteBuffer.position()) + byteBuffer.position()) {\n";


        // Generate the Init method that sets the field in a pre-existing
        // accessor object. This is to allow object reuse.
        tableOrStructInitialize(struct_def, code);
  
        toString(struct_def, &code);

        for (auto it = struct_def.fields.vec.begin();it != struct_def.fields.vec.end();++it) {
          auto &field = **it;
          if (field.deprecated) continue;
          generateStructAccessor(struct_def, field, &code);
          if ((field.value.type.base_type == BASE_TYPE_VECTOR && IsScalar(field.value.type.VectorType().base_type)) || 
          	  field.value.type.base_type == BASE_TYPE_STRING) fieldAsByteBuffer(field, &code);
        }

        code += "\n\tcompanion object {\n";
        if (parser.root_struct_def_ == &struct_def && parser.file_identifier_.length()) generateHasIdentifier(struct_def, code); 

        code += "\t}\n"; // end companion object
        code += "}"; // end class declaration
  
        // create a struct constructor function
        generateStructBuilder(struct_def, &code); 
        
        return SaveFile((struct_def.name + ".kt").c_str(), code, true);
     }
     
      void structImports(const StructDef &/*struct_def*/, std::string &code) {
        code += "import java.nio.*;\nimport com.google.flatbuffers.kotlin.*;\n\n";
      }
      
      /** table generation */
      bool generateTable(const StructDef &struct_def) {
        if (struct_def.generated) return true;
        
        std::string code;
        
        atFileStart(code);
        startNamespace(code);
        tableImports(struct_def, code);   
        GenComment(struct_def.doc_comment, &code, nullptr);
        code += "public open class " + struct_def.name + "(byteBuffer : ByteBuffer = EMPTY_BYTEBUFFER) : Table"; 
        code += "(byteBuffer.order(ByteOrder.LITTLE_ENDIAN), if (byteBuffer === EMPTY_BYTEBUFFER) 0 else byteBuffer.getInt(byteBuffer.position()) + byteBuffer.position()) {\n";


        // Generate the Init method that sets the field in a pre-existing
        // accessor object. This is to allow object reuse.
        tableOrStructInitialize(struct_def, code);
  
        toString(struct_def, &code);

        for (auto it = struct_def.fields.vec.begin();it != struct_def.fields.vec.end();++it) {
          auto &field = **it;
          if (field.deprecated) continue;
          generateStructAccessor(struct_def, field, &code);
          if ((field.value.type.base_type == BASE_TYPE_VECTOR && IsScalar(field.value.type.VectorType().base_type)) || 
          	  field.value.type.base_type == BASE_TYPE_STRING) fieldAsByteBuffer(field, &code);
        }

        code += "\n\tcompanion object {\n";
        if (parser.root_struct_def_ == &struct_def && parser.file_identifier_.length()) generateHasIdentifier(struct_def, code); 

        // Create a set of functions that allow table construction.
        GenTableBuilders(struct_def, &code, parser);

        code += "\t}\n"; // end companion object
        code += "}"; // end class declaration
  
        // create a struct constructor function
        generateCreateTable(struct_def, &code);  
        
        return SaveFile((struct_def.name + ".kt").c_str(), code, true);
      }
      
      void tableImports(const StructDef &/*struct_def*/, std::string &code) {
      	      code += "import java.nio.*;\nimport com.google.flatbuffers.kotlin.*;\n\n";
      }
      
      /** table and struct */
      
      void tableOrStructInitialize(const StructDef &struct_def, std::string & code) {
        code += "\tpublic fun wrap(byteBuffer : ByteBuffer, position : Int = byteBuffer.getInt(byteBuffer.position()) + byteBuffer.position()) : " + struct_def.name + " = apply {";
        code +=  "bb = byteBuffer; ";
        code +=  "bb_pos = position}\n";
      }
      
      void generateHasIdentifier(const StructDef & /*struct_def*/, std::string &code) {
      	 // Check if a buffer has the identifier.
         code += "\tpublic fun hasIdentifier(byteBuffer : ByteBuffer) :Boolean = Table.hasIdentifier(byteBuffer, \"" + parser.file_identifier_ + "\")\n";
      }
      
      /** enum generation */
      
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
      
      void enumImports(const EnumDef &/*enum_def*/, std::string &code) {
      	      code += "import java.nio.*;\nimport com.google.flatbuffers.kotlin.*;\n\n";
      }
      
      void enumDeclaration(const EnumDef &enum_def, std::string & code) {
      	code += "public enum class " + enum_def.name + "(val value: " + userScalarType(enum_def.underlying_type.base_type) + ") {\n";
        enumMembersDeclaration(enum_def, code);
        code += ";\n";
        code += "\t companion object {\n";
        deserializeEnum(enum_def, code);
        code += "}\n"; // end companion object
        code += "}\n"; // end enum
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
      
      void deserializeEnum(const EnumDef &enum_def, std::string & code) {
      	  if (isAnArithmeticProgression(enum_def)) return deserializeEnumWithFunction(enum_def, code);  
      	  if (enum_def.vals.vec.size() < 8) return deserializeEnumWithIfs(enum_def, code);
          deserializeEnumWithMap(enum_def, code);
      }
      
      void deserializeEnumMethodDeclaration(const EnumDef &enum_def, std::string & code) {
      	  code += "\tfun from( value : ";
          code += userScalarType(enum_def.underlying_type.base_type);
          code += ") : " + enum_def.name;
      }
      
      /** fastest, uses no memory */
      void deserializeEnumWithFunction(const EnumDef &enum_def, std::string & code) {
          deserializeEnumMethodDeclaration(enum_def, code);
          if (enum_def.vals.vec.size() == 1) {
		code +=  " = if  (value.toInt() == " + NumToString(enum_def.vals.vec[0]->value) + ") " + enum_def.vals.vec[0]->name + " else throw Exception(\"Bad enum value : $value\")"; 
		return;
	}
	int first = enum_def.vals.vec[0]->value;
	int r = enum_def.vals.vec[1]->value - first; // this is > 0 as enum values must be specified in ascending order
	code +=  " = __enums[";
	/** trying to generate clean code that does res = (value - first) /r without noise */    
	bool parenthesisNeeded = r != 1 && first != 0;
	if (parenthesisNeeded ) code += "(";
	code += "value.toInt()";
	/** we need to avoid the double minus problem */
	if (first > 0) code += " - "  + NumToString(first); else  {if (first < 0) code += " + "  + NumToString(-first);}
	if (parenthesisNeeded ) code += ")";
               if (r != 1) code += " / " + NumToString(r);	
	code += "]\n";
	/** we have to keep a reference to this array, to avoid the defensive array copy hidden in the call to values() */
	code += "\tprivate val __enums = values()\n";
      }
      
      void deserializeEnumWithIfs(const EnumDef &enum_def, std::string & code) {
        deserializeEnumMethodDeclaration(enum_def, code);
        code +=  " = when (value.toInt()) {\n";
        for (auto it = enum_def.vals.vec.begin(); it != enum_def.vals.vec.end(); ++it) {
          auto &ev = **it;
          code += "\t" + NumToString(ev.value) + " -> " + ev.name + "\n";
        }
        code += "\telse -> throw Exception(\"Bad enum value : $value\")\n";
        code += "}\n";
      }
      
      void deserializeEnumWithMap(const EnumDef &enum_def, std::string & code) {
          deserializeEnumMethodDeclaration(enum_def, code);
          code +=  "= map[value.toInt()] ?: throw Exception(\"Bad enum value : $value\")\n";
          code += "private val map = mapOf(";
          for (auto it = enum_def.vals.vec.begin(); it != enum_def.vals.vec.end(); ++it) {
                   auto &ev = **it;
                   if (it != enum_def.vals.vec.begin()) code += ",\n\t";
                   code += NumToString(ev.value) + " to " + ev.name;
          }
          code += ")\n"; 
      }
      

      

      
      
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
      

      
      
      
      
      /** union generation (differs slightly from enums) */
      
      bool generateUnion(const EnumDef &enum_def) {
        if (enum_def.generated) return true;
        std::string code;
        atFileStart(code);
        startNamespace(code);
        //imports(code);
        unionImports(enum_def, code);
        GenComment(enum_def.doc_comment, &code, nullptr);
        unionDeclaration(enum_def, code);
        return SaveFile((enum_def.name + ".kt").c_str(), code, false);
      }
      
      void unionImports(const EnumDef &/*enum_def*/, std::string &code) {
      	  code += "import java.nio.*;\nimport com.google.flatbuffers.kotlin.*;\n\n";
          //code += "import com.google.flatbuffers.kotlin.Table\n\n";
      }
      
      void unionDeclaration(const EnumDef &enum_def, std::string & code) {
      	code += "public enum class " + enum_def.name + "(val value: " + userScalarType(enum_def.underlying_type.base_type) + ") {\n";
        unionMembersDeclaration(enum_def, code);
        code += ";\n";
        code += "\t companion object {\n";
        deserializeUnion(enum_def, code);
        untionToTable(enum_def, code);
        code += "}\n"; // end companion object
        code += "}\n"; // end enum
      }
      
      void unionMembersDeclaration(const EnumDef &enum_def, std::string & code) {
      	return enumMembersDeclaration(enum_def, code);
      }
      
      void deserializeUnion(const EnumDef &enum_def, std::string & code) {
      	return deserializeEnumWithFunction(enum_def, code);
      }
      
      void untionToTable(const EnumDef &enum_def, std::string &code) {
  	   code += "\tfun toTable( value : " + enum_def.name + ") : Table = when (value) {\n";
  	   for (auto it = enum_def.vals.vec.begin(); it != enum_def.vals.vec.end();++it) {
                   auto &ev = **it;
                   if (it == enum_def.vals.vec.begin())  code += "\t" + ev.name + " -> throw Exception(\"void union\")\n";
                   else {
                     if (ev.struct_def) {
                        auto namespac =  wrapInNameSpace(*ev.struct_def) ;
                        if (namespac.length() > ev.name.length()) code += "\t" + ev.name + " -> " + namespac + "()\n";  
                        else  code += "\t" + ev.name + " -> " + namespace_name + "." + ev.name + "()\n";
                     } else  code += "\t" + ev.name + " -> " + namespace_name + "." + ev.name + "()\n";      
                   }
  	   }
  	   code += "}\n";
      }
      
      
      
    };	  
  } // namespace kotlin
  
  bool GenerateKotlin2(const Parser &parser,const std::string &path,const std::string & file_name) {
     kotlin::KotlinGenerator * generator = new kotlin::KotlinGenerator(parser, path, file_name);
     return generator->generate();
  }
}  // namespace flatbuffers
	