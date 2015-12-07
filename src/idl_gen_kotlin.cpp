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
#include <set>


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


/** possible improvements : 
  A) maintainability : implement base abstract class (allowing others to reuse it) & concrete class for Kotlin
    1) DONE ----------remove dependency from the stuff of idl_gen_general that doesn't cleanly separate from kotlin-----------
    2) implement a sharable base class, with sensible defaults, that provides nice utils but does the minimum that a parser should do
        and that has no IF, no switch, no language specific stuff
    3) implement a concrete class for kotlin
  B) correctness : implement flatbuffers features : 
     1) DONE ----------namespaces are broken (only the last part gets exported)-------
     2) DONE ----------required (in constructor)------------
     3) DONE ----------defaults values (in constructor)----------
     4) key
     5) verifyer
     6) id
     7) deprecated
     8) original_order
     9) bit_flags
     10) improve the enum deserialization routine (use a transform function, skips, sparse array, list with binary search or a map)
     11) nested_flatbuffer
     12) key
     
  C) enhancements :
    0) improve enums performance
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

	
	// importing these struct to imitate the general framework and ease the mergin of Kotlin support (when the general framework is ready :/)
	
//	struct LanguageParameters;

// removing dependency from the evil macro that forces you to change stuff in others' code
static   const char *  gtypename[] = {"Byte", // NONE
   "Byte", // UTYPE
   "Boolean", // BOOL
   "Byte", // CHAR
   "Byte", // UCHAR
   "Short", // SHORT
   "Short", // USHORT
   "Int", // INT
   "Int", // UINT
   "Long", // LONG
   "Long", // ULONG
   "Float", // FLOAT
   "Double", // DOUBLE
   "Int", // STRING
   "Int", // VECTOR
   "Int", // STRUCT
   "Int" // UNION
};


// this isn't shared in idl_gen_general... got to duplicate it :/
struct CommentConfig {
  const char *first_line;
  const char *content_line_prefix;
  const char *last_line;
};

	struct LanguageParameters {
  IDLOptions::Language language;
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

	//extern LanguageParameters language_parameters[];
	
	// global in this file
	//auto kotlinLang = language_parameters[IDLOptions::kKotlin];	
	
	 LanguageParameters  kotlinLang = {
    IDLOptions::kMAX,
    false,
    ".kt",
    "String",
    "Boolean ",
    " {\n",
    " final ",
    "final ",
    "final class ",
    "\n",
    "()",
    "",
    " : ",
    "package ",
    ";",
    "",
    "_byteBuffer.order(ByteOrder.LITTLE_ENDIAN) ",
    "position()",
    "offset()",
    "import java.nio.*;\n"
      "import com.google.flatbuffers.kotlin.*;\n\n",
    {
      "/**",
      " *",
      " */",
    },
  };
	
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
	


/** Fix packages !!!!*/
	
	
	// Ensure that a type is prefixed with its namespace whenever it is used
// outside of its namespace.
static std::string WrapInNameSpace(const Parser &parser, const Namespace *ns, const std::string &name) {
  if (parser.namespaces_.back() == ns) return name;
    std::string qualified_name;
    for (auto it = ns->components.begin(); it != ns->components.end(); ++it) qualified_name += *it + ".";
    return qualified_name + name;
}

static std::string WrapInNameSpace(const Parser &parser, const Definition &def) {
  return WrapInNameSpace(parser, def.defined_namespace, def.name);
}

static std::string package(const Parser &parser);

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

	
	static void buildTableOrStringArrayWithVararg( const FieldDef &field, std::string *code_ptr);
	static void buildScalarArrayWithVararg( const FieldDef &field, std::string *code_ptr);
	//static void GenStructBuilder(const StructDef &struct_def, std::string *code_ptr);
	
	static void generateTable(const StructDef &struct_def,std::string *code_ptr);
	
	static void generateStructBuilder(const StructDef &struct_def, std::string *code_ptr);


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

static void enumImports(const EnumDef &enum_def, std::string &code) {
  if (enum_def.is_union)  code += "import com.google.flatbuffers.kotlin.Table\n\n";
}


