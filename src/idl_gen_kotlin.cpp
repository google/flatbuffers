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
	auto kotlinLang = language_parameters[GeneratorOptions::kKotlin];	
	
	std::string GenTypeForUser(const LanguageParameters &lang, const Type &type);
	
	// Find the destination type the user wants to receive the value in (e.g.
	// one size higher signed types for unsigned serialized values in Java).
	Type DestinationType(const LanguageParameters &lang, const Type &type, bool vectorelem);

	// formats the default value in case the field's offset is 0
	std::string GenDefaultValue(const LanguageParameters &lang, const Value &value, bool for_buffer);
	
	// type of the value from getter
	std::string GenTypeGet(const LanguageParameters &lang, const Type &type);

	std::string GenTypeBasic(const LanguageParameters &lang, const Type &type);
	
	// type of the value for users
	std::string GenTypeNameDest(const LanguageParameters &lang, const Type &type);
	
	// for add/put calls
	std::string GenMethod(const LanguageParameters &lang, const Type &type);
	
	//std::string GenSetter(const LanguageParameters &lang,const Type &type) // it uses bb	
namespace kotlin {
	static std::string GenSetter(const LanguageParameters &lang, const Type &type);
	
		
	/* for use in createSruct() args */
	static std::string GenTypeForUserConstructor(const Type &type) {
		return GenTypeForUser(kotlinLang, flatbuffers::DestinationType(kotlinLang, type, false));
	}
	
	static void createArrayOfStruct( const FieldDef &field, std::string *code_ptr);
	static void createArrayOfNonStruct( const FieldDef &field, std::string *code_ptr);
	static void GenStructBuilder(const StructDef &struct_def, std::string *code_ptr);


	// to clean up :
	//static std::string downsizeToStorageValue(const Type &type, const std::string value);
static std::string downsizeToStorageValueForConstructor(const Type &type, const std::string value);
static std::string sanitize(const std::string name, const bool isFirstLetterUpper);
static std::string beforeStorageType(const Type &type);
static std::string beginGet(const Type &type);
static std::string afterStorageType(const Type &type, const bool isLitteral);
static std::string upsizeToUserType(const Type &type, std::string value);
static std::string downsizeToStorageValue(const Type &type, const std::string value, const bool downsizeBool);
static std::string multiplyBySizeOf(const Type &type);

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
  code += "public enum class " + enum_def.name + "(val value: " + flatbuffers::GenTypeGet(kotlinLang, enum_def.underlying_type) + ") {\n";

}

// A single enum member.
static void enumMember(const EnumVal ev,
                       std::string *code_ptr) {
  std::string &code = *code_ptr;
  code += "\t" +  ev.name + "(" + NumToString(ev.value) + ")";
}

static void endEnumDeclaration(const EnumDef &enum_def,std::string *code_ptr) {
  std::string &code = *code_ptr;

  code += "\t companion object {\n\tfun from( value : Int) : " + enum_def.name  + " = when (value) {\n";
  for (auto it = enum_def.vals.vec.begin(); it != enum_def.vals.vec.end();++it) {
        auto &ev = **it;
	code += "\t" + NumToString(ev.value) + " -> " + ev.name + "\n";
  }
   code += "\telse -> throw Exception(\"Bad enum value : $value\")\n";
  code += "}\n}\n";
  code += "}\n";
}

// Initialize a new struct or table from existing data.
static void NewRootTypeFromBuffer(const StructDef &struct_def, std::string *code_ptr) {
  std::string &code = *code_ptr;

  code += "public fun rootAs" + struct_def.name + "(byteBuffer : ByteBuffer, reuse : " + struct_def.name + "? = null) : ";
  code += struct_def.name + " {byteBuffer.order(ByteOrder.LITTLE_ENDIAN); ";
  code += "return (reuse ?: " + struct_def.name + "()).wrap(byteBuffer)}\n";
}

