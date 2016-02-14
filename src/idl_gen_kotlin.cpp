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
#include "flatbuffers/idl_gen_base_code_generator.h"

#ifdef _WIN32
#include <direct.h>
#define PATH_SEPARATOR "\\"
#define mkdir(n, m) _mkdir(n)
#else
#include <sys/stat.h>
#define PATH_SEPARATOR "/"
#endif

/** possible improvements : 
  A) maintainability : implement base abstract class (allowing others to reuse it) & concrete class for Kotlin
    1) DONE ----------remove dependency from the stuff of idl_gen_general that doesn't cleanly separate from kotlin-----------
    2) DONE implement a sharable base class, with sensible defaults, that provides nice utils but does the minimum that a parser should do
        and that has no IF, no switch, no language specific stuff
    3) DONE implement a concrete class for kotlin
  B) correctness : implement flatbuffers features : 
     1) DONE ----------namespaces are broken (only the last part gets exported)-------
     2) DONE ----------required (in constructor)------------
     3) DONE ----------defaults values (in constructor)----------
     4) key
     5) verifyer
     6) id
     7) DONE deprecated
     8) original_order
     9) bit_flags
     10) improve the enum deserialization routine (use a transform function, skips, sparse array, list with binary search or a map)
     11) nested_flatbuffer
     12) key
     13) don't generate mutable methods if the flag is not set for flatc
     
  C) enhancements :
    0) DONE (arithmetic progression for unions) improve enums performance
    1) allocation free toString() (requires recursion public __offset(byteBuffer, position) & cie)
    2) equals
    3) hashCode
    4) rewrite flatbuffer : copy parts from existing Struct/Table
    5) cache stuff that gets allocated (requires position into parent, lookup on get, clean on wrap)
    6) explore : mutate in memory (requires caching stuff), write on copy
    7) reflection on top of binary flatbuffers
    8) parse text schema into a flatbuffer
    9) search table by
    10) order table by (attach index<-> sortedIndex IntArrays)
*/