// Begin enum code with a class declaration.
static void beginEnumDeclaration(const EnumDef &enum_def, std::string *code_ptr) {
  std::string &code = *code_ptr;
  code += "public enum class " + enum_def.name + "(val value: " + GenTypeGet(kotlinLang, enum_def.underlying_type) + ") {\n";

}

// A single enum member.
static void enumMember(const EnumVal ev, std::string *code_ptr) {
  std::string &code = *code_ptr;
  code += "\t" +  ev.name + "(" + NumToString(ev.value) + ")";
}



/** enum in arithmetic progression value = a * index + b */
const int ENUM_ARITHMETIC_PROGRESSION = 0;
// We could also check if enums are implemented with ranges 
// with holes between them and use an increasing bound function 
// to map it back to a contiguous range
// but it's probably overkill and error prone
/** less than 8 values : use multi if */
const int ENUM_WHEN = 1;
/** more values */
// we could also use an (sparse) array as in the C++ code it the max-min value isn't very big
// we could also use a binarysearch
const int ENUM_MAP = 2; 


// Begin enum code with a class declaration.
static int analyzeEnum(const EnumDef &enum_def) {
    // first check if the enums are in an arithmetic progression
    int size =  enum_def.vals.vec.size();
    if (size <= 2) return ENUM_ARITHMETIC_PROGRESSION;
    int nextValue = enum_def.vals.vec[1]->value;
    int r =  nextValue - enum_def.vals.vec[0]->value;
    bool isArithmeticProgression = true;
    for (int index = 2; index < size; index++) {
    	  nextValue += r;
    	  if ( enum_def.vals.vec[index]->value != nextValue) {
    	       isArithmeticProgression = false;
    	       break;
    	  }
    }
    if (isArithmeticProgression) return ENUM_ARITHMETIC_PROGRESSION;
    if (size < 8) return ENUM_WHEN;
    return ENUM_MAP;
}



static void endEnumDeclaration(const EnumDef &enum_def,std::string *code_ptr, const Parser & parser) {
  std::string &code = *code_ptr;

  int analyzedEnum = analyzeEnum(enum_def);
  code += "\t companion object {\n";
  
  // deserializing enums
  code += "\tfun from( value : " + GenTypeGet(kotlinLang, enum_def.underlying_type) + ") : " + enum_def.name;
  
  int first;
  int r;
  switch(analyzedEnum) {
  case ENUM_WHEN :
      code +=  " = when (value.toInt()) {\n";
      for (auto it = enum_def.vals.vec.begin(); it != enum_def.vals.vec.end(); ++it) {
         auto &ev = **it;
         code += "\t" + NumToString(ev.value) + " -> " + ev.name + "\n";
      }
      code += "\telse -> throw Exception(\"Bad enum value : $value\")\n";
      code += "}\n";
      break;
case ENUM_ARITHMETIC_PROGRESSION:
	if (enum_def.vals.vec.size() == 1) {
		code +=  " = if  (value.toInt() == " + NumToString(enum_def.vals.vec[0]->value) + ") " + enum_def.vals.vec[0]->name + " else throw Exception(\"Bad enum value : $value\")"; 
		break;
	}
	first = enum_def.vals.vec[0]->value;
	r = enum_def.vals.vec[1]->value - first;
	code +=  " =values()[(value.toInt()";
	if (first >= 0) code += " - " + NumToString(first); else code += " + " + NumToString(-first);
	code += ") / " + NumToString(r) + "]\n";
	break;
case ENUM_MAP:
	default:
code +=  "= map[value.toInt()] ?: throw Exception(\"Bad enum value : $value\")\n";
code += "private val map = mapOf(";
  for (auto it = enum_def.vals.vec.begin(); it != enum_def.vals.vec.end(); ++it) {
         auto &ev = **it;
         if (it != enum_def.vals.vec.begin()) code += ",\n\t";
         code += NumToString(ev.value) + " to " + ev.name;
      }
code += ")\n";      
	break;
  } 
  
  
  // fetching correct constructor for unions
  if (enum_def.is_union) {
  	  std::string pack = package(parser);
  	  
  	   code += "\tfun toTable( value : " + enum_def.name + ") : Table = when (value) {\n";
  	   for (auto it = enum_def.vals.vec.begin(); it != enum_def.vals.vec.end();++it) {
  	   	   auto &ev = **it;
  	              
  	              

  	   	   if (it == enum_def.vals.vec.begin())  code += "\t" + ev.name + " -> throw Exception(\"void union\")\n";
  	   	   else {
  	   	   	   if (ev.struct_def) {
  	   	   	   	    //auto namespac = ev.struct_def->defined_namespace->GetFullyQualifiedName(name);
  	   	   	   	    auto namespac =  WrapInNameSpace(parser, *ev.struct_def) ;
  	   	   	   	    if (namespac.length() > ev.name.length()) code += "\t" + ev.name + " -> " + namespac + "()\n";  
  	   	   	   	    else  code += "\t" + ev.name + " -> " + pack + "." + ev.name + "()\n";
  	   	   	      } else  code += "\t" + ev.name + " -> " + pack + "." + ev.name + "()\n";
  	   	   	   
  	   } 
  	   }
  	   code += "}\n";
  }
  
  code += "}\n"; // end companion object
  	
  code += "}\n"; // end enum
}



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