// Initialize an existing object with other data, to avoid an allocation.
static void initializeTableAndStruct(const StructDef &struct_def, std::string *code_ptr) {
  std::string &code = *code_ptr;

  code += "public fun wrap(byteBuffer : ByteBuffer, position : Int = byteBuffer.getInt(byteBuffer.position()) + byteBuffer.position()) : " + struct_def.name + " = apply {";
  code +=  "_position = position; ";
  code +=  "_byteBuffer = byteBuffer}\n";
}


/* class */
static void beginClassDeclaration(const StructDef &struct_def, std::string *code_ptr) {
  std::string &code = *code_ptr;
  code += "public open class " + struct_def.name + " : ";
  if (struct_def.fixed) code += "Struct"; else code+= "Table"; 
  code += "() {\n\t";
}

static void beginCompanionObject(std::string *code_ptr) {
  std::string &code = *code_ptr;
  code += "\tcompanion object {\n\t\t";
}

static void endCompanionObject(std::string *code_ptr) {
  std::string &code = *code_ptr;
  code += "\t}\n";
}


static void endClassDeclaration(std::string *code_ptr) {
  std::string &code = *code_ptr;
  code += "\n}";
}

static void getArraySize(const FieldDef &field, std::string *code_ptr) {
  std::string &code = *code_ptr;
  code += "public val " + sanitize(field.name, false) + "Size : Int ";
  code += "get() {val o = _offset(" + NumToString(field.value.offset) + "); ";
  code += "return if (o == 0) 0 else _arraySize(o)}\n";
}

static void getScalarFieldOfStruct(const FieldDef &field, std::string *code_ptr) {
  std::string &code = *code_ptr;
  std::string getter = beginGet(field.value.type);
  std::string setter = GenSetter(kotlinLang, field.value.type);

  if (field.value.type.base_type == BASE_TYPE_STRING) code += "public val "; else  code += "public var ";
  code+= sanitize(field.name, false) + " : " + GenTypeNameDest(kotlinLang, field.value.type);
  code += " get() = " + beforeStorageType(field.value.type) +  getter + "(_position + " + NumToString(field.value.offset) + ")" + afterStorageType(field.value.type, false);

  if (field.value.type.base_type != BASE_TYPE_STRING) {
	code += "; set(value) { " + setter + "(_position + " + NumToString(field.value.offset) + ", ";
	code += downsizeToStorageValue(field.value.type, "value", true) +") }";
  }  
 code += "\n";
}

static void getScalarFieldOfTable(const FieldDef &field, std::string *code_ptr) {
  std::string &code = *code_ptr;
  std::string getter = beginGet(field.value.type);

  code += "public val " + sanitize(field.name, false) + " : " + GenTypeNameDest(kotlinLang, field.value.type);
  code += " get() {val o = _offset(" +NumToString(field.value.offset) + "); ";
  code += "return if (o == 0) ";
  if (field.value.type.base_type == BASE_TYPE_STRING) code += "null"; else code += beforeStorageType(field.value.type) + GenDefaultValue(kotlinLang, field.value, false) + afterStorageType(field.value.type, true);
  code += " else " + beforeStorageType(field.value.type)+ getter + "(o + _position)" + afterStorageType(field.value.type, false) + "}\n"; 

    code += "public fun mutate" + sanitize(field.name, true) + "(value : " + GenTypeNameDest(kotlinLang, field.value.type) + ") :Boolean {";
    code += "val o = _offset(" + NumToString(field.value.offset) + "); ";
    code += "return if (o==0) false else {";
    code += GenSetter(kotlinLang, field.value.type) + "(o + _position, " + downsizeToStorageValue(field.value.type, "value", true); /* downsizeToStorageValue(field.value.type, "value")*/ 
    code += "); true}}\n";
}

