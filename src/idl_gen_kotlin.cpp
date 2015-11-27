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

#ifdef _WIN32
#include <direct.h>
#define PATH_SEPARATOR "\\"
#define mkdir(n, m) _mkdir(n)
#else
#include <sys/stat.h>
#define PATH_SEPARATOR "/"
#endif

namespace flatbuffers {

	
	// importing these struct to imitate the general framework and ease the mergin of Kotlin support (when the general framework is ready :/)
	
//	struct LanguageParameters;

struct CommentConfig {
  const char *first_line;
  const char *content_line_prefix;
  const char *last_line;
};

	struct LanguageParameters {
  GeneratorOptions::Language language;
  // Whether function names in the language typically start with uppercase.
  bool first_camel_upper;
  const char *file_extension;
  const char *string_type;
  const char *bool_type;
  const char *open_curly;
  const char *const_decl;
  const char *unsubclassable_decl;
  const char *enum_decl;
  const char *enum_separator;
  const char *getter_prefix;
  const char *getter_suffix;
  const char *inheritance_marker;
  const char *namespace_ident;
  const char *namespace_begin;
  const char *namespace_end;
  const char *set_bb_byteorder;
  const char *get_bb_position;
  const char *get_fbb_offset;
  const char *includes;
  CommentConfig comment_config;
};

	extern LanguageParameters language_parameters[];
	
	// global in this file
	auto kotlinLang = language_parameters[GeneratorOptions::kKotlin];	
	
namespace kotlin {	
               // functions that can't be reused because they are static : we have to duplicate them 
               static Type DestinationType(const LanguageParameters &lang, const Type &type, bool vectorelem);
	static std::string GenTypeForUser(const LanguageParameters &lang, const Type &type);
	static std::string GenDefaultValue(const LanguageParameters &lang, const Value &value, bool for_buffer);
	static std::string GenTypeGet(const LanguageParameters &lang, const Type &type);
	static std::string GenTypeBasic(const LanguageParameters &lang, const Type &type);
	static std::string GenTypeNameDest(const LanguageParameters &lang, const Type &type);
	static std::string GenMethod(const LanguageParameters &lang, const Type &type);
	static std::string GenSetterKotlin(const LanguageParameters &lang, const Type &type);
	static std::string GenGetterKotlin(const LanguageParameters &lang, const Type &type);

	// necessary to avoid name clash with kotlin's reserved words (val, var, fun...)
              static std::string sanitize(const std::string name, const bool isFirstLetterUpper);
	
	// the kotlin type transforms
	static std::string downsizeToStorageValue(const Type &type, const std::string value, const bool downsizeBool);
	static std::string upsizeToUserType(const Type &type, std::string value);
	static std::string defaultToUserType(const Type &type, const std::string value);	

	/* for use in createSruct() args */
               static std::string downsizeToStorageValueForConstructor(const Type &type, const std::string value);
	static std::string GenTypeForUserConstructor(const Type &type) {
		return GenTypeForUser(kotlinLang, DestinationType(kotlinLang, type, false));
	}
	