static void toString(const StructDef &struct_def, std::string *code_ptr) {
  std::string &code = *code_ptr;

  code += "\toverride public fun toString() : String = \"" + MakeCamel(struct_def.name, true) + "(";  
  bool first = true;  
  for (auto it = struct_def.fields.vec.begin();it != struct_def.fields.vec.end();++it) {
    	    auto &field = **it;
    	    if (field.deprecated) continue;
    	    if (!first) code +=  ",";
    	    first = false;
    	    code += sanitize(field.name, false) + "=";
    	    
    	    if (field.value.type.base_type != BASE_TYPE_VECTOR) code += "$" + sanitize(field.name, false);
    	    else  {
    	    	    code += "${(0 until " + sanitize(field.name, false) + "Size).map({" + sanitize(field.name, false) + "(it).toString()}).joinToString(\", \", \"[\",\"]\")}";
    	    }
   }
    code += ")\"\n";
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

  code += "\tpublic var ";
  code+= sanitize(field.name, false) + " : " + GenTypeNameDest(kotlinLang, field.value.type);
  code += " get() = " + upsizeToUserType(field.value.type,  GenGetterKotlin(kotlinLang, field.value.type) + "(bb_pos + " + NumToString(field.value.offset) + ")");

  code += "; set(value) { " + GenSetterKotlin(kotlinLang, field.value.type) + "(bb_pos + " + NumToString(field.value.offset) + ", ";
  code += downsizeToStorageValue(field.value.type, "value", true) +") }";
 code += "\n";
}

static void getScalarFieldOfTable(const FieldDef &field, std::string *code_ptr) {
  std::string &code = *code_ptr;

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
  
  code += "\tpublic val " + sanitize(field.name, false) + " : " + GenTypeNameDest(kotlinLang, field.value.type) + "? get() {";
  code += "val o = __offset(" +NumToString(field.value.offset) + "); ";
  code += "return if (o == 0) null else __union(" + field.value.type.enum_def->name + ".toTable(" + sanitize(field.name, false) +"Type), o)}\n";
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
  if (vector_type.base_type != BASE_TYPE_STRING) code += defaultToUserType(field.value.type, GenDefaultValue(kotlinLang, field.value, false) ); else code += "null"; 
  code += " else " + upsizeToUserType(field.value.type,  GenGetterKotlin(kotlinLang, field.value.type) + "(__vector(o) + j" + multiplyBySizeOf(vector_type) + ")");
code += "}\n";

 if (field.value.type.element == BASE_TYPE_STRING)  {// string as ByteBuffer
  code += "\tpublic fun " + sanitize(field.name, false) + "Buffer(j : Int) : ByteBuffer? ";
  code += " {val o = __offset(" + NumToString(field.value.offset) + "); ";
  code += "return if (o == 0) null";
  code += " else __string_element_as_bytebuffer(o, j)";
  code += "}\n";
 }

 // mutation
  if (vector_type.base_type != BASE_TYPE_STRING) {
    code += "\tpublic fun mutate" + sanitize(field.name, true) + "(j : Int, value : " + GenTypeNameDest(kotlinLang, vector_type) + ") :Boolean {";
    code += "val o = __offset(" + NumToString(field.value.offset) + "); ";
    code += "return if (o == 0) false else {";
    code += GenSetterKotlin(kotlinLang, vector_type) + "(__vector(o) + j" + multiplyBySizeOf(vector_type);
    code += ", " + downsizeToStorageValue(vector_type, "value", true)   + "); true}}\n";
  }
}