// Get a struct by initializing an existing struct.
// Specific to Struct.
static void getStructFieldOfStruct(const FieldDef &field, std::string *code_ptr) {
  std::string &code = *code_ptr;
//DONE
  code += "public fun " + sanitize(field.name, false) + "(reuse : " + GenTypeNameDest(kotlinLang, field.value.type) + "? = null) : " + GenTypeNameDest(kotlinLang, field.value.type) + " = ";
  code += "(reuse?: " + GenTypeNameDest(kotlinLang, field.value.type) + "()).wrap(_byteBuffer, _position + " + NumToString(field.value.offset) + ")\n";
}

// Get a struct by initializing an existing struct.
// Specific to Table.
static void getStructFieldOfTable(const FieldDef &field, std::string *code_ptr) {
  std::string &code = *code_ptr;
  code += "public fun " + sanitize(field.name, false) + "(reuse : " +GenTypeNameDest(kotlinLang, field.value.type) + "? = null) : " + GenTypeNameDest(kotlinLang, field.value.type)+ "? {";
  code += "val o = _offset(" +NumToString(field.value.offset) + "); ";
  code += "return if (o==0) null else (reuse ?:" + GenTypeNameDest(kotlinLang, field.value.type) + "()).wrap(_byteBuffer, ";
  if (field.value.type.struct_def->fixed) code += "o + _position)"; else code += "_indirect(o + _position))"; 
  code += "}\n";
}

// Get the value of a string.
static void getStringField(const FieldDef &field, std::string *code_ptr) {
  std::string &code = *code_ptr;

  code += "public val " +sanitize(field.name, false) + " : String? get() {";
  code += "val o = _offset(" +NumToString(field.value.offset) + "); ";
  code += "return if (o == 0) null else " + beginGet(field.value.type) + "(o + _position)";
  code += "}\n";
}

// it's better to return null instead of false and an object instead of true ?
// Get the value of a union from an object.
static void getUnion(const FieldDef &field, std::string *code_ptr) {
  std::string &code = *code_ptr;

  code += "public fun " + sanitize(field.name, false) + "(reuse : " + GenTypeNameDest(kotlinLang, field.value.type) + ") : " + GenTypeNameDest(kotlinLang, field.value.type) + "? {";
  code += "val o = _offset(" +NumToString(field.value.offset) + "); ";
  code += "return if (o==0) null else _union(reuse, o)}\n";
}

// Get the value of a vector's struct member.
static void GetMemberOfVectorOfStruct(
                                      const FieldDef &field,
                                      std::string *code_ptr) {
  std::string &code = *code_ptr;
  auto vectortype = field.value.type.VectorType();

  code += "public fun " + sanitize(field.name, false) + "(j :Int, reuse : " + GenTypeNameDest(kotlinLang, field.value.type) + "? = null) : "+ GenTypeNameDest(kotlinLang, field.value.type) + "? {";
  code += "val o = _offset(" +NumToString(field.value.offset) + "); ";
  code += "return if (o==0) null else {";
  code += "val x = _array(o) + j" + multiplyBySizeOf(vectortype) + "; ";
  code += "(reuse ?: " + GenTypeNameDest(kotlinLang, field.value.type) +"() ).wrap(_byteBuffer, x";
  if (!vectortype.struct_def->fixed) code += " + _indirect(x)";
  code += ")}}\n";

getArraySize(field, code_ptr);
}