              static std::string multiplyBySizeOf(const Type &type);

	
	static void createArrayOfStruct( const FieldDef &field, std::string *code_ptr);
	static void createArrayOfNonStruct( const FieldDef &field, std::string *code_ptr);
	static void GenStructBuilder(const StructDef &struct_def, std::string *code_ptr);



std::string LowerFirst(const std::string &in) {
  std::string s;
  for (size_t i = 0; i < in.length(); i++) {
    if (!i) s += static_cast<char>(tolower(in[0])); else s += in[i];
  }
  return s;
}


// Begin by declaring namespace and imports.
static void BeginFile(const std::string name_space_name, const bool needs_imports, std::string *code_ptr) {
  std::string &code = *code_ptr;
  code += "// automatically generated, do not modify\n\n";
  code += "package " + name_space_name + "\n\n";
  if (needs_imports) code += "import java.nio.*;\nimport com.google.flatbuffers.kotlin.*;\n\n";
 
}

// Begin enum code with a class declaration.
static void beginEnumDeclaration(const EnumDef &enum_def, std::string *code_ptr) {
  std::string &code = *code_ptr;
  code += "public enum class " + enum_def.name + "(val value: " + GenTypeGet(kotlinLang, enum_def.underlying_type) + ") {\n";

}

// A single enum member.
static void enumMember(const EnumVal ev,
                       std::string *code_ptr) {
  std::string &code = *code_ptr;
  code += "\t" +  ev.name + "(" + NumToString(ev.value) + ")";
}

static void endEnumDeclaration(const EnumDef &enum_def,std::string *code_ptr) {
  std::string &code = *code_ptr;

  code += "\t companion object {\n\tfun from( value : " + GenTypeGet(kotlinLang, enum_def.underlying_type) + ") : " + enum_def.name  + " = when (value.toInt()) {\n";
  for (auto it = enum_def.vals.vec.begin(); it != enum_def.vals.vec.end();++it) {
        auto &ev = **it;
	code += "\t" + NumToString(ev.value) + " -> " + ev.name + "\n";
  }
   code += "\telse -> throw Exception(\"Bad enum value : $value\")\n";
  code += "}\n}\n";
  code += "}\n";
}

// Initialize a new struct or table from existing data.
/** static method : use with 
     val monster = Monster.wrap(byteBuffer)  // always allocate a new Monster instance
     or
     val monster = reusableMonster.wrap(byteBuffer) // reuse a Monster instance 

*/
/*static void NewRootTypeFromBuffer(const StructDef &struct_def, std::string *code_ptr) {
  std::string &code = *code_ptr;

  code += "public fun rootAs" + struct_def.name + "(byteBuffer : ByteBuffer, reuse : " + struct_def.name + "? = null) : ";
  code += struct_def.name + " {byteBuffer.order(ByteOrder.LITTLE_ENDIAN); ";
  code += "return (reuse ?: " + struct_def.name + "()).wrap(byteBuffer)}\n";

   // static method
       val monster = Monster.wrap(byteBuffer)  // allocates and wrap into a new Monster instance
  
//  code += "public fun wrap(byteBuffer : ByteBuffer) : " + struct_def.name + " = " + struct_def.name + "().wrap(byteBuffer.order(ByteOrder.LITTLE_ENDIAN))\n";
}*/

// Initialize an existing object with other data, to avoid an allocation.
static void initializeTableAndStruct(const StructDef &struct_def, std::string *code_ptr) {
  std::string &code = *code_ptr;

  code += "\tpublic fun wrap(byteBuffer : ByteBuffer, position : Int = byteBuffer.getInt(byteBuffer.position()) + byteBuffer.position()) : " + struct_def.name + " = apply {";
  code +=  "bb = byteBuffer; ";
  code +=  "bb_pos = position}\n";
}


/* class */
static void beginClassDeclaration(const StructDef &struct_def, std::string *code_ptr) {
  std::string &code = *code_ptr;
  code += "public open class " + struct_def.name + "(byteBuffer : ByteBuffer = EMPTY_BYTEBUFFER) : ";
  if (struct_def.fixed) code += "Struct"; else code+= "Table"; 
  code += "(byteBuffer.order(ByteOrder.LITTLE_ENDIAN), if (byteBuffer === EMPTY_BYTEBUFFER) 0 else byteBuffer.getInt(byteBuffer.position()) + byteBuffer.position()) {\n";
}

static void beginCompanionObject(std::string *code_ptr) {
  std::string &code = *code_ptr;
  code += "\n\tcompanion object {\n";
}

static void endCompanionObject(std::string *code_ptr) {
  std::string &code = *code_ptr;
  code += "\t}\n";
}


static void endClassDeclaration(std::string *code_ptr) {
  std::string &code = *code_ptr;
  code += "}";
}

static void getArraySize(const FieldDef &field, std::string *code_ptr) {
  std::string &code = *code_ptr;
  code += "\tpublic val " + sanitize(field.name, false) + "Size : Int ";
  code += "get() {val o = __offset(" + NumToString(field.value.offset) + "); ";
  code += "return if (o == 0) 0 else __vector_len(o)}\n";
}

static void getScalarFieldOfStruct(const FieldDef &field, std::string *code_ptr) {
  std::string &code = *code_ptr;
  //std::string getter =  GenGetterKotlin(kotlinLang, field.value.type);
  //std::string setter = GenSetterKotlin(kotlinLang, field.value.type);

  /*if (field.value.type.base_type == BASE_TYPE_STRING) code += "\tpublic val "; else*/  code += "\tpublic var ";
  code+= sanitize(field.name, false) + " : " + GenTypeNameDest(kotlinLang, field.value.type);
  code += " get() = " + upsizeToUserType(field.value.type,  GenGetterKotlin(kotlinLang, field.value.type) + "(bb_pos + " + NumToString(field.value.offset) + ")");

  //if (field.value.type.base_type != BASE_TYPE_STRING) {
	code += "; set(value) { " + GenSetterKotlin(kotlinLang, field.value.type) + "(bb_pos + " + NumToString(field.value.offset) + ", ";
	code += downsizeToStorageValue(field.value.type, "value", true) +") }";
  //}  
 code += "\n";
}

static void getScalarFieldOfTable(const FieldDef &field, std::string *code_ptr) {
  std::string &code = *code_ptr;
  //std::string getter = GenGetterKotlin(kotlinLang, field.value.type);

  code += "\tpublic val " + sanitize(field.name, false) + " : " + GenTypeNameDest(kotlinLang, field.value.type);
  code += " get() {val o = __offset(" +NumToString(field.value.offset) + "); ";
  code += "return if (o == 0) ";
  if (field.value.type.base_type == BASE_TYPE_STRING) code += "null"; else code += defaultToUserType(field.value.type,  GenDefaultValue(kotlinLang, field.value, false));
  code += " else " + upsizeToUserType(field.value.type, GenGetterKotlin(kotlinLang, field.value.type) + "(o + bb_pos)") + "}\n"; 

    code += "\tpublic fun mutate" + sanitize(field.name, true) + "(value : " + GenTypeNameDest(kotlinLang, field.value.type) + ") :Boolean {";
    code += "val o = __offset(" + NumToString(field.value.offset) + "); ";
    code += "return if (o == 0) false else {";
    code += GenSetterKotlin(kotlinLang, field.value.type) + "(o + bb_pos, " + downsizeToStorageValue(field.value.type, "value", true);
    code += "); true}}\n";
}

// Get a struct by initializing an existing struct.
// Specific to Struct.
static void getStructFieldOfStruct(const FieldDef &field, std::string *code_ptr) {
  std::string &code = *code_ptr;

  code += "\tpublic val " + sanitize(field.name, false) + " : " + GenTypeNameDest(kotlinLang, field.value.type) + " get() = " ;
  code += sanitize(field.name, false) + "(" + GenTypeNameDest(kotlinLang, field.value.type) + "())\n";
  

 code += "\tpublic fun " + sanitize(field.name, false) + "(reuse : " + GenTypeNameDest(kotlinLang, field.value.type) + ") : " + GenTypeNameDest(kotlinLang, field.value.type) + " = ";
  code += "reuse.wrap(bb, bb_pos + " + NumToString(field.value.offset) + ")\n";

  
/*  code += "public fun " + sanitize(field.name, false) + "(reuse : " + GenTypeNameDest(kotlinLang, field.value.type) + "? = null) : " + GenTypeNameDest(kotlinLang, field.value.type) + " = ";
  code += "(reuse?: " + GenTypeNameDest(kotlinLang, field.value.type) + "()).wrap(bb, bb_pos + " + NumToString(field.value.offset) + ")\n";
*/
}

// Get a struct by initializing an existing struct.
// Specific to Table.
static void getStructFieldOfTable(const FieldDef &field, std::string *code_ptr) {
  std::string &code = *code_ptr;
  code += "\tpublic val " + sanitize(field.name, false) + " : " + GenTypeNameDest(kotlinLang, field.value.type)+ "? get() {";
  code += "val o = __offset(" +NumToString(field.value.offset) + "); ";
  code += "return if (o == 0) null else " + GenTypeNameDest(kotlinLang, field.value.type) + "().wrap(bb, ";
  if (field.value.type.struct_def->fixed) code += "o + bb_pos)"; else code += "__indirect(o + bb_pos))"; 
  code += "}\n";
  
  code += "\tpublic fun " + sanitize(field.name, false) + "(reuse : " +GenTypeNameDest(kotlinLang, field.value.type) + ") : " + GenTypeNameDest(kotlinLang, field.value.type)+ "? {";
  code += "val o = __offset(" +NumToString(field.value.offset) + "); ";
  code += "return if (o == 0) null else reuse.wrap(bb, ";
  if (field.value.type.struct_def->fixed) code += "o + bb_pos)"; else code += "__indirect(o + bb_pos))"; 
  code += "}\n";

/*

  code += "public fun " + sanitize(field.name, false) + "(reuse : " +GenTypeNameDest(kotlinLang, field.value.type) + "? = null) : " + GenTypeNameDest(kotlinLang, field.value.type)+ "? {";
  code += "val o = __offset(" +NumToString(field.value.offset) + "); ";
  code += "return if (o == 0) null else (reuse ?:" + GenTypeNameDest(kotlinLang, field.value.type) + "()).wrap(bb, ";
  if (field.value.type.struct_def->fixed) code += "o + bb_pos)"; else code += "__indirect(o + bb_pos))"; 
  code += "}\n";*/
}

// Get the value of a string.
static void getStringField(const FieldDef &field, std::string *code_ptr) {
  std::string &code = *code_ptr;

  code += "\tpublic val " +sanitize(field.name, false) + " : String? get() {";
  code += "val o = __offset(" +NumToString(field.value.offset) + "); ";
  code += "return if (o == 0) null else " + GenGetterKotlin(kotlinLang, field.value.type) + "(o + bb_pos)";
  code += "}\n";
}

// it's better to return null instead of false and an object instead of true ?
// Get the value of a union from an object.
static void getUnion(const FieldDef &field, std::string *code_ptr) {
  std::string &code = *code_ptr;

  code += "\tpublic fun " + sanitize(field.name, false) + "(reuse : " + GenTypeNameDest(kotlinLang, field.value.type) + ") : " + GenTypeNameDest(kotlinLang, field.value.type) + "? {";
  code += "val o = __offset(" +NumToString(field.value.offset) + "); ";
  code += "return if (o == 0) null else __union(reuse, o)}\n";
}

// Get the value of a vector's struct member.
static void GetMemberOfVectorOfStruct(
                                      const FieldDef &field,
                                      std::string *code_ptr) {
  std::string &code = *code_ptr;
  auto vectortype = field.value.type.VectorType();

  getArraySize(field, code_ptr);
  
  code += "\tpublic fun " + sanitize(field.name, false) + "(j :Int, reuse : " + GenTypeNameDest(kotlinLang, field.value.type) + "? = null) : "+ GenTypeNameDest(kotlinLang, field.value.type) + "? {";
  code += "val o = __offset(" +NumToString(field.value.offset) + "); ";
  code += "return if (o == 0) null else {";
  code += "val x = __vector(o) + j" + multiplyBySizeOf(vectortype) + "; ";
  code += "(reuse ?: " + GenTypeNameDest(kotlinLang, field.value.type) +"() ).wrap(bb, x";
  if (!vectortype.struct_def->fixed) code += " + __indirect(x)";
  code += ")}}\n";
}

// Get the value of a vector's non-struct member. Uses a named return
// argument to conveniently set the zero value for the result.
static void GetMemberOfVectorOfNonStruct(
                                         const FieldDef &field,
                                         std::string *code_ptr) {
  std::string &code = *code_ptr;
  auto vector_type = field.value.type.VectorType();
  getArraySize(field, code_ptr);
  
  code += "\tpublic fun " + sanitize(field.name, false) + "(j : Int) : " + GenTypeNameDest(kotlinLang, field.value.type);
  if (vector_type.base_type == BASE_TYPE_STRING) code += "?";
  code += " {val o = __offset(" + NumToString(field.value.offset) + "); ";
  code += "return if (o == 0) ";
  if (vector_type.base_type != BASE_TYPE_STRING) code += defaultToUserType(field.value.type, GenDefaultValue(kotlinLang, field.value, false) )/** fix here */ /*+ afterStorageType(vector_type, true)*/; else code += "null"; 
  code += " else " + upsizeToUserType(field.value.type,  GenGetterKotlin(kotlinLang, field.value.type) + "(__vector(o) + j" + multiplyBySizeOf(vector_type) + ")");
//	if (vector_type.base_type != BASE_TYPE_STRING) code += upsizeToUserType(field.value.type); 
code += "}\n";

  if (vector_type.base_type != BASE_TYPE_STRING) {
    code += "\tpublic fun mutate" + sanitize(field.name, true) + "(j : Int, value : " + GenTypeNameDest(kotlinLang, vector_type) + ") :Boolean {";
    code += "val o = __offset(" + NumToString(field.value.offset) + "); ";
    code += "return if (o == 0) false else {";
    code += GenSetterKotlin(kotlinLang, vector_type) + "(__vector(o) + j" + multiplyBySizeOf(vector_type);
    code += ", " + downsizeToStorageValue(vector_type, "value", true)   /*downsizeToStorageValue(field.value.type, "value")*/ + "); true}}\n";
  }
}


static void fieldAsByteBuffer(const FieldDef &field,
                             std::string *code_ptr) {
                             std::string &code = *code_ptr;
                                      
      code += "\tpublic val " + sanitize(field.name, false);
      code += "AsByteBuffer :ByteBuffer get() = __vector_as_bytebuffer(";
      code += NumToString(field.value.offset) + ", ";
      code += NumToString(field.value.type.base_type == BASE_TYPE_STRING ? 1 :
                          InlineSize(field.value.type.VectorType()));
      code += ")\n";
                             }

// Begin the creator function signature.
static void BeginBuilderArgs(const StructDef &struct_def,
                             std::string *code_ptr) {
  std::string &code = *code_ptr;
//  code += "\t\tfun FlatBufferBuilder.create" + struct_def.name + "(";
  code += "\t\tpublic fun FlatBufferBuilder." + LowerFirst(struct_def.name) + "Of(";
			     }

// Recursively generate arguments for a constructor, to deal with nested
// structs.
static void StructBuilderArgs(const StructDef &struct_def,
                              const char *nameprefix,
                              std::string *code_ptr, bool isRoot) {
  for (auto it = struct_def.fields.vec.begin();it != struct_def.fields.vec.end();++it) {
    auto &field = **it;
    if (IsStruct(field.value.type)) {
      // Generate arguments for a struct inside a struct. To ensure names
      // don't clash, and to make it obvious these arguments are constructing
      // a nested struct, prefix the name with the field name.
      StructBuilderArgs(*field.value.type.struct_def, (nameprefix + (sanitize(field.name, false) + "_")).c_str(), code_ptr, isRoot && it == struct_def.fields.vec.begin());
    } else {
      std::string &code = *code_ptr;
      if (!isRoot || it!= struct_def.fields.vec.begin()) code += ", ";
      code +=  nameprefix +sanitize(field.name, false) + " : " +  GenTypeForUserConstructor(field.value.type); // offset for structs/tables/string
    }
  }
}

// End the creator function signature.
static void EndBuilderArgs(std::string *code_ptr) {
  std::string &code = *code_ptr;
  code += ") :Int = ";
}

// Recursively generate struct construction statements and instert manual
// padding.
static void StructBuilderBody(const StructDef &struct_def,
                              const char *nameprefix,
                              std::string *code_ptr) {
  std::string &code = *code_ptr;
  code += "prep(" + NumToString(struct_def.minalign) + ", " + NumToString(struct_def.bytesize) + ")\n";
  for (auto it = struct_def.fields.vec.rbegin(); it != struct_def.fields.vec.rend(); ++it) {
    auto &field = **it;
    if (field.padding) code += "\t\t\t.pad(" + NumToString(field.padding) + ")\n";
    if (IsStruct(field.value.type)) {
    	    code += "\t\t\t.";
      StructBuilderBody(*field.value.type.struct_def, (nameprefix + (sanitize(field.name, false) + "_")).c_str(), code_ptr);
    } else {
      code += "\t\t\t.add" + GenMethod(kotlinLang, field.value.type) + "(";
      code +=  downsizeToStorageValueForConstructor(field.value.type, nameprefix + sanitize(field.name, false)) + ")\n";
    }
  }
}

static void EndBuilderBody(std::string *code_ptr) {
  std::string &code = *code_ptr;
  code += "\t\t\t.offset()\n";
}

// Get the value of a table's starting offset.
static void GetStartOfTable(const StructDef &struct_def,
                            std::string *code_ptr) {
  std::string &code = *code_ptr;
//  code += "\t\tpublic fun FlatBufferBuilder.start() :FlatBufferBuilder = startObject(" + NumToString(struct_def.fields.vec.size()) + ")\n";

  
  
  
 code += "\t\tpublic fun FlatBufferBuilder." + LowerFirst(struct_def.name) +"Of(action:FlatBufferBuilder.()->Unit) :Int {\n\t\t\tstartObject(" + NumToString(struct_def.fields.vec.size()) + ")\n\t\t\t\taction()\n";  
    code += "\t\t\tval o = endObject()\n";
  
     for (auto it = struct_def.fields.vec.begin();
         it != struct_def.fields.vec.end();
         ++it) {
      auto &field = **it;
      if (!field.deprecated && field.required) {
        code += "\t\t\trequired(o, ";
        code += NumToString(field.value.offset);
        code += ")  // " + field.name + "\n";
      }
    }

code += "\t\t\treturn o\n";
  code += "\t\t}";
  
  
  			    }

// Set the value of a table's field.
static void buildFieldOfTable(const StructDef &struct_def,
                              const FieldDef &field,
                              const size_t offset,
                              std::string *code_ptr) {
  std::string &code = *code_ptr;

/*   code += "\t\tpublic fun add" + sanitize(field.name, true);
  code += "(builder : FlatBufferBuilder, ";*/
  
  code += "\t\tpublic fun FlatBufferBuilder." + sanitize(field.name, false) +"(";
  code += sanitize(field.name, false);
  if (!IsScalar(field.value.type.base_type) && (!struct_def.fixed)) {
    code += "Offset : Int ";
  } else {
    code += " : " + GenTypeForUserConstructor(field.value.type);
  }
   code += ") : FlatBufferBuilder = apply { add";
  //code += ") { builder.add";
  code +=  GenMethod(kotlinLang, field.value.type) + "(" + NumToString(offset) + ", " ;
  if (!IsScalar(field.value.type.base_type) && (!struct_def.fixed)) {
  	  code += sanitize(field.name, false) + "Offset"; // string, union, struct and table
  } else {
    code +=  downsizeToStorageValue(field.value.type, sanitize(field.name, false), false);
  }
   code += ", " + field.value.constant; /** Int */ 
   if (field.value.type.base_type == BASE_TYPE_BOOL) code += "!=0"; // hack to work with booleans... bad :(
   code += ") }\n";
}

// Set the value of one of the members of a table's vector.
static void buildArrayOfTable( const FieldDef &field,
                               std::string *code_ptr) {
  std::string &code = *code_ptr;

  auto vector_type = field.value.type.VectorType();
  auto alignment = InlineAlignment(vector_type);
  auto elem_size = InlineSize(vector_type);

/*  code += "\t\tfun FlatBufferBuilder.start" + sanitize(field.name, true);
  code += "Array(numElems : Int) : FlatBufferBuilder = startArray(";
  code += NumToString(elem_size);
  code += ", numElems, " + NumToString(alignment);
  code += ")\n";*/
  
  code += "\t\tinline public fun FlatBufferBuilder." + LowerFirst(sanitize(field.name, false));
  code += "Of(numElems : Int, action : FlatBufferBuilder.()->Unit) : Int {startArray(";
  code += NumToString(elem_size);
  code += ", numElems, " + NumToString(alignment);
  code += "); action(); return endArray()}\n";
  
  
}

static void createArrayOfStruct( const FieldDef &field, std::string *code_ptr) {
	  std::string &code = *code_ptr;
 //  code += "\t\tfun FlatBufferBuilder.create" + sanitize(field.name, true);
 // code += "Array(offsets : IntArray) ";
  
     code += "\t\tpublic fun FlatBufferBuilder." + LowerFirst(sanitize(field.name, true));
  code += "Of(vararg offsets : Int) ";
  
  code += " : Int {startArray(4, offsets.size, ";
  if (field.value.type.struct_def != nullptr) code += NumToString(field.value.type.struct_def->minalign); else code += "4"; // look into this
  code += "); for (i in offsets.size - 1 downTo 0) addOffset(offsets[i]); return endArray(); }\n";
}

static void createArrayOfNonStruct( const FieldDef &field, std::string *code_ptr) {
	  std::string &code = *code_ptr;
	          auto vector_type = field.value.type.VectorType();
        auto alignment = InlineAlignment(vector_type);
        auto elem_size = InlineSize(vector_type);
   code += "\t\tfun FlatBufferBuilder." + LowerFirst(sanitize(field.name, false));
  code += "Of(vararg data : " + GenTypeForUserConstructor(vector_type) + ")", //+ "Array) "; // must widen types + give enums
  code += " : Int {startArray(";
  code +=  NumToString(elem_size) + ", data.size, ";
  code += NumToString(alignment) +"); for (i in data.size - 1 downTo 0) add" +   GenMethod(kotlinLang, vector_type)+ "(" + downsizeToStorageValue(vector_type, "data[i]", false) + "); return endArray(); }\n";
}

// Get the offset of the end of a table.
static void GetEndOffsetOnTable(const StructDef &struct_def,
                                std::string *code_ptr, const Parser &parser) {
  std::string &code = *code_ptr;
  //code += "\t\tfun end" + struct_def.name + "(builder :FlatBufferBuilder) :Int {\n";
  /*
   code += "\t\tfun FlatBufferBuilder.end() :Int {\n";
  
  code += "\t\t\tval o = endObject()\n";
  
     for (auto it = struct_def.fields.vec.begin();
         it != struct_def.fields.vec.end();
         ++it) {
      auto &field = **it;
      if (!field.deprecated && field.required) {
        code += "\t\t\trequired(o, ";
        code += NumToString(field.value.offset);
        code += ");  // " + field.name + "\n";
      }
    }

code += "\t\t\treturn o\n";
  code += "\t\t}";
 */
 //  code += "\t\tfun finish" + /*MakeCamel(struct_def.name, true)*/ + "Buffer(builder : FlatBufferBuilder, offset : Int) { builder.finish(offset";  
 
  code += "\t\tfun  FlatBufferBuilder.finishBuffer(offset : Int) { finish(offset";  
   if (parser.root_struct_def_ == &struct_def && parser.file_identifier_.length()) code += ", \"" + parser.file_identifier_ + "\"";
      code += ") }\n";
  
}

// Generate a struct field, conditioned on its child type(s).
static void generateStructAccessor(const StructDef &struct_def, const FieldDef &field,std::string *code_ptr) {
  GenComment(field.doc_comment, code_ptr, nullptr, "");
  if (IsScalar(field.value.type.base_type)) {
    if (struct_def.fixed) getScalarFieldOfStruct(field, code_ptr); else getScalarFieldOfTable(field, code_ptr);
  } else {
    switch (field.value.type.base_type) {
      case BASE_TYPE_STRUCT:
        if (struct_def.fixed) getStructFieldOfStruct(field, code_ptr); else  getStructFieldOfTable(field, code_ptr);
        break;
      case BASE_TYPE_STRING:
        getStringField(field, code_ptr);
        break;
      case BASE_TYPE_VECTOR: {
        auto vectortype = field.value.type.VectorType();
        if (vectortype.base_type == BASE_TYPE_STRUCT) GetMemberOfVectorOfStruct(field, code_ptr); else  GetMemberOfVectorOfNonStruct(field, code_ptr);
        break;
      }
      case BASE_TYPE_UNION:
        getUnion(field, code_ptr);
        break;
      default:
        assert(0);
    }
  }
// this is for faster access to the underlying bytes
 /* if (field.value.type.base_type == BASE_TYPE_VECTOR) {
    getArraySize(field, code_ptr);
    if (field.value.type.element == BASE_TYPE_UCHAR) GetUByteSlice(struct_def, field, code_ptr);
  }*/
}

// Generate table constructors, conditioned on its members' types.
static void GenTableBuilders(const StructDef &struct_def,
                             std::string *code_ptr, const Parser &parser) {
  GetStartOfTable(struct_def, code_ptr);

  for (auto it = struct_def.fields.vec.begin();it != struct_def.fields.vec.end();++it) {
    auto &field = **it;
    if (field.deprecated) continue;

    auto offset = it - struct_def.fields.vec.begin();
    buildFieldOfTable(struct_def, field, offset, code_ptr);
    if (field.value.type.base_type == BASE_TYPE_VECTOR) {
    	    buildArrayOfTable(field, code_ptr);
    	    if (field.value.type.element == BASE_TYPE_STRUCT || field.value.type.element == BASE_TYPE_STRING) createArrayOfStruct(field, code_ptr); else createArrayOfNonStruct(field, code_ptr); 
  }
}
  GetEndOffsetOnTable(struct_def, code_ptr, parser);
}

// Generate struct or table methods.
static void GenStruct(const StructDef &struct_def, std::string *code_ptr, StructDef * /*root_struct_def*/, const Parser &parser) {
  if (struct_def.generated) return;


  GenComment(struct_def.doc_comment, code_ptr, nullptr);
  beginClassDeclaration(struct_def, code_ptr);

  // Generate the Init method that sets the field in a pre-existing
  // accessor object. This is to allow object reuse.
  initializeTableAndStruct(struct_def, code_ptr);

  for (auto it = struct_def.fields.vec.begin();it != struct_def.fields.vec.end();++it) {
    auto &field = **it;
    if (field.deprecated) continue;
    generateStructAccessor(struct_def, field, code_ptr);
    if ((field.value.type.base_type == BASE_TYPE_VECTOR &&
          IsScalar(field.value.type.VectorType().base_type)) ||
         field.value.type.base_type == BASE_TYPE_STRING) fieldAsByteBuffer(field, code_ptr);
  }


  beginCompanionObject(code_ptr);
   if (parser.root_struct_def_ == &struct_def) {
      if (parser.file_identifier_.length()) {
        // Check if a buffer has the identifier.
        std::string &code = *code_ptr;
        code += "\tpublic fun ";// + struct_def.name; 
        code += "hasIdentifier(byteBuffer : ByteBuffer) :Boolean = Table.hasIdentifier(byteBuffer, \"" + parser.file_identifier_ + "\")\n";
      }
    }
    
  /*if (&struct_def == root_struct_def) {
    // Generate a special accessor for the table that has been declared as
    // the root type.
    NewRootTypeFromBuffer(struct_def, code_ptr);
  }*/
  if (struct_def.fixed) {
    // create a struct constructor function
    //GenStructBuilder(struct_def, code_ptr);
  } else {
    // Create a set of functions that allow table construction.
    GenTableBuilders(struct_def, code_ptr, parser);
    //GenStructBuilder(struct_def, code_ptr); // added
  }
  endCompanionObject(code_ptr);
  endClassDeclaration(code_ptr);
  
  GenStructBuilder(struct_def, code_ptr);
}

// Generate enum declarations.
static void generateEnum(const EnumDef &enum_def, std::string *code_ptr) {
  if (enum_def.generated) return;

  auto &code = * code_ptr;
  GenComment(enum_def.doc_comment, code_ptr, nullptr);
  beginEnumDeclaration(enum_def, code_ptr);
  for (auto it = enum_def.vals.vec.begin(); it != enum_def.vals.vec.end();++it) {
    auto &ev = **it;
    GenComment(ev.doc_comment, code_ptr, nullptr, "\t");
    if (it != enum_def.vals.vec.begin()) code += ",\n";
    enumMember( ev, code_ptr);
  }
  code += ";\n";
  endEnumDeclaration(enum_def,code_ptr);
}

// Save out the generated code for a Kotlin Table type.
static bool SaveType(const Parser &parser, const Definition &def,
                     const std::string &classcode, const std::string &path,
                     bool needs_imports) {
  if (!classcode.length()) return true;

  std::string namespace_name;
  std::string namespace_dir = path;  // Either empty or ends in separator.
  auto &namespaces = parser.namespaces_.back()->components;
  for (auto it = namespaces.begin(); it != namespaces.end(); ++it) {
    if (namespace_name.length()) {
      namespace_name += ".";
    }
    namespace_name = *it;
    namespace_dir += *it + kPathSeparator;
  }
  EnsureDirExists(namespace_dir);

  std::string code = "";
  BeginFile(namespace_name, needs_imports, &code);
  code += classcode;
  std::string filename = namespace_dir + def.name + ".kt";
  return SaveFile(filename.c_str(), code, false);
}


// takes a scalar value with a user facing type and downsizes it into the corresponding bits with a storage type for constructor methods (offset for tables, ...)
static std::string downsizeToStorageValueForConstructor(const Type &type, const std::string value) {
  if (type.enum_def != nullptr){
	if (type.base_type == BASE_TYPE_UNION) return value; // union_type type 
               else return value + ".value"; // enum class name
  }
  switch (type.base_type) {

  	case BASE_TYPE_UINT:  return value + ".toInt()";
  	case BASE_TYPE_USHORT:  return value + ".toShort()";
  	case BASE_TYPE_UCHAR:  return value + ".toByte()";
  		//case BASE_TYPE_BOOL:  return value;
  		//case BASE_TYPE_VECTOR:  return value;
  	default: return value;
  }
}

// takes a scalar value with a user facing type and downsizes it into the corresponding bits with a storage type
static std::string downsizeToStorageValue(const Type &type, const std::string value, const bool downsizeBool) {
    if (type.enum_def != nullptr) return value + ".value";
    switch (type.base_type) {
  	case BASE_TYPE_BOOL:  return downsizeBool ? "if (" + value + ") 1.toByte() else 0.toByte()" : value;
  	case BASE_TYPE_UINT:  return value + ".and(0xFFFFFFFFL).toInt()";
  	case BASE_TYPE_USHORT:  return value + ".and(0xFFFF).toShort()";
  	case BASE_TYPE_UCHAR:  return value + ".and(0xFF).toByte()";
  	case BASE_TYPE_VECTOR:  return downsizeToStorageValue(type.VectorType(), value, downsizeBool);
  	default: return value;
    }
}

static std::string upsizeToUserType(const Type &type, const std::string value) {
   switch (type.base_type) {
       case BASE_TYPE_UINT:  return value + ".toLong().and(0xFFFFFFFFL)";
       case BASE_TYPE_USHORT:  return value + ".toInt().and(0xFFFF)";
       case BASE_TYPE_UCHAR:  return value + ".toInt().and(0xFF)";
       case BASE_TYPE_VECTOR:  return upsizeToUserType(type.VectorType(), value);
       default:  if (type.enum_def != nullptr && type.base_type != BASE_TYPE_UNION) return type.enum_def->name + ".from(" + value + ")"; else return value;
   }
}

static std::string defaultToUserType(const Type &type, const std::string value) {
   switch (type.base_type) {
       case BASE_TYPE_UINT:  return value + ".toLong().and(0xFFFFFFFFL)";
       case BASE_TYPE_USHORT:  return value + ".toInt().and(0xFFFF)";
       case BASE_TYPE_UCHAR:  return value + ".toInt().and(0xFF)";
       case BASE_TYPE_VECTOR:  return upsizeToUserType(type.VectorType(), value);
       default:  if (type.enum_def != nullptr && type.base_type != BASE_TYPE_UNION) return type.enum_def->name + ".from(" + value + ")"; else return value;
   }
}

static std::string sanitize(const std::string name, const bool isFirstLetterUpper) {
	std::string camelName = MakeCamel(name, isFirstLetterUpper);
	if (camelName.size() >= 1 && camelName.compare(camelName.size() - 1, 1, "_") == 0) return camelName + "_";
	if (camelName == "val") return "val_";
	if (camelName == "var") return "var_";
	if (camelName == "fun") return "fun_";
	return camelName;
}

static std::string multiplyBySizeOf(const Type &type) {
              int a = InlineSize(type);
              if (a == 1) return ""; else return " * " + NumToString(a);
}

// Create a struct with a builder and the struct's arguments.
static void GenStructBuilder(const StructDef &struct_def, std::string *code_ptr) {
  BeginBuilderArgs(struct_def, code_ptr);
  StructBuilderArgs(struct_def, "", code_ptr, true);
  EndBuilderArgs(code_ptr);

  StructBuilderBody(struct_def, "", code_ptr);


  EndBuilderBody(code_ptr);
}


// functions copied from idl_gen_general.cpp because they are static (can't be shared)

static std::string GenTypeBasic(const LanguageParameters &lang,
                                const Type &type) {
  static const char *gtypename[] = {
    #define FLATBUFFERS_TD(ENUM, IDLTYPE, CTYPE, JTYPE, KTYPE, GTYPE, NTYPE, PTYPE) \
        #JTYPE, #KTYPE, #NTYPE, #GTYPE,
      FLATBUFFERS_GEN_TYPES(FLATBUFFERS_TD)
    #undef FLATBUFFERS_TD
  };

  /*if(lang.language == GeneratorOptions::kCSharp && type.base_type == BASE_TYPE_STRUCT) {
    return "Offset<" + type.struct_def->name + ">";
  }*/

  return gtypename[type.base_type * GeneratorOptions::kMAX + lang.language];
}

// Generate type to be used in user-facing API
// removed static to allow reuse in the kotlin external generator
static std::string GenTypeForUser(const LanguageParameters &lang,
                                  const Type &type) {
  if (lang.language == GeneratorOptions::kCSharp) {
    if (type.enum_def != nullptr &&
          type.base_type != BASE_TYPE_UNION) return type.enum_def->name;
  }
  if (lang.language == GeneratorOptions::kKotlin) {
    if (type.enum_def != nullptr && type.base_type != BASE_TYPE_UNION) return type.enum_def->name;
  }
  return GenTypeBasic(lang, type);
}

// removed static for reuse in external code generators
static std::string GenTypeGet(const LanguageParameters &lang,
                              const Type &type);

static std::string GenTypePointer(const LanguageParameters &lang,
                                  const Type &type) {
  switch (type.base_type) {
    case BASE_TYPE_STRING:
      return lang.string_type;
    case BASE_TYPE_VECTOR:
      return GenTypeGet(lang, type.VectorType());
    case BASE_TYPE_STRUCT:
      return type.struct_def->name;
    case BASE_TYPE_UNION:
      // fall through
    default:
      return "Table";
  }
}

// removed static for reuse in external code generators
static std::string GenTypeGet(const LanguageParameters &lang,
                              const Type &type) {
  return IsScalar(type.base_type)
    ? GenTypeBasic(lang, type)
    : GenTypePointer(lang, type);
}

// Find the destination type the user wants to receive the value in (e.g.
// one size higher signed types for unsigned serialized values in Java).
// removed static to allow reuse in external generator files
static Type DestinationType(const LanguageParameters &lang, const Type &type,
                            bool vectorelem) {
  //if (lang.language != GeneratorOptions::kJava && lang.language != GeneratorOptions::kKotlin) return type;
  switch (type.base_type) {
  	  // We use int for both uchar/ushort, since that generally means less casting
    // than using short for uchar.
    case BASE_TYPE_UCHAR:  return Type(BASE_TYPE_INT);
    case BASE_TYPE_USHORT: return Type(BASE_TYPE_INT);
    case BASE_TYPE_UINT:   return Type(BASE_TYPE_LONG);
    case BASE_TYPE_VECTOR:
      if (vectorelem)
        return DestinationType(lang, type.VectorType(), vectorelem);
      // else fall thru:
    default: return type;
  }
}


// Generate destination type name
// removed static for reuse in kotlin code generator
std::string GenTypeNameDest(const LanguageParameters &lang, const Type &type)
{
  /*if (lang.language == GeneratorOptions::kCSharp) {
    // C# enums are represented by themselves
    if (type.enum_def != nullptr && type.base_type != BASE_TYPE_UNION)
      return type.enum_def->name;

    // Unions in C# use a generic Table-derived type for better type safety
    if (type.base_type == BASE_TYPE_UNION)
      return "TTable";
  }*/
  if (lang.language == GeneratorOptions::kKotlin) {
    // Kotlin enums are represented by themselves
    if (type.enum_def != nullptr && type.base_type != BASE_TYPE_UNION) return type.enum_def->name;

    // Unions in Kotlin use a generic Table-derived type 
    if (type.base_type == BASE_TYPE_UNION) return "Table";
  }
  
  // default behavior
  return GenTypeGet(lang, DestinationType(lang, type, true));
}

// removed static to allow reuse
std::string GenDefaultValue(const LanguageParameters &lang, const Value &value, bool for_buffer) {
  /*if (lang.language == GeneratorOptions::kCSharp && !for_buffer) {
    switch(value.type.base_type) {
      case BASE_TYPE_STRING:
        return "default(StringOffset)";
      case BASE_TYPE_STRUCT:
        return "default(Offset<" + value.type.struct_def->name + ">)";
      case BASE_TYPE_VECTOR:
        return "default(VectorOffset)";
      default:
        break;
    }
  }*/

  if (lang.language == GeneratorOptions::kKotlin && !for_buffer) {  
      switch (value.type.base_type) {
      	case BASE_TYPE_CHAR:return value.constant + ".toByte()";
      	case BASE_TYPE_SHORT:return value.constant + ".toShort()";
      	case BASE_TYPE_LONG:
      	case BASE_TYPE_ULONG:return value.constant + "L";
      	case BASE_TYPE_FLOAT:return value.constant + "f";
      	case BASE_TYPE_VECTOR: switch (value.type.element) {
      		case BASE_TYPE_CHAR:return value.constant + ".toByte()";
      		case BASE_TYPE_SHORT:return value.constant + ".toShort()";
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


// Returns the method name for use with add/put calls.
static std::string GenMethod(const LanguageParameters &lang, const Type &type) {
  return IsScalar(type.base_type)
    ? MakeCamel(GenTypeBasic(lang, type))
    : (IsStruct(type) ? "Struct" : "Offset");
}


// Returns the function name that is able to read a value of the given type.
static std::string GenGetterKotlin(const LanguageParameters &lang,
                             const Type &type) {
  switch (type.base_type) {
    case BASE_TYPE_STRING: return "__string";
    case BASE_TYPE_STRUCT: return "__struct";
    case BASE_TYPE_UNION:  return "__union";
    case BASE_TYPE_VECTOR: return GenGetterKotlin(lang, type.VectorType());
    default: {
      std::string getter = "bb.get";
      if (type.base_type == BASE_TYPE_BOOL) {
        getter = "0.toByte()!=" + getter; // Kotlin specific
      } else if (GenTypeBasic(lang, type) != "Byte") { // Kotlin specific
        getter += MakeCamel(GenTypeGet(lang, type));
      }
      return getter;
    }
  }
}

static std::string GenSetterKotlin(const LanguageParameters &lang, const Type &type) {
  if (IsScalar(type.base_type)) {
    std::string setter = "bb.put";
    if (GenTypeBasic(lang, type) != "Byte"  /* Kotlin specific*/&& type.base_type != BASE_TYPE_BOOL) setter += MakeCamel(GenTypeGet(lang, type));
    return setter;
  }
  return "";
}





}  // namespace kotlin

bool GenerateKotlin(const Parser &parser,
                const std::string &path,
                const std::string & /*file_name*/,
                const GeneratorOptions & /*opts*/) {
  for (auto it = parser.enums_.vec.begin();it != parser.enums_.vec.end(); ++it) {
    std::string enumcode;
    kotlin::generateEnum(**it, &enumcode);
    if (!kotlin::SaveType(parser, **it, enumcode, path, false)) return false;
  }

  for (auto it = parser.structs_.vec.begin(); it != parser.structs_.vec.end(); ++it) {
    std::string declcode;
    kotlin::GenStruct(**it, &declcode, parser.root_struct_def_, parser);
    if (!kotlin::SaveType(parser, **it, declcode, path, true)) return false;
  }

  return true;
}

}  // namespace flatbuffers