static void fieldAsByteBuffer(const FieldDef &field, std::string *code_ptr) {
                             std::string &code = *code_ptr;
                                     
      code += "\tpublic val " + sanitize(field.name, false);
      if (field.value.type.base_type == BASE_TYPE_STRING) code += "Buffer";
      code += " : ByteBuffer get() = __vector_as_bytebuffer(";
      code += NumToString(field.value.offset) + ", ";
      code += NumToString(field.value.type.base_type == BASE_TYPE_STRING ? 1 : InlineSize(field.value.type.VectorType()));
      code += ")\n";
}


static bool hasNestedStruct(const StructDef &struct_def) {
 for (auto it = struct_def.fields.vec.begin();it != struct_def.fields.vec.end();++it) {
      auto &field = **it;
      if (IsStruct(field.value.type) && field.value.type.struct_def->fixed) return true;
 }
       return false;
}

static void generateStructBuilder(const StructDef &struct_def, std::string *code_ptr) {
  std::string &code = *code_ptr;
  
  code += "\t\t";
  if (hasNestedStruct(struct_def)) code += "inline "; // only inline if there is a nested struct (or kotlin warns you)
  code += "public fun FlatBufferBuilder." + LowerFirst(struct_def.name) + "Raw(";
  for (auto it = struct_def.fields.vec.begin();it != struct_def.fields.vec.end();++it) {
      auto &field = **it;
      if (it!= struct_def.fields.vec.begin()) code += ", ";
      if (IsStruct(field.value.type)) code += "crossinline " + sanitize(field.name, false) + " : FlatBufferBuilder.()->Int"; else code +=  sanitize(field.name, false)  + " : " + GenTypeForUserConstructor(field.value.type); 
  }
  code += "):Int = with(this) {\n";
  code += "prep(" + NumToString(struct_def.minalign) + ", " + NumToString(struct_def.bytesize) + ")\n";
  for (auto it = struct_def.fields.vec.rbegin(); it != struct_def.fields.vec.rend(); ++it) {
    auto &field = **it;
    if (field.padding) code += "\t\t\tpad(" + NumToString(field.padding) + ")\n";
    if (IsStruct(field.value.type)) code += "\t\t\t" + sanitize(field.name, false) + "()\n"; else {
      code += "\t\t\tadd" + GenMethod(kotlinLang, field.value.type) + "(";
      code +=  downsizeToStorageValueForConstructor(field.value.type, sanitize(field.name, false)) + ")\n";
    }
  }
  code += "\n\toffset()\n\t}\n";
  
  if (hasNestedStruct(struct_def)) code += "inline ";
  code += "public fun FlatBufferBuilder." + LowerFirst(struct_def.name) + "(";  
    for (auto it = struct_def.fields.vec.begin();it != struct_def.fields.vec.end();++it) {
      auto &field = **it;
      if (it!= struct_def.fields.vec.begin()) code += ", ";
      if (IsStruct(field.value.type)) code += "crossinline " + sanitize(field.name, false) + " : FlatBufferBuilder.()->Int"; else code +=  sanitize(field.name, false)  + " : " + GenTypeForUserConstructor(field.value.type); 
  }
    code += "):FlatBufferBuilder.()->Int = {"+ LowerFirst(struct_def.name) + "Raw(";
        for (auto it = struct_def.fields.vec.begin();it != struct_def.fields.vec.end();++it) {
      auto &field = **it;
      if (it!= struct_def.fields.vec.begin()) code += ", ";
      code +=  sanitize(field.name, false); 
  }
  code += ")}\n";
}