// Get the value of a vector's non-struct member. Uses a named return
// argument to conveniently set the zero value for the result.
static void GetMemberOfVectorOfNonStruct(
                                         const FieldDef &field,
                                         std::string *code_ptr) {
  std::string &code = *code_ptr;
  auto vector_type = field.value.type.VectorType();

  code += "public fun " + sanitize(field.name, false) + "(j : Int) : " + GenTypeNameDest(kotlinLang, field.value.type);
  if (vector_type.base_type == BASE_TYPE_STRING) code += "?";
  code += " {val o = _offset(" + NumToString(field.value.offset) + "); ";
  code += "return if (o == 0) ";
  if (vector_type.base_type != BASE_TYPE_STRING) code += GenDefaultValue(kotlinLang, field.value, false) + /** fix here */ afterStorageType(vector_type, true); else code += "null"; 
  code += " else " + upsizeToUserType(field.value.type,  beginGet(field.value.type) + "(_array(o) + j" + multiplyBySizeOf(vector_type) + ")");
//	if (vector_type.base_type != BASE_TYPE_STRING) code += upsizeToUserType(field.value.type); 
code += "}\n";

  if (vector_type.base_type != BASE_TYPE_STRING) {
    code += "public fun mutate" + sanitize(field.name, true) + "(j : Int, value : " + GenTypeNameDest(kotlinLang, vector_type) + ") :Boolean {";
    code += "val o = _offset(" + NumToString(field.value.offset) + "); ";
    code += "return if (o == 0) false else {";
    code += GenSetter(kotlinLang, vector_type) + "(_array(o) + j" + multiplyBySizeOf(vector_type);
    code += ", " + downsizeToStorageValue(vector_type, "value", true)   /*downsizeToStorageValue(field.value.type, "value")*/ + "); true}}\n";
  }

getArraySize(field, code_ptr);
}


static void fieldAsByteBuffer(const FieldDef &field,
                             std::string *code_ptr) {
                             std::string &code = *code_ptr;
                                      
      code += "\tpublic val " + sanitize(field.name, false);
      code += "AsByteBuffer :ByteBuffer get() = _vector_as_bytebuffer(";
      code += NumToString(field.value.offset) + ", ";
      code += NumToString(field.value.type.base_type == BASE_TYPE_STRING ? 1 :
                          InlineSize(field.value.type.VectorType()));
      code += ")\n";
                             }

// Begin the creator function signature.
static void BeginBuilderArgs(const StructDef &struct_def,
                             std::string *code_ptr) {
  std::string &code = *code_ptr;
  code += "\t\tfun create" + struct_def.name;
  code += "(builder : FlatBufferBuilder";
}

// Recursively generate arguments for a constructor, to deal with nested
// structs.
static void StructBuilderArgs(const StructDef &struct_def,
                              const char *nameprefix,
                              std::string *code_ptr) {
  for (auto it = struct_def.fields.vec.begin();it != struct_def.fields.vec.end();++it) {
    auto &field = **it;
    if (IsStruct(field.value.type)) {
      // Generate arguments for a struct inside a struct. To ensure names
      // don't clash, and to make it obvious these arguments are constructing
      // a nested struct, prefix the name with the field name.
      StructBuilderArgs(*field.value.type.struct_def, (nameprefix + (sanitize(field.name, false) + "_")).c_str(), code_ptr);
    } else {
      std::string &code = *code_ptr;
      code += ", ";
      code +=  nameprefix +sanitize(field.name, false) + " : " +  GenTypeForUserConstructor(field.value.type); // offset for structs/tables/string
    }
  }
}

// End the creator function signature.
static void EndBuilderArgs(std::string *code_ptr) {
  std::string &code = *code_ptr;
  code += ") :Int {\n";
}

// Recursively generate struct construction statements and instert manual
// padding.
static void StructBuilderBody(const StructDef &struct_def,
                              const char *nameprefix,
                              std::string *code_ptr) {
  std::string &code = *code_ptr;
  code += "\t\tbuilder.prep(" + NumToString(struct_def.minalign) + ", " + NumToString(struct_def.bytesize) + ")\n";
  for (auto it = struct_def.fields.vec.rbegin(); it != struct_def.fields.vec.rend(); ++it) {
    auto &field = **it;
    if (field.padding) code += "\t\tbuilder.pad(" + NumToString(field.padding) + ")\n";
    if (IsStruct(field.value.type)) {
      StructBuilderBody(*field.value.type.struct_def, (nameprefix + (sanitize(field.name, false) + "_")).c_str(), code_ptr);
    } else {
      code += "\t\tbuilder.add" + GenMethod(kotlinLang, field.value.type) + "(";
      code +=  downsizeToStorageValueForConstructor(field.value.type, nameprefix + sanitize(field.name, false)) + ")\n";
    }
  }
}