namespace flatbuffers {
  namespace kotlin {
  	  // definitions have name, index and namespace
  	  // structDef has name, index, namespace, fields (that have type)
  	  // enumdef is_union
  	  // enumval have name, val (structDef if union)
    static const char *  wireTypes[] = {
      "Byte", // NONE
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
    
    static const std::set<std::string> kotlinKeywords {"class","else", "enum","fun","for", "if", "in", "is", "import", "internal", "override", "package", "private", "protected", "public", "return", "val", "var", "when", "while"};

    class KotlinGenerator : public StronglyTypedGenerator {
    public:
      KotlinGenerator(const Parser &parser_,const std::string &path_,const std::string & file_name_) : StronglyTypedGenerator(parser_, path_, file_name_, kotlinKeywords) {};
    protected:
    	    
      /*const char * writeScalarToWire(BaseType & baseType) {
        return "add" + wireScalarType(baseType)
      }
      
      const char * readScalarFromWire(BaseType & baseType) {
        return "get" + wireScalarType(baseType)
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
  
        toString(struct_def, code);

        for (auto it = struct_def.fields.vec.begin();it != struct_def.fields.vec.end();++it) {
          auto &field = **it;
          if (field.deprecated) continue;
          generateStructAccessor(struct_def, field, code);
          if ((field.value.type.base_type == BASE_TYPE_VECTOR && IsScalar(field.value.type.VectorType().base_type)) || 
          	  field.value.type.base_type == BASE_TYPE_STRING) generateBulkAccessors(field, code);
        }

        code += "\n\tcompanion object {\n";
        if (parser.root_struct_def_ == &struct_def && parser.file_identifier_.length()) generateHasIdentifier(struct_def, code); 

        code += "\t}\n"; // end companion object
        code += "}"; // end class declaration
  
        // create a struct constructor function
        generateCreateStruct(struct_def, code); 
        
        return saveFile(struct_def.name, ".kt", code);
     }
     
      void structImports(const StructDef &/*struct_def*/, std::string &code) {
        code += "import java.nio.*;\nimport com.google.flatbuffers.kotlin.*;\n\n";
      }
      
      void generateStructAccessor(const StructDef &/*struct_def*/, const FieldDef &field, std::string & code) {
        GenComment(field.doc_comment, &code, nullptr, "");
        if (IsScalar(field.value.type.base_type)) getScalarFieldOfStruct(field, code); else switch (field.value.type.base_type) {
          case BASE_TYPE_STRUCT:getStructFieldOfStruct(field, code); return;
          case BASE_TYPE_STRING:getStringField(field, code); return;
          default:assert(0);
        }
      }
      
      void getScalarFieldOfStruct(const FieldDef &field, std::string & code) {
      	// TODO respect flatc's generate mutable flag
        code += "\tpublic var ";
        code += safeName(field) + " : " + userType(field);
        code += " get() = " + upsizeToUserType(field.value.type, wireGetter(field.value.type) + "(bb_pos + " + NumToString(field.value.offset) + ")");

        code += "; set(value) { " + wireSetter(field.value.type.base_type) + "(bb_pos + " + NumToString(field.value.offset) + ", ";
        code += downsizeToStorageValue(field.value.type, "value", true) +") }";
        code += "\n";
      }
      
      // Get a struct by initializing an existing struct.Specific to Struct.
      void getStructFieldOfStruct(const FieldDef &field, std::string & code) {
      	auto structName = field.value.type.struct_def->name;
        code += "\tpublic val " + safeName(field) + " : " + structName + " get() = " ;
        code += safeName(field) + "(" + structName + "())\n";
        code += "\tpublic fun " + safeName(field) + "(reuse : " + structName + ") : " + structName + " = ";
        code += "reuse.wrap(bb, bb_pos + " + NumToString(field.value.offset) + ")\n";
      }
      
      /** create struct */
      void generateCreateStruct(const StructDef &struct_def, std::string & code) {
        code += "\t\t";
        if (hasNestedStruct(struct_def)) code += "inline "; // only inline if there is a nested struct (or kotlin warns you)
        code += "public fun FlatBufferBuilder." + lowerFirst(struct_def.name) + "Raw(";
        for (auto it = struct_def.fields.vec.begin();it != struct_def.fields.vec.end();++it) {
          auto &field = **it;
          if (it!= struct_def.fields.vec.begin()) code += ", ";
          if (IsStruct(field.value.type)) code += "crossinline " + safeName(field) + " : FlatBufferBuilder.()->Int"; 
          else code +=  safeName(field)  + " : " + userTypeForCreation(field);
        }
        code += "):Int = with(this) {\n";
        code += "prep(" + NumToString(struct_def.minalign) + ", " + NumToString(struct_def.bytesize) + ")\n";
        for (auto it = struct_def.fields.vec.rbegin(); it != struct_def.fields.vec.rend(); ++it) {
          auto &field = **it;
          if (field.padding) code += "\t\t\tpad(" + NumToString(field.padding) + ")\n";
          if (IsStruct(field.value.type)) code += "\t\t\t" + safeName(field) + "()\n"; else {
            code += "\t\t\tadd" + wireScalarType(field.value.type.base_type) + "(";
            code +=  downsizeToStorageValue(field.value.type, safeName(field), false) + ")\n";
          }
        }
        code += "\n\toffset()\n\t}\n";
  
        if (hasNestedStruct(struct_def)) code += "inline ";
          code += "public fun FlatBufferBuilder." + lowerFirst(struct_def.name) + "(";  
          for (auto it = struct_def.fields.vec.begin();it != struct_def.fields.vec.end();++it) {
            auto &field = **it;
            if (it!= struct_def.fields.vec.begin()) code += ", ";
            if (IsStruct(field.value.type)) code += "crossinline " + safeName(field) + " : FlatBufferBuilder.()->Int"; 
            else code +=  safeName(field)  + " : " + userTypeForCreation(field); 
          }
          code += "):FlatBufferBuilder.()->Int = {"+ lowerFirst(struct_def.name) + "Raw(";
          for (auto it = struct_def.fields.vec.begin();it != struct_def.fields.vec.end();++it) {
            auto &field = **it;
            if (it!= struct_def.fields.vec.begin()) code += ", ";
            code +=  safeName(field); 
         }
         code += ")}\n";
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
  
        toString(struct_def, code);

        for (auto it = struct_def.fields.vec.begin();it != struct_def.fields.vec.end();++it) {
          auto &field = **it;
          if (field.deprecated) continue;
          generateTableAccessor(struct_def, field, code);
          if ((field.value.type.base_type == BASE_TYPE_VECTOR && IsScalar(field.value.type.VectorType().base_type)) || 
          	  field.value.type.base_type == BASE_TYPE_STRING) generateBulkAccessors(field, code);
        }

        code += "\n\tcompanion object {\n";
        
        if (parser.root_struct_def_ == &struct_def && parser.file_identifier_.length()) generateHasIdentifier(struct_def, code); 

        // Create a set of functions that allow table construction.
        generateTableBuilders(struct_def, code);

        code += "\t}\n"; // end companion object
        code += "}"; // end class declaration
  
        // create a struct constructor function
        generateCreateTable(struct_def, code);  
        
        return saveFile(struct_def.name, ".kt", code);
      }
      
      void tableImports(const StructDef &/*struct_def*/, std::string &code) {
      	      code += "import java.nio.*;\nimport com.google.flatbuffers.kotlin.*;\n\n";
      }
      
      void generateTableAccessor(const StructDef &/*struct_def*/, const FieldDef &field, std::string & code) {
        GenComment(field.doc_comment, &code, nullptr, "");
        if (IsScalar(field.value.type.base_type)) getScalarFieldOfTable(field, code); else switch (field.value.type.base_type) {
          case BASE_TYPE_STRUCT:getStructFieldOfTable(field, code); return;
          case BASE_TYPE_STRING:getStringField(field, code); return;
          case BASE_TYPE_UNION:getUnion(field, code);return;
          case BASE_TYPE_VECTOR: {
            if (field.value.type.element == BASE_TYPE_STRUCT) getMemberOfVectorOfStruct(field, code); else getMemberOfVectorOfNonStruct(field, code);
            return;
          }
          default:assert(0);
        }
      }
      
      /** scalar fields = integers/floats/enums */
      /** pointer fields = strings/structs/tables/unions */
      
      void getScalarFieldOfTable(const FieldDef &field, std::string & code) {
        code += "\tpublic val " + safeName(field) + " : " + userType(field);
        code += " get() {val o = __offset(" + NumToString(field.value.offset) + "); ";
        code += "return if (o == 0) ";
        // !!!!! if (field.value.type.base_type == BASE_TYPE_STRING) code += "null"; else code += defaultToUserType(field.value.type,  defaultValue(kotlinLang, field.value, false));
        code += defaultToUserType(field.value.type, defaultValue(field.value, false));
        // !!!!
        code += " else " + upsizeToUserType(field.value.type, wireGetter(field.value.type) + "(o + bb_pos)") + "}\n"; 

        code += "\tpublic fun mutate" + SafeName(field) + "(value : " + userType(field) + ") :Boolean {";
        code += "val o = __offset(" + NumToString(field.value.offset) + "); ";
        code += "return if (o == 0) false else {";
        code += wireSetter(field.value.type.base_type) + "(o + bb_pos, " + downsizeToStorageValue(field.value.type, "value", true);
        code += "); true}}\n";
      }
      
      // Get a struct by initializing an existing struct.Specific to Table.
      void getStructFieldOfTable(const FieldDef &field, std::string & code) {
      	StructDef * struct_def = field.value.type.struct_def;
        code += "\tpublic val " + safeName(field) + " : " + struct_def->name + "? get() {";
        code += "val o = __offset(" + NumToString(field.value.offset) + "); ";
        code += "return if (o == 0) null else " + struct_def->name + "().wrap(bb, ";
        if (struct_def->fixed) code += "o + bb_pos)"; else code += "__indirect(o + bb_pos))"; 
        code += "}\n";
  
        code += "\tpublic fun " + safeName(field) + "(reuse : " + struct_def->name + ") : " + struct_def->name + "? {";
        code += "val o = __offset(" + NumToString(field.value.offset) + "); ";
        code += "return if (o == 0) null else reuse.wrap(bb, ";
        if (struct_def->fixed) code += "o + bb_pos)"; else code += "__indirect(o + bb_pos))"; 
        code += "}\n";
      }
      
      // it's better to return null instead of false and an object instead of true ?
      // Get the value of a union from an object.
      void getUnion(const FieldDef &field, std::string & code) {
        code += "\tpublic fun " + safeName(field) + "(reuse : Table) : Table? {";
        code += "val o = __offset(" +NumToString(field.value.offset) + "); ";
        code += "return if (o == 0) null else __union(reuse, o)}\n";
  
        code += "\tpublic val " + safeName(field) + " : Table? get() {";
        code += "val o = __offset(" + NumToString(field.value.offset) + "); ";
        code += "return if (o == 0) null else __union(" + field.value.type.enum_def->name + ".toTable(" + safeName(field) +"Type), o)}\n";
      }
      
      void getArraySize(const FieldDef &field, std::string & code) {
        code += "\tpublic val " + safeName(field) + "Size : Int ";
        code += "get() {val o = __offset(" + NumToString(field.value.offset) + "); ";
        code += "return if (o == 0) 0 else __vector_len(o)}\n";
      }
      
      // Get the value of a vector's struct member.
      void getMemberOfVectorOfStruct(const FieldDef &field, std::string & code) {
      	getArraySize(field, code);
        
      	StructDef * struct_def = field.value.type.struct_def;
        code += "\tpublic fun " + safeName(field) + "(j :Int, reuse : " + struct_def->name + "? = null) : "+ struct_def->name + "? {";
        code += "val o = __offset(" + NumToString(field.value.offset) + "); ";
        code += "return if (o == 0) ";
        // you really shouldn't expect elements from an array that is empty or unset 
        code += "throw Exception(\"calling member $j of array " + safeName(field) +" which is either empty or unset\")";
        code += " else {val x = __vector(o) + j" + multiplyBySizeOf(field.value.type.VectorType()) + "; ";
        code += "(reuse ?: " + struct_def->name +"() ).wrap(bb, x";
        if (!struct_def->fixed) code += " + __indirect(x)";
        code += ")}}\n";
      }
      
      // Get the value of a vector's non-struct member. Uses a named return
      // argument to conveniently set the zero value for the result.
      void getMemberOfVectorOfNonStruct(const FieldDef &field, std::string & code) {
        getArraySize(field, code);
      	auto vector_type = field.value.type.VectorType();
      	BaseType element = field.value.type.element;
        code += "\tpublic fun " +  safeName(field) + "(j : Int) : ";
        if (element == BASE_TYPE_STRING) code += "String?"; else code += userType(field);//userScalarType(element);
        code += " {val o = __offset(" + NumToString(field.value.offset) + "); ";
        code += "return if (o == 0) ";
        // you really shouldn't expect elements from an array that is empty or unset 
        code += "throw Exception(\"calling member $j of array " + safeName(field) +" which is either empty or unset\")";
        //if (vector_type.base_type != BASE_TYPE_STRING) code += defaultToUserType(field.value.type, defaultValue(kotlinLang, field.value, false) ); else code += "null"; 
        code += " else " + upsizeToUserType(field.value.type,  wireGetter(vector_type) + "(__vector(o) + j" + multiplyBySizeOf(vector_type) + ")");
        code += "}\n";

        if (element == BASE_TYPE_STRING) generateBulkStringArrayAccessor(field, code);

        // mutation
        if (element != BASE_TYPE_STRING) {
          code += "\tpublic fun mutate" + SafeName(field) + "(j : Int, value : " + userType(field) + ") :Boolean {";
          code += "val o = __offset(" + NumToString(field.value.offset) + "); ";
          code += "return if (o == 0) false else {";
          code += wireSetter(element) + "(__vector(o) + j" + multiplyBySizeOf(vector_type);
          code += ", " + downsizeToStorageValue(vector_type, "value", true)   + "); true}}\n";
        }
      }
      
      void generateBulkStringArrayAccessor(const FieldDef &field, std::string & code) {
      	  code += "\tpublic fun " + safeName(field) + "Buffer(j : Int) : ByteBuffer? ";
          code += " {val o = __offset(" + NumToString(field.value.offset) + "); ";
          code += "return if (o == 0) ";
          // you really shouldn't expect elements from an array that is empty or unset 
          code += "throw Exception(\"calling member $j of array " + safeName(field) +" which is either empty or unset\")";
          code += " else __string_element_as_bytebuffer(o, j)";
          code += "}\n";
      }
      
      void generateBulkAccessors(const FieldDef &field, std::string & code) {
        code += "\tpublic val " + safeName(field);
        if (field.value.type.base_type == BASE_TYPE_STRING) code += "Buffer";
        code += " : ByteBuffer get() = __vector_as_bytebuffer(";
        code += NumToString(field.value.offset) + ", ";
        code += NumToString(field.value.type.base_type == BASE_TYPE_STRING ? 1 : InlineSize(field.value.type.VectorType()));
        code += ")\n";
      }
      /** create table */
      
      void generateCreateTable(const StructDef &struct_def,std::string & code) {
        int num_fields = 0;
        for (auto it = struct_def.fields.vec.begin(); it != struct_def.fields.vec.end(); ++it) {
          auto &field = **it;
          if (field.deprecated) continue; else num_fields++;
        }
        // if there are only deprecated fields in this table, don't generate a create method
        if (!num_fields) return; 
   
        // method declaration
        code += "\t\tpublic fun FlatBufferBuilder." + lowerFirst(struct_def.name) + "(";
        bool first = true;
        // add required parameters first without defaults
        for (auto it = struct_def.fields.vec.begin(); it != struct_def.fields.vec.end(); ++it) {
          auto &field = **it;
          if (field.deprecated || !field.required) continue;
          if (!first) code += ", ";
          first = false;
        
          code += safeName(field);
          if (!IsScalar(field.value.type.base_type) && (!struct_def.fixed)) {
            if (IsStruct(field.value.type) && field.value.type.struct_def->fixed) code += " : FlatBufferBuilder.()->Int "; else code += " : Int ";
          } else code += " : " + userTypeForCreation(field);
        }
        
        // add remaining optional parameters
        for (auto it = struct_def.fields.vec.begin(); it != struct_def.fields.vec.end(); ++it) {
          auto &field = **it;
          if (field.deprecated || field.required) continue;
          if (!first) code += ", ";
          first = false;
        
          code += sanitize(field.name, false);
          if (!IsScalar(field.value.type.base_type) && (!struct_def.fixed)) {
            if (IsStruct(field.value.type) && field.value.type.struct_def->fixed) code += " : FlatBufferBuilder.()->Int "; else code += " : Int ";
          } else code += " : " + userTypeForCreation(field);
          code += " = ";
          if (!IsScalar(field.value.type.base_type) && (!struct_def.fixed)) {   
            if (IsStruct(field.value.type) && field.value.type.struct_def->fixed) code += "{0}"; else code += "0"; 
          } else code += defaultToUserType(field.value.type, defaultValue(field.value, false) ); 
        } 
        // method body
        code += ") = with(this) {\n\t\twith(" + MakeCamel(struct_def.name,true) + " ) {\n\t\t\tstartObject(" + NumToString(struct_def.fields.vec.size()) + ")\n";
      
        for (size_t size = struct_def.sortbysize ? sizeof(largest_scalar_t) : 1;size;size /= 2) {
          for (auto it = struct_def.fields.vec.rbegin();it != struct_def.fields.vec.rend(); ++it) {
            auto &field = **it;
            if (!field.deprecated &&(!struct_def.sortbysize || size == SizeOf(field.value.type.base_type))) {
              /** inlining the field builder */
              auto offset =   struct_def.fields.vec.rend() - 1 -it ;
              code += "\t\t\tadd" + wireTypeForCreation(field)/*GenMethod(kotlinLang, field.value.type)*/ + "(" + NumToString(offset) + ", "  ;
              if (!IsScalar(field.value.type.base_type)) {
  	        code += safeName(field);
  	        if (IsStruct(field.value.type) && field.value.type.struct_def->fixed) code += "()"; 
              } else  code +=  downsizeToStorageValue(field.value.type, safeName(field), false);
              code += ", " + field.value.constant; //Int  
              if (field.value.type.base_type == BASE_TYPE_BOOL) code += "!=0"; // hack to work with booleans... bad :(
                code += ")\n";
              }
          }
        }         
        code += "\t\t\tendObject()\n\t\t}}\n";
      }

      /** mutable table creation */
      void generateTableBuilders(const StructDef &struct_def, std::string &code) {
        for (auto it = struct_def.fields.vec.begin();it != struct_def.fields.vec.end();++it) {
          auto &field = **it;
          if (field.deprecated) continue;
          if (field.value.type.base_type != BASE_TYPE_VECTOR) continue;
          auto vector_type = field.value.type.VectorType();
          auto alignment = InlineAlignment(vector_type);
          auto elem_size = InlineSize(vector_type);
          buildArrayWithLambda(field, code, alignment, elem_size);
          if (field.value.type.element == BASE_TYPE_STRUCT && field.value.type.struct_def->fixed) continue;
          // TODO what about aray of unions ?
          if (IsScalar(field.value.type.element)) buildScalarArrayWithVararg(field, code, alignment, elem_size); else buildTableOrStringArrayWithVararg(field, code); 
        }
        generateFinishBuffer(struct_def, code);
      }
      
      void buildArrayWithLambda(const FieldDef &field, std::string & code, int alignment, int elem_size) {
        code += "\t\tinline public fun FlatBufferBuilder." + safeName(field);
        code += "(numElems : Int, action : FlatBufferBuilder.()->Unit) : Int {startArray(";
        code += NumToString(elem_size);
        code += ", numElems, " + NumToString(alignment);
        code += "); action(); return endArray()}\n";
      }
      
      // TODO replace varargs with array
      void buildScalarArrayWithVararg( const FieldDef &field, std::string &code, int alignment, int elem_size) {
        code += "\t\tfun FlatBufferBuilder." + safeName(field);
        code += "(vararg data : ";
        code += userType(field);
        code += "): Int {startArray(";
        code +=  NumToString(elem_size) + ", data.size, ";
        // TODO addToWire ???
        code += NumToString(alignment) +"); for (i in data.size - 1 downTo 0) add" + wireScalarType(field.value.type.element) + "(" + downsizeToStorageValue(field.value.type.VectorType(), "data[i]", false) + "); return endArray(); }\n";
      }
      
      // TODO replace varargs with array
      void buildTableOrStringArrayWithVararg( const FieldDef &field, std::string & code) {
        code += "\t\tpublic fun FlatBufferBuilder." + safeName(field);
        code += "(vararg offsets : Int) ";
        code += " : Int {startArray(4, offsets.size, ";
        // table or string
        if (field.value.type.struct_def != nullptr) code += NumToString(field.value.type.struct_def->minalign); else code += "4"; // look into this
        code += "); for (i in offsets.size - 1 downTo 0) addOffset(offsets[i]); return endArray(); }\n";
      }
      
      void generateFinishBuffer(const StructDef &struct_def, std::string & code) {
        code += "\t\tfun  FlatBufferBuilder.finishBuffer(offset : Int) { finish(offset";  
        if (parser.root_struct_def_ == &struct_def && parser.file_identifier_.length()) code += ", \"" + parser.file_identifier_ + "\"";
        code += ") }\n";
      }
      
      /** table and struct */
      
      // Get the value of a string.
      void getStringField(const FieldDef &field, std::string & code) {
        code += "\tpublic val " + safeName(field) + " : String? get() {";
        code += "val o = __offset(" + NumToString(field.value.offset) + "); ";
        code += "return if (o == 0) null else " + wireGetter(field.value.type) + "(o + bb_pos)";
        code += "}\n";
      }
      
      void tableOrStructInitialize(const StructDef &struct_def, std::string & code) {
        code += "\tpublic fun wrap(byteBuffer : ByteBuffer, position : Int = byteBuffer.getInt(byteBuffer.position()) + byteBuffer.position()) : " + struct_def.name + " = apply {";
        code +=  "bb = byteBuffer; ";
        code +=  "bb_pos = position}\n";
      }
      
      void generateHasIdentifier(const StructDef & /*struct_def*/, std::string &code) {
      	 // Check if a buffer has the identifier.
         code += "\tpublic fun hasIdentifier(byteBuffer : ByteBuffer) :Boolean = Table.hasIdentifier(byteBuffer, \"" + parser.file_identifier_ + "\")\n";
      }
      
      /** for debugging only, this doesn't reuse accessors and creates new objects like crazy */
      void toString(const StructDef &struct_def, std::string &code) {
        code += "\toverride public fun toString() : String = \"" + MakeCamel(struct_def.name, true) + "(";  
        bool first = true;  
        for (auto it = struct_def.fields.vec.begin();it != struct_def.fields.vec.end();++it) {
    	    auto &field = **it;
    	    if (field.deprecated) continue;
    	    if (!first) code +=  ",";
    	    first = false;
    	    code += safeName(field) + "=";
    	    
    	    if (field.value.type.base_type != BASE_TYPE_VECTOR) code += "$" + safeName(field);
    	    else  code += "${(0 until " + safeName(field) + "Size).map({" + safeName(field) + "(it).toString()}).joinToString(\", \", \"[\",\"]\")}";
        }
        code += ")\"\n";
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
        return saveFile(enum_def.name, ".kt", code);
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
      	  code += "\tfun from(value : ";
          code += wireScalarType(enum_def.underlying_type.base_type);
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
      

      
      /** type conversion && get / put into wire */
      
      
      /** scalar type for the getters/setters on the wire facing api
          this is usually a language type that can hold the schema type 
          and that ressembles it the most,
          except that booleans are serialized as byte (in the spec)
      */
      std::string wireScalarType(const BaseType &base_type) {
      	      return wireTypes[base_type];
      }
      
      /** scalar type for the getters/setters on the user facing api 
          this is the most convenient language type that can hold the wire type
          except that booleans are serialized as byte (see the flatbuffers spec)
      */
      std::string userScalarType(const BaseType &base_type) {
      	      return userTypes[base_type];
      }
      
      std::string userType(const FieldDef &field) {
      	      Type type = field.value.type;
      	      if (type.struct_def != nullptr) return type.struct_def->name; // struct or Table
      	      BaseType base_type = type.base_type;
      	      if (base_type == BASE_TYPE_STRING) return "String";
      	      if (base_type == BASE_TYPE_VECTOR) base_type = type.element;
      	      if (type.enum_def == nullptr) return userTypes[base_type];
      	      if (base_type == BASE_TYPE_UNION) return "Table"; else return type.enum_def->name; // enum or unionType (which is an enum)
      }
      
      /** create methods in this code generator use Int for all pointer type and regular userTypes for scalar types*/
      std::string userTypeForCreation(const FieldDef &field) {
         if (IsScalar(field.value.type.base_type)) return userType(field);
         return "Int";
      }
      
      std::string wireTypeForCreation(const FieldDef &field) {
      	      BaseType base_type = field.value.type.base_type;
         if (IsScalar(base_type)) return wireScalarType(base_type);
         if  (field.value.type.struct_def != nullptr && field.value.type.struct_def->fixed && base_type!= BASE_TYPE_VECTOR) return "Struct"; else return "Offset";
      }
      
      
      std::string wireSetter(const BaseType & base_type) {	   
	if (wireScalarType(base_type) == "Byte" || wireScalarType(base_type) == "Boolean") return "bb.put"; else return "bb.put" + wireScalarType(base_type);
      }
      
           // Returns the function name that is able to read a value of the given type.
     std::string wireGetter(const Type &type) {
       switch (type.base_type) {
         case BASE_TYPE_STRING: return "__string";
         case BASE_TYPE_STRUCT: return "__struct";
         case BASE_TYPE_UNION:  return "__union";
         case BASE_TYPE_VECTOR: return wireGetter(type.VectorType());
         default: {
           if (type.base_type == BASE_TYPE_BOOL) return  "0.toByte()!=bb.get";
           std::string wireType = wireScalarType(type.base_type);
           if (wireType == "Byte") return "bb.get";
           return  std::string("bb.get") + wireType;
    }
  }
}
     
     /** downsize a scalar userType to a scalar wireType 
         with tricky corner cases : 
         1) with vector, you have to transform the element type
         2) with enums, you have to first transform the enum into a scalar userType before you turn it into a scalar wireType
         */
      std::string downsizeToStorageValue(const Type &type, const std::string value, const bool downsizeBool) {
             std::string val(value);
             if (type.enum_def != nullptr) val += ".value";
             switch (type.base_type) {
  	       case BASE_TYPE_BOOL:  return downsizeBool ? "if (" + val + ") 1.toByte() else 0.toByte()" : val;
  	       case BASE_TYPE_UINT:  return val + ".and(0xFFFFFFFFL).toInt()";
  	       case BASE_TYPE_USHORT:  return val + ".and(0xFFFF).toShort()"; // are "ands" really necessary here ? 
  	      case BASE_TYPE_UCHAR:  return val + ".and(0xFF).toByte()";
  	      case BASE_TYPE_VECTOR:  return downsizeToStorageValue(type.VectorType(), value, downsizeBool);
  	     default: return val;
          }
     } 
     
     
     std::string upsizeToUserType(const Type &type, const std::string value) {
       if (type.enum_def != nullptr && type.base_type != BASE_TYPE_UNION) return type.enum_def->name + ".from(" + value + ")";  
       switch (type.base_type) {
       case BASE_TYPE_UINT:  return value + ".toLong().and(0xFFFFFFFFL)";
       case BASE_TYPE_USHORT:  return value + ".toInt().and(0xFFFF)";
       case BASE_TYPE_UCHAR:  return value + ".toInt().and(0xFF)";
       case BASE_TYPE_VECTOR:  return upsizeToUserType(type.VectorType(), value);
       default:  return value;
      }
     }
      





     
     std::string defaultToUserType(const Type &type, const std::string value) {
       // we have to catch enums before the switch because otherwise they follow the path of their underlying base_type.
       if (type.enum_def != nullptr && type.base_type != BASE_TYPE_UNION) return type.enum_def->name + ".from(" + value + ")"; 
       switch (type.base_type) {
       case BASE_TYPE_UINT:  return value + ".toLong().and(0xFFFFFFFFL)";
       case BASE_TYPE_USHORT:  return value + ".toInt().and(0xFFFF)";
       case BASE_TYPE_UCHAR:  return value + ".toInt().and(0xFF)";
       case BASE_TYPE_VECTOR:  return defaultToUserType(type.VectorType(), value);
       default:   return value;
   }
}


// removed static to allow reuse
std::string defaultValue(const Value &value, bool for_buffer) {

  if (!for_buffer) {  
      switch (value.type.base_type) {
      	case BASE_TYPE_CHAR:return "(" + value.constant + ").toByte()";
      	case BASE_TYPE_SHORT:return "(" +value.constant + ").toShort()";
      	case BASE_TYPE_LONG:
      	case BASE_TYPE_ULONG:return value.constant + "L";
      	case BASE_TYPE_FLOAT:return value.constant + "f";
      	case BASE_TYPE_VECTOR: switch (value.type.element) {
      		case BASE_TYPE_CHAR:return "(" +value.constant + ").toByte()";
      		case BASE_TYPE_SHORT:return "(" +value.constant + ").toShort()";
      		case BASE_TYPE_LONG:
      		case BASE_TYPE_ULONG:return value.constant + "L";
      		case BASE_TYPE_FLOAT:return value.constant + "f";
      		default:return value.type.element == BASE_TYPE_BOOL ? (value.constant == "0" ? "false" : "true") : value.constant;
      	}
    	default:break;
      }
  }
  
  return value.type.base_type == BASE_TYPE_BOOL
           ? (value.constant == "0" ? "false" : "true")
           : value.constant;
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
        return saveFile(enum_def.name, ".kt", code);
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
        unionToTable(enum_def, code);
        code += "}\n"; // end companion object
        code += "}\n"; // end enum
      }
      
      void unionMembersDeclaration(const EnumDef &enum_def, std::string & code) {
      	return enumMembersDeclaration(enum_def, code);
      }
      
      void deserializeUnion(const EnumDef &enum_def, std::string & code) {
      	return deserializeEnumWithFunction(enum_def, code);
      }
      
      void unionToTable(const EnumDef &enum_def, std::string &code) {
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
  
  bool GenerateKotlin(const Parser &parser,const std::string &path,const std::string & file_name) {
     kotlin::KotlinGenerator * generator = new kotlin::KotlinGenerator(parser, path, file_name);
     return generator->generate();
  }
}  // namespace flatbuffers
	