static void buildArrayWithLambda( const FieldDef &field, std::string *code_ptr) {
  std::string &code = *code_ptr;

  auto vector_type = field.value.type.VectorType();
  auto alignment = InlineAlignment(vector_type);
  auto elem_size = InlineSize(vector_type);
  
  code += "\t\tinline public fun FlatBufferBuilder." + LowerFirst(sanitize(field.name, false));
  code += "(numElems : Int, action : FlatBufferBuilder.()->Unit) : Int {startArray(";
  code += NumToString(elem_size);
  code += ", numElems, " + NumToString(alignment);
  code += "); action(); return endArray()}\n";
}

static void buildTableOrStringArrayWithVararg( const FieldDef &field, std::string *code_ptr) {
	  std::string &code = *code_ptr;
  
  code += "\t\tpublic fun FlatBufferBuilder." + LowerFirst(sanitize(field.name, true));
  code += "(vararg offsets : Int) ";
  code += " : Int {startArray(4, offsets.size, ";
  // table or string
  if (field.value.type.struct_def != nullptr) code += NumToString(field.value.type.struct_def->minalign); else code += "4"; // look into this
  code += "); for (i in offsets.size - 1 downTo 0) addOffset(offsets[i]); return endArray(); }\n";
}

static void buildScalarArrayWithVararg( const FieldDef &field, std::string *code_ptr) {
    std::string &code = *code_ptr;
    auto vector_type = field.value.type.VectorType();
    auto alignment = InlineAlignment(vector_type);
    auto elem_size = InlineSize(vector_type);
    code += "\t\tfun FlatBufferBuilder." + LowerFirst(sanitize(field.name, false));
    code += "(vararg data : " + GenTypeForUserConstructor(vector_type) + ")", 
    code += " : Int {startArray(";
    code +=  NumToString(elem_size) + ", data.size, ";
    code += NumToString(alignment) +"); for (i in data.size - 1 downTo 0) add" +   GenMethod(kotlinLang, vector_type)+ "(" + downsizeToStorageValue(vector_type, "data[i]", false) + "); return endArray(); }\n";
}