static void EndBuilderBody(std::string *code_ptr) {
  std::string &code = *code_ptr;
  code += "\t\treturn builder.offset()\n";
  code += "\t}\n";
}

// Get the value of a table's starting offset.
static void GetStartOfTable(const StructDef &struct_def,
                            std::string *code_ptr) {
  std::string &code = *code_ptr;
  code += "\t\tpublic fun start" + struct_def.name;
  code += "(builder:FlatBufferBuilder) :Unit = ";
  code += "builder.startObject(";
  code += NumToString(struct_def.fields.vec.size());
  code += ")\n";
}

// Set the value of a table's field.
static void buildFieldOfTable(const StructDef &struct_def,
                              const FieldDef &field,
                              const size_t offset,
                              std::string *code_ptr) {
  std::string &code = *code_ptr;

  code += "\t\tpublic fun add" + sanitize(field.name, true);
  code += "(builder : FlatBufferBuilder, ";
  code += sanitize(field.name, false);
  if (!IsScalar(field.value.type.base_type) && (!struct_def.fixed)) {
    code += "Offset : Int ";
  } else {
    code += " : " + GenTypeForUser(kotlinLang, DestinationType(kotlinLang, field.value.type, false))/*GenTypeForUser(kotlinLang, field.value.type)*/;
  }
  code += ") { builder.add";
  code +=  GenMethod(kotlinLang, field.value.type) + "(" + NumToString(offset) + ", " ;
  if (!IsScalar(field.value.type.base_type) && (!struct_def.fixed)) {
  	  code += sanitize(field.name, false) + "Offset"; // string, union, struct and table
  //  else code += "Struct(" + NumToString(offset) + ", " + sanitize(field.name, false) + "Offset";
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
  code += "\t\tfun start" + sanitize(field.name, true);
  code += "Array(builder :FlatBufferBuilder, numElems : Int) ";
  code += " : Unit = builder.startArray(";
  auto vector_type = field.value.type.VectorType();
  auto alignment = InlineAlignment(vector_type);
  auto elem_size = InlineSize(vector_type);
  code += NumToString(elem_size);
  code += ", numElems, " + NumToString(alignment);
  code += ")\n";
}

static void createArrayOfStruct( const FieldDef &field, std::string *code_ptr) {
	  std::string &code = *code_ptr;
   code += "\t\tfun create" + sanitize(field.name, true);
  code += "Array(builder :FlatBufferBuilder, offsets : IntArray) ";
  
  code += " : Int {builder.startArray(4, offsets.size, ";
  if (field.value.type.struct_def != nullptr) code += NumToString(field.value.type.struct_def->minalign); else code += "4";
  code += "); for (i in offsets.size - 1 downTo 0) builder.addOffset(offsets[i]); return builder.endArray(); }\n";
}

static void createArrayOfNonStruct( const FieldDef &field, std::string *code_ptr) {
	  std::string &code = *code_ptr;
	          auto vector_type = field.value.type.VectorType();
        auto alignment = InlineAlignment(vector_type);
        auto elem_size = InlineSize(vector_type);//NumToString(SizeOf(field.value.type.element))
   code += "\t\tfun create" + sanitize(field.name, true);
  code += "Array(builder :FlatBufferBuilder, data : " + GenTypeForUser(kotlinLang, DestinationType(kotlinLang, vector_type, false))/*GenTypeBasic(kotlinLang, vector_type)*/ + "Array) "; // must widen types + give enums
  code += " : Int {builder.startArray(";
  code +=  NumToString(elem_size) + ", data.size, ";
  code += NumToString(alignment) +"); for (i in data.size - 1 downTo 0) builder.add" +   GenMethod(kotlinLang, vector_type)/*GenTypeGet(kotlinLang, field.value.type.VectorType())*/ + "(" + downsizeToStorageValue(vector_type, "data[i]", false) + "); return builder.endArray(); }\n";
}

// Get the offset of the end of a table.
static void GetEndOffsetOnTable(const StructDef &struct_def,
                                std::string *code_ptr, const Parser &parser) {
  std::string &code = *code_ptr;
  code += "\t\tfun end" + struct_def.name + "(builder :FlatBufferBuilder) :Int {\n\t";
  

  code += "\tval o = builder.endObject()\n\t";
  
     for (auto it = struct_def.fields.vec.begin();
         it != struct_def.fields.vec.end();
         ++it) {
      auto &field = **it;
      if (!field.deprecated && field.required) {
        code += "\tbuilder.required(o, ";
        code += NumToString(field.value.offset);
        code += ");  // " + field.name + "\n";
      }
    }

code += "\treturn o\n";
  code += "}";
  
  code += "\t\tfun finish" + MakeCamel(struct_def.name, true) + "Buffer(builder : FlatBufferBuilder, offset : Int) { builder.finish(offset";  
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
static void GenStruct(const StructDef &struct_def, std::string *code_ptr, StructDef *root_struct_def, const Parser &parser) {
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
    
  if (&struct_def == root_struct_def) {
    // Generate a special accessor for the table that has been declared as
    // the root type.
    NewRootTypeFromBuffer(struct_def, code_ptr);
  }
  if (struct_def.fixed) {
    // create a struct constructor function
    GenStructBuilder(struct_def, code_ptr);
  } else {
    // Create a set of functions that allow table construction.
    GenTableBuilders(struct_def, code_ptr, parser);
    GenStructBuilder(struct_def, code_ptr); // added
  }
  endCompanionObject(code_ptr);
  endClassDeclaration(code_ptr);
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




/** setters */

static std::string downsizeToStorageValueForConstructor(const Type &type, const std::string value) {
       if (type.enum_def != nullptr){
	if (type.base_type == BASE_TYPE_UNION) return value; // union_type type 
         else return value + ".value"; // enum class name
       }
	// transform kotlin value to storage value
  switch (type.base_type) {
  case BASE_TYPE_BOOL:  return value;//"if (" + value + ") 1.toByte() else 0.toByte()";
  case BASE_TYPE_UINT:  return value + ".toInt()";
  case BASE_TYPE_USHORT:  return value + ".toShort()";
  case BASE_TYPE_UCHAR:  return value + ".toByte()";
  case BASE_TYPE_VECTOR:  return value; // or value + "toInt()";
  default: return value;
}
}

/*
static std::string downsizeToStorageValue(const Type &type, const std::string value) {
    if (type.enum_def != nullptr) {// Fix this
    	    return value + ".value";
       // if (type.enum_def->is_union){
    }
	// transform kotlin value to storage value
  switch (type.base_type) {
  case BASE_TYPE_BOOL:  return value;//"if (" + value + ") 1.toByte() else 0.toByte()";
  case BASE_TYPE_UINT:  return value + ".and(0xFFFFFFFFL).toInt()";
  case BASE_TYPE_USHORT:  return value + ".and(0xFFFF).toShort()";
  case BASE_TYPE_UCHAR:  return value + ".and(0xFF).toByte()";
  case BASE_TYPE_VECTOR:  return downsizeToStorageValue(type.VectorType(), value);
  default: return value;
}
}*/





/** getters */


static std::string beforeStorageType(const Type &type) {
	
	//return SourceCast(GeneratorOptions::Language::kKotlin, type);
	
    if (type.enum_def != nullptr && type.base_type != BASE_TYPE_UNION) return type.enum_def->name + ".from("; // enum class name
    //if (type.enum_def != nullptr && !type.enum_def->is_union) return type.enum_def->name + ".from("; // enum class name
    return "";
}

// Returns the function name that is able to read a value of the given type.
static std::string GenGetterN(const LanguageParameters &lang,
                             const Type &type) {
  switch (type.base_type) {
    case BASE_TYPE_STRING: return "_string";
    case BASE_TYPE_STRUCT: return "_struct";
    case BASE_TYPE_UNION:  return "_union";
    case BASE_TYPE_VECTOR: return GenGetterN(lang, type.VectorType());
    default: {
      std::string getter = "_byteBuffer.get";
      if (type.base_type == BASE_TYPE_BOOL) {
        getter = "0.toByte()!=" + getter;
      } else if (GenTypeBasic(lang, type) != "Byte") {
        getter += MakeCamel(GenTypeGet(lang, type));
      }
      return getter;
    }
  }
}


static std::string GenSetter(const LanguageParameters &lang, const Type &type) {
  if (IsScalar(type.base_type)) {
    std::string setter = "_byteBuffer.put";
    if (GenTypeBasic(lang, type) != "Byte" && 
        type.base_type != BASE_TYPE_BOOL) {
      setter += MakeCamel(GenTypeGet(lang, type));
    }
    return setter;
  } else {
    return "";
  }
}


// improve SourceCast where source is the value that goes in a put call
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


// Returns the function name that is able to read a value of the given type.
static std::string beginGet(const Type &type) {
	return GenGetterN(kotlinLang, type);
}

static std::string upsizeToUserType(const Type &type, const std::string value) {
   if (!IsScalar(type.base_type) && type.base_type != BASE_TYPE_VECTOR) return value;
  // transform storage value to kotlin value
  switch (type.base_type) {
  //case BASE_TYPE_BOOL:  return "";
  case BASE_TYPE_UNION: return type.enum_def->name + ".from(" + value + ")";
  case BASE_TYPE_UINT:  return value + ".toLong().and(0xFFFFFFFFL)";
  case BASE_TYPE_USHORT:  return value + ".toInt().and(0xFFFF)";
  case BASE_TYPE_UCHAR:  return value + ".toInt().and(0xFF)";
  case BASE_TYPE_VECTOR:  return upsizeToUserType(type.VectorType(), value);
  default: return value;
}
}



/** converts to kotlinType from storageType */
static std::string afterStorageType(const Type &type, const bool isLitteral) {
    if (type.enum_def != nullptr) {
    	    return ".toInt())";
       // if (type.enum_def->is_union) {return ".toInt())";}//return GenTypeGet(type); // union_type type 
       // else return ")"; // enum class name
    }

switch (type.base_type) {
case BASE_TYPE_BOOL: return "";//if (isLitteral) return "!= 0"; else return "!=0.toByte()";
case BASE_TYPE_NONE:
case BASE_TYPE_UTYPE:
case BASE_TYPE_CHAR:return ".toByte()";
case BASE_TYPE_SHORT:return ".toShort()";
case BASE_TYPE_UCHAR:return ".toInt().and(0xFF)";
case BASE_TYPE_USHORT:return ".toInt().and(0xFFFF)";
case BASE_TYPE_UINT: return ".toLong().and(0xFFFFFFFFL)";
case BASE_TYPE_LONG:
case BASE_TYPE_ULONG: if (isLitteral) return ""; else return ".toLong()";
case BASE_TYPE_FLOAT: if (isLitteral) return ""; else return ".toFloat()";
case BASE_TYPE_DOUBLE:return ".toDouble()";
case BASE_TYPE_VECTOR:return afterStorageType(type.VectorType(), isLitteral);
case BASE_TYPE_STRING:
case BASE_TYPE_STRUCT:
case BASE_TYPE_UNION:
case BASE_TYPE_INT:
    default:return "";
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
  StructBuilderArgs(struct_def, "", code_ptr);
  EndBuilderArgs(code_ptr);

  StructBuilderBody(struct_def, "", code_ptr);


  EndBuilderBody(code_ptr);
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