// Get the offset of the end of a table.
static void generateFinishBuffer(const StructDef &struct_def, std::string *code_ptr, const Parser &parser) {
  std::string &code = *code_ptr;
 
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
static void GenTableBuilders(const StructDef &struct_def, std::string *code_ptr, const Parser &parser) {
    for (auto it = struct_def.fields.vec.begin();it != struct_def.fields.vec.end();++it) {
        auto &field = **it;
        if (field.deprecated) continue;
        if (field.value.type.base_type != BASE_TYPE_VECTOR) continue;
        buildArrayWithLambda(field, code_ptr);
        if (field.value.type.element == BASE_TYPE_STRUCT && field.value.type.struct_def->fixed) continue;
        if (IsScalar(field.value.type.element)) buildScalarArrayWithVararg(field, code_ptr);  else  buildTableOrStringArrayWithVararg(field, code_ptr); 
    }
    generateFinishBuffer(struct_def, code_ptr, parser);
}

// Generate struct or table methods.
static void GenStruct(const StructDef &struct_def, std::string *code_ptr, StructDef * /*root_struct_def*/, const Parser &parser) {
  if (struct_def.generated) return;

  GenComment(struct_def.doc_comment, code_ptr, nullptr);
  beginClassDeclaration(struct_def, code_ptr);

  // Generate the Init method that sets the field in a pre-existing
  // accessor object. This is to allow object reuse.
  initializeTableAndStruct(struct_def, code_ptr);
  
  toString(struct_def, code_ptr);

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

    // Create a set of functions that allow table construction.
    if (!struct_def.fixed) GenTableBuilders(struct_def, code_ptr, parser);

  endCompanionObject(code_ptr);
  endClassDeclaration(code_ptr);
  
  // create a struct constructor function
  if (struct_def.fixed) generateStructBuilder(struct_def, code_ptr);
  else generateTable(struct_def, code_ptr);  
}





// Generate enum declarations.
static void generateEnum(const EnumDef &enum_def, std::string *code_ptr, const Parser & parser) {
  if (enum_def.generated || !enum_def.vals.vec.size()) return;
  
  auto &code = * code_ptr;
  GenComment(enum_def.doc_comment, code_ptr, nullptr);
  
  enumImports(enum_def, code);
  beginEnumDeclaration(enum_def, code_ptr);
  for (auto it = enum_def.vals.vec.begin(); it != enum_def.vals.vec.end();++it) {
    auto &ev = **it;
    GenComment(ev.doc_comment, code_ptr, nullptr, "\t");
    if (it != enum_def.vals.vec.begin()) code += ",\n";
    enumMember( ev, code_ptr);
  }
  code += ";\n";
  endEnumDeclaration(enum_def, code_ptr, parser);
}

static std::string package(const Parser &parser) {
  std::string namespace_name;
  auto &namespaces = parser.namespaces_.back()->components;
  for (auto it = namespaces.begin(); it != namespaces.end(); ++it) {
    if (namespace_name.length()) namespace_name += ".";
    namespace_name += *it;
  }
  return namespace_name;
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
    if (namespace_name.length()) namespace_name += ".";
    
    namespace_name += *it;
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

static const std::set<std::string> keywords {"val","var","fun","for","while","if","else", "class", "enum", "private", "public", "protected", "internal", "override", "package", "import", "return", "when", "is", "in"};

static std::string sanitize(const std::string name, const bool isFirstLetterUpper) {
	// transforms name with _ inside into camelCase
	std::string camelName = MakeCamel(name, isFirstLetterUpper);
	// if there is a trailing "_", add one
	if (camelName.size() >= 1 && camelName.compare(camelName.size() - 1, 1, "_") == 0) return camelName + "_";
	// if tis is a reserved word in kotlin, add a trailing "_"
	if (keywords.find(camelName) != keywords.end()) return camelName + "_"; else return camelName;
}

static std::string multiplyBySizeOf(const Type &type) {
              int a = InlineSize(type);
              if (a == 1) return ""; else return " * " + NumToString(a);
}

// functions copied from idl_gen_general.cpp because they are static (can't be shared)

static std::string GenTypeBasic(const LanguageParameters &/*lang*/,
                                const Type &type) {

// DIE evil macro !

  /*static const char *gtypename[] = {
    #define FLATBUFFERS_TD(ENUM, IDLTYPE, CTYPE, JTYPE, KTYPE, GTYPE, NTYPE, PTYPE) \
        #JTYPE, #KTYPE, #NTYPE, #GTYPE,
      FLATBUFFERS_GEN_TYPES(FLATBUFFERS_TD)
    #undef FLATBUFFERS_TD
  };





  return gtypename[type.base_type * IDLOptions::kMAX + lang.language];
*/
return gtypename[type.base_type/* * IDLOptions::kMAX + lang.language*/];
  				}

// Generate type to be used in user-facing API
// removed static to allow reuse in the kotlin external generator
static std::string GenTypeForUser(const LanguageParameters &lang,
                                  const Type &type) {
  //if (lang.language == IDLOptions::kKotlin) {
    if (type.enum_def != nullptr && type.base_type != BASE_TYPE_UNION) return type.enum_def->name;
  //}
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
  //if (lang.language != IDLOptions::kJava && lang.language != IDLOptions::kKotlin) return type;
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
std::string GenTypeNameDest(const LanguageParameters &lang, const Type &type) {

 // if (lang.language == IDLOptions::kKotlin) {
    // Kotlin enums are represented by themselves
    if (type.enum_def != nullptr && type.base_type != BASE_TYPE_UNION) return type.enum_def->name;

    // Unions in Kotlin use a generic Table-derived type 
    if (type.base_type == BASE_TYPE_UNION) return "Table";
  //}
  
  // default behavior
  return GenTypeGet(lang, DestinationType(lang, type, true));
}

// removed static to allow reuse
std::string GenDefaultValue(const LanguageParameters &/*lang*/, const Value &value, bool for_buffer) {

  if (/*lang.language == IDLOptions::kKotlin &&*/ !for_buffer) {  
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


// Generate a method that creates a table in one go. This is only possible
static void generateTable(const StructDef &struct_def,std::string *code_ptr) {
    int num_fields = 0;
    for (auto it = struct_def.fields.vec.begin(); it != struct_def.fields.vec.end(); ++it) {
      auto &field = **it;
      if (field.deprecated) continue; else num_fields++;
    }

   if (!num_fields) return;
   std::string &code = *code_ptr;
      code += "\t\tpublic fun FlatBufferBuilder." + LowerFirst(struct_def.name) + "(";
      bool first = true;
      // add required parameters first without defaults
      for (auto it = struct_def.fields.vec.begin(); it != struct_def.fields.vec.end(); ++it) {
        auto &field = **it;
        if (field.deprecated || !field.required) continue;
        if (!first) code += ", ";
        first = false;
        
        code += sanitize(field.name, false);
        if (!IsScalar(field.value.type.base_type) && (!struct_def.fixed)) {
        	if (IsStruct(field.value.type) && field.value.type.struct_def->fixed) code += " : FlatBufferBuilder.()->Int "; else code += " : Int ";
        }
        else code += " : " + GenTypeForUserConstructor(field.value.type);
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
        }
        else code += " : " + GenTypeForUserConstructor(field.value.type);
        code += " = ";
        if (!IsScalar(field.value.type.base_type) && (!struct_def.fixed)) {   
        	if (IsStruct(field.value.type) && field.value.type.struct_def->fixed) code += "{0}"; else code += "0"; 
        } else code += defaultToUserType(field.value.type, GenDefaultValue(kotlinLang, field.value, false) ); // add defaults later + " = " +  field.value.constant; 
      }      
      code += ") = with(this) {\n\t\twith(" + MakeCamel(struct_def.name,true) + " ) {\n\t\t\tstartObject(" + NumToString(struct_def.fields.vec.size()) + ")\n";
      
      for (size_t size = struct_def.sortbysize ? sizeof(largest_scalar_t) : 1;size;size /= 2) {
        for (auto it = struct_def.fields.vec.rbegin();it != struct_def.fields.vec.rend(); ++it) {
          auto &field = **it;
          if (!field.deprecated &&(!struct_def.sortbysize || size == SizeOf(field.value.type.base_type))) {

      /** inlining the field builder */
            auto offset =   struct_def.fields.vec.rend() - 1 -it ;
      code += "\t\t\tadd" + GenMethod(kotlinLang, field.value.type) + "(" + NumToString(offset) + ", "  ;
      if (!IsScalar(field.value.type.base_type)) {
  	  code += sanitize(field.name, false);
  	  if (IsStruct(field.value.type) && field.value.type.struct_def->fixed) code += "()"; 
       } else  code +=  downsizeToStorageValue(field.value.type, sanitize(field.name, false), false);
   code += ", " + field.value.constant; //Int  
   if (field.value.type.base_type == BASE_TYPE_BOOL) code += "!=0"; // hack to work with booleans... bad :(
           code += ")\n";
          }
        }
      }         
      code += "\t\t\tendObject()\n\t\t}}\n";
}











}  // namespace kotlin

bool GenerateKotlin(const Parser &parser,
                const std::string &path,
                const std::string & /*file_name*//*,
                const IDLOptions & opts*/) {
  for (auto it = parser.enums_.vec.begin();it != parser.enums_.vec.end(); ++it) {
    std::string enumcode;
    kotlin::generateEnum(**it, &enumcode, parser);
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

